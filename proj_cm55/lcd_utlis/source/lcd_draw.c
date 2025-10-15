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

/*****************************************************************************
 * Header Files
 *****************************************************************************/
#include <stdlib.h>
#include "cy_syslib.h"
#include "ifx_lcd_utils.h"
#include "ifx_image_utils.h"
#include "lcd_bsp.h"

/*******************************************************************************
* Global variables
*******************************************************************************/
static uint32_t    Display_Width = DISPLAY_W;
static uint32_t    Display_Height = DISPLAY_H;


/*******************************************************************************
* Function Name: ifx_lcd_set_Display_size
********************************************************************************
* Summary:
*  Sets the display width and height.
*
* Parameters:
*  uint32_t width: Width of the display
*  uint32_t height: Height of the display
*
* Return:
*  None
*
*******************************************************************************/
void ifx_lcd_set_Display_size( uint32_t width, uint32_t height )
{
    Display_Width  = width;
    Display_Height = height;
}

/*******************************************************************************
* Function Name: ifx_lcd_get_Display_Width
********************************************************************************
* Summary:
*  Gets the display width.
*
* Parameters:
*  None
*
* Return:
*  uint32_t: Display width
*
*******************************************************************************/
inline
uint32_t ifx_lcd_get_Display_Width()
{
    return Display_Width;
}

/*******************************************************************************
* Function Name: ifx_lcd_get_Display_Height
********************************************************************************
* Summary:
*  Gets the display height.
*
* Parameters:
*  None
*
* Return:
*  uint32_t: Display height
*
*******************************************************************************/
inline
uint32_t ifx_lcd_get_Display_Height()
{
    return Display_Height;
}

/*******************************************************************************
* Function Name: ifx_lcd_set_FGcolor
********************************************************************************
* Summary:
*  Sets the foreground color.
*
* Parameters:
*  uint8_t r: Red color value (0-255)
*  uint8_t g: Green color value (0-255)
*  uint8_t b: Blue color value (0-255)
*
* Return:
*  uint32_t: Result of setting the foreground color
*
*******************************************************************************/
inline
uint32_t ifx_lcd_set_FGcolor( uint8_t r, uint8_t g, uint8_t b )
{
    return bsp_lcd_set_FGcolor( r, g, b );
}

/*******************************************************************************
* Function Name: ifx_lcd_set_BGcolor
********************************************************************************
* Summary:
*  Sets the background color.
*
* Parameters:
*  uint8_t r: Red color value (0-255)
*  uint8_t g: Green color value (0-255)
*  uint8_t b: Blue color value (0-255)
*
* Return:
*  uint32_t: Result of setting the background color
*
*******************************************************************************/
inline
uint32_t ifx_lcd_set_BGcolor( uint8_t r, uint8_t g, uint8_t b )
{
    return bsp_lcd_set_BGcolor( r, g, b );
}

/*******************************************************************************
* Function Name: ifx_lcd_get_FGcolor
********************************************************************************
* Summary:
*  Gets the current foreground color.
*
* Parameters:
*  None
*
* Return:
*  uint32_t: Current foreground color
*
*******************************************************************************/
inline
uint32_t ifx_lcd_get_FGcolor()
{
    return bsp_lcd_get_FGcolor();
}

/*******************************************************************************
* Function Name: ifx_lcd_get_BGcolor
********************************************************************************
* Summary:
*  Gets the current background color.
*
* Parameters:
*  None
*
* Return:
*  uint32_t: Current background color
*
*******************************************************************************/
inline
uint32_t ifx_lcd_get_BGcolor()
{
    return bsp_lcd_get_BGcolor();
}
 
/*******************************************************************************
* Function Name: ifx_lcd_draw_Pixel
********************************************************************************
* Summary:
*  Draws a pixel with the specified RGB color at the defined point.
*
* Parameters:
*  uint16_t x: Drawing position on X axis
*  uint16_t y: Drawing position on Y axis
*  uint32_t color: Foreground/background color
*
* Assumption:
*  User should ensure the location is valid on the LCD display.
*
* Return:
*  None
*
*******************************************************************************/
inline
void ifx_lcd_draw_Pixel( uint16_t x, uint16_t y, uint32_t color )
{
    bsp_lcd_draw_Pixel( x, y, color );
}


