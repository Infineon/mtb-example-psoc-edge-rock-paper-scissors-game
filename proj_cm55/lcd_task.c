/******************************************************************************
* File Name:   lcd_task.c
*
* Description: This file implements the LCD display modules.
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
* Header File
*******************************************************************************/
#include "lcd_task.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "app_i2s.h"
#include "camera_not_supported_img.h"
#include "cybsp.h"
#include "definitions.h"
#include "home_btn_img.h"
#include "ifx_image_utils.h"
#include "inference_task.h"
#include "mtb_ml_gen/model_info.h" /*import CLASS_STRING_LIST and its values*/
#include "mtb_ctp_ft5406.h"
#include "mtb_disp_dsi_waveshare_4p3.h"
#include "no_camera_img.h"
#include "object_detection_structs.h"
#include "retarget_io_init.h"
#include "task.h"
#include "time_utils.h"
#include "usb_camera_task.h"

#include "a_wins.h"
#include "aibot.h"
#include "b_wins.h"
#include "draw.h"
#include "draw_star.h"
#include "final_draw.h"
#include "footer_N.h"
#include "game_start_star.h"
#include "go.h"
#include "header1.h"
#include "img_assets/final_draw.h"
#include "img_assets/final_winner.h"
#include "img_assets/win_final_aibot.h"
#include "img_assets/win_final_playerA.h"
#include "img_assets/win_final_playerB.h"
#include "img_assets/win_final_robot.h"
#include "img_assets/winner_banner.h"
#include "img_assets/winner_star.h"
#include "loss_star.h"
#include "none.h"
#include "none_left.h"
#include "none_right.h"
#include "paper_left.h"
#include "paper_left_box.h"
#include "paper_right.h"
#include "paper_right_box.h"
#include "player_a.h"
#include "player_b.h"
#include "r_win.h"
#include "ready.h"
#include "robot.h"
#include "rock_left.h"
#include "rock_left_box.h"
#include "rock_right.h"
#include "rock_right_box.h"
#include "round1.h"
#include "round2.h"
#include "round3.h"
#include "round_ai_win.h"
#include "sci_left.h"
#include "sci_left_box.h"
#include "sci_right.h"
#include "sci_right_box.h"
#include "splash.h"
#include "start.h"
#include "mode.h"
#include "steady.h"
#include "home_btn.h"
#if defined(MTB_SHARED_MEM)
#include "shared_mem.h"
#endif

/*******************************************************************************
 * Macros
 *******************************************************************************/
#define TOUCH_TIMER_PERIOD_MS               (150U)
#define NO_CAMERA_IMG_X_POS                                                    \
    ((MTB_DISP_WAVESHARE_4P3_HOR_RES / 2U) - ((NO_CAMERA_IMG_WIDTH / 2U) + 10))
#define NO_CAMERA_IMG_Y_POS                                                    \
    ((MTB_DISP_WAVESHARE_4P3_VER_RES / 2U) - (NO_CAMERA_IMG_HEIGHT / 2U))

#define CAMERA_NOT_SUPPORTED_IMG_X_POS                                         \
    ((MTB_DISP_WAVESHARE_4P3_HOR_RES / 2U) -                                   \
     ((CAMERA_NOT_SUPPORTED_IMG_WIDTH / 2U) + 10))
#define CAMERA_NOT_SUPPORTED_IMG_Y_POS                                         \
    ((MTB_DISP_WAVESHARE_4P3_VER_RES / 2U) -                                   \
     (CAMERA_NOT_SUPPORTED_IMG_HEIGHT / 2U))

#define GFX_TASK_DELAY_MS                   (30U)
#define I2C_CONTROLLER_IRQ_PRIORITY         (4U)
#define GPU_INT_PRIORITY                    (3U)
#define DC_INT_PRIORITY                     (3U)
#define COLOR_DEPTH                         (16U)
#define BITS_PER_PIXEL                      (8U)

#define DEFAULT_GPU_CMD_BUFFER_SIZE         ((64U) * (1024U))
#define GPU_TESSELLATION_BUFFER_SIZE        ((DISPLAY_H) * 128U)

#define FRAME_BUFFER_SIZE                   ((DISPLAY_W) * (DISPLAY_H) * ((COLOR_DEPTH) / (BITS_PER_PIXEL)))

#define VGLITE_HEAP_SIZE                    (((FRAME_BUFFER_SIZE) * (3)) + \
                                             (((DEFAULT_GPU_CMD_BUFFER_SIZE) + (GPU_TESSELLATION_BUFFER_SIZE)) * (NUM_IMAGE_BUFFERS)) + \
                                             ((CAMERA_BUFFER_SIZE) * (NUM_IMAGE_BUFFERS + 1)))

#define GPU_MEM_BASE                        (0x0U)
#define TICK_VAL                            (1U)

#define WHITE_COLOR                         (0x00FFFFFFU)
#define BLACK_COLOR                         (0x00000000U)
#define TARGET_NUM_FRAMES                   (15U)
#define RESET_VAL                           (0U)
#define OUTER_BOX_X_POS                     (0U)
#define OUTER_BOX_Y_POS                     (0U)
#define OUTER_BOX_HIGHT                     (512U)
#define OUTER_BOX_WIDTH                     (512U)
#define POSTER_VIEW                         (1U)

#define BOX_X_POS                           (0U)
#define BOX_Y_POS                           (0U)
#define BOX_HIGHT                           (100U)
#define BOX_WIDTH                           (100U)

/* round dimension */
#define ROUND_WIDTH                         (283U)
#define ROUND_HEIGHT                        (62U)

/* star display positions */
#define STAR_1_x                            (75U)
#define STAR_2_x                            (128U)
#define STAR_3_x                            (179U)
#define STAR_4_x                            (592U)
#define STAR_5_x                            (643U)
#define STAR_6_x                            (694U)
#define STAR_H 81

/* star dimension */
#define STAR_W                              (42U)
#define STAR_HT                             (40U)

/* Buttons dimension */
#define START_BUTTON_WIDTH                  (161U)
#define START_BUTTON_HEIGHT                 (137U)

#define PLAYER_1_BUTTON_WIDTH               (216U)
#define PLAYER_1_BUTTON_HEIGHT              (80U)

#define PLAYER_2_BUTTON_WIDTH               (216U)
#define PLAYER_2_BUTTON_HEIGHT              (100U)

#define HOME_BUTTON_WIDTH_GAME              (55U)
#define HOME_BUTTON_HEIGHT_GAME             (55U)

#define COMPUTER_BUTTON_WIDTH               (129U)
#define COMPUTER_BUTTON_HEIGHT              (101U)

#define CLOSE_BUTTON_WIDTH                  (45U)
#define CLOSE_BUTTON_HEIGHT                 (45U)

#define PLAYER1_LEFT_IMG_X_POS              (0U)
#define PLAYER1_LEFT_IMG_CENTER_Y_CALC      (480 - gestures[gesture].left_h) / 2
#define PLAYER1_MAP_X_POS                   (0U)
#define PLAYER1_MAP_Y_POS                   (344U)

#define PLAYER2_RIGHT_IMG_X_POS             (505U)
#define PLAYER2_RIGHT_IMG_CENTER_Y_CALC     (480 - gestures[gesture].right_h) / 2
#define PLAYER2_MAP_X_POS                   (561U)
#define PLAYER2_MAP_Y_POS                   (344U)

/*  Timer periods */
#define DEFERRED_TIMER_PERIOD_MS            (10U)
#define GAME_END_TIMER_PERIOD_MS            (2000U)
#define SHORT_DELAY_TIMER_PERIOD_MS         (1U)

/* Score counting */
#define MAX_ROUNDS                          (3U)
#define GESTURE_MODULO_VALUE                (7U)
#define SCREEN_CENTER_X                     (400U) // Calculated as 800 / 2

/* Gesture values */
#define ROCK_GESTURE                        (2U)
#define PAPER_GESTURE                       (1U)
#define SCISSORS_GESTURE                    (0U)

/* Winner poster positions */
#define WINNER_MODE_X_POS                  (228U)
#define WINNER_MODE_Y_POS                  (147U)
#define WINNER_POSTER_X_POS                (220U)
#define WINNER_POSTER_Y_POS                (139U)
#define FINAL_WINNER_X_POS                 (275U)
#define FINAL_WINNER_Y_POS                 (215U)
#define PLAYER_A_FINAL_X_POS               (240U)
#define PLAYER_A_FINAL_Y_POS               (260U)
#define PLAYER_B_FINAL_X_POS               (240U)
#define PLAYER_B_FINAL_Y_POS               (260U)
#define ROBOT_FINAL_X_POS                  (280U)
#define ROBOT_FINAL_Y_POS                  (260U)
#define AIBOT_FINAL_X_POS                  (280U)
#define AIBOT_FINAL_Y_POS                  (260U)
#define DRAW_FINAL_X_POS                   (306U)
#define DRAW_FINAL_Y_POS                   (220U)

/* Result display positions */
#define DRAW_MAP_X_POS                     (315U)
#define DRAW_MAP_Y_POS                     (415U)
#define WINNER_A_X_POS                     (271U)
#define WINNER_A_Y_POS                     (415U)
#define WINNER_B_X_POS                     (271U)
#define WINNER_B_Y_POS                     (414U)
#define WINNER_ROBOT_X_POS                 (271U)
#define WINNER_ROBOT_Y_POS                 (414U)
#define WINNER_AI_X_POS                    (271U)
#define WINNER_AI_Y_POS                    (414U)

/* None display positions */
#define NONE_LEFT_MAP_X_POS                (0U)
#define NONE_LEFT_MAP_Y_POS                (344U)
#define NONE_IMG_X_POS                     (30U)
#define NONE_IMG_CENTER_Y_CALC             (480 - NONE_IMG_HEIGHT) / 2
#define NONE_RIGHT_MAP_X_POS               (561U)
#define NONE_RIGHT_MAP_Y_POS               (344U)
#define NONE_RIGHT_IMG_X_POS               (614U)

/* Random gesture range */
#define RANDOM_GESTURE_MAX                (3U)

/* Header and footer positions */
#define HEADER_X_POS                      (0U)
#define HEADER_Y_POS                      (0U)
#define FOOTER_X_POS                      (0U)
#define FOOTER_Y_POS                      (480U - FOOTER_HEIGHT)

/* Countdown display positions */
#define COUNTDOWN_X_POS                   (320U)
#define COUNTDOWN_Y_POS                   (75U)

/* Player label positions */
#define PLAYER_A_LABEL_X_POS              (10U)
#define PLAYER_A_LABEL_Y_POS              (420U)
#define PLAYER_B_LABEL_X_POS              (550U)
#define PLAYER_B_LABEL_Y_POS              (420U)
#define ROBOT_LABEL_X_POS                 (555U)
#define ROBOT_LABEL_Y_POS                 (420U)
#define AIBOT_LABEL_X_POS                 (555U)
#define AIBOT_LABEL_Y_POS                 (420U)

