/*!
* @file wm_button.c
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.1.0-prerelase.1
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "wifiManager_private.h"
#include "wm_button.h"
#include "wm_wifi.h"

static const char *TAG = "WM_BUTTON";

///> Button Task Handler
TaskHandle_t button_task_handle;

/*!
* @brief Button GPIO ISR Handler
*
* This function is the ISR handler for the button GPIO.
*
* @param arg GPIO Number
*/
void IRAM_ATTR gpio_isr_handler(void* arg) {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xEventGroupSetBitsFromISR(wm_task_event_group, WM_EVENTG_TASK_BUTTON_PRESSED, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken) {
				portYIELD_FROM_ISR();
		}
}

/*!
* @brief Button GPIO Initialization
*
*/
esp_err_t button_init() {
	esp_err_t ret = ESP_FAIL;

	gpio_config_t io_conf = {
		#ifdef BUTTON_INTR_POSEDGE
		.intr_type = GPIO_INTR_POSEDGE,
		#endif
		#ifdef BUTTON_INTR_NEGEDGE
		.intr_type = GPIO_INTR_NEGEDGE,
		#endif  
		.pin_bit_mask = (1ULL << BUTTON_GPIO),
		.mode = GPIO_MODE_INPUT,
		#ifdef GPIO_PULLUP
		.pull_up_en = 1,
		#endif
		#ifdef GPIO_PULLDOWN
		.pull_down_en = 1,
		#endif 
	};

	ret = gpio_config(&io_conf);
	if(ret != ESP_OK) {
		ESP_LOGE(TAG, "Button GPIO Config Failed");
		return ret;
	}

	ret =	gpio_install_isr_service(0);
	if(ret != ESP_OK) {
		ESP_LOGE(TAG, "Button ISR Service Install Failed");
		return ret;
	}

	ret = gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, (void*) BUTTON_GPIO);
	if(ret != ESP_OK) {
		ESP_LOGE(TAG, "Button ISR Handler Add Failed");
		return ret;
	}
	return ESP_OK;
}

/*!
* @brief Button Task
*
* This function is the task for the button interrupt.
*
* @param pvParameters Task parameters
*/
void button_task(void *pvParameters) {
	ESP_LOGI(TAG, "Button Task Started");
	esp_err_t ret = button_init();
	if(ret != ESP_OK) {
		ESP_LOGE(TAG, "Button Init Failed");
		vTaskDelete(NULL);
	}
	while(1) {
		xEventGroupWaitBits(wm_task_event_group, WM_EVENTG_TASK_BUTTON_PRESSED, pdTRUE, pdFALSE, portMAX_DELAY);
		ESP_LOGI(TAG, "Button Pressed");
		//if((xEventGroupGetBits(wm_main_event_group) & WM_EVENTG_MAIN_HTTP_OPEN) != WM_EVENTG_MAIN_HTTP_OPEN) {
			ESP_LOGI(TAG, "Button Pressed, Clear Process Started");
			xEventGroupSetBits(wm_wifi_event_group, WM_EVENTG_WIFI_CONNECT_FAIL);
		//}
	}
}


