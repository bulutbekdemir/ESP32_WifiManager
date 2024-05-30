/*!
* @file wifiManager.c
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.2.4-prerelase.3+4 
*/
#include "esp_log.h"

#include "wifiManager.h"
#include "wifiManager_private.h"
#include "wm_wifi.h"
#include "wm_nvs.h"
#include "wm_httpServer.h"

static const char *TAG = "WIFI_MANAGER_MAIN";

/*!
* @brief Wifi Manager Init Task Handler
*
*/
TaskHandle_t wm_init_task_handle; /*!< Wifi Manager Init Task Handle */

/*!
* @brief Wifi Manager NVS Event Group Handler
*
*/
EventGroupHandle_t wm_nvs_event_group; /*!< Event Group Handle */

/*!
* @brief Wifi Manager Wifi Event Group Handler
*
*/
EventGroupHandle_t wm_wifi_event_group; /*!< Event Group Handle */

/*!
* @brief Wifi Manager Main Event Group Handler
*
*/
EventGroupHandle_t wm_main_event_group; /*!< Event Group Handle */

/*!
* @brief Wifi Manager Wifi Task Handler
*
*/
EventGroupHandle_t wm_task_event_group;

///>Function Prototypes
static void wm_http_server_start(void);
static void wm_http_server_stop(void);
static void wm_scan_task_start(void);
static void wm_scan_task_stop(void);

///>Task Prototypes
static void wm_init_task(void *pvParameters);

esp_err_t wifiManager_init ()
{
	esp_err_t ret = ESP_FAIL;
	BaseType_t xReturned = pdFAIL;

	ESP_LOGI(TAG, "Wifi Manager Init Started");

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

	///> Initialize Task Event Group
	wm_task_event_group = xEventGroupCreate();
	if (wm_task_event_group == NULL)
	{
		ESP_LOGE(TAG, "Failed to create Task Event Group");
		return ret;
	}

	///> Initialize Wifi Config Queue
	wm_queue_wifi_config_handle = xQueueCreate(1, sizeof(wm_queue_wifi_config_t));
	if (wm_queue_wifi_config_handle == NULL)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Config Queue");
		return ret;
	}

	ESP_LOGI(TAG, "Starting Wifi Connect Task");

	///> Initialize Wifi Connect Task
	xReturned = xTaskCreatePinnedToCore(&wm_wifi_connect_task, "wm_wifi_connect_task", WIFI_CONNECT_TASK_STACK_SIZE, NULL, \
																WIFI_CONNECT_TASK_PRIORITY, &wm_wifi_connect_task_handle	, WIFI_CONNECT_TASK_CORE_ID);
	if (xReturned != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Connect Task");
		return ret;
	}

	ESP_LOGI(TAG, "Starting NVS Task");

	///> Initialize NVS Taski
	xReturned = xTaskCreatePinnedToCore(&wm_nvs_task, "wm_nvs_task", NVS_TASK_STACK_SIZE, NULL, \
																NVS_TASK_PRIORITY, &wm_nvs_task_handle, NVS_TASK_CORE_ID);
	if (xReturned != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create NVS Task");
		return ret;
	}

	ESP_LOGI(TAG, "Wifi Manager Init Finished, Starting Wifi Manager Init Task");

	xReturned = xTaskCreatePinnedToCore(&wm_init_task, "wm_init_task", WIFI_MANAGER_INIT_TASK_STACK_SIZE, NULL, \
																WIFI_MANAGER_INIT_TASK_PRIORITY, &wm_init_task_handle, WIFI_MANAGER_INIT_TASK_CORE_ID);
	if (xReturned != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Manager Init Task");
		return ret;
	}

	return ESP_OK;
}

/*!
* @brief Wifi Manager Init Task
*
* This function is the main task for the Wifi Manager.
*/
static void wm_init_task(void *pvParameters)
{
	ESP_LOGI(TAG, "Wifi Manager Init Task Started");
	xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_READ_CREDS);
	while (1)
	{
		EventBits_t uxBits = xEventGroupWaitBits(wm_task_event_group, 	WM_EVENTG_TASK_ALL_INIT | WM_EVENTG_TASK_WIFI_INIT 
																						| WM_EVENTG_TASK_DEINIT, pdTRUE, pdFALSE, portMAX_DELAY);

		if ((uxBits & WM_EVENTG_TASK_WIFI_INIT) != 0)
		{
			ESP_LOGI(TAG , "NVS Creds Found, Starting only Wifi Application");
			xEventGroupSetBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECT_FROM_NVS);
		}else if ((uxBits & WM_EVENTG_TASK_ALL_INIT) != 0){
			ESP_LOGI(TAG, "NVS Creds Not Found, Starting Wifi Application and HTTP Server");
			wm_scan_task_start();
			xEventGroupWaitBits(wm_main_event_group, WM_EVENTG_MAIN_SCAN_TASK_OPEN, pdFALSE, pdFALSE, portMAX_DELAY);
			wm_http_server_start();
			xEventGroupWaitBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_OPEN, pdFALSE, pdFALSE, portMAX_DELAY);
			ESP_LOGI(TAG, "ALL Init Done!");
			xEventGroupSetBits(wm_task_event_group, WM_EVENTG_TASK_ALL_INIT_DONE);
		}else if ((uxBits & WM_EVENTG_TASK_DEINIT) != 0){
			ESP_LOGI(TAG, "Deinit Task Triggered");
			wm_http_server_stop();
			wm_scan_task_stop();
			xEventGroupSetBits(wm_task_event_group, WM_EVENTG_TASK_DEINIT_DONE);
			vTaskDelete(NULL);
		}
	}
}

/*!
* @brief HTTP Server Start Function
*
* This function starts the HTTP server.
*/
static void wm_http_server_start(void)
{
	BaseType_t xReturn = http_server_init();
	if (xReturn != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to start HTTP Server");
		return;
	}
	xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_OPEN);
}

/*!
* @brief HTTP Server Stop Function
*
* This function stops the HTTP server.
*/
static void wm_http_server_stop(void)
{
	BaseType_t xReturn = http_server_stop();
	if (xReturn != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to stop HTTP Server");
		return;
	}
	vQueueDelete(wm_http_event_group);
	if (wm_http_event_group != NULL)
	{
		wm_http_event_group = NULL;
	}
	xEventGroupClearBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_OPEN);
	xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_HTTP_CLOSED);
}

/*!
* @brief Task Starter Function
*
* This function starts the task.
*/
static void wm_scan_task_start(void)
{
	BaseType_t xReturned = pdFAIL;
	xReturned = xTaskCreatePinnedToCore(&wm_wifi_scan_task, "wm_scan_task", WIFI_SCAN_TASK_STACK_SIZE, NULL, \
							WIFI_SCAN_TASK_PRIORITY, NULL, WIFI_SCAN_TASK_CORE_ID);
	if (xReturned != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create Wifi Scan Task");
		return;
	}
	xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_SCAN_TASK_OPEN);
}

/*!
* @brief Scan Task Stop Function
*
* This function stops the scan task.
*/
static void wm_scan_task_stop(void)
{
	vQueueDelete(wm_queue_wifi_scan_handle);
	if (wm_queue_wifi_scan_handle != NULL)
	{
		wm_queue_wifi_scan_handle = NULL;
	}
	vTaskDelete(wm_wifi_scan_task_handle);
	if (wm_wifi_scan_task_handle != NULL)
	{
		wm_wifi_scan_task_handle = NULL;
	}
	xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_SCAN_TASK_CLOSED);
}

