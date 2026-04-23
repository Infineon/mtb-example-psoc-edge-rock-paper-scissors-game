/******************************************************************************
* File Name : i2s_playback.c
*
* Description : Source file for Audio Playback via I2S.
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

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdio.h>
#include "projdefs.h"
#include "app_i2s.h"
#include "lcd_task.h"
#include "music_file_array/ai_bot.h"
#include "music_file_array/double_player.h"
#include "music_file_array/game_draw.h"
#include "music_file_array/player_a_win_the_game.h"
#include "music_file_array/player_b_win_the_game.h"
#include "music_file_array/psoc_edge.h"
#include "music_file_array/ready_steady_go.h"
#include "music_file_array/robot_win_the_game.h"
#include "music_file_array/single_player.h"
#include "retarget_io_init.h"

#define AUDIO_TASK_DELAY_500_MS               (500U)
#define AUDIO_TASK_DELAY_1000_MS              (1000U)
/*******************************************************************************
* Global Variables
*******************************************************************************/
TaskHandle_t voice_thread;

uint16_t *wave_data;
mtb_hal_i2c_t MW_I2C_hal_obj;
cy_stc_scb_i2c_context_t disp_i2c_controller_context;
uint32_t i2s_txcount = 0;
bool audio_playback_ended = false;
uint16_t zeros_data[HW_FIFO_HALF_SIZE/2] = {0};
volatile GameEvent_t g_gameEvent = EVENT_NONE;
volatile bool i2s_flag = false;
WavePlayCounter wave_play_counter;

mtb_hal_i2c_cfg_t i2c_config = 
{
    .is_target = false,
    .address = I2C_ADDRESS,
    .frequency_hz = I2C_FREQUENCY_HZ,
    .address_mask = MTB_HAL_I2C_DEFAULT_ADDR_MASK,
    .enable_address_callback = false
};

const cy_stc_sysint_t i2s_isr_txcfg =
{ 
    .intrSrc = (IRQn_Type) tdm_0_interrupts_tx_0_IRQn,
    .intrPriority = I2S_ISR_PRIORITY,
};
/******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
 * Function Name: app_i2s_init
 ********************************************************************************
* Summary: Initialize I2S interrupt and I2S
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_i2s_init(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Initialize the I2S interrupt */
    result = Cy_SysInt_Init(&i2s_isr_txcfg, i2s_tx_interrupt_handler);

    if(CY_RSLT_SUCCESS != result)
    {
        handle_app_error();
    }

    NVIC_EnableIRQ(i2s_isr_txcfg.intrSrc);

   /* Initialize the I2S */
    cy_en_tdm_status_t volatile return_status = Cy_AudioTDM_Init(TDM_STRUCT0, &CYBSP_TDM_CONTROLLER_0_config);
    
    if (CY_TDM_SUCCESS != return_status)
    {
        printf(">>> Error: I2S init failed with error code: 0x%0lX\n", (unsigned long)return_status);
        handle_app_error();
    }
}

