/******************************************************************************
* File Name:   lcd_draw.c
*
* Description: This file implements the direct draw functions to the display frame buffer.
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
#include <stdlib.h>
#include "lcd_task.h"
#include "lcd_bsp.h"
#include "ifx_image_utils.h"
/*******************************************************************************
* Macros
*******************************************************************************/
#define LCD_Addr  ((LCD_TYPE_t *) renderTarget->memory)

/*******************************************************************************
* Local variables
*******************************************************************************/
uint32_t    LCD_Colors[2] = {
                                0X00000000,     // background: black
                                0X00FFFFFF      // foreground: white
                            };


/*******************************************************************************
* Function Name: bsp_lcd_set_FGcolor
********************************************************************************
* Summary:
*  Set foreground color to LCD
*
* Parameters:
*  uint16_t r: Red component
*  uint16_t g: Green component
*  uint16_t b: Blue component
*
* Return:
*  uint32_t color
*
*******************************************************************************/

 uint32_t bsp_lcd_set_FGcolor( uint16_t r, uint16_t g, uint16_t b )
 {
    LCD_Colors[1] =
    #ifndef LCD_COLOR_XRGB32
            ifx_pixel_RGB888_to_RGB565(r, g, b);
    #else
            ifx_pixel_RGB888_to_RGBX32(r, g, b);
    #endif  // LCD_COLOR_XRGB32

    return LCD_Colors[1];
 }

 /*******************************************************************************
 * Function Name: bsp_lcd_set_BGcolor
 ********************************************************************************
 * Summary:
 *  Set background color to LCD
 *
 * Parameters:
 *  uint16_t r: Red component
 *  uint16_t g: Green component
 *  uint16_t b: Blue component
 *
 * Return:
 *  uint32_t color
 *
 *******************************************************************************/
 uint32_t bsp_lcd_set_BGcolor( uint16_t r, uint16_t g, uint16_t b )
 {
    LCD_Colors[0] =
    #ifndef LCD_COLOR_XRGB32
            ifx_pixel_RGB888_to_RGB565(r, g, b);
    #else
            ifx_pixel_RGB888_to_RGBX32(r, g, b);
    #endif  // LCD_COLOR_XRGB32

    return LCD_Colors[0];
 }
 /*******************************************************************************
 * Function Name: bsp_lcd_get_FGcolor
 ********************************************************************************
 * Summary:
 *  Set Foreground color to LCD
 *
 * Parameters:
 * void
 *
 * Return:
 *  uint32_t color
 *
 *******************************************************************************/
 uint32_t bsp_lcd_get_FGcolor(void)
 {
    return LCD_Colors[1];
 }

 /*******************************************************************************
 * Function Name: bsp_lcd_set_BGcolor
 ********************************************************************************
 * Summary:
 *  Set foreground color to LCD
 *
 * Parameters:
 * void
 *
 * Return:
 *  uint32_t color
 *
 *******************************************************************************/
 uint32_t bsp_lcd_get_BGcolor()
 {
    return LCD_Colors[0];
 }

 /*******************************************************************************
 * Function Name: bsp_lcd_draw_Pixel_RGB565
 ********************************************************************************
 * Summary:
 *  Draws pixel with 16-bit RGB565 color to defined point on the LCD display.
 *
 * Parameters:
 * x           : drawing position on X axis
 * y           : drawing position on Y axis
 * rgbColor    : 16-bit RGB565 or 32-bit XRGB color value
 *
 * Return:
 *  None
 *
 *******************************************************************************/
static inline __attribute__((used)) void bsp_lcd_draw_Pixel_RGB565(uint16_t x, uint16_t y, uint32_t rgbColor)
{
    LCD_Addr[INDEX2D(y, x, DISPLAY_W)] = rgbColor;
}


CY_SECTION_ITCM_BEGIN
/*******************************************************************************
* Function Name: bsp_lcd_draw_Pixel
********************************************************************************
* Summary:
*  Draws pixel with RGB color to defined point on the LCD display.
*
* Parameters:
* x           : drawing position on X axis
* y           : drawing position on Y axis
* rgbColor    : 16-bit RGB565 or 32-bit XRGB color value
*
* Return:
*  None
*
*******************************************************************************/
void bsp_lcd_draw_Pixel( uint16_t x, uint16_t y, uint32_t rgbColor )
{
    LCD_Addr[INDEX2D(y, x, DISPLAY_W)] = rgbColor;
}
CY_SECTION_ITCM_END

