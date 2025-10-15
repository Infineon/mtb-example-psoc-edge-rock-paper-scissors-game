/******************************************************************************
* File Name:   lcd_task.h
*
* Description: This is the header file of LCD display functions.
*
* Related Document: See README.md
*
*
********************************************************************************
* (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
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
/*******************************************************************************
 * Header Guards
 *******************************************************************************/
#ifndef _LCD_TASK_H_
#define _LCD_TASK_H_

/*******************************************************************************
 * Included Headers
 *******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "vg_lite.h"
#include "vg_lite_platform.h"
#include "object_detection_lib.h"
#include "app_i2s.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define NUM_IMAGE_BUFFERS                 (2U)

/* Countdown states */
#define COUNTDOWN_START_VALUE             (4U)
#define COUNTDOWN_READY_VALUE             (3U)
#define COUNTDOWN_STEADY_VALUE            (2U)
#define COUNTDOWN_GO_VALUE                (1U)
#define COUNTDOWN_DONE_VALUE              (0U)
#define MAX_NUMBER_OF_ROUNDS              (3U)
/*******************************************************************************
* Global Variables
*******************************************************************************/
typedef enum
{
    COUNTDOWN_START = COUNTDOWN_START_VALUE,
    COUNTDOWN_READY = COUNTDOWN_READY_VALUE,
    COUNTDOWN_STEADY = COUNTDOWN_STEADY_VALUE,
    COUNTDOWN_GO = COUNTDOWN_GO_VALUE,
    COUNTDOWN_DONE = COUNTDOWN_DONE_VALUE
} countdown_state_t;

extern volatile int countdown_state;
extern cy_semaphore_t   model_semaphore;    /* usb_semaphore */
extern int8_t           bgr888_int8[];
extern bool trigger;
extern volatile bool splash_show;
extern volatile bool result_draw;
extern volatile bool close_touch;
extern volatile bool result_show;
extern volatile bool computer_show;
extern volatile bool player1;
extern volatile bool player2;
extern volatile bool computer;
extern volatile bool robo_show;
extern volatile bool detect_show;
extern volatile bool win_poster_show;
extern volatile bool one;
extern uint8_t round_game;
extern bool first_time;
extern TaskHandle_t  gfx_thread;
extern  TaskHandle_t  touch_thread;
extern bool playerA_wins[MAX_NUMBER_OF_ROUNDS];
extern bool playerB_wins[MAX_NUMBER_OF_ROUNDS];
extern bool playerA_loss[MAX_NUMBER_OF_ROUNDS];
extern bool playerB_loss[MAX_NUMBER_OF_ROUNDS];
extern bool Draw_players[MAX_NUMBER_OF_ROUNDS];
extern volatile bool splash_audio_start;
extern vg_lite_buffer_t *renderTarget;
extern vg_lite_buffer_t usb_yuv_frames[];
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
int8_t * draw( void );
void update_box_data ( vg_lite_buffer_t *renderTarget, prediction_OD_t  *prediction );
void draw_screen_elements(int countdown_state);
void draw_round_and_scoreboard(void);
void draw_player1(int gesture);
void draw_player2(int gesture);
void display_outcome(prediction_OD_t *prediction);
int gesture_assign(int32_t id);
void countdown_timer_cb(TimerHandle_t xTimer);
void mcu_soft_reset();
void cm55_touch_task(void *arg);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _LCD_TASK_H_ */
