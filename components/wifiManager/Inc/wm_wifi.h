/*!
* @file wm_wifi.h
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.1.1-prerelase.1+1
*/
#ifndef WIFI_APP_H_
#define WIFI_APP_H_

#include "stdint.h"

#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"

#define REF_COUNT_FIELD int ref_count;

#define WIFI_AP_SSID	CONFIG_WIFI_AP_SSID	// "ESP_WM_AP" is default
#define WIFI_AP_PASS	CONFIG_WIFI_AP_PASS	// "Esp1234!" is default
#define WIFI_AP_CHANNEL	CONFIG_WIFI_AP_CHANNEL	// 1 is default
#define WIFI_AP_MAX_CONNECTIONS	CONFIG_WIFI_AP_MAX_CONNECTIONS	// 4 is default
#define WIFI_AP_SSID_HIDDEN	CONFIG_WIFI_AP_SSID_HIDDEN	// 0 is default
#define WIFI_AP_BEACON_INTERVAL	CONFIG_WIFI_AP_BEACON_INTERVAL	// 100 is default
#define WIFI_AP_IP_ADDR	CONFIG_WIFI_AP_IP_ADDR	// "192.168.0.24" is default
#define WIFI_AP_IP_GATEWAY	CONFIG_WIFI_AP_IP_GATEWAY	// "192.168.0.24" is default
#define WIFI_AP_IP_NETMASK	CONFIG_WIFI_AP_IP_NETMASK	// "255.255.255.0" is default

#define WIFI_BANDWIDTH		CONFIG_WIFI_BANDWIDTH		// WIFI_BW_HT20 is default NOTE: best for low change of interference
#define WIFI_STA_POWER_SAVE	CONFIG_WIFI_STA_POWER_SAVE	// 0 is default
#define MAX_SSID_LENGTH		CONFIG_MAX_SSID_LENGTH		// 32 is default
#define MAX_PASSWORD_LENGTH	CONFIG_MAX_PASSWORD_LENGTH	// 64 is default
#define MAX_CONNECTION_RETRIES	CONFIG_MAX_CONNECTION_RETRIES	// 5 is default

#define MAX_SCAN_LIST_SIZE	CONFIG_MAX_SCAN_LIST_SIZE	// 10 is default
#define WIFI_SCAN_SSID		CONFIG_WIFI_SCAN_SSID		// 0 is default, otherwise specify the SSID
#define WIFI_SCAN_BSSID		CONFIG_WIFI_SCAN_BSSID		// 0 is default, otherwise specify the BSSID
#define WIFI_SCAN_CHANNEL	CONFIG_WIFI_SCAN_CHANNEL		// 0 is default, 1-13 is valid
#define WIFI_SCAN_SHOW_HIDDEN	CONFIG_WIFI_SCAN_SHOW_HIDDEN	// 0 is default NOTE: set to 1 to show hidden SSIDs
#define WIFI_SCAN_TYPE		CONFIG_WIFI_SCAN_TYPE		// WIFI_SCAN_TYPE_ACTIVE is default
#define WIFI_SCAN_TIME_MIN		CONFIG_WIFI_SCAN_TIME_MIN		// 0 is default, otherwise specify the time in ms
#define WIFI_SCAN_TIME_MAX		CONFIG_WIFI_SCAN_TIME_MAX		// 0 is default, otherwise specify the time in ms

/*!
* @brief Struct type for scanned wifi networks
*
*/
typedef struct {
	uint16_t ap_count;
	wifi_ap_record_t ap_records[MAX_SCAN_LIST_SIZE];
	REF_COUNT_FIELD;
}wifi_app_wifi_scan_t;


/*!
* @brief Wifi Manager Wifi Config Queue
*
* This struct defines the Wifi Manager Wifi Config Queue.
*/
typedef struct {
	wifi_config_t wifi_config;
}wm_queue_wifi_config_t;

/*!
* @brief Wifi Manager Wifi Config Queue Handler
*
*/
extern QueueHandle_t wm_queue_wifi_config_handle; /*!< Wifi Config Queue Handle */

/*!
* @brief Wifi Manager Wifi Scan Queue Handler
*
*/
extern QueueHandle_t wm_queue_wifi_scan_handle; /*!< Wifi Scan Queue Handle */

/*!
* @brief Wifi Manager Wifi Connect Task Handler
*
*/
extern TaskHandle_t wm_wifi_connect_task_handle; /*!< Wifi Connect Task Handle */

/*!
* @brief Wifi Manager Wifi Scan Task Handler
*
*/
extern TaskHandle_t wm_wifi_scan_task_handle; /*!< Wifi Scan Task Handle */

/*!
* @brief Wifi Manager Wifi Connect Task
*
*/
void wm_wifi_connect_task(void *pvParameters);

/*!
* @brief Wifi Manager Wifi Scan Task
*
*/
void wm_wifi_scan_task(void *pvParameters);

/*!
 * @brief Send Message to Wifi Config Queue
 * 
 */
BaseType_t wm_wifi_send_message(wifi_config_t *wifi_config);

/*!
 * @brief Receive Message from Wifi Config Queue
 * 
 */
BaseType_t wm_wifi_receive_message(wifi_config_t *wifi_config);

/*!
 * @brief Send Message to Wifi Scan Queue
 * 
 */
BaseType_t wm_wifi_send_scan_message(wifi_app_wifi_scan_t *wifi_scan_msg);

/*!
 * @brief Receive Message from Wifi Scan Queue
 * 
 */
BaseType_t wm_wifi_receive_scan_message(wifi_app_wifi_scan_t *wifi_scan_msg);

#endif /* WIFI_APP_H_ */