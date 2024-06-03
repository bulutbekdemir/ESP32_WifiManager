/*!
* @file wm_wifi.c
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.1.1-prerelase.2+4
*/
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi_netif.h"
#include "lwip/netdb.h"
#ifdef CONFIG_USE_DYNAMIC_AP_NAME
#include "esp_mac.h"
#endif

#include "wifiManager_private.h"
#include "wm_generalMacros.h"
#include "wm_wifi.h"

static const char *TAG = "WM_WIFI";

/// @brief Wifi Station Netif
esp_netif_t *esp_sta_netif;
/// @brief Wifi Access Point Netif
esp_netif_t *esp_ap_netif;

/*!
* @brief Wifi Manager Wifi Config Queue Handler
*
*/
QueueHandle_t wm_queue_wifi_config_handle; /*!< Wifi Config Queue Handle */

/*!
* @brief Wifi Manager Wifi Scan Queue Handler
*
*/
QueueHandle_t wm_queue_wifi_scan_handle; /*!< Wifi Scan Queue Handle */

/*!
* @brief Wifi Manager Wifi Connect Task Handler
*
*/
TaskHandle_t wm_wifi_connect_task_handle; /*!< Wifi Connect Task Handle */

/*!
* @brief Wifi Manager Wifi Scan Task Handler
*
*/
TaskHandle_t wm_wifi_scan_task_handle; /*!< Wifi Scan Task Handle */

/// @brief Wifi STA function declaration
static esp_err_t wm_wifi_connect_sta(wifi_config_t *wifi_config);
/// @brief Wifi Default Wifi Init function declaration
static void wm_wifi_default_wifi_init(void);
/// @brief Wifi Event Handler Init function declaration
static void wm_wifi_event_handler_init(void);
/// @brief Wifi Init function declaration
static void wm_wifi_init(void);
/// @brief Wifi APSTA function declaration
static void wm_wifi_connect_apsta(void);
/// @brief Wifi AP Close function declaration
static BaseType_t wm_wifi_ap_close(void);
/// @brief Wifi Scan function declaration
static void wm_wifi_scan(wifi_app_wifi_scan_t *wifi_scan_list);
/// @brief Wifi Connect from HTTP function declaration
static esp_err_t wm_wifi_connect_from_http(wifi_config_t *wifi_config);

/*!
* @brief Struct initialization, deinitialization, retain and release functions
* @note This functions meant to be used with struct types that are meant to be used with reference counting.
*
*/
INIT_FUNC(wifi_app_wifi_scan_t);
DEINIT_FUNC(wifi_app_wifi_scan_t);
RETAIN_FUNC(wifi_app_wifi_scan_t); ///! TODO: Delete when release version is ready 
RELEASE_FUNC(wifi_app_wifi_scan_t);


/*!
 * @brief Wifi Application Event Handler
 * @param arg_data, that is passed to the handler when its called
 * @param event_base, the event base that the event is registered to
 * @param event_id, the event id that is being called
 * @param event_data, the data that is passed to the event
 * 
 */
