#include "ili9341.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

enum {
	MemoryAccessControlNormalOrder, MemoryAccessControlReverseOrder
} MemoryAccessControlRefreshOrder;

enum {
	MemoryAccessControlColorOrderRGB, MemoryAccessControlColorOrderBGR
} MemoryAccessControlColorOrder;

static lcdPropertiesTypeDef lcdProperties = { ILI9341_FB_PIXEL_WIDTH,
ILI9341_FB_PIXEL_HEIGHT, LCD_ORIENTATION_PORTRAIT, true, true };
//static lcdFontPropTypeDef lcdFont = { COLOR_YELLOW, COLOR_BLACK, &Font12, 1 };
static lcdCursorPosTypeDef cursorXY = { 0, 0 };

static unsigned char lcdPortraitConfig = 0;
static unsigned char lcdLandscapeConfig = 0;
static unsigned char lcdPortraitMirrorConfig = 0;
static unsigned char lcdLandscapeMirrorConfig = 0;

static void lcdReset(void);
static void lcdWriteCommand(unsigned char command);
static void lcdWriteData(unsigned short data);
static unsigned short lcdReadData(void);

static unsigned char lcdBuildMemoryAccessControlConfig(
bool rowAddressOrder,
bool columnAddressOrder,
bool rowColumnExchange,
bool verticalRefreshOrder,
bool colorOrder,
bool horizontalRefreshOrder);

void lcdFillRGB(uint16_t color) {
	lcdSetWindow(0, 0, ILI9341_PHY_PIXEL_WIDTH - 1,
			ILI9341_PHY_PIXEL_HEIGHT - 1);
	int dimensions = ILI9341_PHY_PIXEL_WIDTH * ILI9341_PHY_PIXEL_HEIGHT;
	while (dimensions--) {
		lcdWriteData(color);
	}
}