/* Round image position */
#define ROUND_IMAGE_X_POS                  (260U)
#define ROUND_IMAGE_Y_POS                  (75U)

/* FPS reporting */
#define FPS_REPORT_INTERVAL_MS             (1000U)
#define FPS_CALCULATION_FACTOR             (1000.0f)

/* Button positions */
#define START_BUTTON_X_POS                 (319U)
#define START_BUTTON_Y_POS                 (160U)
#define PLAYER_1_BUTTON_X_POS              (90U)
#define PLAYER_1_BUTTON_Y_POS              (355U)
#define PLAYER_2_BUTTON_X_POS              (517U)
#define PLAYER_2_BUTTON_Y_POS              (355U)
#define COMPUTER_BUTTON_X_POS              (338U)
#define COMPUTER_BUTTON_Y_POS              (351U)
#define CLOSE_BUTTON_X_POS                 (528U)
#define CLOSE_BUTTON_Y_POS                 (130U)

/* Timer periods */
#define COUNTDOWN_TIMER_PERIOD_MS          (1500U)
#define GO_TIMER_PERIOD_MS                 (2000U)

/* Other constants */
#define WATCHDOG_TIMEOUT_MS                (10000.0f)
#define BYTES_PER_PIXEL                    (2U)
#define FINAL_ROUND                        (2U)

/* Image dimensions */
#define WINNER_MODE_WIDTH                  (55U)
#define WINNER_MODE_HEIGHT                 (40U)
#define WINNER_POSTER_WIDTH                (360U)
#define WINNER_POSTER_HEIGHT               (200U)
#define FINAL_WINNER_WIDTH                 (245U)
#define FINAL_WINNER_HEIGHT                (44U)
#define PLAYER_A_FINAL_WIDTH               (323U)
#define PLAYER_A_FINAL_HEIGHT              (54U)
#define PLAYER_B_FINAL_WIDTH               (319U)
#define PLAYER_B_FINAL_HEIGHT              (54U)
#define ROBOT_FINAL_WIDTH                  (264U)
#define ROBOT_FINAL_HEIGHT                 (53U)
#define AIBOT_FINAL_WIDTH                  (255U)
#define AIBOT_FINAL_HEIGHT                 (54U)
#define DRAW_FINAL_WIDTH                   (190U)
#define DRAW_FINAL_HEIGHT                  (59U)
#define DRAW_MAP_WIDTH                     (169U)
#define DRAW_MAP_HEIGHT                    (62U)
#define WINNER_A_WIDTH                     (276U)
#define WINNER_A_HEIGHT                    (62U)
#define WINNER_B_WIDTH                     (276U)
#define WINNER_B_HEIGHT                    (62U)
#define WINNER_ROBOT_WIDTH                 (276U)
#define WINNER_ROBOT_HEIGHT                (62U)
#define WINNER_AI_WIDTH                    (276U)
#define WINNER_AI_HEIGHT                   (62U)
#define NONE_LEFT_MAP_WIDTH                (241U)
#define NONE_LEFT_MAP_HEIGHT               (62U)
#define NONE_RIGHT_MAP_WIDTH               (241U)
#define NONE_RIGHT_MAP_HEIGHT              (62U)
#define NONE_IMG_WIDTH                     (156U)
#define NONE_IMG_HEIGHT                    (156U)

/* Header and footer dimensions */
#define HEADER_WIDTH                       (832U)
#define HEADER_HEIGHT                      (76U)
#define FOOTER_WIDTH                       (832U)
#define FOOTER_HEIGHT                      (76U)

/* Countdown image dimensions */
#define COUNTDOWN_IMG_WIDTH                (160U)
#define COUNTDOWN_IMG_HEIGHT               (331U)

/* Player label dimensions */
#define PLAYER_A_LABEL_WIDTH               (251U)
#define PLAYER_A_LABEL_HEIGHT              (48U)
#define PLAYER_B_LABEL_WIDTH               (242U)
#define PLAYER_B_LABEL_HEIGHT              (48U)
#define ROBOT_LABEL_WIDTH                  (226U)
#define ROBOT_LABEL_HEIGHT                 (52U)
#define AIBOT_LABEL_WIDTH                  (221U)
#define AIBOT_LABEL_HEIGHT                 (48U)
/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
void ifx_image_conv_RGB565_to_RGB888_i8(uint8_t *src_bgr565, int width,
                                        int height, int8_t *dst_rgb888_i8,
                                        int dst_width, int dst_height);
void ifx_lcd_display_Rect_new(uint16_t x0, uint16_t y0, uint8_t *image,
                              uint16_t width, uint16_t height);
void ifx_lcd_display_Rect(uint16_t x0, uint16_t y0, uint8_t *image,
                          uint16_t width, uint16_t height);
uint32_t calculate_idle_percentage(void);
uint32_t ifx_lcd_get_Display_Width();
uint32_t ifx_lcd_get_Display_Height();

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
extern prediction_OD_t Prediction;
extern volatile float Inference_time;
extern cy_stc_scb_uart_context_t DEBUG_UART_context;
static GFXSS_Type *base = (GFXSS_Type *)GFXSS;
static float last_successful_frame_time = 0;
static int recovery_attempts = 0;
volatile bool button_debouncing = false;
volatile uint32_t button_debounce_timestamp = 0;
bool trigger = false;
TaskHandle_t touch_thread;
static uint16_t home_btn_x_pos;
static uint16_t home_btn_y_pos;
cy_stc_gfx_context_t gfx_context;
vg_lite_buffer_t *renderTarget;
vg_lite_buffer_t usb_yuv_frames[NUM_IMAGE_BUFFERS];
vg_lite_buffer_t bgr565;
vg_lite_buffer_t black;

/* flags to control mechanism */
volatile bool uart_continue_flag = false;
volatile bool splash_show = true;
volatile bool result_draw = true;
volatile bool one = true;
bool first_time = true;
volatile bool close_touch = false;
volatile bool result_show = false;
volatile bool computer_show = false;
volatile bool player1 = false;
volatile bool player2 = false;
volatile bool computer = false;
volatile bool robo_show = true;
volatile bool detect_show = false;
uint8_t round_game = 0;
volatile bool game_start = false;
volatile bool audio_notification_sent = false;
volatile bool splash_audio_sync = true;

volatile bool splash_audio_start = true;
volatile bool splash_audio_stop = true;

bool playerA_wins[3] = {false, false, false};
bool playerB_wins[3] = {false, false, false};
bool playerA_loss[3] = {false, false, false};
bool playerB_loss[3] = {false, false, false};
bool Draw_players[3] = {false, false, false};

/* timer handler */
extern TimerHandle_t countdown_timer;
TimerHandle_t deferred_timer;

/* Dimension Variables */
int start_btn_x_pos = START_BUTTON_X_POS;
int start_btn_y_pos = START_BUTTON_Y_POS;

int player_1_btn_x_pos = PLAYER_1_BUTTON_X_POS;
int player_1_btn_y_pos = PLAYER_1_BUTTON_Y_POS;

int player_2_btn_x_pos = PLAYER_2_BUTTON_X_POS;
int player_2_btn_y_pos = PLAYER_2_BUTTON_Y_POS;

int computer_btn_x_pos = COMPUTER_BUTTON_X_POS;
int computer_btn_y_pos = COMPUTER_BUTTON_Y_POS;

int mode_btn_x_pos = WINNER_MODE_X_POS;
int mode_btn_y_pos = WINNER_MODE_Y_POS;

int close_btn_x_pos = CLOSE_BUTTON_X_POS;
int close_btn_y_pos = CLOSE_BUTTON_Y_POS;

int player1_class = -1;
int player2_class = -1;

volatile int countdown_state = COUNTDOWN_START;

CY_SECTION(".cy_socmem_data")
__attribute__((
    aligned(64))) int8_t bgr888_int8[(IMAGE_HEIGHT) * (IMAGE_WIDTH) * 3] = {0}; /* BGR888 integer buffer */

static vg_lite_matrix_t matrix;
extern SemaphoreHandle_t usb_semaphore;

/* Contiguous memory for VGLite heap */
CY_SECTION(".cy_xip")
__attribute((used)) uint8_t contiguous_mem[VGLITE_HEAP_SIZE];

volatile void *vglite_heap_base = &contiguous_mem;   /* VGLite heap base address */
volatile bool fb_pending = false;                   /* Framebuffer pending flag */
volatile bool win_poster_show = false;
static uint32_t frame_count = 0;
static TickType_t last_report_tick = 0;
TaskHandle_t gfx_thread;
vg_lite_buffer_t display_buffer[3]; /* double display frame buffers */

/* scale factor from the camera image to the display */
float scale_Cam2Disp;
static int display_offset_x = 0, display_offset_y = 0;
extern uint8_t _device_connected;
volatile float time_start1;
/*******************************************************************************
 * Global Variables - Shared memory variable
 *******************************************************************************/
#if defined(MTB_SHARED_MEM)
oob_shared_data_t oob_shared_data_ns;
#endif

typedef struct 
{
    const uint8_t *left_img;
    int left_w, left_h;
    const uint8_t *right_img;
    int right_w, right_h;
    const uint8_t *map_img_l;
    int map_w_l, map_h_l;
    const uint8_t *map_img_r;
    int map_w_r, map_h_r;
} GestureAssets;

GestureAssets gestures[3] = 
{
    {Scissors_Left_map, Scissors_Left_map_w, Scissors_Left_map_h,
     Scissors_Right_map, Scissors_Right_map_w, Scissors_Right_map_h,
     Scissors_Left_map_box, Scissors_Left_map_box_w, Scissors_Left_map_box_h,
     Scissors_Right_map_box, Scissors_Right_map_box_w,
     Scissors_Right_map_box_h},
    {Paper_Left_map, Paper_Left_map_w, Paper_Left_map_h, Paper_Right_map,
     Paper_Right_map_w, Paper_Right_map_h, Paper_Left_map_box,
     Paper_Left_map_box_w, Paper_Left_map_box_h, Paper_Right_map_box,
     Paper_Right_map_box_w, Paper_Right_map_box_h},
    {Rock_Left_map, Rock_Left_map_w, Rock_Left_map_h, Rock_Right_map,
     Rock_Right_map_w, Rock_Right_map_h, Rock_Left_map_box, Rock_Left_map_box_w,
     Rock_Left_map_box_h, Rock_Right_map_box, Rock_Right_map_box_w,
     Rock_Right_map_box_h},
};

typedef struct
{
    const uint8_t *round_img;
    int round_w, round_h;
} RoundAssets;

RoundAssets round_image[3] = {{Round_1_map, ROUND_WIDTH, ROUND_HEIGHT},
                              {Round_2_map, ROUND_WIDTH, ROUND_HEIGHT},
                              {Round_3_map, ROUND_WIDTH, ROUND_HEIGHT}};