static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	static uint8_t wifi_connect_retry = 0;
	if(event_base == WIFI_EVENT)
	{
		switch(event_id)
		{
			case WIFI_EVENT_STA_START:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
				break;
			case WIFI_EVENT_STA_CONNECTED:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
				break;
			case WIFI_EVENT_STA_DISCONNECTED:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");

				wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = (wifi_event_sta_disconnected_t *)malloc(sizeof(wifi_event_sta_disconnected_t));
				*wifi_event_sta_disconnected = *((wifi_event_sta_disconnected_t *)event_data);
				EventBits_t mainStaBits = xEventGroupGetBits(wm_main_event_group);
				switch (wifi_event_sta_disconnected->reason)
				{
					/*
					case WIFI_REASON_NO_AP_FOUND:
						ESP_LOGI(TAG, "No AP Found");
						xEventGroupSetBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECT_FAIL);
						break;*/
					case WIFI_REASON_AUTH_FAIL:
						ESP_LOGI(TAG, "Auth Fail");
						if(++wifi_connect_retry < 2)
						{
							esp_wifi_connect();
						}else {
							ESP_LOGI(TAG, "Max Retries Reached for AUTH fail");
							if((mainStaBits & WM_EVENTG_MAIN_HTTP_OPEN) == WM_EVENTG_MAIN_HTTP_OPEN)
							{
								xEventGroupSetBits(wm_http_event_group, WM_EVENTG_HTTP_WIFI_AUTH_FAIL);
							}else
							{
								xEventGroupSetBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECT_FAIL);
							}
						}	
						break;
					default:
						#ifndef USE_BUTTON_INT
							wifi_connect_retry++;
						#endif
							if(wifi_event_sta_disconnected->reason == WIFI_REASON_NO_AP_FOUND)
							{
								ESP_LOGW(TAG, "Reason: NO_AP_FOUND");
							}else{
								ESP_LOGW(TAG, "Reason: %d", wifi_event_sta_disconnected->reason); 
							}
						#ifndef USE_BUTTON_INT
							if(wifi_connect_retry < MAX_CONNECTION_RETRIES)
							{
								ESP_LOGI(TAG, "Retrying Wifi Connection %d", wifi_connect_retry );  
								esp_wifi_connect();
							}
							else
							{
								ESP_LOGE(TAG, "Max Connection Retries Reached");
								wifi_connect_retry = 0;
								xEventGroupSetBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECT_FAIL);
							}
						#else
							esp_wifi_connect();
						#endif
					break;
				}
				break;
			case WIFI_EVENT_STA_AUTHMODE_CHANGE:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_AUTHMODE_CHANGE");
				break;
			case WIFI_EVENT_STA_WPS_ER_SUCCESS:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_SUCCESS");
				break;
			case WIFI_EVENT_STA_WPS_ER_FAILED:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_FAILED");
				break;
			case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_TIMEOUT");
				break;
			case WIFI_EVENT_STA_WPS_ER_PIN:
				ESP_LOGI(TAG, "WIFI_EVENT_STA_WPS_ER_PIN");
				break;
			case WIFI_EVENT_AP_START:
				ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
				break;
			case WIFI_EVENT_AP_STOP:
				ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
				break;
			case WIFI_EVENT_AP_STACONNECTED:
				ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
				break;
			case WIFI_EVENT_AP_STADISCONNECTED:
				ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
				break;
			case WIFI_EVENT_SCAN_DONE:
				ESP_LOGI(TAG, "WIFI_EVENT_SCAN_DONE");
				xEventGroupSetBits(wm_wifi_event_group, WM_EVENTG_WIFI_SCAN_DONE);
				break;
			default:
				break;
			}
	}else if(event_base == IP_EVENT)
	{
		switch(event_id)
		{
			/*! 
			* @note Just one event is handled here because we are only interested in 
			*			the IP address and only one event is registered
			*/			
			case IP_EVENT_STA_GOT_IP:
				ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
				wifi_connect_retry = 0;
				xEventGroupSetBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECTED);
				break;
			default:
				break;
		}
	}
}

/*!
 * @brief Wifi Connect Task
 * @param pvParameters, void pointer to the parameters passed to the task
 * 
 */