/*******************************************************************************
 * Function Name: i2s_tx_interrupt_handler
 ********************************************************************************
* Summary: I2S transmit interrupt handler function.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void i2s_tx_interrupt_handler(void)
{
    /* Get interrupt status and check for tigger interrupt and errors */
    uint32_t intr_status = Cy_AudioTDM_GetTxInterruptStatusMasked(TDM_STRUCT0_TX);

    if(CY_TDM_INTR_TX_FIFO_TRIGGER & intr_status)
    {
        for(int i=0; i < HW_FIFO_HALF_SIZE/2; i++)
        {
            /* Write same data for Left and Right channels in FIFO */
            Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) (wave_data[i2s_txcount]));
            Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) (wave_data[i2s_txcount++]));

            /* If the end of the wave data is reached, reset i2s_txcount and set end of playback */
            switch(wave_play_counter)
            {
                case SINGLE_PLAYER:
                    if (i2s_txcount >= single_player_size/2)
                    {
                        i2s_txcount = 0;
                        /* End of Playback */
                        audio_playback_ended = true;   
                        app_i2s_deactivate();
                    }
                    break;
                case DOUBLE_PLAYER:
                    if (i2s_txcount >= double_player_size/2)
                    {
                        i2s_txcount = 0;
                        /* End of Playback */
                        audio_playback_ended = true;   
                        app_i2s_deactivate();
                    }
                    break;
                case AI_BOT:
                    if (i2s_txcount >= ai_bot_size/2)
                    {
                        i2s_txcount = 0;
                        /* End of Playback */
                        audio_playback_ended = true;   
                        app_i2s_deactivate();
                    }
                    break;
                case READY_STEADY_GO:
                    if (i2s_txcount >= ready_steady_go_size/2)
                    {
                        i2s_txcount = 0;
                        /* End of Playback */
                        audio_playback_ended = true;   
                        app_i2s_deactivate();
                    }
                    break;
                case PLAYER_A_WIN_THE_GAME:
                    if (i2s_txcount >= player_a_win_the_game_size/2)
                    {
                        i2s_txcount = 0;
                        /* End of Playback */
                        audio_playback_ended = true;   
                        app_i2s_deactivate();
                    }
                    break;
                case PLAYER_B_WIN_THE_GAME:
                    if (i2s_txcount >= player_b_win_the_game_size/2)
                    {
                        i2s_txcount = 0;
                        /* End of Playback */
                        audio_playback_ended = true; 
                        app_i2s_deactivate();  
                    }
                    break;
                case ROBOT_WIN_THE_GAME:
                    if (i2s_txcount >= robot_win_the_game_size/2)
                    {
                        i2s_txcount = 0;
                        /* End of Playback */
                        audio_playback_ended = true;   
                        app_i2s_deactivate();
                    }
                    break;
                case GAME_DRAW:
                    if (i2s_txcount >= game_draw_size/2)
                    {
                        i2s_txcount = 0;
                        /* End of Playback */
                        audio_playback_ended = true;   
                        app_i2s_deactivate();
                    }
                    break;
                case PSOC_EDGE_START:
                    if (i2s_txcount >= psoc_edge_size/2)
                    {
                        app_i2s_deactivate();
                        i2s_txcount = 0;
                        wave_play_counter = PSOC_EDGE_START;
                        wave_data = (uint16_t*)&psoc_edge[0];
                        app_i2s_activate();
                    }
                    break;     

                case PSOC_EDGE_END:
                    if (i2s_txcount >= psoc_edge_size/2)
                    {
                        i2s_txcount = 0;
                        wave_play_counter = PSOC_EDGE_END;
                        app_i2s_deactivate();
                    }
                    break;

                default:
                   
                    break;
            }

          
        }
    }
    else if(CY_TDM_INTR_TX_FIFO_UNDERFLOW & intr_status)
    {
        handle_app_error();
    }

    /* Clear all Tx I2S Interrupt */
    Cy_AudioTDM_ClearTxInterrupt(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);
}

/*******************************************************************************
 * Function Name: app_tlv_codec_init
 ********************************************************************************
* Summary: Initializes the I2C and TLV codec.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_tlv_codec_init(void)
{

    /* Initialize I2C used to configure TLV codec */
    tlv_codec_i2c_init();

    /* TLV codec (TLV320DAC3100) library */
    mtb_tlv320dac3100_init(&MW_I2C_hal_obj);

    /* Configure internal clock dividers to achieve desired sample rate */
    mtb_tlv320dac3100_configure_clocking(MCLK_HZ, SAMPLE_RATE_HZ, I2S_WORD_LENGTH, 
                                         TLV320DAC3100_SPK_AUDIO_OUTPUT);
    
   /* Adjust speaker output volume */
   mtb_tlv320dac3100_adjust_speaker_output_volume(DEFAULT_VOLUME);
    
   /* Activate TLV codec (TLV320DAC3100) */
    mtb_tlv320dac3100_activate();
}

/*******************************************************************************
 * Function Name: app_i2s_activate
 ********************************************************************************
* Summary: Activate I2S Tx interrupt
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_i2s_activate(void)
{
    /* Activate and enable I2S TX interrupts */
    Cy_AudioI2S_DisableTx(TDM_STRUCT0_TX);
    Cy_AudioI2S_EnableTx(TDM_STRUCT0_TX);
    Cy_AudioTDM_ActivateTx(TDM_STRUCT0_TX);
    Cy_AudioI2S_EnableTx(TDM_STRUCT0_TX);
}