typedef struct 
{
    int x;
    int y;
    const uint8_t *img;
    int w;
    int h;
} StarAsset;

StarAsset stars[] = 
{
    {STAR_1_x, STAR_H, Start_Star_map, Start_Star_map_w, Start_Star_map_h},
    {STAR_2_x, STAR_H, Start_Star_map, Start_Star_map_w, Start_Star_map_h},
    {STAR_3_x, STAR_H, Start_Star_map, Start_Star_map_w, Start_Star_map_h},
    {STAR_4_x, STAR_H, Start_Star_map, Start_Star_map_w, Start_Star_map_h},
    {STAR_5_x, STAR_H, Start_Star_map, Start_Star_map_w, Start_Star_map_h},
    {STAR_6_x, STAR_H, Start_Star_map, Start_Star_map_w, Start_Star_map_h}
};

StarAsset stars_W[] = 
{
    {STAR_1_x, STAR_H, Winner_Star_map, STAR_W, STAR_HT},
    {STAR_2_x, STAR_H, Winner_Star_map, STAR_W, STAR_HT},
    {STAR_3_x, STAR_H, Winner_Star_map, STAR_W, STAR_HT},
    {STAR_4_x, STAR_H, Winner_Star_map, STAR_W, STAR_HT},
    {STAR_5_x, STAR_H, Winner_Star_map, STAR_W, STAR_HT},
    {STAR_6_x, STAR_H, Winner_Star_map, STAR_W, STAR_HT}
};
StarAsset stars_L[] = {
{   
    STAR_1_x, STAR_H, Lose_Star_map, STAR_W, STAR_HT},
    {STAR_2_x, STAR_H, Lose_Star_map, STAR_W, STAR_HT},
    {STAR_3_x, STAR_H, Lose_Star_map, STAR_W, STAR_HT},
    {STAR_4_x, STAR_H, Lose_Star_map, STAR_W, STAR_HT},
    {STAR_5_x, STAR_H, Lose_Star_map, STAR_W, STAR_HT},
    {STAR_6_x, STAR_H, Lose_Star_map, STAR_W, STAR_HT}
};
StarAsset stars_D[] = {
{   
    STAR_1_x, STAR_H, draw_02_map, STAR_W, STAR_HT},
    {STAR_2_x, STAR_H, draw_02_map, STAR_W, STAR_HT},
    {STAR_3_x, STAR_H, draw_02_map, STAR_W, STAR_HT},
    {STAR_4_x, STAR_H, draw_02_map, STAR_W, STAR_HT},
    {STAR_5_x, STAR_H, draw_02_map, STAR_W, STAR_HT},
    {STAR_6_x, STAR_H, draw_02_map, STAR_W, STAR_HT}
};

static int16_t pathData[] = {
    2, 100, 100,                                               /* Move to (minX, minY) */
    4, 300, 100,                                               /* Line from (minX, minY) to (maxX, minY) */
    4, 300, 200,                                               /* Line from (maxX, minY) to (maxX, maxY) */
    4, 100, 200,                                               /* Line from (maxX, maxY) to (minX, maxY) */
    4, 100, 100,                                               /* Line from (minX, maxY) to (minX, minY) */
    0,
};

static vg_lite_path_t path = {
    .bounding_box = {0, 0, CAMERA_WIDTH, CAMERA_HEIGHT},        /* left, top, right, bottom */
    .quality = VG_LITE_HIGH,                                   /* Quality */
    .format = VG_LITE_S16,                                     /* Format */
    .uploaded = {0},                                           /* Uploaded */
    .path_length = sizeof(pathData),                           /* Path length */
    .path = pathData,                                          /* Path data */
    .path_changed = 1                                          /* Path changed */
};

/*******************************************************************************
 * Global Variables - I2C Controller Configuration
 *******************************************************************************/
cy_stc_scb_i2c_context_t i2c_controller_context;     /* I2C controller context */
mtb_ctp_ft5406_config_t ft5406_config =              /* Touch controller configuration */
{
    .i2c_base = CYBSP_I2C_CONTROLLER_HW,             /* I2C base hardware */
    .i2c_context =&i2c_controller_context            /* Pointer to I2C context */
};

/*******************************************************************************
 * Global Variables - Interrupt Configurations
 *******************************************************************************/
cy_stc_sysint_t dc_irq_cfg =                                   /* DC Interrupt Configuration */
{
    .intrSrc      = gfxss_interrupt_dc_IRQn,                   /* DC interrupt source */
    .intrPriority = DC_INT_PRIORITY                            /* DC interrupt priority */
};

cy_stc_sysint_t gpu_irq_cfg =                                  /* GPU Interrupt Configuration */
{
    .intrSrc      = gfxss_interrupt_gpu_IRQn,                  /* GPU interrupt source */
    .intrPriority = GPU_INT_PRIORITY                           /* GPU interrupt priority */
};

cy_stc_sysint_t i2c_controller_irq_cfg =                       /* I2C Controller Interrupt Configuration */
{
    .intrSrc      = CYBSP_I2C_CONTROLLER_IRQ,                  /* I2C controller interrupt source */
    .intrPriority = I2C_CONTROLLER_IRQ_PRIORITY,                /* I2C controller interrupt priority */
};



/*******************************************************************************
* Function Name: vDeferredActionCallback
********************************************************************************
* Description:
* Callback function invoked by a timer. This function is responsible for
* performing deferred actions after a certain time period has elapsed.
* It checks the current round of the game and updates the countdown state
* or triggers the display of the win poster accordingly.
*
* Parameters:
*   TimerHandle_t xTimer - Handle of the timer that invoked this callback
*
* Return:
*   None
********************************************************************************/
static void vDeferredActionCallback(TimerHandle_t xTimer)
 {
    if (round_game <= FINAL_ROUND - 1) 
    {
        countdown_state = COUNTDOWN_START;
    }
    if (round_game == FINAL_ROUND)
    {
        close_touch = true;
        win_poster_show = true;
    }

    one = false;
#ifdef DEBUG_PRINT
    printf("Deferred action executed after 4 sec!\n");
#endif
}

/*******************************************************************************
* Function Name: countdown_timer_cb
********************************************************************************
* Description:
* Timer callback for state change
*
* Parameters:
*   TimerHandle_t xTimer - Handle of the timer that invoked this callback
*
* Return:
*   None
********************************************************************************/
void countdown_timer_cb(TimerHandle_t xTimer)
{
    (void)xTimer;
    TickType_t xTimeNow = COUNTDOWN_TIMER_PERIOD_MS;
    switch (countdown_state)
    {

        case COUNTDOWN_START:
        detect_show = false;
        one = false;
        result_show = false;
        countdown_state = COUNTDOWN_READY;
        break;

        case COUNTDOWN_READY:
        countdown_state = COUNTDOWN_STEADY;
        break;

        case COUNTDOWN_STEADY:
        countdown_state = COUNTDOWN_GO;
        break;

        case COUNTDOWN_GO:
        detect_show = true;
        xTimerChangePeriod(countdown_timer, GO_TIMER_PERIOD_MS,
                           COUNTDOWN_TIMER_PERIOD_MS);
        uart_continue_flag = true;
        computer_show = true;
        countdown_state = COUNTDOWN_DONE;
        break;

        case COUNTDOWN_DONE:
        one = true;
        result_show = true;
        break;
        default:
        break;
    }
    if((countdown_state != COUNTDOWN_DONE) || (countdown_state == COUNTDOWN_GO))
    {
        BaseType_t count_result = xTimerChangePeriod(countdown_timer, xTimeNow,
                           COUNTDOWN_TIMER_PERIOD_MS);
        if(count_result!=pdPASS)
        {
            printf("[ERROR] Countdown timer period change failed!\n");
            CY_ASSERT(0);
        }
    }
}

/*******************************************************************************
* Function Name: start_countdown
********************************************************************************
* Description:
* Timer start for count down
*
* Parameters:
*  None
*
* Return:
*   None
********************************************************************************/
/* */
void start_countdown(void)
{
    if (xTimerStart(countdown_timer, 0) != pdPASS)
    {
        printf("[ERROR] Countdown timer start failed!\n");
        CY_ASSERT(0);
    }
}

CY_SECTION_ITCM_BEGIN
/*******************************************************************************
* Function Name: mirrorImage
********************************************************************************
* Description: Mirrors an image horizontally by swapping pixels from left to right
*              in the provided buffer. The function assumes a fixed bytes-per-pixel
*              value of 2 (e.g., for RGB565 format) and operates on a buffer with
*              dimensions defined by CAMERA_WIDTH and CAMERA_HEIGHT.
* Parameters:
*   - buffer: Pointer to the vg_lite_buffer_t structure containing the image data
*
* Return:
*   None
********************************************************************************/
void mirrorImage(vg_lite_buffer_t *buffer)
{

    uint8_t temp[4];
    uint8_t *start, *end;
    int m, n;
    int bytes_per_pixel = 0;

    bytes_per_pixel = BYTES_PER_PIXEL;

    for (m = 0; m < CAMERA_HEIGHT; m++)
    {

        start = buffer->memory + m * (CAMERA_WIDTH * bytes_per_pixel);

        end = start + (CAMERA_WIDTH - 1) * bytes_per_pixel;

        for (n = 0; n < CAMERA_WIDTH / 2; n++)
        {

            for (int i = 0; i < bytes_per_pixel; i++)
            {
                temp[i] = start[i];
            }

            for (int i = 0; i < bytes_per_pixel; i++)
            {
                start[i] = end[i];
            }

            for (int i = 0; i < bytes_per_pixel; i++) 
            {
                end[i] = temp[i];
            }

            start += bytes_per_pixel;
            end -= bytes_per_pixel;
        }
    }
}
CY_SECTION_ITCM_END

/*******************************************************************************
* Function Name: cleanup
********************************************************************************
* Description: Deallocates resources and frees memory used by the VGLite
*              graphics library. This function should be called to ensure proper
*              cleanup of VGLite resources when they are no longer needed.
* Parameters:
*   None
*
* Return:
*   None
********************************************************************************/
static void cleanup(void) 
{
    /* Deallocate all the resource and free up all the memory */
    vg_lite_close();
}