/*******************************************************************************
* Function Name: ifx_lcd_draw_H_Line
********************************************************************************
* Summary:
*  Draws a horizontal line with RGB color.
*
* Parameters:
*  int16_t x0: Start position on X axis
*  int16_t y0: Start position on Y axis
*  int16_t x1: End position on X axis
*
* Return:
*  None
*
*******************************************************************************/
 CY_SECTION_ITCM_BEGIN
void ifx_lcd_draw_H_Line( int16_t x0, int16_t y0, int16_t x1 )
{
    int min_x = min(x0, x1);
    int max_x = max(x0, x1);

    // check if the whole line is out of the display
    if ( max_x < 0  ||  y0 < 0  ||  Display_Width <= min_x  ||  Display_Height <= y0 )
        return;

    uint32_t    fgColor = ifx_lcd_get_FGcolor();

    bsp_lcd_draw_H_Line( (uint16_t) max(min_x, 0), (uint16_t) y0, (uint16_t) min(max_x, Display_Width - 1), fgColor );
}
CY_SECTION_ITCM_END

/*******************************************************************************
* Function Name: ifx_lcd_draw_V_Line
********************************************************************************
* Summary:
*  Draws a vertical line with RGB color.
*
* Parameters:
*  int16_t x0: Start position on X axis
*  int16_t y0: Start position on Y axis
*  int16_t y1: End position on Y axis
*
* Return:
*  None
*
*******************************************************************************/
CY_SECTION_ITCM_BEGIN
void ifx_lcd_draw_V_Line( int16_t x0, int16_t y0, int16_t y1 )
{
    int min_y = min(y0, y1);
    int max_y = max(y0, y1);

    // check if the whole line is out of the display
    if ( x0 < 0  ||  max_y < 0  || Display_Width <= x0  ||  Display_Height <= min_y )
        return;

    uint32_t    fgColor = ifx_lcd_get_FGcolor();

    bsp_lcd_draw_V_Line( (uint16_t) x0, (uint16_t) max(min_y, 0), (uint16_t) min(max_y, Display_Height - 1), fgColor );
}
CY_SECTION_ITCM_END

/*******************************************************************************
* Function Name: ifx_lcd_draw_Line
********************************************************************************
* Summary:
*  Draws a line with RGB color from one point to another.
*
* Parameters:
*  int16_t x0: Start position on X axis
*  int16_t y0: Start position on Y axis
*  int16_t x1: End position on X axis
*  int16_t y1: End position on Y axis
*
* Return:
*  None
*
*******************************************************************************/
 CY_SECTION_ITCM_BEGIN
