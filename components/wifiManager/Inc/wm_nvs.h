/*!
* @file wm_nvs.h
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.1.0-prerelase.1
*/
#ifndef WM_NVS_H_
#define WM_NVS_H_

#include "wifiManager_private.h"

/*!
* @brief NVS Task Handler
*
*/
extern TaskHandle_t wm_nvs_task_handle;

/*!
* @brief NVS Task
*
* This function is the task for the NVS operations.
*
* @param pvParameters Task parameters
*/
void wm_nvs_task(void *pvParameters);

#endif /* WM_NVS_H_ */