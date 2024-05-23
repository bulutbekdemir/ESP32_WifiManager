/*!
* @file wifiManager.h
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.1.0-prerelase.1
*/
#ifndef WIFI_MANAGER_H_
#define WIFI_MANAGER_H_

#include "wifiManager_private.h"

/*!
* @brief Wifi Manager Init function
*/
esp_err_t wifiManager_init();

/*!
* @brief Wifi Manager Deinit function
*/
esp_err_t wifiManager_deinit();

#endif /* WIFI_MANAGER_H_ */
