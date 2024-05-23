/*!
* @file wifiManager.c
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.2.0-prerelase.3 
*/
#include "wifiManager.h"

static const char *TAG = "WIFI_MANAGER_MAIN";

esp_err_t wifiManager_init ()
{
	esp_err_t ret = ESP_FAIL;
	BaseType_t xReturned = pdFAIL;

	///> Initialize Wifi Manager Main Event Group
	wm_main_event_group = xEventGroupCreate();
	if (wm_main_event_group == NULL)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Manager Main Event Group");
		return ret;
	}
	
	///> Initialize Wifi Event Group
	wm_wifi_event_group = xEventGroupCreate();
	if (wm_wifi_event_group == NULL)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Event Group");
		return ret;
	}

	///> Initialize NVS Event Group
	wm_nvs_event_group = xEventGroupCreate();
	if (wm_nvs_event_group == NULL)
	{
		ESP_LOGE(TAG, "Failed to create NVS Event Group");
		return ret;
	}

	///> Initialize Wifi Config Queue
	wm_queue_wifi_config_handle = xQueueCreate(1, sizeof(wm_queue_wifi_config_t));
	if (wm_queue_wifi_config_handle == NULL)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Config Queue");
		return ret;
	}

	///> Initialize Wifi Connect Task
	xReturned = xTaskCreatePinnedToCore(&wm_wifi_connect_task, "wm_wifi_connect_task", WIFI_CONNECT_TASK_STACK_SIZE, NULL, \
																WIFI_CONNECT_TASK_PRIORITY, &wm_wifi_connect_task_handle	, WIFI_CONNECT_CORE_ID);
	if (xReturned != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Connect Task");
		return ret;
	}

	///> Initialize NVS Taski
	xReturned = xTaskCreatePinnedToCore(&wm_nvs_task, "wm_nvs_task", NVS_TASK_STACK_SIZE, NULL, \
																NVS_TASK_PRIORITY, &wm_nvs_task_handle, NVS_CORE_ID);
	if (xReturned != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create NVS Task");
		return ret;
	}

	xReturned = xTaskCreatePinnedToCore(&wm_init_task, "wm_init_task", WIFI_MANAGER_INIT_TASK_STACK_SIZE, NULL, \
																WIFI_MANAGER_INIT_TASK_PRIORITY, &wm_init_task_handle, WIFI_MANAGER_INIT_TASK_CORE_ID);
	if (xReturned != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Manager Init Task");
		return ret;
	}

	return ESP_OK;
}

static void wm_init_task(void *pvParameters)
{
	EventBits_t uxBits = xEventGroupWaitBits(wm_nvs_event_group, WM_EVENTG_NVS_CREDS_FOUND | WM_EVENTG_NVS_CREDS_NOT_FOUND, \
																			pdTRUE, pdFALSE, portMAX_DELAY);
	if ((uxBits & WM_EVENTG_NVS_CREDS_FOUND) != 0)
	{
		/*! Do not start the HTTP Server and related tasks if the NVS Credentials are found */
		ESP_LOGI(TAG, "NVS Credentials Found");
		xEventGroupSetBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECT);
		xEventGroupWaitBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECTED, pdTRUE, pdFALSE, portMAX_DELAY);
	}
	else if ((uxBits & WM_EVENTG_NVS_CREDS_NOT_FOUND) != 0)
	{
		/*! Start the HTTP Server and related tasks if the NVS Credentials are not found */
		ESP_LOGI(TAG, "NVS Credentials Not Found");

		xEventGroupWaitBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_OPEN);

	}
	xEventGroupWaitBits(wm_main_event_group, WM_EVENTG_MAIN_CLOSE_SERVER_AND_AP, pdTRUE, pdFALSE, portMAX_DELAY);
	xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_BLOCK_REQ);
	
	vTaskDelete(NULL);
}

/*!
* @brief Delete Event Group
*
* This function deletes the event group.
*/
void wm_delete_event_group(EventGroupHandle_t *event_group)
{
	if (event_group != NULL)
	{
		vEventGroupDelete(*event_group);,
		*event_group = NULL;
	}
}