void ifx_lcd_draw_Line( int16_t x0, int16_t y0, int16_t x1, int16_t y1 )
{
    int min_x = min(x0, x1);
    int max_x = max(x0, x1);
    int min_y = min(y0, y1);
    int max_y = max(y0, y1);
    uint32_t    fgColor = ifx_lcd_get_FGcolor();
    int16_t     tmp;

    // check if the whole line is out of the display
    if ( max_x < 0  ||  max_y < 0  ||  Display_Width <= min_x  ||  Display_Height <= min_y )
        return;

    if ( x0 == x1 )
    {
        if ( y0 == y1 )
        {
            ifx_lcd_draw_Pixel( (uint16_t)x0, (uint16_t)y0, fgColor );
        } 
        else
        {
            // vertical line
            min_y = max(min_y, 0);
            max_y = min(max_y, Display_Height - 1);
            bsp_lcd_draw_V_Line( (uint16_t) x0, (uint16_t) min_y, (uint16_t) max_y, fgColor );
        }
        return;
    } 
    else if ( y0 == y1 )
    {
        // horizontal line
        min_x = max(min_x, 0);
        max_x = min(max_x, Display_Width - 1);
        bsp_lcd_draw_H_Line( (uint16_t) min_x, (uint16_t) y0, (uint16_t) max_x, fgColor );
        return;
    }

    if ( max_x - min_x >= max_y - min_y )
    {     // (line length in X) >= (line length in Y)
        if ( x0 > x1 )
        {
            swap( x0, x1, tmp );
            swap( y0, y1, tmp );
        }

        float   dx = x1 - x0;
        float   dy = y1 - y0;
        float   k  = dy / dx;
        float   fy = y0 + 0.5f;
        uint16_t    min_x = x0;
        uint16_t    max_x = min( x1, Display_Width - 1 );

        if ( x0 < 0 )
        {
            // move to the valid display area in X
            fy -= k * x0;
            min_x = 0;
        }
        for ( uint16_t ix = min_x; ix <= max_x; ix++, fy += k )
        {
            int16_t iy = fy;
            if ( 0 <= iy  &&  iy < Display_Height )
                ifx_lcd_draw_Pixel( ix, (uint16_t)iy, fgColor );
        }
    }
    else
    {
        if ( y0 > y1 )
        {
            swap( x0, x1, tmp );
            swap( y0, y1, tmp );
        }

        float   dx = x1 - x0;
        float   dy = y1 - y0;
        float   k  = dx / dy;
        float   fx = x0 + 0.5f;
        uint16_t    min_y = y0;
        uint16_t    max_y = min( y1, Display_Height - 1 );

        if ( y0 < 0 )
        {
            // move to the valid display area in Y
            fx -= k * y0;
            min_y = 0;
        }
        for ( uint16_t iy = min_y; iy <= max_y; iy++, fx += k )
        {
            int16_t ix = fx;
            if ( 0 <= ix  &&  ix < Display_Width )
                ifx_lcd_draw_Pixel( (uint16_t)ix, iy, fgColor );
        }
    }
}
CY_SECTION_ITCM_END


/*******************************************************************************
* Function Name: ifx_lcd_draw_Rect
********************************************************************************
* Summary:
*  Draws a rectangle with RGB color.
*
* Parameters:
*  int16_t x0: X value of one corner of the rectangle
*  int16_t y0: Y value of one corner of the rectangle
*  int16_t x1: X value of the opposite corner of the rectangle
*  int16_t y1: Y value of the opposite corner of the rectangle
*
* Return:
*  None
*
*******************************************************************************/
 CY_SECTION_ITCM_BEGIN
void ifx_lcd_draw_Rect( int16_t x0, int16_t y0, int16_t x1, int16_t y1 )
{
    int min_x = min(x0, x1);
    int max_x = max(x0, x1);
    int min_y = min(y0, y1);
    int max_y = max(y0, y1);

    // check if the whole line is out of the display
    if ( max_x < 0  ||  max_y < 0  ||  Display_Width <= min_x  ||  Display_Height <= min_y )
        return;

    uint32_t    fgColor = ifx_lcd_get_FGcolor();

    bsp_lcd_draw_H_Line( min_x, min_y, max_x, fgColor );
    bsp_lcd_draw_V_Line( min_x, min_y, max_y, fgColor );
    bsp_lcd_draw_H_Line( min_x, max_y, max_x, fgColor );
    bsp_lcd_draw_V_Line( max_x, min_y, max_y, fgColor );
}
CY_SECTION_ITCM_END

/*******************************************************************************
* Function Name: ifx_lcd_draw_Bitmap
********************************************************************************
* Summary:
*  Draws a character bitmap at the specified point in the LCD buffer.
*
* Parameters:
*  int16_t x: Drawing position on X axis
*  int16_t y: Drawing position on Y axis
*  const char *bitmap: Pointer to the bitmap data for console
*  uint16_t width: Width of the bitmap in bits
*  uint16_t height: Height of the bitmap in bits
*
* Return:
*  None
*
*******************************************************************************/
CY_SECTION_ITCM_BEGIN
void ifx_lcd_draw_Bitmap( int16_t x, int16_t y, const char *bitmap, uint16_t width, uint16_t height )
{
    if ( x < 0  ||  Display_Width  < x + width  ||
         y < 0  ||  Display_Height < y + height )
        return;

    uint32_t    fgColor = ifx_lcd_get_FGcolor();
    uint32_t    bgColor = ifx_lcd_get_BGcolor();
    const char  *pBitmap = bitmap;
    int         width_byte = width / 8;     // round down to BYTE unit

    for ( int j = 0; j < width_byte; j++, x += 8 )
    {
        for ( int pos = 0; pos < height; pos++ )
        {
            uint8_t byte = (uint8_t) *pBitmap++;

            for ( int t = 0; t < 8; t++ )
            {
                uint32_t    color = ( (byte << t) & 0x80u ) ? fgColor : bgColor;

                ifx_lcd_draw_Pixel( x + t, y + pos, color );
            }
        }
    }
    if ( width % 8 )
    {
        for ( int pos = 0; pos < height; pos++ )
        {
            uint8_t byte = (uint8_t) *pBitmap++;

            for ( int t = 0; t < width % 8; t++ )
            {
                uint32_t    color = ( (byte << t) & 0x80u ) ? fgColor : bgColor;

                ifx_lcd_draw_Pixel( x + t, y + pos, color );
            }
        }
    }
}
CY_SECTION_ITCM_END