/******************************************************************************
* Function Name: app_i2s_deactivate
*******************************************************************************
* Summary:
*   Disable the I2S interrupt and clear TX FIFO and disable the I2S
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void app_i2s_deactivate(void)
{
    /* There is no API to clear the HW FIFO. Disabling and enabling the TX
     * will clear the FIFO as a side effect.
     */
    Cy_AudioTDM_DeActivateTx(TDM_STRUCT0_TX);
    Cy_AudioI2S_DisableTx(TDM_STRUCT0_TX);
    Cy_AudioI2S_EnableTx(TDM_STRUCT0_TX);
    Cy_AudioTDM_DeActivateTx(TDM_STRUCT0_TX);
    Cy_AudioI2S_DisableTx(TDM_STRUCT0_TX);
}

/*******************************************************************************
* Function Name: app_i2s_enable
********************************************************************************
* Summary:
*   Enable I2S TX, its interrupts and fill the FIFO with zeros to start TX.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_i2s_enable(void)
{
    /* Enable the I2S TX interface */
    Cy_AudioTDM_EnableTx(TDM_STRUCT0_TX);

    /* Clear TX interrupts */
    Cy_AudioTDM_ClearTxInterrupt(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);
    Cy_AudioTDM_SetTxInterruptMask(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);

    /* Fill TX FIFO before it is activated with Zeros */
    for(int i=0; i < HW_FIFO_HALF_SIZE/2; i++)
    {
        /* Write data in FIFO */
        Cy_AudioTDM_WriteTxData(TDM0_TDM_STRUCT0_TDM_TX_STRUCT, (uint32_t) (zeros_data[i]));
        Cy_AudioTDM_WriteTxData(TDM0_TDM_STRUCT0_TDM_TX_STRUCT, (uint32_t) (zeros_data[i]));
    }
}

/*******************************************************************************
* Function Name: tlv_codec_i2c_init
********************************************************************************
* Summary:
*   Initilaize the I2C for the TLV codec.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void tlv_codec_i2c_init(void)
{
    cy_en_scb_i2c_status_t result;
    cy_rslt_t hal_result;

    /* Initialize and enable the I2C in controller mode. */
    result = Cy_SCB_I2C_Init(CYBSP_I2C_CONTROLLER_HW, &CYBSP_I2C_CONTROLLER_config, &disp_i2c_controller_context);
    if (CY_RSLT_SUCCESS != result)
    {
        printf(">>> Error: I2C initilization failed with error code: 0x%0lX\n", (unsigned long)result);
        handle_app_error();
    }

    /* Enable I2C hardware. */
    Cy_SCB_I2C_Enable(CYBSP_I2C_CONTROLLER_HW);


    /* I2C HAL init */
    hal_result = mtb_hal_i2c_setup(&MW_I2C_hal_obj, &CYBSP_I2C_CONTROLLER_hal_config, &disp_i2c_controller_context, NULL);
    if (CY_RSLT_SUCCESS != hal_result)
    {
        printf(">>> Error: I2C HAL setup failed with error code: 0x%0lX\n", (unsigned long)hal_result);
        handle_app_error();
    }

    /* Configure the I2C block. Master/Slave specific functions only work when the block is configured to desired mode */
    hal_result = mtb_hal_i2c_configure(&MW_I2C_hal_obj, &i2c_config);
    if (CY_RSLT_SUCCESS != hal_result)
    {
        printf(">>> Error: I2C HAL configure failed with error code: 0x%0lX\n", (unsigned long)hal_result);
        handle_app_error();
    }
}

/******************************************************************************
* Function Name: tlv_codec_i2c_deinit
*******************************************************************************
* Summary:
*   De-initilaize the I2C used by the TLV codec.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void tlv_codec_i2c_deinit(void)
{
    /* Disable and deinitialize the I2C resource */
    Cy_SCB_I2C_Disable(CYBSP_I2C_CONTROLLER_HW, &disp_i2c_controller_context);
    Cy_SCB_I2C_DeInit(CYBSP_I2C_CONTROLLER_HW);
}