CY_SECTION_ITCM_BEGIN
 /*******************************************************************************
 * Function Name: bsp_lcd_draw_H_Line
 ********************************************************************************
 * Summary:
 *  Draws a horizontal line with RGB color on the LCD display.
 *
 * Parameters:
 *  x0          : drawing start position on X axis
 *  y0          : drawing start position on Y axis
 *  x1          : drawing end position on X axis
 *  rgbColor    : 16-bit RGB565 or 32-bit XRGB color value
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void bsp_lcd_draw_H_Line( uint16_t x0, uint16_t y0, uint16_t x1, uint32_t rgbColor )
{
    uint    kk = INDEX2D(y0, x0, DISPLAY_W);

    for ( uint x = x0; x <= x1; x++ )
        LCD_Addr[kk++] = rgbColor;
}
CY_SECTION_ITCM_END

CY_SECTION_ITCM_BEGIN
 /*******************************************************************************
 * Function Name: bsp_lcd_draw_V_Line
 ********************************************************************************
 * Summary:
 *  Draws a vertical line with RGB color on the LCD display.
 *
 * Parameters:
 *  x0          : drawing start position on X axis
 *  y0          : drawing start position on Y axis
 *  x1          : drawing end position on Y axis
 *  rgbColor    : 16-bit RGB565 or 32-bit XRGB color value
 *
 * Return:
 *  None
 *
 *******************************************************************************/
void bsp_lcd_draw_V_Line( uint16_t x0, uint16_t y0, uint16_t y1, uint32_t rgbColor )
{
    uint    kk = INDEX2D(y0, x0, DISPLAY_W);

    for ( uint y = y0; y <= y1; y++, kk += DISPLAY_W )
        LCD_Addr[kk] = rgbColor;
}
CY_SECTION_ITCM_END

/*******************************************************************************
* Function Name: bsp_lcd_display_Rect_new
********************************************************************************
* Summary:
*  Copy a rectangular area and remove pure RGB black pixel from int8-normalized
*  24-bit RGB source image to the designated location on the display.
*
* Parameters:
*  x0          : starting point of line on X axis on the display
*  y0          : starting point of line on Y axis on the display
*  image        : pointer to the int8-normalized source image
*  width       : width of the source image
*  height      : height of the source image
*
* Return:
*  None
*
*******************************************************************************/
void bsp_lcd_display_Rect_new( uint16_t x0, uint16_t y0, uint8_t *image, uint16_t width, uint16_t height )
{
     // check if the whole rectangle is outside the display
    if ( x0 >= DISPLAY_W  ||  y0 >= DISPLAY_H )
        return;

    uint32_t h = min(height, DISPLAY_H - y0);
    uint32_t w = min(width,  DISPLAY_W - x0);

    uint8_t *pSrc = image;
    LCD_TYPE_t *pDst = &LCD_Addr[INDEX2D(y0, x0, DISPLAY_W)];

    for ( uint32_t y = 0; y < h; y++ )
    {
        uint8_t *ps = pSrc;

        for ( uint32_t x = 0; x < w; x++ )
        {
            uint32_t r = (uint32_t)(*ps++);
            uint32_t g = (uint32_t)(*ps++);
            uint32_t b = (uint32_t)(*ps++);

            // Skip drawing pure black pixels (0,0,0) → transparency
            if (!(r == 0 && g == 0 && b == 0))
            {
                pDst[x] =
                #ifndef LCD_COLOR_XRGB32
                    (uint16_t)ifx_pixel_RGB888_to_RGB565(b, g, r);
                #else
                    (uint32_t)ifx_pixel_RGB888_to_RGBX32(r, g, b);
                #endif
            }
        }

        pDst += DISPLAY_W;     // move to next LCD row
        pSrc += width * 3;     // move to next image row (RGB888 → 3 bytes/pixel)
    }
}

/*******************************************************************************
* Function Name: bsp_lcd_display_Rect
********************************************************************************
* Summary:
*  Copy a rectangular area from 24-bit RGB source image to the designated
*  location on the display.
*
* Parameters:
*  x0          : starting point of line on X axis on the display
*  y0          : starting point of line on Y axis on the display
*  image        : pointer to the 24-bit RGB source image
*  width       : width of the source image
*  height      : height of the source image
*
* Return:
*  None
*
*******************************************************************************/
void bsp_lcd_display_Rect( uint16_t x0, uint16_t y0, uint8_t *image, uint16_t width, uint16_t height )
{
    // check if the whole line is out of the display
    if ( x0 >= DISPLAY_W  ||  y0 >= DISPLAY_H )
        return;

    uint32_t    h = min( height, DISPLAY_H - y0 );
    uint32_t    w = min( width,  DISPLAY_W - x0 );
    uint8_t     *pSrc = image;
    LCD_TYPE_t  *pDst = &LCD_Addr[INDEX2D(y0, x0, DISPLAY_W)];

    for ( uint32_t y = 0; y < h; y++ )
    {
        uint8_t *ps = pSrc;

        for ( uint32_t x = 0; x < w; x++ ) {
            uint32_t    r = (uint32_t)(*ps++);
            uint32_t    g = (uint32_t)(*ps++);
            uint32_t    b = (uint32_t)(*ps++);

            pDst[x] = 
            #ifndef LCD_COLOR_XRGB32
                    (uint16_t)ifx_pixel_RGB888_to_RGB565( b, g, r );
            #else
                    (uint32_t)ifx_pixel_RGB888_to_RGBX32( r, g, b );
            #endif  // LCD_COLOR_XRGB32
        }
        pDst += DISPLAY_W;
        pSrc += width * 3;
    }
}