void lcdInit(void) {
	lcdPortraitConfig = lcdBuildMemoryAccessControlConfig(
			MemoryAccessControlNormalOrder,		// rowAddressOrder
			MemoryAccessControlReverseOrder,	// columnAddressOrder
			MemoryAccessControlNormalOrder,		// rowColumnExchange
			MemoryAccessControlNormalOrder,		// verticalRefreshOrder
			MemoryAccessControlColorOrderBGR,	// colorOrder
			MemoryAccessControlNormalOrder);	// horizontalRefreshOrder

	lcdLandscapeConfig = lcdBuildMemoryAccessControlConfig(
			MemoryAccessControlNormalOrder,		// rowAddressOrder
			MemoryAccessControlNormalOrder,		// columnAddressOrder
			MemoryAccessControlReverseOrder,	// rowColumnExchange
			MemoryAccessControlNormalOrder,		// verticalRefreshOrder
			MemoryAccessControlColorOrderBGR,	// colorOrder
			MemoryAccessControlNormalOrder);	// horizontalRefreshOrder

	lcdPortraitMirrorConfig = lcdBuildMemoryAccessControlConfig(
			MemoryAccessControlReverseOrder,	// rowAddressOrder
			MemoryAccessControlNormalOrder,		// columnAddressOrder
			MemoryAccessControlNormalOrder,		// rowColumnExchange
			MemoryAccessControlNormalOrder,		// verticalRefreshOrder
			MemoryAccessControlColorOrderBGR,	// colorOrder
			MemoryAccessControlNormalOrder);	// horizontalRefreshOrder

	lcdLandscapeMirrorConfig = lcdBuildMemoryAccessControlConfig(
			MemoryAccessControlReverseOrder,	// rowAddressOrder
			MemoryAccessControlReverseOrder,	// columnAddressOrder
			MemoryAccessControlReverseOrder,	// rowColumnExchange
			MemoryAccessControlNormalOrder,		// verticalRefreshOrder
			MemoryAccessControlColorOrderBGR,	// colorOrder
			MemoryAccessControlNormalOrder);	// horizontalRefreshOrder

	lcdReset();

	lcdWriteCommand(ILI9341_DISPLAYOFF);

	lcdWriteCommand(0xCF);
	lcdWriteData(0x00);
	lcdWriteData(0x83);
	lcdWriteData(0x30);

	lcdWriteCommand(0xED);
	lcdWriteData(0x64);
	lcdWriteData(0x03);
	lcdWriteData(0x12);
	lcdWriteData(0x81);

	lcdWriteCommand(0xE8);
	lcdWriteData(0x85);
	lcdWriteData(0x01);
	lcdWriteData(0x79);

	lcdWriteCommand(0xCB);
	lcdWriteData(0x39);
	lcdWriteData(0x2C);
	lcdWriteData(0x00);
	lcdWriteData(0x34);
	lcdWriteData(0x02);

	lcdWriteCommand(0xF7);
	lcdWriteData(0x20);

	lcdWriteCommand(0xEA);
	lcdWriteData(0x00);
	lcdWriteData(0x00);

	lcdWriteCommand(ILI9341_POWERCONTROL1);
	lcdWriteData(0x26);

	lcdWriteCommand(ILI9341_POWERCONTROL2);
	lcdWriteData(0x11);

	lcdWriteCommand(ILI9341_VCOMCONTROL1);
	lcdWriteData(0x35);
	lcdWriteData(0x3E);

	lcdWriteCommand(ILI9341_VCOMCONTROL2);
	lcdWriteData(0xBE);

	lcdWriteCommand(ILI9341_MEMCONTROL);
	lcdWriteData(lcdPortraitConfig);

	lcdWriteCommand(ILI9341_PIXELFORMAT);
	lcdWriteData(0x55);

	lcdWriteCommand(ILI9341_FRAMECONTROLNORMAL);
	lcdWriteData(0x00);
	lcdWriteData(0x1B);

	lcdWriteCommand(0xF2);
	lcdWriteData(0x08);

	lcdWriteCommand(ILI9341_GAMMASET);
	lcdWriteData(0x01);

	lcdWriteCommand(ILI9341_POSITIVEGAMMCORR);
	lcdWriteData(0x1F);
	lcdWriteData(0x1A);
	lcdWriteData(0x18);
	lcdWriteData(0x0A);
	lcdWriteData(0x0F);
	lcdWriteData(0x06);
	lcdWriteData(0x45);
	lcdWriteData(0x87);
	lcdWriteData(0x32);
	lcdWriteData(0x0A);
	lcdWriteData(0x07);
	lcdWriteData(0x02);
	lcdWriteData(0x07);
	lcdWriteData(0x05);
	lcdWriteData(0x00);

	lcdWriteCommand(ILI9341_NEGATIVEGAMMCORR);
	lcdWriteData(0x00);
	lcdWriteData(0x25);
	lcdWriteData(0x27);
	lcdWriteData(0x05);
	lcdWriteData(0x10);
	lcdWriteData(0x09);
	lcdWriteData(0x3A);
	lcdWriteData(0x78);
	lcdWriteData(0x4D);
	lcdWriteData(0x05);
	lcdWriteData(0x18);
	lcdWriteData(0x0D);
	lcdWriteData(0x38);
	lcdWriteData(0x3A);
	lcdWriteData(0x1F);

	lcdWriteCommand(ILI9341_COLADDRSET);
	lcdWriteData(0x00);
	lcdWriteData(0x00);
	lcdWriteData(0x00);
	lcdWriteData(0xEF);

	lcdWriteCommand(ILI9341_PAGEADDRSET);
	lcdWriteData(0x00);
	lcdWriteData(0x00);
	lcdWriteData(0x01);
	lcdWriteData(0x3F);

	lcdWriteCommand(ILI9341_ENTRYMODE);
	lcdWriteData(0x07);

	lcdWriteCommand(ILI9341_DISPLAYFUNC);
	lcdWriteData(0x0A);
	lcdWriteData(0x82);
	lcdWriteData(0x27);
	lcdWriteData(0x00);

	lcdWriteCommand(ILI9341_SLEEPOUT);
	HAL_Delay(10);
	lcdWriteCommand(ILI9341_DISPLAYON);
}