void wm_wifi_connect_task(void *pvParameters)
{
	esp_err_t ret = ESP_FAIL;
	EventBits_t uxBits;
	wm_queue_wifi_config_t wifi_config_msg;

	wm_wifi_init();

	while (1)
	{
		ESP_LOGI(TAG, "Waiting for Wifi Connect Event");
		uxBits = xEventGroupWaitBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECT_FROM_NVS | WM_EVENTG_WIFI_CONNECTED | WM_EVENTG_WIFI_CONNECT_FAIL 
																| WM_EVENTG_WIFI_CONNECT_FROM_HTTP, pdTRUE, pdFALSE, portMAX_DELAY); 
		
		xEventGroupSetBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
		if ((uxBits & WM_EVENTG_WIFI_CONNECT_FROM_HTTP) == WM_EVENTG_WIFI_CONNECT_FROM_HTTP)
		{
			ESP_LOGI(TAG, "Wifi Connect Event Received");

			// Receive the message from the queue
			if (xQueueReceive(wm_queue_wifi_config_handle, &wifi_config_msg, portMAX_DELAY) == pdPASS)
			{
				ESP_LOGI(TAG, "SSID: %s", wifi_config_msg.wifi_config.sta.ssid);
				ESP_LOGI(TAG, "Password: %s", wifi_config_msg.wifi_config.sta.password);

				ret = wm_wifi_connect_from_http(&wifi_config_msg.wifi_config);
				if (ret != ESP_OK)
				{
					ESP_LOGE(TAG, "Wifi Connect Failed because of %s", esp_err_to_name(ret));	
				}
			}
		}
		else if ((uxBits & WM_EVENTG_WIFI_CONNECT_FROM_NVS) == WM_EVENTG_WIFI_CONNECT_FROM_NVS)
		{
			ESP_LOGI(TAG, "Wifi Connect Event Received");

			// Receive the message from the queue
			if (xQueueReceive(wm_queue_wifi_config_handle, &wifi_config_msg, portMAX_DELAY) == pdPASS)
			{
				ESP_LOGI(TAG, "SSID: %s", wifi_config_msg.wifi_config.sta.ssid);
				ESP_LOGI(TAG, "Password: %s", wifi_config_msg.wifi_config.sta.password);

				ret = wm_wifi_connect_sta(&wifi_config_msg.wifi_config);
				if (ret != ESP_OK)
				{
					ESP_LOGE(TAG, "Wifi Connect Failed because of %s", esp_err_to_name(ret));	
				}
			}
		}
		else if ((uxBits & WM_EVENTG_WIFI_CONNECTED) == WM_EVENTG_WIFI_CONNECTED)
		{
			ESP_LOGI(TAG, "Wifi Connect Event Received");
			EventBits_t mainBits;
			mainBits = xEventGroupGetBits(wm_task_event_group);
			if((mainBits & WM_EVENTG_TASK_ALL_INIT_DONE) == WM_EVENTG_TASK_ALL_INIT_DONE)
			{
				xEventGroupSetBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
				BaseType_t xStatus = wm_wifi_ap_close();
				if(xStatus == pdPASS)
				{
					xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_WRITE_CREDS);
					wm_wifi_send_message(&wifi_config_msg.wifi_config);
					xEventGroupWaitBits(wm_nvs_event_group, WM_EVENTG_NVS_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
					xEventGroupClearBits(wm_main_event_group, WM_EVENTG_MAIN_AP_OPEN);
					xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_AP_CLOSED);
					xEventGroupSetBits(wm_task_event_group, WM_EVENTG_TASK_DEINIT);
				}
			}	
		}
		else if ((uxBits & WM_EVENTG_WIFI_CONNECT_FAIL) == WM_EVENTG_WIFI_CONNECT_FAIL)
		{
			ESP_LOGE(TAG, "Wifi Connect Failed");
			if((xEventGroupGetBits(wm_task_event_group) & WM_EVENTG_TASK_DEINIT_DONE) == WM_EVENTG_TASK_DEINIT_DONE)
			{
				ESP_LOGW(TAG, "Wifi Connect Failed when HTTP already deinited, Restarting");
				xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_CLEAR_CREDS);
				xEventGroupWaitBits(wm_nvs_event_group, WM_EVENTG_NVS_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
				esp_restart(); 
			}else if((xEventGroupGetBits(wm_main_event_group) & WM_EVENTG_MAIN_HTTP_OPEN) == WM_EVENTG_MAIN_HTTP_OPEN)
			{
				ESP_LOGI(TAG, "Send disconnected info to HTTP Server");
				xEventGroupSetBits(wm_http_event_group, WM_EVENTG_HTTP_WIFI_CONNECT_FAIL);
			}else 
			{
				ESP_LOGI(TAG, "Starting AP");
				wm_wifi_connect_apsta();
				if((xEventGroupGetBits(wm_nvs_event_group) & WM_EVENTG_NVS_CREDS_FOUND) == WM_EVENTG_NVS_CREDS_FOUND)
				{
					xEventGroupSetBits(wm_nvs_event_group, WM_EVENTG_NVS_CLEAR_CREDS);
					xEventGroupWaitBits(wm_nvs_event_group, WM_EVENTG_NVS_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
				}
				if((xEventGroupGetBits(wm_task_event_group) & WM_EVENTG_TASK_ALL_INIT) == WM_EVENTG_TASK_ALL_INIT)
				{
					ESP_LOGI(TAG, "Standard Init Process");
				}else{
					xEventGroupSetBits(wm_task_event_group, WM_EVENTG_TASK_ALL_INIT);
				}
				xEventGroupSetBits(wm_main_event_group, WM_EVENTG_MAIN_AP_OPEN);
			}
		}
		xEventGroupClearBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
	}
}

