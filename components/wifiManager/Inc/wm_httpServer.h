/*!
* @file wm_httpServer.h
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.2.0-prerelase.2
*/
#ifndef WM_HTTPSERVER_H_
#define WM_HTTPSERVER_H_

#include "wifiManager_private.h"
#include "esp_http_server.h"

/*!
* @brief HTTP Wifi Request Handler Semaphore
* @note This semaphore is used to handle the wifi request from the HTTP server.
*/
SemaphoreHandle_t wm_http_wifi_request_semaphore;

/*!
* @brief HTTP Server Task Handler
*
*/
TaskHandle_t wm_http_server_task_handle;

/*!
* @brief HTTP Server Start Function
*
* This function starts the HTTP server.
*/
BaseType_t http_server_init(void)	

/*!
* @brief HTTP Server Stop Function
*
* This function stops the HTTP server.
*/
BaseType_t http_server_stop(void);

#endif /* WM_HTTPSERVER_H_ */