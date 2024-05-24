/*!
* @file wm_nvs.h
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.2.0-prerelase.0
*/
#include "esp_err.h"
#include "nvs_flash.h"


#include "wifiManager_private.h"
#include "wm_nvs.h"

static const char *TAG = "WM_NVS";
const char wm_nvs_namespace[] = "wifi_manager";

static esp_err_t wm_nvs_read ()
{
	char *ssid; 
	char *password;

	size_t ssid_len = MAX_SSID_LENGTH;
	size_t password_len = MAX_PASSWORD_LENGTH;

	nvs_handle_t nvs_handle;
	esp_err_t err;


	err = nvs_open(&wm_nvs_namespace, NVS_READONLY, &nvs_handle);
	if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
			xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_FAIL);
			return ESP_FAIL;
	}

	err = nvs_get_str(nvs_handle, "ssid", ssid, &ssid_len);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
			ESP_LOGE(TAG, "Error (%s) reading SSID!", esp_err_to_name(err));
			return ESP_FAIL;
	}

	err = nvs_get_str(nvs_handle, "password", password, &password_len);
	if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
			ESP_LOGE(TAG, "Error (%s) reading password!", esp_err_to_name(err));
	}

	nvs_close(nvs_handle);

  wifi_config_t wifi_config = {
		.sta = {
			.ssid = (uint8_t *)ssid,
			.password = (uint8_t *)password
	} };

	wm_wifi_send_message(&wifi_config);

	ESP_LOGI(TAG, "SSID: %s", ssid);
	ESP_LOGI(TAG, "Password: %s", password);

	free(ssid);
	free(password);

	return ESP_OK;
}

/*!
* @brief NVS Write Function
*
* This function writes the wifi credentials to the NVS.
*/
static void wm_nvs_write ()
{
	wifi_config_t wifi_config;
	wm_wifi_receive_message(&wifi_config);

	nvs_handle_t nvs_handle;
	esp_err_t err;

	err = nvs_open(&wm_nvs_namespace, NVS_READWRITE, &nvs_handle);
	if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
			xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_FAIL);
			return;
	}

	err = nvs_set_str(nvs_handle, "ssid", (char *)wifi_config.sta.ssid);
	if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error (%s) writing SSID!", esp_err_to_name(err));
	}

	err = nvs_set_str(nvs_handle, "password", (char *)wifi_config.sta.password);
	if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error (%s) writing password!", esp_err_to_name(err));
	}

	err = nvs_commit(nvs_handle);
	if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error (%s) committing NVS!", esp_err_to_name(err));
	}

	nvs_close(nvs_handle);

	xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_DONE);
}


/*!
* @brief NVS Clear Function
*
* This function clears the wifi credentials from the NVS.
*/
void wm_nvs_clear ()
{
	nvs_handle_t nvs_handle;
	esp_err_t err;

	err = nvs_open(&wm_nvs_namespace, NVS_READWRITE, &nvs_handle);
	if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
			xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_FAIL);
			return;
	}

	err = nvs_erase_all(nvs_handle);
	if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error (%s) erasing NVS!", esp_err_to_name(err));
	}

	err = nvs_commit(nvs_handle);
	if (err != ESP_OK) {
			ESP_LOGE(TAG, "Error (%s) committing NVS!", esp_err_to_name(err));
	}

	nvs_close(nvs_handle);

	xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_DONE);
}


void wm_nvs_task(void *pvParameters)
{
	esp_err_t ret = ESP_FAIL;

	while (1)
	{
		EventBits_t uxBits = xEventGroupWaitBits(wm_nvs_event_group, WM_EVENTG_NVS_READ_CREDS | WM_EVENTG_NVS_WRITE_CREDS | WM_EVENTG_NVS_CLEAR_CREDS, \
																			pdTRUE, pdFALSE, portMAX_DELAY);
		if ((uxBits & WM_EVENTG_NVS_WRITE_CREDS) != 0)
		{
			xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_BLOCK_REQ);
			ESP_LOGI(TAG, "NVS Write Event Triggered");
			wm_nvs_write();
		}
		else if ((uxBits & WM_EVENTG_NVS_READ_CREDS) != 0)
		{
			ESP_LOGI(TAG, "NVS Read Event Triggered");
			xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_BLOCK_REQ);
			esp_err_t err =wm_nvs_read();
			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "NVS Read Failed");
				xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_CREDS_NOT_FOUND);
			}
			else
			{
				xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_CREDS_FOUND);
			}
		}
		else if((uxBits & WM_EVENTG_NVS_CLEAR_CREDS) != 0)
		{
			xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_BLOCK_REQ);
			ESP_LOGI(TAG, "NVS Clear Event Triggered");
			wm_nvs_clear();
		}
	}
}