/*!
 * @brief Event handler for wifi application
 * 
*/
static void wm_wifi_event_handler_init(void)
{
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_event_handler_instance_t instance_wifi_event;
	esp_event_handler_instance_t instance_ip_event;

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_app_event_handler, NULL, &instance_ip_event));
}

/*! 
 * @brief Initializes the TCP stack and default wifi configuration
*/
static void wm_wifi_default_wifi_init(void)
{
	//Initialize the TCP stack
	ESP_ERROR_CHECK(esp_netif_init());

	esp_sta_netif = esp_netif_create_default_wifi_sta();
	esp_ap_netif = esp_netif_create_default_wifi_ap();

	//Default wifi configuration
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	ESP_LOGI(TAG, "Wifi Default Init Finished");

}

/*!
* @brief Wifi Connect from HTTP function
* @note This function connects to the wifi network with the given wifi config.
*
* @param wifi_config Wifi config
* @return esp_err_t Returns ESP_OK if the connection is successful otherwise ESP_FAIL
*/
static esp_err_t wm_wifi_connect_from_http(wifi_config_t *wifi_config_params)
{
	ESP_LOGI(TAG, "Connecting to Wifi from HTTP %s", wifi_config_params->sta.password);
	if(strcmp((char *)wifi_config_params->sta.password, "\0") == 0)
	{
		ESP_LOGI(TAG, "Connecting to Open Network");
		wifi_config_t wifi_config = {
			.sta = {
				.ssid = "",
				.password = "",
				.threshold.authmode = WIFI_AUTH_OPEN,
			}
		};
		
		strncpy((char *)wifi_config.sta.ssid, (char *)wifi_config_params->sta.ssid, sizeof(wifi_config_params->sta.ssid) - 1);

		ESP_LOGI(TAG, "Connecting to SSID:%s with password:%s", wifi_config_params->sta.ssid, wifi_config_params->sta.password);

		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
		ESP_ERROR_CHECK(esp_wifi_connect());
	
	}else
	{
		wifi_config_t wifi_config = {
			.sta = {
				.ssid = "",
				.password = ""
			}
		};

		strncpy((char *)wifi_config.sta.ssid, (char *)wifi_config_params->sta.ssid, sizeof(wifi_config_params->sta.ssid) - 1);
		strncpy((char *)wifi_config.sta.password, (char *)wifi_config_params->sta.password, sizeof(wifi_config_params->sta.password) - 1);

		ESP_LOGI(TAG, "Connecting to SSID:%s with password:%s", wifi_config_params->sta.ssid, wifi_config_params->sta.password);

		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
		ESP_ERROR_CHECK(esp_wifi_connect());	
	}

	return ESP_OK;
}

