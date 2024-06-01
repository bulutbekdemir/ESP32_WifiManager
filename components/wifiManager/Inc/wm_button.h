/*!
* @file wm_button.h
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.1.0-prerelase.1
*/
#ifndef WM_BUTTON_H_
#define WM_BUTTON_H_

#define BUTTON_GPIO CONFIG_BUTTON_INT_PIN /*!< Button GPIO */

#ifdef CONFIG_BUTTON_FALLING_EDGE
#define BUTTON_INTR_NEGEDGE /*!< Button Interrupt Falling Edge */
#endif
#ifdef CONFIG_BUTTON_RISING_EDGE
#define BUTTON_INTR_POSEDGE /*!< Button Interrupt Rising Edge */
#endif

#ifdef CONFIG_BUTTON_PULLUP
#define GPIO_PULLUP /*!< Button Pullup */
#endif

#ifdef CONFIG_BUTTON_PULLDOWN
#define GPIO_PULLDOWN /*!< Button Pulldown */
#endif

/*!
* @brief Button Task Handler
*
*/
extern TaskHandle_t button_task_handle;

/*!
* @brief Button Task
*
* This function is the task for the button interrupt.
*
* @param pvParameters Task parameters
*/
void button_task(void *pvParameters); 


#endif /* WM_BUTTON_H_ */