CY_SECTION_ITCM_BEGIN
/*******************************************************************************
* Function Name: draw
********************************************************************************
 * Description: Processes and renders an image from a USB camera buffer. The function
 *              performs the following steps:
 *              1. Waits for a ready image buffer from the camera.
 *              2. Converts a 320x240 YUYV 422 image to 320x240 BGR565 format.
 *              3. Optionally mirrors the image if the 3MP camera is disabled.
 *              4. Scales the 320x240 BGR565 image to 800x600 BGR565 for display.
 *              5. Converts the 320x240 BGR565 image to 256x240 BGR888 format (offset by -128).
 *              6. Tracks performance metrics for each step and returns the BGR888 buffer.
 *              The function handles errors by invoking cleanup and asserting on failure.
* Parameters:
*   bgr888_int8 Pointer to the int8_t BGR888 buffer containing the processed image data
*
* Return:
*   None
*
********************************************************************************/
int8_t *draw(void) 
{
    vg_lite_error_t error;
    volatile uint32_t time_draw_start = ifx_time_get_ms_f();
    extern uint8_t lastBuffer;
    extern video_buffer_t image_buff[];

    uint32_t workBuffer = lastBuffer;
    while (image_buff[workBuffer].BufReady == 0)
    {

        for (int32_t ii = 0; ii < NUM_IMAGE_BUFFERS; ii++)
            if (image_buff[(workBuffer + ii) % NUM_IMAGE_BUFFERS].BufReady ==1)
            {
                workBuffer = (workBuffer + ii) % NUM_IMAGE_BUFFERS;
                break;
            }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    /* reset all other buffers to available for input from camera */
    for (int32_t ii = 1; ii < NUM_IMAGE_BUFFERS; ii++)
        image_buff[(workBuffer + ii) % NUM_IMAGE_BUFFERS].BufReady = 0;

    /* convert 320x240 YUYV 422 image into 320*240 BGR565 (scale:1) */
    volatile uint32_t time_draw_1 = ifx_time_get_ms_f();
    error = vg_lite_blit(&bgr565, &usb_yuv_frames[workBuffer],
                         NULL, /* identity matrix */
                         VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
    if (error) {
        printf("\r\nvg_lite_blit() (320x240 YUYV 422 ==> 320*240 BGR565) "
               "returned error %d\r\n",
               error);
        cleanup();
        CY_ASSERT(0);
    }

    vg_lite_finish();

    if (!point3mp_camera_enabled)
    {
        mirrorImage(&bgr565);
    }
    /* convert 320x240 BGR565 image into 800x600 BGR565 (scale:2.5) */
    volatile uint32_t time_draw_3 = ifx_time_get_ms_f();
    error = vg_lite_blit(renderTarget, &bgr565,
                         &matrix,
                         VG_LITE_BLEND_NONE,
                         0, VG_LITE_FILTER_POINT);
    if (error) 
    {
        printf("\r\nvg_lite_blit() (320x240 BGR565 ==> 800x600 display BGR565) "
               "returned error %d\r\n",
               error);
        cleanup();
        CY_ASSERT(0);
    }
    vg_lite_finish();

    /* Clear USB buffer */
    image_buff[workBuffer].NumBytes = 0;
    image_buff[workBuffer].BufReady = 0;

    /* Convert 320x240 BGR565 image into 256x240 BGR888 - 128 */
    volatile uint32_t time_draw_5 = ifx_time_get_ms_f();
    ifx_image_conv_RGB565_to_RGB888_i8(bgr565.memory, CAMERA_WIDTH,
                                       CAMERA_HEIGHT, bgr888_int8, IMAGE_WIDTH,
                                       IMAGE_HEIGHT);

    volatile uint32_t time_draw_end = ifx_time_get_ms_f();
    /* performance measures: time */
    extern float Prep_Wait_Buf, Prep_YUV422_to_bgr565, Prep_bgr565_to_Disp,
        Prep_RGB565_to_RGB888;
    Prep_Wait_Buf = time_draw_1 - time_draw_start;
    Prep_YUV422_to_bgr565 = time_draw_3 - time_draw_1;
    Prep_bgr565_to_Disp = time_draw_5 - time_draw_3;
    Prep_RGB565_to_RGB888 = time_draw_end - time_draw_5;

    return bgr888_int8;
}
CY_SECTION_ITCM_END

/*******************************************************************************
* Function Name: dc_irq_handler
********************************************************************************
* Summary: This is the display controller I2C interrupt handler.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
static void dc_irq_handler(void)
{
    fb_pending = false;
    Cy_GFXSS_Clear_DC_Interrupt(base, &gfx_context);
}

/*******************************************************************************
* Function Name: dc_gpu_irq_handlerirq_handler
********************************************************************************
* Summary: This is the GPU interrupt handler.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
static void gpu_irq_handler(void)
{
    Cy_GFXSS_Clear_GPU_Interrupt(base, &gfx_context);
    vg_lite_IRQHandler();
}

/*******************************************************************************
* Function Name: draw_rectangle
*******************************************************************************
*
* Summary:
*  Draws a rectangle on the render target using the VGLite graphics library.
*
* Parameters:
*  color  - The color of the rectangle in 32-bit format.
*  min_x  - The x-coordinate of the top-left corner of the rectangle.
*  min_y  - The y-coordinate of the top-left corner of the rectangle.
*  max_x  - The x-coordinate of the bottom-right corner of the rectangle.
*  max_y  - The y-coordinate of the bottom-right corner of the rectangle.
*
* Return:
*  None
*
*
*******************************************************************************/
void draw_rectangle(uint32_t color, int16_t min_x, int16_t min_y, int16_t max_x,
                    int16_t max_y)
{
    pathData[1] = min_x;
    pathData[2] = min_y; /* top-left */
    pathData[4] = max_x;
    pathData[5] = min_y; /* top-right */
    pathData[7] = max_x;
    pathData[8] = max_y; /* bottom-right */
    pathData[10] = min_x;
    pathData[11] = max_y; /* bottom-left */
    pathData[13] = min_x;
    pathData[14] = min_y; /* top-left */
    vg_lite_draw(renderTarget, &path, VG_LITE_FILL_EVEN_ODD, &matrix,
                 VG_LITE_BLEND_SRC_OVER, color);
}

/*******************************************************************************
* Function Name: getGestureName
*******************************************************************************
*
* Summary:
*  Returns the name of a gesture based on its numerical value
*
* Parameters:
* int gesture - The numerical value of the gesture
*
* Return:
*  None
*
*
*******************************************************************************/
const char *getGestureName(int gesture)
{
    switch (gesture)
    {
     case 0:
        return "Scissors";
     case 1:
        return "Paper";
     case 2:
        return "Rock";
     default:
        return "Unknown";
    }
}

/*******************************************************************************
* Function Name: gesture_assign
*******************************************************************************
*
* Summary:
*  Assigns the gesture based on id numerical value
*
* Parameters:
* int id - The numerical value of the gesture
*
* Return:
*  int gesture : assigned mapped gesture
*
*
*******************************************************************************/
int gesture_assign(int32_t id)
{
    int gesture = -1;
    if (id == 0)
    {
        gesture = 0;
    } 
    else if (id == 1)
    {
        gesture = 1;
    }
    else if (id == 2)
    {
        gesture = 2;
    }

    return gesture;
}

/*******************************************************************************
* Function Name: draw_player1
*******************************************************************************
*
* Summary:
*  Draws player 1 image
*
* Parameters:
* int gesture - gesture like rock paper or scissors
*
* Return:
*  None
*
*
*******************************************************************************/
void draw_player1(int gesture)
{
    if (gesture < 0)
        return;
    ifx_lcd_display_Rect_new(
        PLAYER1_LEFT_IMG_X_POS, (480 - gestures[gesture].left_h) / 2,
        (uint8_t *)gestures[gesture].left_img, gestures[gesture].left_w,
        gestures[gesture].left_h);
    ifx_lcd_display_Rect_new(PLAYER1_MAP_X_POS, PLAYER1_MAP_Y_POS,
                             (uint8_t *)gestures[gesture].map_img_l,
                             gestures[gesture].map_w_l,
                             gestures[gesture].map_h_l);
}

/*******************************************************************************
* Function Name: draw_player2
*******************************************************************************
*
* Summary:
*  Draws player 2 image
*
* Parameters:
* int gesture - gesture like rock paper or scissors
*
* Return:
*  None
*
*
*******************************************************************************/
void draw_player2(int gesture)
{
    if (gesture < 0)
        return;
    ifx_lcd_display_Rect_new(
        PLAYER2_RIGHT_IMG_X_POS, (480 - gestures[gesture].right_h) / 2,
        (uint8_t *)gestures[gesture].right_img, gestures[gesture].right_w,
        gestures[gesture].right_h);
    ifx_lcd_display_Rect_new(PLAYER2_MAP_X_POS, PLAYER2_MAP_Y_POS,
                             (uint8_t *)gestures[gesture].map_img_r,
                             gestures[gesture].map_w_r,
                             gestures[gesture].map_h_r);
}

/*******************************************************************************
* Function Name: draw_screen_elements
*******************************************************************************
*
* Summary:
*  Draws screen elements like Counter, footer ,header etc on LCD screen
*
* Parameters:
* int countdown_state - countdown_state
*
* Return:
*  None
*
*
*******************************************************************************/
void draw_screen_elements(int countdown_state)
{
    /* ---- Countdown ---- */
    switch (countdown_state)
    {
     case COUNTDOWN_START:
        ifx_lcd_display_Rect_new(COUNTDOWN_X_POS, COUNTDOWN_Y_POS,
                                 (uint8_t *)Start_map, COUNTDOWN_IMG_WIDTH,
                                 COUNTDOWN_IMG_HEIGHT);
        break;

     case COUNTDOWN_READY:
        ifx_lcd_display_Rect_new(COUNTDOWN_X_POS, COUNTDOWN_Y_POS,
                                 (uint8_t *)Ready_map, COUNTDOWN_IMG_WIDTH,
                                 COUNTDOWN_IMG_HEIGHT);
        break;

     case COUNTDOWN_STEADY:
        ifx_lcd_display_Rect_new(COUNTDOWN_X_POS, COUNTDOWN_Y_POS,
                                 (uint8_t *)Steady_map, COUNTDOWN_IMG_WIDTH,
                                 COUNTDOWN_IMG_HEIGHT);
        break;

     case COUNTDOWN_GO:
        ifx_lcd_display_Rect_new(COUNTDOWN_X_POS, COUNTDOWN_Y_POS,
                                 (uint8_t *)Go_map, COUNTDOWN_IMG_WIDTH,
                                 COUNTDOWN_IMG_HEIGHT);
        break;

     case COUNTDOWN_DONE:
        ifx_lcd_display_Rect_new(COUNTDOWN_X_POS, COUNTDOWN_Y_POS,
                                 (uint8_t *)Go_map, COUNTDOWN_IMG_WIDTH,
                                 COUNTDOWN_IMG_HEIGHT);
        uart_continue_flag = false;
        break;

     default:
        ifx_lcd_display_Rect_new(COUNTDOWN_X_POS, COUNTDOWN_Y_POS,
                                 (uint8_t *)Start_map, COUNTDOWN_IMG_WIDTH,
                                 COUNTDOWN_IMG_HEIGHT);
        break;
    }

    /* ---- Footer ---- */
    ifx_lcd_display_Rect(FOOTER_X_POS, FOOTER_Y_POS, (uint8_t *)Footer_N_map,
                         FOOTER_WIDTH, FOOTER_HEIGHT);

    /* ---- Player Labels ---- */
    if (player2)
    {
        ifx_lcd_display_Rect_new(PLAYER_A_LABEL_X_POS, PLAYER_A_LABEL_Y_POS,
                                 (uint8_t *)player_a, PLAYER_A_LABEL_WIDTH,
                                 PLAYER_A_LABEL_HEIGHT);
        ifx_lcd_display_Rect_new(PLAYER_B_LABEL_X_POS, PLAYER_B_LABEL_Y_POS,
                                 (uint8_t *)Player_B, PLAYER_B_LABEL_WIDTH,
                                 PLAYER_B_LABEL_HEIGHT);
    }
    else if (player1)
    {
        ifx_lcd_display_Rect_new(PLAYER_A_LABEL_X_POS, PLAYER_A_LABEL_Y_POS,
                                 (uint8_t *)player_a, PLAYER_A_LABEL_WIDTH,
                                 PLAYER_A_LABEL_HEIGHT);
        ifx_lcd_display_Rect_new(ROBOT_LABEL_X_POS, ROBOT_LABEL_Y_POS,
                                 (uint8_t *)Robot_map, ROBOT_LABEL_WIDTH,
                                 ROBOT_LABEL_HEIGHT);
    }
    else if (computer)
    {
        ifx_lcd_display_Rect_new(PLAYER_A_LABEL_X_POS, PLAYER_A_LABEL_Y_POS,
                                 (uint8_t *)player_a, PLAYER_A_LABEL_WIDTH,
                                 PLAYER_A_LABEL_HEIGHT);
        ifx_lcd_display_Rect_new(AIBOT_LABEL_X_POS, AIBOT_LABEL_Y_POS,
                                 (uint8_t *)AIbot_map, AIBOT_LABEL_WIDTH,
                                 AIBOT_LABEL_HEIGHT);
    }
}

/*******************************************************************************
* Function Name: draw_round_and_scoreboard
*******************************************************************************
*
* Summary:
*  Draws current round and score board for the game
*
* Parameters:
* None
*
* Return:
*  None
*
*
*******************************************************************************/
void draw_round_and_scoreboard(void)
{
    /* ---- Draw Round Image ---- */
    if (round_game <= 2)
    {
        ifx_lcd_display_Rect_new(ROUND_IMAGE_X_POS, ROUND_IMAGE_Y_POS,
                                 (uint8_t *)round_image[round_game].round_img,
                                 round_image[round_game].round_w,
                                 round_image[round_game].round_h);
    }
    else 
    {
        round_game = 0; /* reset round index */
    }

    /* ---- Draw Empty Stars (Placeholders) ---- */
    for (int i = 0; i < (sizeof(stars) / sizeof(stars[0])); i++)
    {
        ifx_lcd_display_Rect_new(stars[i].x, stars[i].y,
                                 (uint8_t *)stars[i].img, stars[i].w,
                                 stars[i].h);
    }

    /* ---- Draw Player A Results ---- */
    for (int i = 0; i < 3; i++) 
    {
        if (playerA_wins[i])
        {
            ifx_lcd_display_Rect_new(stars_W[i].x, stars_W[i].y,
                                     (uint8_t *)Winner_Star_map, stars_W[i].w,
                                     stars_W[i].h);
        }

        if (playerA_loss[i])
        {
            ifx_lcd_display_Rect_new(stars_L[i].x, stars_L[i].y,
                                     (uint8_t *)Lose_Star_map, stars_L[i].w,
                                     stars_L[i].h);
        }

        if (Draw_players[i])
        {
            /* Player A side draw star */
            ifx_lcd_display_Rect_new(stars_D[i].x, stars_D[i].y,
                                     (uint8_t *)draw_02_map, stars_D[i].w,
                                     stars_D[i].h);

            /* Player B side draw star (paired with same round) */
            ifx_lcd_display_Rect_new(stars_D[i + 3].x, stars_D[i + 3].y,
                                     (uint8_t *)draw_02_map, stars_D[i + 3].w,
                                     stars_D[i + 3].h);
        }
    }

    /* ---- Draw Player B Results ---- */
    for (int i = 0; i < 3; i++)
    {
        if (playerB_wins[i])
        {
            ifx_lcd_display_Rect_new(stars_W[i + 3].x, stars_W[i + 3].y,
                                     (uint8_t *)Winner_Star_map,
                                     stars_W[i + 3].w, stars_W[i + 3].h);
        }

        if (playerB_loss[i])
        {
            ifx_lcd_display_Rect_new(stars_L[i + 3].x, stars_L[i + 3].y,
                                     (uint8_t *)Lose_Star_map, stars_L[i + 3].w,
                                     stars_L[i + 3].h);
        }
    }
}

/*******************************************************************************
* Function Name: show_final_winner
*******************************************************************************
*
* Summary:
*  Generic function to display the final winner
*
* Parameters:
* int x_pos
* int y_pos
* const uint8_t *player_img
*  GameEvent_t event
*  uint16_t width
*  uint16_t height
*
* Return:
*  None
*
*
*******************************************************************************/
static void show_final_winner(int x_pos, int y_pos, const uint8_t *player_img,
                              GameEvent_t event, uint16_t width, uint16_t height)
{
    ifx_lcd_display_Rect_new(x_pos, y_pos, (uint8_t *)player_img, width, height);

    /* Notify voice thread once */
    if (audio_notification_sent)
    {
        audio_notification_sent = false;
        g_gameEvent = event;
        xTaskNotifyGive(voice_thread);
    }
}

/*******************************************************************************
* Function Name: start_deferred_timer
*******************************************************************************
*
* Summary:
*  Optimized deferred timer function
*
* Parameters:
*  None
*
* Return:
*  None
*
*
*******************************************************************************/
void start_deferred_timer(void)
{
    if (one) 
    {
        one = false; /* prevent multiple starts */
        audio_notification_sent = true;

        if (deferred_timer == NULL)
        {
            deferred_timer = xTimerCreate(
                "DeferredAction", pdMS_TO_TICKS(DEFERRED_TIMER_PERIOD_MS),
                pdFALSE, (void *)0, vDeferredActionCallback);
        }

        if (deferred_timer != NULL)
        {
            TickType_t period = (round_game == FINAL_ROUND)
                                    ? pdMS_TO_TICKS(GAME_END_TIMER_PERIOD_MS)
                                    : pdMS_TO_TICKS(SHORT_DELAY_TIMER_PERIOD_MS);
            printf("round_game %d", round_game);
            xTimerChangePeriod(deferred_timer, period, 0);
            xTimerStart(deferred_timer, 0);

#ifdef DEBUG_PRINT
            printf("Deferred timer started (%lu ms)\n",
                   (unsigned long)(period * portTICK_PERIOD_MS));
#endif
        }
    }

    if (round_game == FINAL_ROUND && win_poster_show)
    {
        int scoreA = 0, scoreB = 0;
        for (int j = 0; j < MAX_ROUNDS; j++)
        {
            if (playerA_wins[j]) scoreA++;
            if (playerB_wins[j]) scoreB++;
        }

        /* Show the winner poster */
        ifx_lcd_display_Rect(WINNER_POSTER_X_POS, WINNER_POSTER_Y_POS,
                             (uint8_t *)Winner_poster_map,
                             WINNER_POSTER_WIDTH, WINNER_POSTER_HEIGHT);

        if (scoreA > scoreB)
        {
            show_final_winner(PLAYER_A_FINAL_X_POS, PLAYER_A_FINAL_Y_POS,
                              Player_A_final_map, EVENT_PLAYER_A_WIN,
                              PLAYER_A_FINAL_WIDTH, PLAYER_A_FINAL_HEIGHT);

            show_final_winner(FINAL_WINNER_X_POS, FINAL_WINNER_Y_POS,
                              final_Winner_map, EVENT_PLAYER_A_WIN,
                              FINAL_WINNER_WIDTH, FINAL_WINNER_HEIGHT);
        }
        else if (scoreB > scoreA)
        {
            if (player2)
            {
                show_final_winner(PLAYER_B_FINAL_X_POS, PLAYER_B_FINAL_Y_POS,
                                  Player_B_final_map, EVENT_PLAYER_B_WIN,
                                  PLAYER_B_FINAL_WIDTH, PLAYER_B_FINAL_HEIGHT);

                show_final_winner(FINAL_WINNER_X_POS, FINAL_WINNER_Y_POS,
                                  final_Winner_map, EVENT_PLAYER_B_WIN,
                                  FINAL_WINNER_WIDTH, FINAL_WINNER_HEIGHT);
            }
            else if (player1)
            {
                show_final_winner(ROBOT_FINAL_X_POS, ROBOT_FINAL_Y_POS,
                                  Robot_final_map, EVENT_COMPUTER_WIN,
                                  ROBOT_FINAL_WIDTH, ROBOT_FINAL_HEIGHT);

                show_final_winner(FINAL_WINNER_X_POS, FINAL_WINNER_Y_POS,
                                  final_Winner_map, EVENT_COMPUTER_WIN,
                                  FINAL_WINNER_WIDTH, FINAL_WINNER_HEIGHT);
            }
            else if (computer)
            {
                show_final_winner(AIBOT_FINAL_X_POS, AIBOT_FINAL_Y_POS,
                                  final_aibot_map, EVENT_COMPUTER_WIN,
                                  AIBOT_FINAL_WIDTH, AIBOT_FINAL_HEIGHT);

                show_final_winner(FINAL_WINNER_X_POS, FINAL_WINNER_Y_POS,
                                  final_Winner_map, EVENT_COMPUTER_WIN,
                                  FINAL_WINNER_WIDTH, FINAL_WINNER_HEIGHT);
            }
        }
        else /* Draw case */
        {
            show_final_winner(DRAW_FINAL_X_POS, DRAW_FINAL_Y_POS,
                              final_winner_Draw_map, EVENT_DRAW,
                              DRAW_FINAL_WIDTH, DRAW_FINAL_HEIGHT);
        }

            ifx_lcd_display_Rect_new(WINNER_MODE_X_POS, WINNER_MODE_Y_POS,
                             (uint8_t *)Mode_map,
                             WINNER_MODE_WIDTH, WINNER_MODE_HEIGHT);
             
    }
}
/*******************************************************************************
* Function Name: display_outcome
*******************************************************************************
*
* Summary:
*  Displays outcome of the model
*
* Parameters:
*  prediction_OD_t *prediction : Rock
*  paper or scissors prediction
*
* Return:
*  None
*
*
*******************************************************************************/
void display_outcome(prediction_OD_t *prediction)
{
    if (player1_class >= 0 && player2_class >= 0 && detect_show)
    {
        draw_player1(player1_class);
        draw_player2(player2_class);

        if (player1_class == player2_class)
        {

            Draw_players[round_game] = true;
            ifx_lcd_display_Rect(DRAW_MAP_X_POS, DRAW_MAP_Y_POS,
                                 (uint8_t *)Draw_map, DRAW_MAP_WIDTH,
                                 DRAW_MAP_HEIGHT);
        } 
        else if ((player1_class == 0 && player2_class == 1) ||
                   (player1_class == 2 && player2_class == 0) ||
                   (player1_class == 1 && player2_class == 2))
        {

            ifx_lcd_display_Rect(WINNER_A_X_POS, WINNER_A_Y_POS,
                                 (uint8_t *)Winner_A_map, WINNER_A_WIDTH,
                                 WINNER_A_HEIGHT);
            if (round_game < MAX_ROUNDS)
            {
                playerA_wins[round_game] = true;
                playerB_loss[round_game] = true;
            }
        } 
        else
        {

            if (player2)
                ifx_lcd_display_Rect(WINNER_B_X_POS, WINNER_B_Y_POS,
                                     (uint8_t *)Winner_B_map, WINNER_B_WIDTH,
                                     WINNER_B_HEIGHT);
            else if (player1)
            {
                ifx_lcd_display_Rect(WINNER_ROBOT_X_POS, WINNER_ROBOT_Y_POS,
                                     (uint8_t *)Winner_Robot_map,
                                     WINNER_ROBOT_WIDTH, WINNER_ROBOT_HEIGHT);
            } 
            else if (computer)
            {
                ifx_lcd_display_Rect(WINNER_AI_X_POS, WINNER_AI_Y_POS,
                                     (uint8_t *)Winner_AI_map, WINNER_AI_WIDTH,
                                     WINNER_AI_HEIGHT);
            }
            if (round_game < MAX_ROUNDS)
            {
                playerB_wins[round_game] = true;
                playerA_loss[round_game] = true;
            }
        }

        start_deferred_timer();
    } else if (result_show && (player1_class == -1 || player2_class == -1))
    {

        draw_player1(player1_class);
        draw_player2(player2_class);

        ifx_lcd_display_Rect(315, 415, (uint8_t *)Draw_map, 169, 62);

        if (player1_class == -1) 
        {
            ifx_lcd_display_Rect_new(NONE_LEFT_MAP_X_POS, NONE_LEFT_MAP_Y_POS,
                                     (uint8_t *)None_Left_Map,
                                     NONE_LEFT_MAP_WIDTH, NONE_LEFT_MAP_HEIGHT);
            ifx_lcd_display_Rect_new(
                NONE_IMG_X_POS, (480 - NONE_IMG_HEIGHT) / 2,
                (uint8_t *)None_map, NONE_IMG_WIDTH, NONE_IMG_HEIGHT);
        }
        if (player2)
        {
            if (player2_class == -1)
            {
                ifx_lcd_display_Rect_new(
                    NONE_RIGHT_MAP_X_POS, NONE_RIGHT_MAP_Y_POS,
                    (uint8_t *)None_Right_map, NONE_RIGHT_MAP_WIDTH,
                    NONE_RIGHT_MAP_HEIGHT);
                ifx_lcd_display_Rect_new(
                    NONE_RIGHT_IMG_X_POS, (480 - NONE_IMG_HEIGHT) / 2,
                    (uint8_t *)None_map, NONE_IMG_WIDTH, NONE_IMG_HEIGHT);
            }
        }
        if (player1 || computer)
        {
            static int robot_new;
            if (robo_show)
            {
                robot_new = rand() % RANDOM_GESTURE_MAX;
                robo_show = false;
            }
            draw_player2(robot_new);
        }
        Draw_players[round_game] = true;
        start_deferred_timer();
    } 
    else if (result_show && (prediction->count == 0))
    {

        ifx_lcd_display_Rect(DRAW_MAP_X_POS, DRAW_MAP_Y_POS,
                             (uint8_t *)Draw_map, 169, 62);
        Draw_players[round_game] = true;
        start_deferred_timer();
    }
}

/*******************************************************************************
* Function Name: update_box_data
*******************************************************************************
*
* Summary:
*  Update LCD with model output
*
* Parameters:
*  vg_lite_buffer_t *renderTarget
*  prediction_OD_t *prediction : Rock
*  paper or scissors prediction
*
* Return:
*  None
*
*
*******************************************************************************/
void update_box_data(vg_lite_buffer_t *renderTarget,
                     prediction_OD_t *prediction)
{
    player1_class = -1;
    player2_class = -1;
    for (int32_t i = 0; i < prediction->count; i++) 
    {
        int32_t jj = i << 2;
        int32_t id = prediction->class_id[i];

        uint32_t xmin =
            (uint32_t)(prediction->bbox_int16[jj] * scale_Cam2Disp) +
            display_offset_x;
        uint32_t xmax =
            (uint32_t)(prediction->bbox_int16[jj + 2] * scale_Cam2Disp) +
            display_offset_x;

        /* Assign the Proper Gesture according to Player_Id */

        uint32_t x_center = (xmin + xmax) / 2;
        int player_id = (x_center < SCREEN_CENTER_X) ? 1 : 2;
        if (player2)
        {
            int gesture = gesture_assign(id);

            (player_id == 1) ? (player1_class = gesture)
                             : (player2_class = gesture);
        }

        if ((player1 || computer) && (player_id == 1)) 
        {
            static int robot;
            int gesture = gesture_assign(id);
            if (player_id == 1)
            {
                player1_class = gesture;
            }

            if (player1_class >= 0 && computer_show) 
            {
                computer_show = false;

                if (player1)
                {
                    robot = rand() % 3;
                    break;
                } else if (computer && (player1_class >= 0))
                {

                    if (player1_class == ROCK_GESTURE)
                        robot = PAPER_GESTURE;
                    else if (player1_class == PAPER_GESTURE)
                        robot = SCISSORS_GESTURE;
                    else if (player1_class == SCISSORS_GESTURE)
                        robot = ROCK_GESTURE;

                    break;
                }

                printf("Player %d shows %s\n", player_id,
                       getGestureName(gesture));
                printf("Computer shows %s\n", getGestureName(robot));
            }

            player2_class = robot;
        }
    }

    /* Calculate the result and display the result */
    display_outcome((prediction_OD_t *)&prediction);
}
/*******************************************************************************
* Function Name: VG_LITE_ERROR_EXIT
*******************************************************************************
*
* Summary:
*  Handles VGLite error conditions by printing an error message with the status
*  code, calling the cleanup function to deallocate resources, and triggering an
*  assertion to halt execution.
*
* Parameters:
*  msg           - Pointer to a character string describing the error context.
*  vglite_status - VGLite error code indicating the specific error condition.
*
* Return:
*  None
*
*
*******************************************************************************/
void VG_LITE_ERROR_EXIT(char *msg, vg_lite_error_t vglite_status)
{
    printf("%s %d\r\n", msg, vglite_status);
    cleanup();
    CY_ASSERT(0);
}

/* Soft reset the MCU */
void mcu_soft_reset()
{
    oob_shared_data_ns.is_valid = 2;
    oob_shared_data_ns.app_boot_address = 0xFFFFFFFF;
    shared_mem_write(&oob_shared_data_ns);
    vg_lite_clear(renderTarget, NULL, BLACK_COLOR);
    vg_lite_close();
    __NVIC_SystemReset();
}

/*******************************************************************************
 * Function Name: disp_i2c_controller_interrupt
 ********************************************************************************
 * Summary:
 *  I2C controller ISR which invokes Cy_SCB_I2C_Interrupt to perform I2C
 *transfer as controller.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void i2c_controller_interrupt(void)
{ 
    Cy_SCB_I2C_Interrupt(CYBSP_I2C_CONTROLLER_HW, &i2c_controller_context);
}

/*******************************************************************************
* Function Name: touch_read_timer_cb
*******************************************************************************
*
* Summary:
*  Timer callback function for reading touch events from the touch controller.
*  Processes single-touch events, applies debouncing, and checks if the touch
*  coordinates fall within the specified area. If a valid touch is detected
*  action is taken care
*
* Parameters:
*  xTimer - Handle to the FreeRTOS timer that triggered the callback.
*
* Return:
*  None
*
*******************************************************************************/
void cm55_touch_task(void *arg)
{ 
    cy_en_scb_i2c_status_t i2c_status = CY_SCB_I2C_SUCCESS;
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0)
    {
        i2c_status = mtb_ctp_ft5406_init(&ft5406_config);

        if (CY_SCB_I2C_SUCCESS != i2c_status)
        {
            printf("[ERROR] Touch driver initialization failed: %d\r\n",
                   i2c_status);
            CY_ASSERT(0);
        }

        for (;;)
        {
            vTaskDelay(pdMS_TO_TICKS(50));
            cy_en_scb_i2c_status_t i2c_status = CY_SCB_I2C_SUCCESS;
            static int touch_x = 0;
            static int touch_y = 0;
            volatile int x_val = 0;
            volatile int y_val = 0;
            mtb_ctp_touch_event_t touch_event = MTB_CTP_TOUCH_UP;

            i2c_status = mtb_ctp_ft5406_get_single_touch(&touch_event, &touch_x,
                                                         &touch_y);

            if ((CY_SCB_I2C_SUCCESS == i2c_status) &&
                ((MTB_CTP_TOUCH_DOWN == touch_event) ||
                 (MTB_CTP_TOUCH_CONTACT == touch_event)))
                {

                if (!button_debouncing)
                {
                    /* Set the debouncing flag */
                    button_debouncing = true;

                    /* Record the current timestamp */
                    button_debounce_timestamp =
                        (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
                }

                if (button_debouncing &&
                    (((xTaskGetTickCount() * portTICK_PERIOD_MS)) -
                         button_debounce_timestamp >=
                     2 * portTICK_PERIOD_MS))
                {
                    button_debouncing = false;
                    x_val = ((ifx_lcd_get_Display_Width() - 1) - 32) - touch_x;
                    y_val = (ifx_lcd_get_Display_Height() - 1) - touch_y;

                        #if 0
                        printf("x = %d\r\n",x_val);
                        printf("y = %d\r\n",y_val);
                        printf("x = %d\r\n",touch_x);
                        printf("y = %d\r\n",touch_y);

                        printf("xrange1 = %d\r\n",(home_btn_x_pos - 10));
                        printf("xrange2 = %d\r\n",(home_btn_x_pos + (HOME_BUTTON_WIDTH)));
                        printf("yrange1 = %d\r\n",(home_btn_y_pos - 10));
                        printf("yrange2 = %d\r\n",(home_btn_y_pos + (HOME_BUTTON_HEIGHT)));
                        #endif

                     if ((x_val >= (home_btn_x_pos - 10)) &&
                        (x_val <= (home_btn_x_pos + HOME_BUTTON_WIDTH)) &&
                        (y_val >= (home_btn_y_pos - 10)) &&
                        (y_val <= (home_btn_y_pos + HOME_BUTTON_HEIGHT)))
                    {
                        
                            #if defined(MTB_SHARED_MEM)
                            oob_shared_data_ns.is_valid = 2;
                            oob_shared_data_ns.app_boot_address = 0xFFFFFFFF;
                            shared_mem_write(&oob_shared_data_ns);
                            vg_lite_clear(renderTarget, NULL, BLACK_COLOR);
                            vg_lite_close();
                            __NVIC_SystemReset();
                            #endif
                        
                    }
                    /* -------- Start button -------- */

                    else if ((x_val >= (start_btn_x_pos)) &&
                             (x_val <=
                              (start_btn_x_pos + START_BUTTON_WIDTH)) &&
                             (y_val >= (start_btn_y_pos)) &&
                             (y_val <=
                              (start_btn_y_pos + START_BUTTON_HEIGHT)) &&
                             countdown_state == 4 && (!splash_show))
                    {
                        if ((player1 || player2 || computer))
                        {

                            trigger = true;
                            g_gameEvent = EVENT_START;
                            xTaskNotifyGive(voice_thread);
                        }
                    }

                    /* -------- player 1 button -------- */
                    else if ((x_val >= (player_1_btn_x_pos - 10)) &&
                             (x_val <=
                              (player_1_btn_x_pos + PLAYER_1_BUTTON_WIDTH)) &&
                             (y_val >= (player_1_btn_y_pos - 10)) &&
                             (y_val <=
                              (player_1_btn_y_pos + PLAYER_1_BUTTON_HEIGHT)) &&
                             splash_show)
                    {
                        player1 = true;
                        splash_show = false;
                        g_gameEvent = EVENT_SINGLE_PLAYER;
                        xTaskNotifyGive(voice_thread);
                    }

                    /* -------- player 2 button -------- */
                    else if ((x_val >= (player_2_btn_x_pos - 10)) &&
                             (x_val <=
                              (player_2_btn_x_pos + PLAYER_2_BUTTON_WIDTH)) &&
                             (y_val >= (player_2_btn_y_pos - 10)) &&
                             (y_val <=
                              (player_2_btn_y_pos + PLAYER_2_BUTTON_HEIGHT)) &&
                             splash_show)
                    {

                        player2 = true;
                        splash_show = false;
                        g_gameEvent = EVENT_DOUBLE_PLAYER;
                        xTaskNotifyGive(voice_thread);
                    }

                    /* -------- computer button -------- */
                    else if ((x_val >= (computer_btn_x_pos)) &&
                             (x_val <=
                              (computer_btn_x_pos + COMPUTER_BUTTON_WIDTH)) &&
                             (y_val >= (computer_btn_y_pos)) &&
                             (y_val <=
                              (computer_btn_y_pos + COMPUTER_BUTTON_HEIGHT)) &&
                             splash_show)
                    {

                        computer = true;
                        splash_show = false;
                        g_gameEvent = EVENT_AI_BOT;
                        xTaskNotifyGive(voice_thread);
                    }
                    /* -------- Switch Mode button -------- */
                    else if ((x_val >= (mode_btn_x_pos )) &&
                             (x_val <=
                              (mode_btn_x_pos + WINNER_MODE_WIDTH)) &&
                             (y_val >= (mode_btn_y_pos)) &&
                             (y_val <=
                              (mode_btn_y_pos + WINNER_MODE_HEIGHT)) &&
                             (!splash_show) && (round_game == FINAL_ROUND))
                    {

                     
                        splash_show = true;
                        win_poster_show = false;                
                        player2 = false;
                        player1 = false;
                        computer = false;
                        countdown_state = COUNTDOWN_START;
                        splash_audio_start= true;
                        one = false;
                        result_show = false;
                        detect_show = false;

                        for (int i = 0; i < 3; i++) 
                        {
                            playerA_wins[i] = false;
                            playerB_wins[i] = false;
                            playerA_loss[i] = false;
                            playerB_loss[i] = false;
                            Draw_players[i] = false;
                        }

                        round_game = 0;
                        first_time = true;
                        
                    }

                    /* -------- close button -------- */
                    else if ((x_val >= (close_btn_x_pos)) &&
                             (x_val <=
                              (close_btn_x_pos + CLOSE_BUTTON_WIDTH)) &&
                             (y_val >= (close_btn_y_pos)) &&
                             (y_val <=
                              (close_btn_y_pos + CLOSE_BUTTON_HEIGHT)) &&
                             close_touch)
                    {
                        close_touch = false;
                        countdown_state = COUNTDOWN_START;
                        win_poster_show = false;
                        one = false;
                        result_show = false;
                        detect_show = false;

                        for (int i = 0; i < 3; i++) 
                        {
                            playerA_wins[i] = false;
                            playerB_wins[i] = false;
                            playerA_loss[i] = false;
                            playerB_loss[i] = false;
                            Draw_players[i] = false;
                        }

                        round_game = 0;
                        first_time = true;
                    }
                }
            }
        }
    }
}

/*******************************************************************************
* Function Name: VG_switch_frame
*******************************************************************************
*
* Summary:
*  Switches the video/graphics layer frame buffer and manages buffer swapping for
*  display rendering. Signals the USB semaphore based on camera enable status.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void VG_switch_frame()
{
    static int current_buffer = 0;

    /* Sets Video/Graphics layer buffer address and transfer the frame buffer to
     * DC */
    Cy_GFXSS_Set_FrameBuffer(base, (uint32_t *)renderTarget->address,
                             &gfx_context);
    __DMB();

    /* Swap buffers */
    current_buffer ^= 1;
    renderTarget = &display_buffer[current_buffer];

    __DMB();

    if (!logitech_camera_enabled)
    {
        xSemaphoreGive(usb_semaphore);
    } 
    else
     
    {
        BaseType_t result = xSemaphoreTake(usb_semaphore, portMAX_DELAY);
        if (result != pdPASS)
        {
            printf("[USB Camera] USB Semphore set failed\r\n");
        }
    }
}

/*******************************************************************************
* Function Name: init_buffer_system
*******************************************************************************
*
* Summary:
*  Initializes the buffer system by resetting image buffer states, clearing buffer
*  counters, and resetting timeout tracking variables.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void init_buffer_system(void)
{
    printf("[LCD_TASK] Initializing buffer system\n");

    // Ensure clean startup state
    for (int i = 0; i < NUM_IMAGE_BUFFERS; i++)
    {
        image_buff[i].BufReady = 0;
        image_buff[i].NumBytes = 0;
    }
    __DMB();

    lastBuffer = 0;
    __DMB();

    // Reset timeout tracking
    last_successful_frame_time = 0;
    recovery_attempts = 0;

    printf("[LCD_TASK] Buffer system initialized\n");
}

/*******************************************************************************
* Function Name: cm55_ns_gfx_task
*******************************************************************************
*
* Summary:
*  Initializes and manages the graphics subsystem, including display controller,
*  I2C, VGLite, and buffer systems. Handles rendering of camera frames and game
*  status
*
* Parameters:
*  arg - Unused task argument
*
* Return:
*  None
*
*******************************************************************************/
void cm55_ns_gfx_task(void *arg)
{
    CY_UNUSED_PARAMETER(arg);
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0)
    {

        vg_lite_error_t vglite_status = VG_LITE_SUCCESS;

        cy_en_gfx_status_t status = CY_GFX_SUCCESS;

        cy_en_sysint_status_t sysint_status = CY_SYSINT_SUCCESS;
        cy_en_scb_i2c_status_t i2c_result = CY_SCB_I2C_SUCCESS;

        /* Set frame buffer address to the GFXSS configuration structure */
        GFXSS_config.dc_cfg->gfx_layer_config->buffer_address =
            (gctADDRESS *)vglite_heap_base;
        GFXSS_config.dc_cfg->gfx_layer_config->uv_buffer_address =
            (gctADDRESS *)vglite_heap_base;

        /* Initializes the graphics subsystem according to the configuration */
        status = Cy_GFXSS_Init(base, &GFXSS_config, &gfx_context);
        if (CY_GFX_SUCCESS != status)
        {
            printf("Graphics subsystem initialization failed: Cy_GFXSS_Init() "
                   "returned error %d\r\n",
                   status);
            CY_ASSERT(0);
        }

        // setup Display Controller
        sysint_status = Cy_SysInt_Init(&dc_irq_cfg, dc_irq_handler);

        if (CY_SYSINT_SUCCESS != sysint_status)
        {
            printf("Error in registering DC interrupt: %d\r\n", sysint_status);
            CY_ASSERT(0);
        }
       
        NVIC_EnableIRQ(GFXSS_DC_IRQ); /* Enable interrupt in NVIC. */

        Cy_GFXSS_Clear_DC_Interrupt(base, &gfx_context);

        /* Initialize GFX GPU interrupt */
        sysint_status = Cy_SysInt_Init(&gpu_irq_cfg, gpu_irq_handler);

        if (CY_SYSINT_SUCCESS != sysint_status)
        {
            printf("Error in registering GPU interrupt: %d\r\n", sysint_status);
            CY_ASSERT(0);
        }

        /* Enable GPU interrupt */
        Cy_GFXSS_Enable_GPU_Interrupt(base);

        /* Enable GFX GPU interrupt in NVIC. */
        NVIC_EnableIRQ(GFXSS_GPU_IRQ);

        /* Initialize the I2C in controller mode. */
        i2c_result = Cy_SCB_I2C_Init(CYBSP_I2C_CONTROLLER_HW,
                                     &CYBSP_I2C_CONTROLLER_config,
                                     &i2c_controller_context);

        if (CY_SCB_I2C_SUCCESS != i2c_result)
        {
            printf("I2C controller initialization failed !!\n");
            CY_ASSERT(0);
        }

        /* Initialize the I2C interrupt */
        sysint_status =
            Cy_SysInt_Init(&i2c_controller_irq_cfg, &i2c_controller_interrupt);

        if (CY_SYSINT_SUCCESS != sysint_status)
        {
            printf("I2C controller interrupt initialization failed\r\n");
            CY_ASSERT(0);
        }

        /* Enable the I2C interrupts. */
        NVIC_EnableIRQ(i2c_controller_irq_cfg.intrSrc);

        /* Enable the I2C */
        Cy_SCB_I2C_Enable(CYBSP_I2C_CONTROLLER_HW);

        i2c_result = mtb_disp_waveshare_4p3_init(CYBSP_I2C_CONTROLLER_HW,
                                                 &i2c_controller_context);

        if (CY_SCB_I2C_SUCCESS != i2c_result)
        {
            printf(
                "Waveshare 4.3-Inch display init failed with status = %u\r\n",
                (unsigned int)i2c_result);
            CY_ASSERT(0);
        }
         xTaskNotifyGive(touch_thread);
        /***************************************************************/

        vg_module_parameters_t vg_params;
        vg_params.register_mem_base = (uint32_t)GFXSS_GFXSS_GPU_GCNANO;
        vg_params.gpu_mem_base[0] = GPU_MEM_BASE;
        vg_params.contiguous_mem_base[0] = (volatile void *)vglite_heap_base;
        vg_params.contiguous_mem_size[0] = VGLITE_HEAP_SIZE;

        // Init VGLite
        vg_lite_init_mem(&vg_params);

        /* Initialize the draw */
        vglite_status = vg_lite_init(DISPLAY_W, DISPLAY_H);
        if (VG_LITE_SUCCESS != vglite_status)
        {
            VG_LITE_ERROR_EXIT("vg_lite engine init failed: vg_lite_init() "
                               "returned error %d\r\n",
                               vglite_status);
        }

        // setup double display-frame-buffers
        for (int32_t ii = 0; ii < 2; ii++)
        {
            display_buffer[ii].width = DISPLAY_W;
            display_buffer[ii].height = DISPLAY_H;
            display_buffer[ii].format = VG_LITE_BGR565;
            vglite_status = vg_lite_allocate(&display_buffer[ii]);
            if (VG_LITE_SUCCESS != vglite_status)
            {
                VG_LITE_ERROR_EXIT(
                    "display_buffer[] allocation failed in vglite space: "
                    "vg_lite_allocate() returned error %d\r\n",
                    vglite_status);
            }
        }
        renderTarget = &display_buffer[0];

        /* Allocate the camera buffers */
        for (int32_t i = 0; i < NUM_IMAGE_BUFFERS; i++)
        {
            usb_yuv_frames[i].width = CAMERA_WIDTH;
            usb_yuv_frames[i].height = CAMERA_HEIGHT;
            usb_yuv_frames[i].format = VG_LITE_YUYV;
            usb_yuv_frames[i].image_mode = VG_LITE_NORMAL_IMAGE_MODE;
            vglite_status = vg_lite_allocate(&usb_yuv_frames[i]);
            if (VG_LITE_SUCCESS != vglite_status)
            {
                VG_LITE_ERROR_EXIT(
                    "camera buffers allocation failed in vglite space: "
                    "vg_lite_allocate() returned error %d\r\n",
                    vglite_status);
            }
        }

        /* Allocate the work camera buffer */
        bgr565.width = CAMERA_WIDTH;
        bgr565.height = CAMERA_HEIGHT;
        bgr565.format = VG_LITE_BGR565;
        bgr565.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
        vglite_status = vg_lite_allocate(&bgr565);
        if (VG_LITE_SUCCESS != vglite_status)
        {
            VG_LITE_ERROR_EXIT(
                "work camera image bgr565 allocation failed in vglite space: "
                "vg_lite_allocate() returned error %d\r\n",
                vglite_status);
        }

        /* Clear the buffer with White */
        vglite_status = vg_lite_clear(renderTarget, NULL, BLACK_COLOR);
        if (VG_LITE_SUCCESS != vglite_status)
        {
            VG_LITE_ERROR_EXIT(
                "Clear failed: vg_lite_clear() returned error %d\r\n",
                vglite_status);
        }

        /***************************************************************/

        /* Define the transformation matrix that will be applied from the camera
         * image to the display */
        vg_lite_identity(&matrix);

        /* scale factor from the camera image to the display */
        float scale_Cam2Disp_x = (float)(DISPLAY_W) / (float)CAMERA_WIDTH;
        float scale_Cam2Disp_y = (float)DISPLAY_H / (float)CAMERA_HEIGHT;
        scale_Cam2Disp = max(scale_Cam2Disp_x, scale_Cam2Disp_y);
        vg_lite_scale(scale_Cam2Disp, scale_Cam2Disp, &matrix);

        // Move the scaled-frame to the display center
        float translate_x =
            ((DISPLAY_W) / scale_Cam2Disp - CAMERA_WIDTH) * 0.5f;
        float translate_y = (DISPLAY_H / scale_Cam2Disp - CAMERA_HEIGHT) * 0.5f;
        vg_lite_translate(translate_x, translate_y, &matrix);

        // Move the scaled-frame to the display center
        display_offset_x = ((DISPLAY_W)-scale_Cam2Disp * IMAGE_WIDTH) / 2;
        display_offset_y = (DISPLAY_H - scale_Cam2Disp * CAMERA_HEIGHT) / 2;

        /* Delay for USB enumeration to complete before rendering
         * data to the display
         */
        vTaskDelay(pdMS_TO_TICKS(1500));

        home_btn_x_pos = ifx_lcd_get_Display_Width() - HOME_BUTTON_WIDTH - 40;
        home_btn_y_pos = 10;
        // Initialize buffer states - clear any startup artifacts
        printf("[LCD_TASK] Clearing all buffers at startup");
        for (int i = 0; i < NUM_IMAGE_BUFFERS; i++)
        {
            image_buff[i].BufReady = 0;
            image_buff[i].NumBytes = 0;
        }
        __DMB();

        init_buffer_system();
        
        for (;;)
        {
        
            // static volatile float time_prev;
            if (!_device_connected)
            {
                int i = 0;
                while (i < 2)
                {
                    i++;
                    __DMB();

                    vg_lite_finish();

                    Cy_GFXSS_Set_FrameBuffer(
                        base, (uint32_t *)renderTarget->address, &gfx_context);

                    __DMB();

                    /* Clear the buffer with White */
                    vglite_status =
                        vg_lite_clear(renderTarget, NULL, BLACK_COLOR);
                    if (VG_LITE_SUCCESS != vglite_status)
                    {
                        VG_LITE_ERROR_EXIT("Clear failed: vg_lite_clear() "
                                           "returned error %d\r\n",
                                           vglite_status);
                    }

                    __DMB();

                    /***************************************************************/
                    if (splash_audio_stop) 
                    {
                    g_gameEvent = EVENT_PSOC_EDGE_STOP;
                    splash_audio_stop = false;
                    xTaskNotifyGive(voice_thread);
                    splash_audio_start = true;
                   }
                    splash_show = false;
                    game_start = true;
                    ifx_lcd_display_Rect(
                        NO_CAMERA_IMG_X_POS, NO_CAMERA_IMG_Y_POS,
                        (uint8_t *)no_camera_img_map, NO_CAMERA_IMG_WIDTH,
                        NO_CAMERA_IMG_HEIGHT);

                    ifx_lcd_display_Rect(home_btn_x_pos, home_btn_y_pos,
                                         (uint8_t *)home_btn_img_map,
                                         HOME_BUTTON_WIDTH, HOME_BUTTON_HEIGHT);

                    //          /* Sets Video/Graphics layer buffer address and
                    //        transfer the frame buffer to DC */
                    Cy_GFXSS_Set_FrameBuffer(
                        base, (uint32_t *)renderTarget->address, &gfx_context);

                    __DMB();
                }
            } 
            else
            {
                if (camera_not_supported)
                {
                    int i = 0;
                    while (i < 2)
                    {
                        i++;
                        __DMB();

                        vg_lite_finish();
                        Cy_GFXSS_Set_FrameBuffer(
                            base, (uint32_t *)renderTarget->address,
                            &gfx_context);

                        __DMB();

                        /* Clear the buffer with White */
                        vglite_status =
                            vg_lite_clear(renderTarget, NULL, BLACK_COLOR);
                        if (VG_LITE_SUCCESS != vglite_status)
                        {
                            VG_LITE_ERROR_EXIT("Clear failed: vg_lite_clear() "
                                               "returned error %d\r\n",
                                               vglite_status);
                        }

                        __DMB();

                        /***************************************************************/
                        splash_show = false;
                        game_start = true;

                        ifx_lcd_display_Rect(
                            CAMERA_NOT_SUPPORTED_IMG_X_POS,
                            CAMERA_NOT_SUPPORTED_IMG_Y_POS,
                            (uint8_t *)camera_not_supported_img_map,
                            CAMERA_NOT_SUPPORTED_IMG_WIDTH,
                            CAMERA_NOT_SUPPORTED_IMG_HEIGHT);

                        ifx_lcd_display_Rect(home_btn_x_pos, home_btn_y_pos,
                                             (uint8_t *)home_btn_img_map,
                                             HOME_BUTTON_WIDTH,
                                             HOME_BUTTON_HEIGHT);

                        //          /* Sets Video/Graphics layer buffer address
                        //        and transfer the frame buffer to DC */
                        Cy_GFXSS_Set_FrameBuffer(
                            base, (uint32_t *)renderTarget->address,
                            &gfx_context);

                        __DMB();
                    }

                    while (_device_connected)
                        ;
                }
            }
            BaseType_t result = xSemaphoreTake(model_semaphore, portMAX_DELAY);
            if (result == pdPASS)
            {

                if (_device_connected)
                {
                    if (!(player1 || player2 || computer))
                    {
                        splash_show = true;
                    }

                    TickType_t now_tick = xTaskGetTickCount();
                    /* Header_Image */
                    ifx_lcd_display_Rect(HEADER_X_POS, HEADER_Y_POS,
                                         (uint8_t *)Header_map, HEADER_WIDTH,
                                         HEADER_HEIGHT);

                    /* RPS_Demo state management */
                    draw_screen_elements(countdown_state);

                    if (trigger == true)
                    {

                        if (first_time)
                        {
                            first_time = false;
                        } 
                        else
                        {
                            round_game++;
                        }

                        xTimerChangePeriod(countdown_timer, 1, 100);
                        trigger = false;
                        start_countdown();
                    }
                    /*draw round indicator and scoreboard stars*/
                    draw_round_and_scoreboard();
                    update_box_data(renderTarget, &Prediction);

#ifdef IMAGE_CAPTURE
                    static int32_t count = 0;
                    count++;
                    if (count > 10)
                    {
                        send_image_uart();
                    }
#endif
#ifdef POSTER_VIEW
                    if (splash_show)
                    {
                        memcpy((void *)renderTarget->address, Artboard_map,
                               832 * 480 * 2);
                        ifx_lcd_display_Rect_new(home_btn_x_pos, home_btn_y_pos,
                                         (uint8_t *)home_icon_map, HOME_BUTTON_WIDTH_GAME,
                                         HOME_BUTTON_HEIGHT_GAME);
                        if (splash_audio_start)
                        {
                            g_gameEvent = EVENT_PSOC_EDGE_START;
                            splash_audio_start = false;
                            xTaskNotifyGive(voice_thread);
                            splash_audio_stop = true;
                        }
                    }
#endif
                    VG_switch_frame();
                    frame_count++;

                    if ((now_tick - last_report_tick) >=
                        pdMS_TO_TICKS(FPS_REPORT_INTERVAL_MS))
                    {
                        
#ifdef DEBUG_PRINT
                        printf("[LCD Display] %5.2f FPS (avg over %u ms)\r\n",
                               fps,
                               (unsigned int)(now_tick - last_report_tick));
#endif
                        frame_count = 0;
                        last_report_tick = now_tick;
                    }
                }
            }
        }
    }
}