/*!
* @brief Wifi STA Init function
* @note This function connects to the wifi network with the given wifi config.
*
* @param wifi_config Wifi config
* @return esp_err_t Returns ESP_OK if the connection is successful otherwise ESP_FAIL
*/
static esp_err_t wm_wifi_connect_sta(wifi_config_t *wifi_config_params)
{
  wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = ""
        }
    };

		strncpy((char *)wifi_config.sta.ssid, (char *)wifi_config_params->sta.ssid, sizeof(wifi_config_params->sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, (char *)wifi_config_params->sta.password, sizeof(wifi_config_params->sta.password) - 1);

    ESP_LOGI(TAG, "Connecting to SSID:%s with password:%s", wifi_config_params->sta.ssid, wifi_config_params->sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

		return ESP_OK;
}

/*!
* @brief Wifi APSTA Init function
* 
*/
static void wm_wifi_connect_apsta(void)
{
#ifdef CONFIG_USE_DYNAMIC_AP_NAME
	uint8_t mac[6];  // Array to hold the MAC address
	esp_err_t result = esp_efuse_mac_get_default(mac);  // Retrieve the MAC address

	if (result != ESP_OK) {
			printf("Failed to get MAC address\n");
	}
#endif

	wifi_config_t wifi_ap_config = {
		.ap = {
				.ssid = WIFI_AP_SSID,
				.password = WIFI_AP_PASS,
				.channel = WIFI_AP_CHANNEL,
				.ssid_hidden = WIFI_AP_SSID_HIDDEN,
				.max_connection = WIFI_AP_MAX_CONNECTIONS,
				.beacon_interval = WIFI_AP_BEACON_INTERVAL,
				.authmode = WIFI_AUTH_WPA_WPA2_PSK
		}
	};

#ifdef CONFIG_USE_DYNAMIC_AP_NAME
	snprintf((char *)wifi_ap_config.ap.ssid + strlen((char *)wifi_ap_config.ap.ssid), 
					sizeof(wifi_ap_config.ap.ssid) - strlen((char *)wifi_ap_config.ap.ssid), 
					"%02X%02X%02X%02X%02X%02X",
					mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif

	if (strlen((char *)wifi_ap_config.ap.password) == 0) {
			wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
	}
		//Configuration DHCP for the IP address for SoftAP
	esp_netif_ip_info_t ap_ip_info;
	memset(&ap_ip_info, 0, sizeof(ap_ip_info)); //we want to set everything to 0 for clean start

	//Must call it first
	esp_netif_dhcps_stop(esp_ap_netif); ///> stop the DHCP server before updating DHCP related information
	
	inet_pton(AF_INET, WIFI_AP_IP_ADDR, &ap_ip_info.ip);
	inet_pton(AF_INET, WIFI_AP_IP_GATEWAY, &ap_ip_info.gw); ///> Assign the Static Ip ,gateway and netmask address'
	inet_pton(AF_INET, WIFI_AP_IP_NETMASK, &ap_ip_info.netmask);
	ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_ap_netif, &ap_ip_info));

	ESP_LOGI(TAG, "Setting up AP with SSID:%s and password:%s", wifi_ap_config.ap.ssid, wifi_ap_config.ap.password);
	ESP_LOGI(TAG, "AP IP Address: %s", WIFI_AP_IP_ADDR);
	ESP_LOGI(TAG, "AP Gateway: %s", WIFI_AP_IP_GATEWAY);
	ESP_LOGI(TAG, "AP Netmask: %s", WIFI_AP_IP_NETMASK);

	ESP_ERROR_CHECK(esp_wifi_stop()); ///> Stop the wifi

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA)); ///> Set the wifi mode to Access Point and Station
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config)); ///> Set the configuration for the Access Point
	ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_BANDWIDTH)); ///> Set the bandwidth for the Access Point, default is 20Mhz best for low change of interference
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE)); ///> Set the power save mode for the wifi, default is NONE

	ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_ap_netif)); ///> start the DHCP server after updating DHCP related information
	ESP_ERROR_CHECK(esp_wifi_start()); ///> Start the wifi
}
	
/*!
 * @brief Initializes the wifi application
 * 
 */
static void wm_wifi_init(void)
{
	ESP_LOGI(TAG, "wm Wifi Init");
	//Initialize the wifi event handler
	wm_wifi_event_handler_init();

	//Initialize the default wifi configuration
	wm_wifi_default_wifi_init();
}

/*!
* @brief Wifi Close AP function
* @note This function closes the wifi access point.
*
* @return BaseType_t Returns pdPASS if the wifi access point is closed successfully otherwise pdFAIL
*/
static BaseType_t wm_wifi_ap_close(void)
{
	wifi_mode_t mode;
	ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_get_mode(&mode));
	if(mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
	{
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
		return pdPASS;
	}
	return pdFAIL;
}

/*!
*  @brief Wifi Scan Task
*	@param pvParameters, void pointer to the parameters passed to the task
*
*/
void wm_wifi_scan_task(void *pvParameters)
{
	wm_queue_wifi_scan_handle = xQueueCreate(1, sizeof(wifi_app_wifi_scan_t));

	while (1)
	{
		xEventGroupWaitBits(wm_wifi_event_group, WM_EVENTG_WIFI_SCAN_START, pdTRUE, pdFALSE, portMAX_DELAY);
		xEventGroupSetBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
		wifi_app_wifi_scan_t *wifi_scan_list = wifi_app_wifi_scan_t_init();
		wm_wifi_scan(wifi_scan_list);
		xEventGroupClearBits(wm_http_event_group, WM_EVENTG_HTTP_BLOCK_REQ);
		wm_wifi_send_scan_message(wifi_scan_list);
		ESP_LOGI(TAG, "Wifi Scan Task Finished");
		xEventGroupWaitBits(wm_wifi_event_group, WM_EVENTG_WIFI_SCAN_RESULT_SENT, pdTRUE, pdFALSE, portMAX_DELAY);
		wifi_app_wifi_scan_t_deinit(wifi_scan_list);
	}
}

