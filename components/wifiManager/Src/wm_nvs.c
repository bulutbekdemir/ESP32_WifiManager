/*!
* @file wm_nvs.h
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.3.1-prerelase.2+2
*/
#include "string.h"

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifiManager_private.h"
#include "wm_nvs.h"
#include "wm_wifi.h"

static const char *TAG = "WM_NVS";
const char wm_nvs_namespace[] = "wifi_manager";

/*!
* @brief NVS Task Handler
*
*/
TaskHandle_t wm_nvs_task_handle;

/*!
* @brief NVS Read Function
*
* This function reads the credentials from the NVS.
*
* @return esp_err_t Returns ESP_OK if the credentials are found, ESP_FAIL otherwise.
*/
static esp_err_t wm_nvs_read ()
{
	wifi_config_t *wifi_config = (wifi_config_t *)malloc(sizeof(wifi_config_t));

	nvs_handle_t nvs_handle;

	if(nvs_open(wm_nvs_namespace, NVS_READONLY, &nvs_handle) == ESP_OK)
	{
		size_t ssid_len = 0;
		size_t pass_len = 0;

		if(nvs_get_blob(nvs_handle, "ssid", NULL, &ssid_len) == ESP_OK)
		{
			if(nvs_get_blob(nvs_handle, "ssid", wifi_config->sta.ssid, &ssid_len) == ESP_OK)
			{
				ESP_LOGI(TAG, "SSID: %s", wifi_config->sta.ssid);
			}
		}else {
			ESP_LOGE(TAG, "SSID Not Found");
			return ESP_FAIL;
		}

		if(nvs_get_blob(nvs_handle, "password", NULL, &pass_len) == ESP_OK)
		{
			if(nvs_get_blob(nvs_handle, "password", wifi_config->sta.password, &pass_len) == ESP_OK)
			{
				ESP_LOGI(TAG, "Password: %s", wifi_config->sta.password);
			}
		}else {
			ESP_LOGE(TAG, "Password Not Found");
			wm_wifi_send_message(wifi_config);
			return ESP_OK;
		}

		nvs_close(nvs_handle);
		wm_wifi_send_message(wifi_config);
	
		return ESP_OK;
	}else {
		ESP_LOGE(TAG, "NVS Open Failed");
		return ESP_FAIL;
	}
	return ESP_FAIL;
}

/*!
* @brief NVS Write Function
*
* This function writes the credentials to the NVS.
*
*/
static void wm_nvs_write()
{
	wifi_config_t *wifi_config = (wifi_config_t *)malloc(sizeof(wifi_config_t));
	wm_wifi_receive_message(wifi_config);

	nvs_handle_t nvs_handle;

	if(nvs_open(wm_nvs_namespace, NVS_READWRITE, &nvs_handle) == ESP_OK)
	{
		if(nvs_set_blob(nvs_handle, "ssid", wifi_config->sta.ssid, MAX_SSID_LEN) == ESP_OK)
		{
			ESP_LOGI(TAG, "SSID: %s", wifi_config->sta.ssid);
		}else {
			ESP_LOGE(TAG, "SSID Write Failed");
		}

		if(nvs_set_blob(nvs_handle, "password", wifi_config->sta.password, MAX_PASSWORD_LENGTH) == ESP_OK)
		{
			ESP_LOGI(TAG, "Password: %s", wifi_config->sta.password);
		}else {
			ESP_LOGE(TAG, "Password Write Failed");
		}

		nvs_close(nvs_handle);
		xEventGroupClearBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
		xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_DONE);
	}else {
		ESP_LOGE(TAG, "NVS Open Failed");
	}
}

/*!
* @brief NVS Clear Function
*
* This function clears the credentials from the NVS.
*
*/
static void wm_nvs_clear()
{
	nvs_handle_t nvs_handle;

	if(nvs_open(wm_nvs_namespace, NVS_READWRITE, &nvs_handle) == ESP_OK)
	{
		if(nvs_erase_key(nvs_handle, "ssid") == ESP_OK)
		{
			ESP_LOGI(TAG, "SSID Cleared");
		}else {
			ESP_LOGE(TAG, "SSID Clear Failed");
		}

		if(nvs_erase_key(nvs_handle, "password") == ESP_OK)
		{
			ESP_LOGI(TAG, "Password Cleared");
		}else {
			ESP_LOGE(TAG, "Password Clear Failed");
		}

		nvs_close(nvs_handle);
		xEventGroupClearBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
		xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_DONE);
	}else {
		ESP_LOGE(TAG, "NVS Open Failed");
	}
}

void wm_nvs_task(void *pvParameters)
{
	while (1)
	{
		EventBits_t uxBits = xEventGroupWaitBits(wm_nvs_event_group, WM_EVENTG_NVS_READ_CREDS | WM_EVENTG_NVS_WRITE_CREDS | WM_EVENTG_NVS_CLEAR_CREDS, \
																			pdTRUE, pdFALSE, portMAX_DELAY);
		if ((uxBits & WM_EVENTG_NVS_WRITE_CREDS) != 0)
		{
			xEventGroupSetBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
			ESP_LOGI(TAG, "NVS Write Event Triggered");
			wm_nvs_write();
		}
		else if ((uxBits & WM_EVENTG_NVS_READ_CREDS) != 0)
		{
			ESP_LOGI(TAG, "NVS Read Event Triggered");
			xEventGroupSetBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
			esp_err_t err =wm_nvs_read();
			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "NVS Read Failed");
				xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_CREDS_NOT_FOUND);
				xEventGroupSetBits(wm_task_event_group, WM_EVENTG_TASK_ALL_INIT);
			}
			else
			{
				xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_CREDS_FOUND);
				xEventGroupSetBits(wm_task_event_group, WM_EVENTG_TASK_WIFI_INIT);
			}
		}
		else if((uxBits & WM_EVENTG_NVS_CLEAR_CREDS) != 0)
		{
			xEventGroupSetBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
			ESP_LOGI(TAG, "NVS Clear Event Triggered");
			wm_nvs_clear();
		}
	}
}

