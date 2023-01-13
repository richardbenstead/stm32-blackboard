#ifndef ILI9341_H_
#define ILI9341_H_

#include "main.h"
#include "colors.h"
#include "registers.h"
//#include "fonts.h"
#include "image.h"
#include <stdbool.h>

#define LCD_BL_ON()  HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET)
#define LCD_BL_OFF() HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET)

#define LCD_BASE0        		((uint32_t)0x60000000)
#define LCD_BASE1        		((uint32_t)0x60080000)

#define LCD_CmdWrite(command)	*(volatile uint16_t *) (LCD_BASE0) = (command)
#define LCD_DataWrite(data)		*(volatile uint16_t *) (LCD_BASE1) = (data)
#define	LCD_StatusRead()		*(volatile uint16_t *) (LCD_BASE0) //if use read  Mcu interface DB0~DB15 needs increase pull high
#define	LCD_DataRead()			*(volatile uint16_t *) (LCD_BASE1) //if use read  Mcu interface DB0~DB15 needs increase pull high

#define ILI9341_PHY_PIXEL_WIDTH		320
#define ILI9341_PHY_PIXEL_HEIGHT 	240

#define ILI9341_FB_PIXEL_WIDTH		160
#define ILI9341_FB_PIXEL_HEIGHT 	240
#define ILI9341_PIXEL_COUNT		ILI9341_FB_PIXEL_WIDTH * ILI9341_FB_PIXEL_HEIGHT

// Any LCD needs to implement these common methods, which allow the low-level
// Initialization and pixel-setting details to be abstracted away from the
// higher level drawing and graphics code.

typedef enum {
	LCD_ORIENTATION_PORTRAIT = 0,
	LCD_ORIENTATION_LANDSCAPE = 1,
	LCD_ORIENTATION_PORTRAIT_MIRROR = 2,
	LCD_ORIENTATION_LANDSCAPE_MIRROR = 3
} lcdOrientationTypeDef;

/**
 * @brief  Draw Properties structures definition
 */
typedef struct {
	uint32_t TextColor;
	uint32_t BackColor;
	//sFONT *pFont;
	uint8_t TextWrap;
} lcdFontPropTypeDef;

typedef struct {
	unsigned short x;
	unsigned short y;
} lcdCursorPosTypeDef;

// This struct is used to indicate the capabilities of different LCDs
typedef struct {
	uint16_t width;         // LCD width in pixels (default orientation)
	uint16_t height;        // LCD height in pixels (default orientation)
	lcdOrientationTypeDef orientation; // Whether the LCD orientation can be modified
	bool touchscreen;   // Whether the LCD has a touch screen
	bool hwscrolling;   // Whether the LCD support HW scrolling
} lcdPropertiesTypeDef;

void lcdFillRGB(uint16_t color);
void lcdInit(void);
void lcdHome(void);
void lcdSetOrientation(lcdOrientationTypeDef orientation);
void lcdSetCursor(unsigned short x, unsigned short y);
void lcdSetWindow(unsigned short x0, unsigned short y0, unsigned short x1,
		unsigned short y1);
void lcdBacklightOff(void);
void lcdBacklightOn(void);
void lcdInversionOff(void);
void lcdInversionOn(void);
void lcdDisplayOff(void);
void lcdDisplayOn(void);
void lcdTearingOff(void);
void lcdTearingOn(bool m);
uint16_t lcdGetWidth(void);
uint16_t lcdGetHeight(void);
uint16_t lcdGetControllerID(void);
lcdOrientationTypeDef lcdGetOrientation(void);
lcdPropertiesTypeDef lcdGetProperties(void);
uint16_t lcdReadPixel(uint16_t x, uint16_t y);

#endif /* ILI9341_H_ */