/*******************************************************************************
* Function Name: bsp_lcd_display_Rect_i8
********************************************************************************
* Summary:
*  Copy a rectangular area from int8-normalized 24-bit RGB source image to the
*  designated location on the display.
*
* Parameters:
*  x0          : starting point of line on X axis on the display
*  y0          : starting point of line on Y axis on the display
*  image        : pointer to the int8-normalized source image
*  width       : width of the source image
*  height      : height of the source image
*
* Return:
*  None
*
*******************************************************************************/
void bsp_lcd_display_Rect_i8( uint16_t x0, uint16_t y0, int8_t *image_i8, uint16_t width, uint16_t height )
{
    // check if the whole line is out of the display
    if ( x0 >= DISPLAY_W  ||  y0 >= DISPLAY_H )
        return;

    uint32_t    h = min( height, DISPLAY_H - y0 );
    uint32_t    w = min( width,  DISPLAY_W - x0 );
    int8_t      *pSrc = image_i8;
    LCD_TYPE_t  *pDst = &LCD_Addr[INDEX2D(y0, x0, DISPLAY_W)];

    for ( uint32_t y = 0; y < h; y++ )
    {
        int8_t      *ps = pSrc;

        for ( uint32_t x = 0; x < w; x++ ) {
            uint32_t    r = (uint32_t)(*ps++ + 128);
            uint32_t    g = (uint32_t)(*ps++ + 128);
            uint32_t    b = (uint32_t)(*ps++ + 128);

            pDst[x] = 
            #ifndef LCD_COLOR_XRGB32
                    (uint16_t)ifx_pixel_RGB888_to_RGB565( r, g, b );
            #else
                    (uint32_t)ifx_pixel_RGB888_to_RGBX32( r, g, b );
            #endif  // LCD_COLOR_XRGB32
        }
        pDst += DISPLAY_W;
        pSrc += width * 3;
    }
}

/*******************************************************************************
* Function Name: bsp_lcd_display_Rect_i8
********************************************************************************
* Summary:
*  Copy a rectangular area from int8-normalized source image to the designated
*  location on the display with scale (>= 1).
*
* Parameters:
*  x0          : starting point of line on X axis on the display
*  y0          : starting point of line on Y axis on the display
*  image        : pointer to the int8-normalized source image
*  width       : width of the source image
*  height      : height of the source image
*  scale       : integer scale (>= 1)
*
* Return:
*  None
*
*******************************************************************************/
void bsp_lcd_display_Rect_scale_i8( uint16_t x0, uint16_t y0, int8_t *pSrc, uint16_t width, uint16_t height, uint16_t scale )
{
    uint32_t    h = min( height, (DISPLAY_H - y0) / scale );
    uint32_t    w = min( width,  (DISPLAY_W - x0) / scale );

    for ( uint32_t y = 0; y < h; y++ )
    {
        int8_t      *ps = &pSrc[y * width * 3];
        LCD_TYPE_t  *pd = &LCD_Addr[(y0 + y * scale) * DISPLAY_W + x0];

        for ( uint32_t x = 0; x < w; x++ )
        {
            uint32_t    r = (uint32_t)(*ps++ + 128);
            uint32_t    g = (uint32_t)(*ps++ + 128);
            uint32_t    b = (uint32_t)(*ps++ + 128);
            uint32_t    color =            
            #ifndef LCD_COLOR_XRGB32
                ifx_pixel_RGB888_to_RGB565( r, g, b );
            #else
                ifx_pixel_RGB888_to_RGBX32( r, g, b );
            #endif  // LCD_COLOR_XRGB32

            for ( int ky = 0; ky < scale; ky++ )
                for ( int kx = 0; kx < scale; kx++ )
                    pd[INDEX2D(ky, kx, DISPLAY_W)] = color;

            pd += scale;
        }
    }
}
