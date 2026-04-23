/*******************************************************************************
 * File Name        : time_utils.c
 *
 * Description      : This file contains utility for timer
 *
 * Related Document : See README.md
 *
********************************************************************************
* (c) 2025-2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/
/*****************************************************************************
 * Header Files
 *****************************************************************************/
#include "cybsp.h"
#include "FreeRTOS.h"
#include "timers.h"

#define TIMER_RESTART_VALUE (0U)
#define MILLI_CONVERSER     0.01f
/* ****************************************************************************
 * Global variable
 * ****************************************************************************/
uint32_t timerCount = 0;

/* Timer object used */
mtb_hal_timer_t timer_obj;

/* *******************************************************************************
* Function Name: get_time_ms_f
* *******************************************************************************
* Summary:
*  Convert tick count to milliseconds and returns the value.
*
* Parameters:
*  void
*
* Return:
*  uint32_t - tick count in milliseconds
* *******************************************************************************/
 uint32_t get_time_ms_f(void)
 {
     /* Convert tick count to milliseconds */
     return (xTaskGetTickCount() * portTICK_PERIOD_MS);
 }

/* *******************************************************************************
* Function Name: ifx_time_start
* *******************************************************************************
* Summary:
*  Start the timer with the configured settings.
*
* Parameters:
*  void
*
* Return:
*  result - Success or error
* *******************************************************************************/
 cy_rslt_t ifx_time_start(void)
 {
     cy_rslt_t result = CY_RSLT_SUCCESS;

     result = Cy_TCPWM_Counter_Init(CYBSP_TCPWM_0_GRP_0_COUNTER_2_HW,
             CYBSP_TCPWM_0_GRP_0_COUNTER_2_NUM, &CYBSP_TCPWM_0_GRP_0_COUNTER_2_config);

     if (CY_RSLT_SUCCESS == result)
     {
         result = mtb_hal_timer_setup(&timer_obj, &CYBSP_TCPWM_0_GRP_0_COUNTER_2_hal_config, NULL);
         if (CY_RSLT_SUCCESS == result)
         {
             Cy_TCPWM_Counter_Enable(CYBSP_TCPWM_0_GRP_0_COUNTER_2_HW, CYBSP_TCPWM_0_GRP_0_COUNTER_2_NUM);
             if (CY_RSLT_SUCCESS == result)
             {
                 /* Start the timer with the configured settings */
                 result = mtb_hal_timer_start(&timer_obj);
             }
         }
     }

     return result;
 }

 /* *******************************************************************************
 * Function Name: get_time_ms_f
 * *******************************************************************************
 * Summary:
 *  Reset the timer/counter value.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 * *******************************************************************************/
 void ifx_time_reset()
 {
    (void) mtb_hal_timer_reset(&timer_obj, TIMER_RESTART_VALUE);
 }

 /* *******************************************************************************
 * Function Name: get_time_ms_f
 * *******************************************************************************
 * Summary:
 *  Reads the current value from the timer/counter and returns the value.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  uint32_t - current value from the timer/counter
 * *******************************************************************************/
 uint32_t ifx_time_getCount(void)
 {
    return mtb_hal_timer_read(&timer_obj);
 }

 /* *******************************************************************************
 * Function Name: get_time_ms_f
 * *******************************************************************************
 * Summary:
 *  Reads the current value from the timer/counter, convert it to milliseconds and
 *  returns the value.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  uint32_t - current value in milliseconds
 * *******************************************************************************/
 float ifx_time_get_ms_f(void)
 {
    return mtb_hal_timer_read(&timer_obj) * MILLI_CONVERSER;
 }