/*!
* @brief Wifi Scan function
* @note This function scans the wifi networks and stores the scanned networks in the wifi scan list.
*
* @param wifi_scan_list Wifi scan list
*/
static void wm_wifi_scan(wifi_app_wifi_scan_t *wifi_scan_list)
{

	wifi_app_wifi_scan_t_retain(wifi_scan_list);
	wifi_scan_list->ap_count = MAX_SCAN_LIST_SIZE;

	ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&(*wifi_scan_list).ap_count));
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&(*wifi_scan_list).ap_count, (*wifi_scan_list).ap_records));

	ESP_LOGI(TAG, "Number of APs found: %d %s", (*wifi_scan_list).ap_count, (*wifi_scan_list).ap_records[0].ssid);
	
	xEventGroupWaitBits(wm_wifi_event_group, WM_EVENTG_WIFI_SCAN_DONE, pdTRUE, pdFALSE, portMAX_DELAY);
	ESP_LOGI(TAG, "Scan done");
	wifi_app_wifi_scan_t_release(wifi_scan_list);
}

/*!
* @brief Wifi Queue Send Message function
* @note This function sends the wifi config message to the wifi config queue.
*
* @param wifi_config Wifi config message
* @return BaseType_t Returns pdPASS if the message is sent successfully otherwise pdFAIL
*/
BaseType_t wm_wifi_send_message(wifi_config_t *wifi_config)
{
    wm_queue_wifi_config_t wifi_config_msg;
    wifi_config_msg.wifi_config = *wifi_config; 

    // Send the message to the queue
    return xQueueSend(wm_queue_wifi_config_handle, &wifi_config_msg, portMAX_DELAY);
}

/*!
* @brief Wifi Queue Receive Message function
* @note This function receives the wifi config message from the wifi config queue.
*
* @param wifi_config Wifi config 
* @return BaseType_t Returns pdPASS if the message is received successfully otherwise pdFAIL
*/
BaseType_t wm_wifi_receive_message(wifi_config_t *wifi_config)
{
		wm_queue_wifi_config_t wifi_config_msg;
		// Receive the message from the queue
		BaseType_t xStatus = xQueueReceive(wm_queue_wifi_config_handle, &wifi_config_msg, portMAX_DELAY);
		*wifi_config = wifi_config_msg.wifi_config;

		return xStatus;
}

/*!
* @brief Wifi Queue Send Scan function
* @note This function sends the wifi scan message to the wifi scan queue.
*
* @param wifi_scan_msg Wifi scan message
* @return BaseType_t Returns pdPASS if the message is sent successfully otherwise pdFAIL
*/
BaseType_t wm_wifi_send_scan_message(wifi_app_wifi_scan_t *wifi_scan_msg)
{
	ESP_LOGI(TAG, "Sending Scan Message to Queue %s %d %s", wifi_scan_msg->ap_records[0].ssid, wifi_scan_msg->ap_records[0].rssi, wifi_scan_msg->ap_records[1].ssid);
	// Send the message to the queue
	return xQueueSend(wm_queue_wifi_scan_handle,(void *) wifi_scan_msg, portMAX_DELAY);
}

/*!
* @brief Wifi Queue Receive Scan function
* @note This function receives the wifi scan message from the wifi scan queue.
*
* @param wifi_scan_msg Wifi scan message
* @return BaseType_t Returns pdPASS if the message is received successfully otherwise pdFAIL
*/
BaseType_t wm_wifi_receive_scan_message(wifi_app_wifi_scan_t *wifi_scan_msg)
{
	ESP_LOGI(TAG, "Waiting for Receive Scan Message");
	// Receive the message from the queue
	return xQueueReceive(wm_queue_wifi_scan_handle,(void *) wifi_scan_msg, portMAX_DELAY);
}
