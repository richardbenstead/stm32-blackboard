/******************************************************************************
 *  ILI9341_T4 library for driving an ILI9341 screen via SPI with a Teensy 4/4.1
 *  Implements vsync and differential updates from a memory framebuffer.
 *
 *  Copyright (c) 2020 Arvind Singh.  All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *******************************************************************************/

#include "ILI9341Driver.h"
#include "font_ILI9341_T4.h"
#include <cstring>

extern "C" {
#include "ili9341.h"
}

namespace ILI9341_T4 {

/**********************************************************************************************************
 * Screen orientation
 ***********************************************************************************************************/

void ILI9341Driver::setRotation(uint8_t m) {
	m = _clip(m, (uint8_t) 0, (uint8_t) 3);
//	if (m == _rotation)
//		return;

	_rotation = m;
	switch (m) {
	case 0: // portrait 240x320
	case 2: // portrait 240x320
		_width = ILI9341_FB_PIXEL_WIDTH;
		_height = ILI9341_FB_PIXEL_HEIGHT;
		break;
	case 1: // landscape 320x240
	case 3: // landscape 320x240
		_width = ILI9341_FB_PIXEL_HEIGHT;
		_height = ILI9341_FB_PIXEL_WIDTH;
		break;
	}
}

/**********************************************************************************************************
 * Differential updates
 ***********************************************************************************************************/

void ILI9341Driver::setDiffGap(int gap) {
	_diff_gap = ILI9341Driver::_clip<int>((int) gap, (int) 2,
			(int) ILI9341_T4_NB_PIXELS);
}

void ILI9341Driver::setDiffCompareMask(uint16_t mask) {
	if (mask == 65535)
		mask = 0;
	_compare_mask = mask;
}

void ILI9341Driver::setDiffCompareMask(int bitskip_red, int bitskip_green,
		int bitskip_blue) {
	_compare_mask = (((uint16_t) (((0xFF >> bitskip_red) << bitskip_red) & 31))
			<< 11)
			| (((uint16_t) (((0xFF >> bitskip_green) << bitskip_green) & 63))
					<< 5)
			| ((uint16_t) (((0xFF >> bitskip_blue) << bitskip_blue) & 31));
	if (_compare_mask == 65535)
		_compare_mask = 0;
}

/**********************************************************************************************************
 * Update
 ***********************************************************************************************************/

void ILI9341Driver::updateRegion(bool redrawNow, const uint16_t *fb, int xmin,
		int xmax, int ymin, int ymax, int stride) {
	if (fb == nullptr)
		return;
	if (stride < 0) {
		stride = xmax - xmin + 1;

		//	_diff1.computeDiff(_fb1, nullptr, fb, xmin, xmax, ymin, ymax, stride, _rotation, _diff_gap, true, _compare_mask);
		return;
	}
}

void ILI9341Driver::update(const uint16_t *fb, bool force_full_redraw) {

	// write full PASET/CASET now and we shall only update the start position from now on.
	lcdSetWindow(0, 0, ILI9341_PHY_PIXEL_WIDTH, ILI9341_PHY_PIXEL_HEIGHT);

	for (int i = 0; i < ILI9341_T4_NB_PIXELS; ++i) {
		LCD_DataWrite(fb[i]);
		LCD_DataWrite(fb[i]);
	}
	LCD_CmdWrite(ILI9341_NOP);
}

void ILI9341Driver::_pushpixels_mode0(const uint16_t *fb, int x, int y,
		int len) {
	const uint16_t *p = fb + x + (y * ILI9341_FB_PIXEL_WIDTH);
	while (len-- > 0) {
		LCD_DataWrite(*p++);
	}
}

void ILI9341Driver::_pushpixels_mode1(const uint16_t *fb, int xx, int yy,
		int len) {
	int x = yy;
	int y = ILI9341_FB_PIXEL_WIDTH - 1 - xx;
	while (len-- > 0) {
		LCD_DataWrite(fb[x + ILI9341_FB_PIXEL_HEIGHT * y]);
		y--;
		if (y < 0) {
			y = ILI9341_FB_PIXEL_WIDTH - 1;
			x++;
		}
	}
}

void ILI9341Driver::_pushpixels_mode2(const uint16_t *fb, int xx, int yy,
		int len) {
	int x = ILI9341_FB_PIXEL_WIDTH - 1 - xx;
	int y = ILI9341_FB_PIXEL_HEIGHT - 1 - yy;
	const uint16_t *p = fb + x + (y * ILI9341_FB_PIXEL_WIDTH);
	while (len-- > 0) {
		LCD_DataWrite(*p--);
	}
}

void ILI9341Driver::_pushpixels_mode3(const uint16_t *fb, int xx, int yy,
		int len) {
	int x = ILI9341_FB_PIXEL_HEIGHT - 1 - yy;
	int y = xx;
	while (len-- > 0) {
		LCD_DataWrite(fb[x + ILI9341_FB_PIXEL_HEIGHT * y]);
		y++;
		if (y >= ILI9341_FB_PIXEL_WIDTH) {
			y = 0;
			x--;
		}
	}
}


/**********************************************************************************************************
 * Drawing characters
 * (adapted from the tgx library)
 ***********************************************************************************************************/

uint32_t ILI9341Driver::_fetchbits_unsigned(const uint8_t *p, uint32_t index,
		uint32_t required) {
	uint32_t val;
	uint8_t *s = (uint8_t*) &p[index >> 3];
#ifdef UNALIGNED_IS_SAFE        // is this defined anywhere ? 
        val = *(uint32_t*)s; // read 4 bytes - unaligned is ok
        val = __builtin_bswap32(val); // change to big-endian order
    #else
	val = s[0] << 24;
	val |= (s[1] << 16);
	val |= (s[2] << 8);
	val |= s[3];
#endif
	val <<= (index & 7); // shift out used bits
	if (32 - (index & 7) < required) { // need to get more bits
		val |= (s[4] >> (8 - (index & 7)));
	}
	val >>= (32 - required); // right align the bits
	return val;
}

uint32_t ILI9341Driver::_fetchbits_signed(const uint8_t *p, uint32_t index,
		uint32_t required) {
	uint32_t val = _fetchbits_unsigned(p, index, required);
	if (val & (1 << (required - 1))) {
		return (int32_t) val - (1 << required);
	}
	return (int32_t) val;
}

bool ILI9341Driver::_clipit(int &x, int &y, int &sx, int &sy, int &b_left,
		int &b_up, int lx, int ly) {
	b_left = 0;
	b_up = 0;
	if ((sx < 1) || (sy < 1) || (y >= ly) || (y + sy <= 0) || (x >= lx)
			|| (x + sx <= 0)) { // completely outside of image
		return false;
	}
	if (y < 0) {
		b_up = -y;
		sy += y;
		y = 0;
	}
	if (y + sy > ly) {
		sy = ly - y;
	}
	if (x < 0) {
		b_left = -x;
		sx += x;
		x = 0;
	}
	if (x + sx > lx) {
		sx = lx - x;
	}
	return true;
}

void ILI9341Driver::_measureChar(char c, int pos_x, int pos_y, int &min_x,
		int &max_x, int &min_y, int &max_y, const void *pfont, int &xadvance) {
	const ILI9341_t3_font_t &font = *((const ILI9341_t3_font_t*) pfont);
	uint8_t n = (uint8_t) c;
	if ((n >= font.index1_first) && (n <= font.index1_last)) {
		n -= font.index1_first;
	} else if ((n >= font.index2_first) && (n <= font.index2_last)) {
		n = (n - font.index2_first)
				+ (font.index1_last - font.index1_first + 1);
	} else { // no char to draw
		xadvance = 0;
		return; // nothing to draw. 
	}
	uint8_t *data = (uint8_t*) font.data
			+ _fetchbits_unsigned(font.index, (n * font.bits_index),
					font.bits_index);
	int32_t off = 0;
	uint32_t encoding = _fetchbits_unsigned(data, off, 3);
	if (encoding != 0) { // wrong/unsupported format
		xadvance = 0;
		return;
	}
	off += 3;
	const int sx = (int) _fetchbits_unsigned(data, off, font.bits_width);
	off += font.bits_width;
	//const int sy = (int) _fetchbits_unsigned(data, off, font.bits_height);
	off += font.bits_height;
	const int xoffset = (int) _fetchbits_signed(data, off, font.bits_xoffset);
	off += font.bits_xoffset;
	//const int yoffset = (int)_fetchbits_signed(data, off, font.bits_yoffset);
	off += font.bits_yoffset;
	xadvance = (int) _fetchbits_unsigned(data, off, font.bits_delta);
	min_x = pos_x;
	max_x = pos_x + xoffset + sx - 1;
	min_y = pos_y - font.cap_height - 2;
	max_y = min_y + font.line_space - 1;
}

void ILI9341Driver::_measureText(const char *text, int pos_x, int pos_y,
		int &min_x, int &max_x, int &min_y, int &max_y, const void *pfont,
		bool start_newline_at_0) {
	const int startx = start_newline_at_0 ? 0 : pos_x;
	min_x = pos_x;
	max_x = pos_x;
	min_y = pos_y;
	max_y = pos_y;
	const size_t l = strlen(text);
	for (size_t i = 0; i < l; i++) {
		const char c = text[i];
		if (c == '\n') {
			pos_x = startx;
			pos_y += ((const ILI9341_t3_font_t*) pfont)->line_space;
		} else {
			int xa = 0;
			int mx = min_x;
			int Mx = max_x;
			int my = min_y;
			int My = max_y;
			_measureChar(c, pos_x, pos_y, mx, Mx, my, My, pfont, xa);
			if (mx < min_x)
				min_x = mx;
			if (my < min_y)
				min_y = my;
			if (Mx > max_x)
				max_x = Mx;
			if (My > max_y)
				max_y = My;
			pos_x += xa;
		}
	}
}

void ILI9341Driver::_drawTextILI(const char *text, int pos_x, int pos_y,
		uint16_t col, const void *pfont, bool start_newline_at_0, int lx,
		int ly, int stride, uint16_t *buffer, float opacity) {
	if (opacity <= 0.0f)
		return;
	if (opacity >= 1.0f)
		opacity = 1.0f;
	const int startx = start_newline_at_0 ? 0 : pos_x;
	const size_t l = strlen(text);
	for (size_t i = 0; i < l; i++) {
		const char c = text[i];
		if (c == '\n') {
			pos_x = startx;
			pos_y += ((const ILI9341_t3_font_t*) pfont)->line_space;
		} else {
			_drawCharILI(c, pos_x, pos_y, col, pfont, lx, ly, stride, buffer,
					opacity);
		}
	}
}

void ILI9341Driver::_drawCharILI(char c, int &pos_x, int &pos_y, uint16_t col,
		const void *pfont, int lx, int ly, int stride, uint16_t *buffer,
		float opacity) {
	const ILI9341_t3_font_t &font = *((const ILI9341_t3_font_t*) pfont);
	uint8_t n = (uint8_t) c;
	if ((n >= font.index1_first) && (n <= font.index1_last)) {
		n -= font.index1_first;
	} else if ((n >= font.index2_first) && (n <= font.index2_last)) {
		n = (n - font.index2_first)
				+ (font.index1_last - font.index1_first + 1);
	} else { // no char to draw
		return;
	}
	uint8_t *data = (uint8_t*) font.data
			+ _fetchbits_unsigned(font.index, (n * font.bits_index),
					font.bits_index);
	int32_t off = 0;
	uint32_t encoding = _fetchbits_unsigned(data, off, 3);
	if (encoding != 0)
		return; // wrong/unsupported format
	off += 3;
	int sx = (int) _fetchbits_unsigned(data, off, font.bits_width);
	off += font.bits_width;
	int sy = (int) _fetchbits_unsigned(data, off, font.bits_height);
	off += font.bits_height;
	const int xoffset = (int) _fetchbits_signed(data, off, font.bits_xoffset);
	off += font.bits_xoffset;
	const int yoffset = (int) _fetchbits_signed(data, off, font.bits_yoffset);
	off += font.bits_yoffset;
	const int delta = (int) _fetchbits_unsigned(data, off, font.bits_delta);
	off += font.bits_delta;
	int x = pos_x + xoffset;
	int y = pos_y - sy - yoffset;
	const int rsx = sx; // save the real bitmap width; 
	int b_left, b_up;
	if ((_clipit(x, y, sx, sy, b_left, b_up, lx, ly)) && (font.version == 23)
			&& (font.reserved == 2)) { // only draw antialised, 4bpp, v2.3 characters
		data += (off >> 3) + ((off & 7) ? 1 : 0); // bitmap begins at the next byte boundary
		_drawCharBitmap_4BPP(data, rsx, b_up, b_left, sx, sy, x, y, col, stride,
				buffer, opacity);
	}
	pos_x += delta;
	return;
}

void ILI9341Driver::_drawCharBitmap_4BPP(const uint8_t *bitmap, int rsx,
		int b_up, int b_left, int sx, int sy, int x, int y, uint16_t col,
		int stride, uint16_t *buffer, float opacity) {
	const int iop = 137 * (int) (256 * opacity);
	if (sx >= 2) { // each row has at least 2 pixels
		for (int dy = 0; dy < sy; dy++) {
			int32_t off = (b_up + dy) * (rsx) + (b_left);
			uint16_t *p = buffer + (stride) * (y + dy) + (x);
			int dx = sx;
			if (off & 1) { // not at the start of a bitmap byte: we first finish it. 
				const uint8_t b = bitmap[off >> 1];
				const int v = (b & 15);
				*p = _blend32(*p, col, (v * iop) >> 14);
				p++;
				off++;
				dx--;
			}
			while (dx >= 2) {
				const uint8_t b = bitmap[off >> 1];
				if (b) {
					{
						const int v = ((b & 240) >> 4);
						p[0] = _blend32(p[0], col, (v * iop) >> 14);
					}
					{
						const int v = (b & 15);
						p[1] = _blend32(p[1], col, (v * iop) >> 14);
					}
				}
				off += 2;
				p += 2;
				dx -= 2;
			}
			if (dx > 0) {
				const uint8_t b = bitmap[off >> 1];
				const int v = ((b & 240) >> 4);
				*p = _blend32(*p, col, (v * iop) >> 14);
			}
		}
	} else { // each row has a single pixel 
		uint16_t *p = buffer + (stride) * (y) + (x);
		int32_t off = (b_up) * (rsx) + (b_left);
		while (sy-- > 0) {
			const uint8_t b = bitmap[off >> 1];
			const int v = (off & 1) ? (b & 15) : ((b & 240) >> 4);
			*p = _blend32(*p, col, (v * iop) >> 14);
			p += stride;
			off += rsx;
		}
	}
}

void ILI9341Driver::_fillRect(int xmin, int xmax, int ymin, int ymax, int lx,
		int ly, int stride, uint16_t *buffer, uint16_t color, float opacity) {
	if (opacity <= 0.0f)
		return;
	if (xmin < 0)
		xmin = 0;
	if (xmax >= lx)
		xmax = lx - 1;
	if (ymin < 0)
		ymin = 0;
	if (ymax >= ly)
		ymax = ly - 1;

	const uint32_t a = (opacity >= 1.0f) ? 32 : (32 * opacity);
	for (int j = ymin; j <= ymax; j++) {
		for (int i = xmin; i <= xmax; i++) {
			auto &d = buffer[i + j * stride];
			d = _blend32(d, color, a);
		}
	}
}

void ILI9341Driver::_uploadText(const char *text, int pos_x, int pos_y,
		uint16_t col, uint16_t col_bg, const void *pfont,
		bool start_newline_at_0) {
	const int startx = start_newline_at_0 ? 0 : pos_x;
	const size_t l = strlen(text);
	for (size_t i = 0; i < l; i++) {
		const char c = text[i];
		if (c == '\n') {
			pos_x = startx;
			pos_y += ((const ILI9341_t3_font_t*) pfont)->line_space;
		} else {
			_uploadChar(c, pos_x, pos_y, col, col_bg, pfont);
		}
	}
}

void ILI9341Driver::_uploadChar(char c, int &pos_x, int &pos_y, uint16_t col,
		uint16_t col_bg, const void *pfont) {
	const int MAX_CHAR_SIZE_LX = 20;
	const int MAX_CHAR_SIZE_LY = 20;

	uint16_t buffer[MAX_CHAR_SIZE_LX * MAX_CHAR_SIZE_LY]; // memory buffer large enough to hold 1 char

	for (int i = 0; i < MAX_CHAR_SIZE_LX * MAX_CHAR_SIZE_LY; i++)
		buffer[i] = col_bg; // clear to the background color

	int min_x, min_y, max_x, max_y, xa;
	_measureChar(c, 0, 0, min_x, max_x, min_y, max_y, pfont, xa); // find out the dimension of the char

	max_x -= min_x;         // set top left corner at (0,0)
	int nx = -min_x;        //
	min_x = 0;              // 
	max_y -= min_y;         //
	int ny = -min_y;        //
	min_y = 0;              //

	_drawCharILI(c, nx, ny, col, pfont, MAX_CHAR_SIZE_LX, MAX_CHAR_SIZE_LY,
			MAX_CHAR_SIZE_LX, buffer, 1.0f); // draw the char on the buffer
	_updateRectNow(buffer, pos_x, pos_x + max_x, pos_y - ny, pos_y + max_y - ny,
			MAX_CHAR_SIZE_LX); // upload it to the screen

	pos_x += xa; //(nx + min_x);

}

void ILI9341Driver::overlayText(uint16_t *fb, const char *text, int position,
		int line, int font_size, uint16_t fg_color, float fg_opacity,
		uint16_t bg_color, float bg_opacity, bool extend_bk_whole_width) {

	const ILI9341_t3_font_t *pfont;
	if (font_size < 12)
		pfont = &font_ILI9341_T4_OpenSans_Bold_10;
	else if (font_size < 14)
		pfont = &font_ILI9341_T4_OpenSans_Bold_12;
	else if (font_size < 16)
		pfont = &font_ILI9341_T4_OpenSans_Bold_14;
	else
		pfont = &font_ILI9341_T4_OpenSans_Bold_16;

	int x = 0;
	int y = 0;
	int tt_xmin, tt_xmax, tt_ymin, tt_ymax;
	_measureText(text, x, y, tt_xmin, tt_xmax, tt_ymin, tt_ymax, pfont, false);

	tt_xmin--;
	tt_xmax++;

	int dx, dy;
	switch (position) {
	case 1: {
		dx = _width - 1 - tt_xmax;
		dy = _height - 1 - tt_ymax - line * pfont->line_space;
		break;
	}
	case 2: {
		dx = -tt_xmin;
		dy = _height - 1 - tt_ymax - line * pfont->line_space;
		break;
	}
	case 3: {
		dx = -tt_xmin;
		dy = -tt_ymin + line * pfont->line_space;
		break;
	}
	default: {
		dx = _width - 1 - tt_xmax;
		dy = -tt_ymin + line * pfont->line_space;
		break;
	}
	}

	x += dx;
	y += dy;

	tt_xmin += dx;
	tt_xmax += dx;
	tt_ymin += dy;
	tt_ymax += dy;

	if (extend_bk_whole_width) { // overwrite
		tt_xmin = 0;
		tt_xmax = _width - 1;
	}

	_fillRect(tt_xmin, tt_xmax, tt_ymin, tt_ymax, _width, _height, _width, fb,
			bg_color, bg_opacity);
	_drawTextILI(text, x, y, fg_color, pfont, false, _width, _height, _width,
			fb, fg_opacity);
}

/*
 int16_t ILI9341Driver::_besttwoavg(int16_t x, int16_t y, int16_t z) {
 int16_t da, db, dc;
 int16_t reta = 0;
 if (x > y)
 da = x - y;
 else
 da = y - x;
 if (x > z)
 db = x - z;
 else
 db = z - x;
 if (z > y)
 dc = z - y;
 else
 dc = y - z;
 if (da <= db && da <= dc)
 reta = (x + y) >> 1;
 else if (db <= da && db <= dc)
 reta = (x + z) >> 1;
 else
 reta = (y + z) >> 1; //    else if ( dc <= da && dc <= db ) reta = (x + y) >> 1;
 return (reta);
 }*/

}

/** end of file */