void lcdHome(void) {
	cursorXY.x = 0;
	cursorXY.y = 0;
	lcdSetWindow(0, 0, lcdProperties.width - 1, lcdProperties.height - 1);
}


void lcdSetOrientation(lcdOrientationTypeDef value) {
	lcdProperties.orientation = value;
	lcdWriteCommand(ILI9341_MEMCONTROL);

	switch (lcdProperties.orientation) {
	case LCD_ORIENTATION_PORTRAIT:
		lcdWriteData(lcdPortraitConfig);
		lcdProperties.width = ILI9341_FB_PIXEL_WIDTH;
		lcdProperties.height = ILI9341_FB_PIXEL_HEIGHT;
		break;
	case LCD_ORIENTATION_PORTRAIT_MIRROR:
		lcdWriteData(lcdPortraitMirrorConfig);
		lcdProperties.width = ILI9341_FB_PIXEL_WIDTH;
		lcdProperties.height = ILI9341_FB_PIXEL_HEIGHT;
		break;
	case LCD_ORIENTATION_LANDSCAPE:
		lcdWriteData(lcdLandscapeConfig);
		lcdProperties.width = ILI9341_FB_PIXEL_HEIGHT;
		lcdProperties.height = ILI9341_FB_PIXEL_WIDTH;
		break;
	case LCD_ORIENTATION_LANDSCAPE_MIRROR:
		lcdWriteData(lcdLandscapeMirrorConfig);
		lcdProperties.width = ILI9341_FB_PIXEL_HEIGHT;
		lcdProperties.height = ILI9341_FB_PIXEL_WIDTH;
		break;
	default:
		break;
	}

	//lcdWriteCommand(ILI9341_MEMORYWRITE);
	lcdSetWindow(0, 0, lcdProperties.width - 1, lcdProperties.height - 1);
}

void lcdSetCursor(unsigned short x, unsigned short y) {
	cursorXY.x = x;
	cursorXY.y = y;
	lcdSetWindow(x, y, x, y);
}

/**
 * \brief Sets window address
 *
 * \param x0         Left top window x-coordinate
 * \param y0         Left top window y-coordinate
 * \param x1         Right bottom window x-coordinate
 * \param y1         Right bottom window y-coordinate
 *
 * \return void
 */
void lcdSetWindow(unsigned short x0, unsigned short y0, unsigned short x1,
		unsigned short y1) {
	lcdWriteCommand(ILI9341_COLADDRSET);
	lcdWriteData((x0 >> 8) & 0xFF);
	lcdWriteData(x0 & 0xFF);
	lcdWriteData((x1 >> 8) & 0xFF);
	lcdWriteData(x1 & 0xFF);
	lcdWriteCommand(ILI9341_PAGEADDRSET);
	lcdWriteData((y0 >> 8) & 0xFF);
	lcdWriteData(y0 & 0xFF);
	lcdWriteData((y1 >> 8) & 0xFF);
	lcdWriteData(y1 & 0xFF);
	lcdWriteCommand(ILI9341_MEMORYWRITE);
}

void lcdBacklightOff(void) {
	LCD_BL_OFF();
}

void lcdBacklightOn(void) {
	LCD_BL_ON();
}

void lcdInversionOff(void) {
	lcdWriteCommand(ILI9341_INVERTOFF);
}

void lcdInversionOn(void) {
	lcdWriteCommand(ILI9341_INVERTON);
}

void lcdDisplayOff(void) {
	lcdWriteCommand(ILI9341_DISPLAYOFF);
	LCD_BL_OFF();
}