/*******************************************************************************
* Function Name: ifx_lcd_display_Rect
********************************************************************************
* Summary:
*  Copies a rectangular area from a 24-bit RGB source image to the designated
*  location on the display.
*
* Parameters:
*  uint16_t x0: Starting point on X axis on the display
*  uint16_t y0: Starting point on Y axis on the display
*  uint8_t *image: Pointer to the 24-bit RGB source image
*  uint16_t width: Width of the source image
*  uint16_t height: Height of the source image
*
* Return:
*  None
*
*******************************************************************************/
inline
void ifx_lcd_display_Rect( uint16_t x0, uint16_t y0, uint8_t *image, uint16_t width, uint16_t height )
{
    bsp_lcd_display_Rect( x0, y0, image, width, height );
}

/*******************************************************************************
* Function Name: ifx_lcd_display_Rect_i8
********************************************************************************
* Summary:
*  Copies a rectangular area from an int8-normalized source image to the
*  designated location on the display.
*
* Parameters:
*  uint16_t x0: Starting point on X axis on the display
*  uint16_t y0: Starting point on Y axis on the display
*  int8_t *image_i8: Pointer to the int8-normalized source image
*  uint16_t width: Width of the source image
*  uint16_t height: Height of the source image
*
* Return:
*  None
*
*******************************************************************************/
inline
void ifx_lcd_display_Rect_i8( uint16_t x0, uint16_t y0, int8_t *image_i8, uint16_t width, uint16_t height )
{
    bsp_lcd_display_Rect_i8( x0, y0, image_i8, width, height );
}

/*******************************************************************************
* Function Name: ifx_lcd_display_Rect_scale_i8
********************************************************************************
* Summary:
*  Copies a rectangular area from an int8-normalized source image to the
*  designated location on the display with a specified integer scale (>= 1).
*
* Parameters:
*  uint16_t x0: Starting point on X axis on the display
*  uint16_t y0: Starting point on Y axis on the display
*  int8_t *image_i8: Pointer to the int8-normalized source image
*  uint16_t width: Width of the source image
*  uint16_t height: Height of the source image
*  uint16_t scale: Integer scale factor (>= 1)
*
* Return:
*  None
*
*******************************************************************************/
inline
void ifx_lcd_display_Rect_scale_i8( uint16_t x0, uint16_t y0, int8_t *image_i8, uint16_t width, uint16_t height, uint16_t scale )
{
    bsp_lcd_display_Rect_scale_i8( x0, y0, image_i8, width, height, scale );
}

/*******************************************************************************
* Function Name: ifx_lcd_display_Rect_i8
********************************************************************************
* Summary:
*  Copies a rectangular area from an source image to the
*  designated location on the display.
*
* Parameters:
*  uint16_t x0: Starting point on X axis on the display
*  uint16_t y0: Starting point on Y axis on the display
*  int8_t *image_i8: Pointer to the int8-normalized source image
*  uint16_t width: Width of the source image
*  uint16_t height: Height of the source image
*
* Return:
*  None
*
*******************************************************************************/
void ifx_lcd_display_Rect_new( uint16_t x0, uint16_t y0, uint8_t *image, uint16_t width, uint16_t height )
{
    bsp_lcd_display_Rect_new( x0, y0, image, width, height );
}
