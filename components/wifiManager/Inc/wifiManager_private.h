/*!
* @file wifiManager_private.h
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.3.1-prerelase.7
*/
#ifndef __WIFI_MANAGER_PRIVATE_H__
#define __WIFI_MANAGER_PRIVATE_H__

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "lwip/netdb.h"

#include "wm_wifi.h"
#include "wm_nvs.h"

//Wifi Manager Init task
#define WIFI_MANAGER_INIT_TASK_PRIORITY			CONFIG_WIFI_MANAGER_INIT_TASK_PRIORITY // 6 is default
#define WIFI_MANAGER_INIT_TASK_STACK_SIZE		CONFIG_WIFI_MANAGER_INIT_TASK_STACK_SIZE // 2048 is default
#define WIFI_MANAGER_INIT_TASK_CORE_ID			CONFIG_WIFI_MANAGER_INIT_TASK_CORE_ID // 0 is default

//Wifi application task
#define WIFI_CONNECT_TASK_PRIORITY			CONFIG_WIFI_CONNECT_TASK_PRIORITY // 5 is default
#define WIFI_CONNECT_TASK_STACK_SIZE		CONFIG_WIFI_CONNECT_TASK_STACK_SIZE // 4096 is default
#define WIFI_CONNECT_TASK_CORE_ID				CONFIG_WIFI_CONNECT_TASK_CORE_ID // 0 is default

//Wifi Scan task
#define WIFI_SCAN_TASK_PRIORITY					CONFIG_WIFI_SCAN_TASK_PRIORITY // 5 is default
#define WIFI_SCAN_TASK_STACK_SIZE				CONFIG_WIFI_SCAN_TASK_STACK_SIZE // 4096 is default
#define WIFI_SCAN_TASK_CORE_ID					CONFIG_WIFI_SCAN_TASK_CORE_ID // 0 is default

//NVS task
#define NVS_TASK_PRIORITY							CONFIG_NVS_TASK_PRIORITY // 5 is default
#define NVS_TASK_STACK_SIZE						CONFIG_NVS_TASK_STACK_SIZE // 2048 is default
#define NVS_TASK_CORE_ID							CONFIG_NVS_TASK_CORE_ID // 0 is default

//HTTP Server task
#define HTTP_SERVER_TASK_PRIORITY			CONFIG_HTTP_SERVER_TASK_PRIORITY // 4 is default
#define HTTP_SERVER_TASK_STACK_SIZE		CONFIG_HTTP_SERVER_TASK_STACK_SIZE // 8192 is default
#define HTTP_SERVER_TASK_CORE_ID			CONFIG_HTTP_SERVER_TASK_CORE_ID // 0 is default

/*!
* @brief Wifi Manager Main Event Group
*
* This enum defines the flags for the Wifi Manager Main Event Group.
*/
typedef enum 
{
	//WM_EVENTG_MAIN_CLOSE_SERVER_AND_AP = 1 << 0,     /*!< Flag for Wifi AP and HTTP Server Close Task Trigger */
	WM_EVENTG_MAIN_AP_OPEN = 1 << 0,    /*!< Flag for Wifi AP Open */
	WM_EVENTG_MAIN_AP_CLOSED = 1 << 1,    /*!< Flag for Wifi AP Closed */
	WM_EVENTG_MAIN_HTTP_OPEN = 1 << 2,    /*!< Flag for HTTP Server Open */
	WM_EVENTG_MAIN_HTTP_CLOSED = 1 << 3,   /*!< Flag for HTTP Server Closed */
	WM_EVENTG_MAIN_SCAN_TASK_OPEN = 1 << 4, /*!< Flag for Start Task Sequencer Scan Task Open */
	WM_EVENTG_MAIN_SCAN_TASK_CLOSED = 1 << 5, /*!< Flag for Start Task Sequencer Scan Task Closed */
	WM_EVENTG_MAIN_HTTP_BLOCK_REQ = 1 << 6, /*!< Flag for HTTP Server Block Request */
} wm_main_event_group_e; /*!< Wifi Manager Main Event Group Enum */

/*!
* @brief Wifi Manager Main Event Group Handler
*
*/
EventGroupHandle_t wm_main_event_group; /*!< Event Group Handle */

/*!
* @brief Wifi Event Group 
*
* This enum defines the flags for the Wifi Event Group.
*/
typedef enum 
{
	WM_EVENTG_WIFI_CONNECT = 1 << 0, /*!< Flag for Wifi Connect Task Trigger */
	WM_EVENTG_WIFI_CONNECTED = 1 << 1, /*!< Flag for Wifi Connected */
	WM_EVENTG_WIFI_CONNECT_FAIL = 1 << 2, /*!< Flag for Wifi Disconnected */
	WM_EVENTG_WIFI_SCAN_START = 1 << 3, /*!< Flag for Wifi Scan Start */
	WM_EVENTG_WIFI_SCAN_DONE = 1 << 4, /*!< Flag for Wifi Scan Done */
	WM_EVENTG_WIFI_SCAN_RESULT_SENT = 1 << 5, /*!< Flag for Wifi Scan Result Sent */
	WM_EVENTG_WIFI_HTTP_CONNECT_FAIL = 1 << 6, /*!< Flag for Wifi HTTP Connect Fail */
} wm_wifi_event_group_e; /*!< Wifi Event Group Enum */

/*!
* @brief Wifi Manager Wifi Event Group Handler
*
*/
EventGroupHandle_t wm_wifi_event_group; /*!< Event Group Handle */

/*!
* @brief Wifi Manager NVS Event Group
*
* This enum defines the flags for the Wifi Manager NVS Event Group.
*/
typedef enum
{
	WM_EVENTG_NVS_READ_CREDS = 1 << 0, /*!< Flag for NVS Read Creds */
	WM_EVENTG_NVS_CREDS_FOUND = 1 << 1, /*!< Flag for NVS Creds Found */
	WM_EVENTG_NVS_CREDS_NOT_FOUND = 1 << 2, /*!< Flag for NVS Creds Not Found */
	WM_EVENTG_NVS_WRITE_CREDS = 1 << 3, /*!< Flag for NVS Write Creds */
	WM_EVENTG_NVS_DONE = 1 << 4, /*!< Flag for NVS Task Finished */
	WM_EVENTG_NVS_CLEAR_CREDS = 1 << 5, /*!< Flag for NVS Clear Creds */
	WM_EVENTG_NVS_FAIL = 1 << 6, /*!< Flag for NVS Task Fail */
} wm_nvs_event_group_e; /*!< Wifi Manager NVS Event Group Enum */

/*!
* @brief Wifi Manager NVS Event Group Handler
*
*/
EventGroupHandle_t wm_nvs_event_group; /*!< Event Group Handle */

/*!
* @brief Wifi Manager Init Task Handler
*
*/
TaskHandle_t wm_init_task_handle; /*!< Wifi Manager Init Task Handle */


#endif /* __WIFI_MANAGER_PRIVATE_H__ */