/******************************************************************************
* Function Name: app_i2s_disable
*******************************************************************************
* Summary:
*   Disable I2S TX and interrupts.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void app_i2s_disable(void)
{
    /* Enable the I2S TX interface */
    Cy_AudioTDM_DisableTx(TDM_STRUCT0_TX);

    /* Clear TX interrupts */
    Cy_AudioTDM_ClearTxInterrupt(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);
    Cy_AudioTDM_ClearTxTriggerInterruptMask(TDM_STRUCT0_TX);
}

/*******************************************************************************
* Function Name: cm55_audio_task
********************************************************************************
* Summary:
*  FreeRTOS task that handles audio play back for the game
*
* Parameters:
*  arg - Pointer to task parameters (unused in this implementation).
*
* Return:
*  None
*
*******************************************************************************/

void cm55_audio_task(void *arg)
{
    app_i2s_init();
    app_tlv_codec_init();
    app_i2s_enable();
    xTaskNotifyGive(gfx_thread);

    GameEvent_t event;
  
    for(;;)
    {   
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0 )
        {
            printf("notification received in audio task\r\n");
            event = g_gameEvent;       /* Read shared variable   */
            g_gameEvent = EVENT_NONE; /* Clear it after consuming */

            switch (event)
            {
                case EVENT_START:
                    printf("Start\n\r");
                    wave_data = (uint16_t*)&ready_steady_go[0];
                    wave_play_counter = READY_STEADY_GO;
                    app_i2s_activate();
                    break;

                case EVENT_SINGLE_PLAYER:
                    printf("Single Player\n\r");
                    app_i2s_deactivate();
                    vTaskDelay(pdMS_TO_TICKS(AUDIO_TASK_DELAY_500_MS));
                    wave_data = (uint16_t*)&single_player[0];
                    wave_play_counter = SINGLE_PLAYER;
                    app_i2s_activate();
                    break;

                case EVENT_DOUBLE_PLAYER:
                    printf("Double Player\n\r");
                    app_i2s_deactivate();
                    vTaskDelay(pdMS_TO_TICKS(AUDIO_TASK_DELAY_500_MS));
                    wave_data = (uint16_t*)&double_player[0];
                    wave_play_counter = DOUBLE_PLAYER;
                    app_i2s_activate();
                    break;

                case EVENT_AI_BOT:
                    printf("AI Bot\n\r");
                    app_i2s_deactivate();
                    vTaskDelay(pdMS_TO_TICKS(AUDIO_TASK_DELAY_500_MS));
                    wave_data = (uint16_t*)&ai_bot[0];
                    wave_play_counter = AI_BOT;
                    app_i2s_activate();
                    break;

                case EVENT_PLAYER_A_WIN:
                    printf("Player A Wins\n\r");
                    wave_play_counter = PLAYER_A_WIN_THE_GAME;
                    wave_data = (uint16_t*)&player_a_win_the_game[0];
                    app_i2s_activate();
                    break;

                case EVENT_PLAYER_B_WIN:
                    printf("Player B Wins\n\r");
                    wave_play_counter = PLAYER_B_WIN_THE_GAME;
                    wave_data = (uint16_t*)&player_b_win_the_game[0];   
                    app_i2s_activate();
                    break;

                case EVENT_COMPUTER_WIN:
                    printf("Computer Wins\n\r");
                    wave_play_counter = ROBOT_WIN_THE_GAME;
                    wave_data = (uint16_t*)&robot_win_the_game[0];
                    app_i2s_activate();
                    break;

                case EVENT_DRAW:
                    printf("Draw\n\r");
                    wave_play_counter = GAME_DRAW;
                    wave_data = (uint16_t*)&game_draw[0];
                    app_i2s_activate();
                    break;

                case EVENT_PSOC_EDGE_START:
                    printf("PSOC Edge START \n\r");
                    app_i2s_deactivate();
                    vTaskDelay(pdMS_TO_TICKS(AUDIO_TASK_DELAY_1000_MS));
                    wave_play_counter = PSOC_EDGE_START;
                    wave_data = (uint16_t*)&psoc_edge[0];
                    app_i2s_activate();
                    break;                 

                case EVENT_PSOC_EDGE_STOP:
                    printf("PSOC Edge STOP\n\r");
                    wave_play_counter = PSOC_EDGE_END;
                    app_i2s_deactivate();
                    break;                 

                default:
                    /* Unknown event */
                    printf("Unknown Event\n\r");
                    break;
            }
        }
    }
}

/* [] END OF FILE */