void lcdDisplayOn(void) {
	lcdWriteCommand(ILI9341_DISPLAYON);
	LCD_BL_ON();
}

void lcdTearingOff(void) {
	lcdWriteCommand(ILI9341_TEARINGEFFECTOFF);
}

void lcdTearingOn(bool m) {
	lcdWriteCommand(ILI9341_TEARINGEFFECTON);
	lcdWriteData(m);
}

uint16_t lcdGetWidth(void) {
	return lcdProperties.width;
}

uint16_t lcdGetHeight(void) {
	return lcdProperties.height;
}

uint16_t lcdGetControllerID(void) {
	uint16_t id;
	lcdWriteCommand(ILI9341_READID4);
	id = lcdReadData();
	id = lcdReadData();
	id = ((uint16_t) lcdReadData() << 8);
	id |= lcdReadData();
	return id;
}

lcdOrientationTypeDef lcdGetOrientation(void) {
	return lcdProperties.orientation;
}
/*
 sFONT* lcdGetTextFont(void) {
 return lcdFont.pFont;
 }*/

lcdPropertiesTypeDef lcdGetProperties(void) {
	return lcdProperties;
}

/**
 * \brief Reads a point from the specified coordinates
 *
 * \param x        x-Coordinate
 * \param y        y-Coordinate
 *
 * \return uint16_t     Color
 */
uint16_t lcdReadPixel(uint16_t x, uint16_t y) {
	uint16_t temp[3];
	// Clip
	if ((x < 0) || (y < 0) || (x >= lcdProperties.width)
			|| (y >= lcdProperties.height))
		return 0;

	lcdWriteCommand(ILI9341_COLADDRSET);
	lcdWriteData((x >> 8) & 0xFF);
	lcdWriteData(x & 0xFF);

	lcdWriteCommand(ILI9341_PAGEADDRSET);
	lcdWriteData((y >> 8) & 0xFF);
	lcdWriteData(y & 0xFF);

	lcdWriteCommand(ILI9341_MEMORYREAD);

	temp[0] = lcdReadData(); // dummy read
	temp[1] = lcdReadData();
	temp[2] = lcdReadData();

	return lcdColor565((temp[1] >> 8) & 0xFF, temp[1] & 0xFF,
			(temp[2] >> 8) & 0xFF);
}

/*---------Static functions--------------------------*/
/*
static void lcdDrawPixels(uint16_t x, uint16_t y, uint16_t *data,
	uint32_t dataLength) {
	uint32_t i = 0;

	lcdSetWindow(x, y, lcdProperties.width - 1, lcdProperties.height - 1);

	do {
		lcdWriteData(data[i++]);
	} while (i < dataLength);
}
*/

static void lcdReset(void) {
	lcdWriteCommand(ILI9341_SOFTRESET);
	HAL_Delay(50);
}

// Write an 8 bit command to the IC driver
static void lcdWriteCommand(unsigned char command) {
	LCD_CmdWrite(command);
}

// Write an 16 bit data word to the IC driver
static void lcdWriteData(unsigned short data) {
	LCD_DataWrite(data);
}

static unsigned short lcdReadData(void) {
	return LCD_DataRead();
}

static unsigned char lcdBuildMemoryAccessControlConfig(
bool rowAddressOrder,
bool columnAddressOrder,
bool rowColumnExchange,
bool verticalRefreshOrder,
bool colorOrder,
bool horizontalRefreshOrder) {
	unsigned char value = 0;
	if (horizontalRefreshOrder)
		value |= ILI9341_MADCTL_MH;
	if (colorOrder)
		value |= ILI9341_MADCTL_BGR;
	if (verticalRefreshOrder)
		value |= ILI9341_MADCTL_ML;
	if (rowColumnExchange)
		value |= ILI9341_MADCTL_MV;
	if (columnAddressOrder)
		value |= ILI9341_MADCTL_MX;
	if (rowAddressOrder)
		value |= ILI9341_MADCTL_MY;
	return value;
}

