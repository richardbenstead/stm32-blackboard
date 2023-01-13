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

/**
 *CREDITS. Parts of this code is based on:
 *
 * (1) KurtE's highly optimized library for ILI9341: https://github.com/KurtE/ILI9341_t3n
 *     -> for SPI / DMA and all the fancy low level hardware stuff... beautiful !
 *
 * (2) PJRC's XPT2048 library https://github.com/PaulStoffregen/XPT2046_Touchscreen
 *     -> for all the touchscreen related methods.
 **/

#ifndef _ILI9341_T4_ILI9341Driver_H_
#define _ILI9341_T4_ILI9341Driver_H_

// only c++, no plain c
#ifdef __cplusplus

#include "DiffBuff.h"

// #include <DMAChannel.h>
// #include <SPI.h>
#include <stdint.h>
#include <cstdio>

namespace ILI9341_T4 {

/** a few colors */

#define ILI9341_T4_COLOR_BLACK 0x0
#define ILI9341_T4_COLOR_WHITE 0xffff
#define ILI9341_T4_COLOR_RED 0xf800
#define ILI9341_T4_COLOR_BLUE 0x1f
#define ILI9341_T4_COLOR_GREEN 0x7e0
#define ILI9341_T4_COLOR_PURPLE 0x8010
#define ILI9341_T4_COLOR_ORANGE 0xfc20
#define ILI9341_T4_COLOR_CYAN 0x7ff
#define ILI9341_T4_COLOR_LIME 0x7e0
#define ILI9341_T4_COLOR_SALMON 0xfc0e
#define ILI9341_T4_COLOR_MAROON 0x8000
#define ILI9341_T4_COLOR_YELLOW 0xffe0
#define ILI9341_T4_COLOR_MAJENTA 0xf81f
#define ILI9341_T4_COLOR_OLIVE 0x8400
#define ILI9341_T4_COLOR_TEAL 0x410
#define ILI9341_T4_COLOR_GRAY 0x8410
#define ILI9341_T4_COLOR_SILVER 0xc618
#define ILI9341_T4_COLOR_NAVY 0x10

/** Configuration */

#define ILI9341_T4_DEFAULT_VSYNC_SPACING 2           // vsync on with framerate = refreshrate/2 = 45FPS. 
#define ILI9341_T4_DEFAULT_DIFF_GAP 6                // default gap for diffs (typ. between 4 and 50)
#define ILI9341_T4_DEFAULT_LATE_START_RATIO 0.3f     // default "proportion" of the frame admissible for late frame start when using vsync. 

#define ILI9341_T4_TRANSACTION_DURATION 3           // number of pixels that could be uploaded during a typical CASET/PASET/RAWR sequence. 
#define ILI9341_T4_RETRY_INIT 5                     // number of times we try initialization in begin() before returning an error. 
#define ILI9341_T4_TFTWIDTH ILI9341_FB_PIXEL_WIDTH                     // screen dimension x (in default orientation 0)
#define ILI9341_T4_TFTHEIGHT ILI9341_FB_PIXEL_HEIGHT                    // screen dimension y (in default orientation 0)
#define ILI9341_T4_NB_SCANLINES ILI9341_T4_TFTHEIGHT// scanlines are mapped to the screen height
#define ILI9341_T4_MIN_WAIT_TIME  300               // minimum waiting time (in us) before drawing again when catching up with the scanline

#define ILI9341_T4_NB_PIXELS (ILI9341_T4_TFTWIDTH * ILI9341_T4_TFTHEIGHT)   // total number of pixels

#define ILI9341_T4_MAX_VSYNC_SPACING 5              // maximum number of screen refresh between frames (for sync clock stability). 
#define ILI9341_T4_DEFAULT_IRQ_PRIORITY 128                 // default priority at which we run all irqs (dma, pit timer and spi interrupts).
#define ILI9341_T4_MAX_DELAY_MICROSECONDS 1000000   // maximum waiting time (1 second)

#define ILI9341_T4_SELFDIAG_OK 0xC0                 // value returned by selfDiagStatus() if everything is OK.

#define ILI9441_T4_DEFAULT_FPS_COUNTER_COLOR_FG ILI9341_T4_COLOR_WHITE  // default values (color/opacity/position)
#define ILI9441_T4_DEFAULT_FPS_COUNTER_COLOR_BG ILI9341_T4_COLOR_BLUE   // for the FPS counter
#define ILI9441_T4_DEFAULT_FPS_COUNTER_OPACITY 0.5f                     // 
#define ILI9441_T4_DEFAULT_FPS_COUNTER_POSITION 0                       //

#define ILI9341_T4_ALWAYS_INLINE __attribute__((always_inline))

class ILI9341Driver {
public:

	/**
	 * Query the value of the self-diagnostic register.
	 * 
	 * Should return ILI9341_T4_SELFDIAG_OK = 0xC0 if everything is fine.
	 **/
	int selfDiagStatus();

	/**
	 * Print some info about the screen status (for debug purpose).
	 * Output is sent to the stream set with the `output()` method.
	 * Use printStats() instead to get statistics for optimization purposes. 
	 **/
	void printStatus();

	/**
	 * Enter/exit sleep mode.
	 **/
	void sleep(bool enable);

	/**
	 * Invert the display colors.
	 **/
	void invertDisplay(bool i);

	/**
	 * Set the vertical scroll offset.
	 *
	 * Default value is 0 (no scroll). When an offset is set, the framebuffer is shifted vertically on 
	 * the screen by the given offset. This means that the following (hardware) mapping is performed:
	 * 
	 * - framebuffer line i  =>  drawn at scanline (i - offset) mod TFT_HEIGHT. 
	 * 
	 * offset can be any value (positive or negative) so that incrementing / decrementing it enables
	 * to scroll up or down continuously.
	 **/
	void setScroll(int offset = 0);

	/** The 4 possible orientations */
	enum {
		PORTRAIT_240x320 = 0,
		LANDSCAPE_320x240 = 1,
		PORTRAIT_240x320_FLIPPED = 2,
		LANDSCAPE_320x240_FLIPPED = 3,
	};

	void setRotation(uint8_t r);

	int getRotation() const {
		return _rotation;
	}

	/**
	 * Return the screen _width (w.r.t the current orientation).
	 **/
	int width() const {
		return _width;
	}

	/**
	 * Return the screen height (w.r.t the current orientation).
	 **/
	int height() const {
		return _height;
	}

	/***************************************************************************************************
	 ****************************************************************************************************
	 *
	 * Screen refresh rate. 
	 * 
	 * -> these methods are used to set the screen refresh rate (number of time the display is refreshed 
	 *    per second). This rate is important because it is related to the actual framerate via the 
	 *    vsync_spacing parameter (c.f. the vsync setting section). 
	 *
	 ****************************************************************************************************
	 ****************************************************************************************************/

	/**
	 * set the refresh mode between 0 and 31.
	 *
	 * - 0  : fastest refresh rate (around than 120/140hz). 
	 * - 31 : slowest refresh rate (around 30/40hz).
	 * 
	 * NOTE: the exact refresh rate for a given mode varies from display to display. 
	 *       Once the mode set, use getRefreshRate() to find out the refresh rate.
	 *
	 * By default the refresh mode selected is 0 (fastest possible). 
	 * 
	 * Remark: calling this method resets the statistics.
	 **/
	void setRefreshMode(int mode);

	/**
	 * Return the current refresh mode. 
	 *
	 * - 0  : fastest refresh rate (around than 120/140Hz). 
	 * - 31 : slowest refresh rate (around 30/40Hz).   
	 *
	 **/
	int getRefreshMode() const {
		return _refreshmode;
	}

	/**
	 * Set the gap used when creating diffs. 
	 * 
	 * [See the DiffBuff class for more detail on the gap parameter].

	 * This parameter correspond to the number of consecutive unchanged pixels needed to break a SPI 
	 * transaction. A smaller value will give results in less pixels being uploaded but will, on the 
	 * other hand create larger diffs... The optimal value should be between 4 and 20 and will depend 
	 * on the kind of graphics drawn. 
	 * 
	 * As a rule of thumb:
	 * 
	 * - Gap larger than 10 for diff buffer with less than 4K of memory
	 * - Gap between 6 to 10  for diff buffers with size between 4K to 8K.  
	 * - Gap between 4 to 6 for diff buffers with size larger then 8K. 
	 * 
	 * You can use the printStats() to check how much memory the diff buffer typically consume. If the
	 * diffs buffers overflow too often, you should either increase the gap or increase their size.
	 * 
	 * Remark: calling this method resets the statistics.
	 **/
	void setDiffGap(int gap = ILI9341_T4_DEFAULT_DIFF_GAP);

	/**
	 * Return the current gap parameter used when creating diffs. 
	 **/
	int getDiffGap() const {
		return _diff_gap;
	}

	/**
	 * Set the mask used when creating a diff to check is a pixel is the same in both framebuffers. 
	 * If the mask set is non-zero, then only the bits set in the mask are used for the comparison 
	 * so pixels with different values may be considered equal and may not redrawn.
	 * 
	 * Setting a mask may be useful when the framebuffer being uploaded to the screen comes from
	 * a camera or another source that introduces random noise that would prevent the diff from
	 * finding large region of identical pixels (hence making the diff pretty useless) but when 
	 * it does not really matter to have a 'perfect' copy of the framebuffer on the screen. 
	 * 
	 * Typically, one wants to set the lower bits on each channel color to 0 so that color that
	 * are 'close' are not always redrawn (see the other method version below). 
	 * 
	 * If called without argument, the compare mask is set to 0 hence disabled and strict equality
	 * is enforced when creating diffs (default behavior).
	 * 
	 * -> RGB565 layout as uint16: bit 15            bit 0
	 *                                  RRRRR GGGGGG BBBBB
	 **/
	void setDiffCompareMask(uint16_t mask = 0);

	/**
	 * Set the compare mask by specifying for each color channel the number of lower bits
	 * that should be ignored. 
	 * 
	 * Recall that there are 5 bits for the blue and red channel and 6 bits for the green
	 * channel. 
	 * 
	 * -> RGB565 layout as uint16: bit 15            bit 0
	 *                                  RRRRR GGGGGG BBBBB
	 **/
	void setDiffCompareMask(int bitskip_red, int bitskip_green,
			int bitskip_blue);

	/**
	 * Return the value of the current compare_mask. 
	 * 
	 * Return 0 if the mask is not set and strict comparison of pixel colors is enforced 
	 * (which is the default value).
	 **/
	uint16_t getCompareMask() const {
		return _compare_mask;
	}

	/***************************************************************************************************
	 ****************************************************************************************************
	 *
	 * Screen updates
	 *
	 ****************************************************************************************************
	 ****************************************************************************************************/

	/**
	 * Clear the screen to a single color (default black). 
	 *
	 * This operation is done immediately (i.e. not async) so the screen is cleared on return.
	 **/
	void clear(uint16_t color = 0);

	/**
	 *                                 MAIN SCREEN UPDATE METHOD
	 *
	 * Push a framebuffer to be displayed on the screen. The behavior of the method depend on the
	 * current buffering mode and the vsync_spacing parameter.
	 *
	 * - fb : the framebuffer to draw unto the screen.     
	 *  
	 * - force_full_redraw: If set to true, then differential update is disabled for this particular   
	 *                      frame and the whole screen is updated (even when a diff could have been used). 
	 *                      Normally, this option should not be needed except in the very special cases 
	 *                      where one knows for sure that the diff will be useless so disabling it saves 
	 *                      some CPU times that would have been used for creating the diff (around 1us 
	 *                      normally).
	 *
	 * WHEN THE METHOD RETURNS, THE FRAME MAY OR MAY NOT ALREADY BE DISPLAYED ONT THE SCREEN BUT
	 * THE INPUT FRAMEBUFFER fb CAN STILL BE REUSED IMMEDIATELY IN ANY CASE. (when using async updates,
	 * the internal framebuffer is used to save a copy of the framebuffer).
	 *
	 * 
	 * The exact behavior of the method depends on bufferingMode():
	 *
	 * 
	 * -> NO_BUFFERING (i.e. no internal framebuffer set):
	 *
	 *   The framebuffer is pushed to the screen immediately and the method returns only when upload is 
	 *   complete. In this mode, differential upload are always disabled: the whole screen is updated
	 *   and diff buffers, if present, are ignored. 
	 *
	 *   - if vsync_spacing <= 0, upload to the screen start immediately (no VSync).
	 *
	 *   - if vsync_spacing >= 1, screen upload is synchronized with the screen refresh (VSync) and the
	 *     method waits until vsync_spacing refreshes have occurred since the previous update to insure
	 *     a constant framerate equal to (refresh_rate/vsync_spacing).
	 * 
	 *
	 * -> DOUBLE_BUFFERING (internal framebuffer set)
	 *
	 *   All updates are done asynchronously via DMA and the method returns asap. If 1 or 2 diff buffers
	 *   are present, they are automatically used to perform differential update: only the portion of
	 *   the screen whose content has changed is redrawn. 
	 *
	 *   - if vsync_spacing = -1, upload to the screen starts immediately unless there is already a
	 *     transfer in progress in which case the frame is simply dropped and the method return without
	 *     doing anything (no VSync).
	 *
	 *   - if vsync_spacing = 0, upload to the screen start immediately unless there is already a
	 *     transfer in progress in which case the method waits until the transfer completes and then 
	 *     starts another async transfer immediately (no VSync).
	 *
	 *   - if vsync_spacing > 0. screen upload is synchronized with the screen refresh and the
	 *     method waits until vsync_spacing refreshes have occurred since the previous update to
	 *     insure a constant framerate equal to (refresh_rate/vsync_spacing). If a transfer is
	 *     already in progress, it waits for it to complete before scheduling the next transfer
	 *     via DMA and returning.
	 * 
	 *
	 * NOTE: 
	 *       (1) double buffering give a HUGE improvement over the no buffering method at the
	 *           expense of an additional internal memory framebuffer (150Kb).
	 *
	 *       (2) Setting two diffs buffers instead of one cost only a few additional kilobytes
	 *           yet will usually improve the max framerate significantly since it enables the
	 *           driver to compute the next diff while the previous update is still ongoing.
	 *        
	 *              
	 * ADVICE:  US AN INTERNAL FRAMEBUFFER + 2 DIFF BUFFERS (WITH SIZE RANGING FROM 5K TO 10K).
	 * 
	 **/
	void update(const uint16_t *fb, bool force_full_redraw = false);

	/**
	 *                             PARTIAL SCREEN UPDATE METHOD
	 *
	 * Update only part of the screen. The behavior of the method depend on the current buffering 
	 * mode and the vsync_spacing parameter.
	 *
	 * WHEN THE METHOD RETURNS, THE FRAME MAY OR MAY NOT ALREADY BE DISPLAYED ON THE SCREEN BUT
	 * THE INPUT FRAMEBUFFER fb CAN STILL BE REUSED IMMEDIATELY IN ANY CASE (A COPY IS MADE WHEN
	 * USING ASYNC UPDATES).
	 *
	 * Parameters:
	 * 
	 * - fb : framebuffer to the rectangular region to update.     
	 * 
	 * - [xmin, xmax] x [ymin, ymax] : region of the screen to update  
	 * 
	 * - stride : stride for the supplied framebuffer fb
	 *
	 *            The layout of fb is such that, 
	 *   
	 *                 screen pixel(xmin + i, ymin + j) = fb[i + stride*j]
	 *                 
	 *            -> If stride is not specified, it defaults to (xmax - xmin + 1) which is the
	 *                width of the rectangular region.
	 *    
	 * - redrawNow: - If set to true, the screen is redrawn immediately (async if an internal   
	 * -              framebuffer is set). 
	 *              - If set to false and an internal framebuffer is available, then the changes   
	 *                are stored in the internal framebuffer but are not drawn on the screen. 
	 *                This permits to call regionUpdate() several times without drawing onto the 
	 *                screen and then draw all the changes simultaneously when needed. This is 
	 *                particularly convenient when using the lvgl library. 
	 *
	 *
	 * NOTE: (1) Similarly to the 'update()' method, this method will use VSync when enabled
	 *           depending on the value of the vsync_spacing parameter 
	 *
	 *       (2) In there is no internal buffer, then screen is updated immediately even if
	 *           redrawNow=false.  
	 *
	 *       (3) For this method, TWO DIFF BUFFERS ARE REQUIRED FOR DIFFERENTIAL UPDATE !
	 *           Setting only one diff buffer will disable differential updates :(
	 *
	 **/
	void updateRegion(bool redrawNow, const uint16_t *fb, int xmin, int xmax,
			int ymin, int ymax, int stride = -1);

	/**
	 * Overlay a text on the supplied framebuffer at a given position and with 
	 * given color (for text and background). 
	 * 
	 * This method is useful for printing out simple debug information... For more advance 
	 * text formatting, use a dedicated graphic library to draw on the framebuffer such as tgx 
	 * (https://github.com/vindar/tgx)
	 *
	 * - fb : the framebuffer to draw onto
	 * - position : position of the counter on the framebuffer:
	 *              0= top right,  1=bottom right,  2=bottom left,  3=top left
	 * - line : line offset for the beginning of text w.r.t. the position     
	 * - font : size of the font to use (10, 12, 14 or 16 pt)
	 * - fg_color : text color
	 * - fg_opacity : text opacity between 0.0f (fully transparent) and 1.0f (fully opaque).
	 * - bg_color : background color
	 * - bg_opacity : background opacity between 0.0f (fully transparent) and 1.0f (fully opaque).
	 * - extend_bk_whole_width : true to extend the background rectangle to the whole width of the screen
	 **/
	void overlayText(uint16_t *fb, const char *text, int position, int line,
			int font_size, uint16_t fg_color = ILI9341_T4_COLOR_WHITE,
			float fg_opacity = 1.0f, uint16_t bg_color = ILI9341_T4_COLOR_BLACK,
			float bk_opacity = 0.0f, bool extend_bg_whole_width = false);

private:

	/**********************************************************************************************************
	 * General settings.
	 ***********************************************************************************************************/

	typedef void (*callback_t)(void*);           // function callback signature 
	using methodCB_t = void (ILI9341Driver::*)(void); // typedef to method callback. 

	int16_t _width, _height;  // Display w/h as modified by current rotation    
	int _rotation;                          // current screen orientation
	int _refreshmode; // refresh mode (between 0 = fastest refresh rate and 15 = slowest refresh rate). 

	int _irq_priority; // priority at which we run all IRQ's (dma, pit timer and spi interrupts)

	/** helper methods for writing to _outputStream (without using variadic parameters...) */
	template<typename T> void _print(const T &v) const {
		printf(v);
	}

	template<typename T> void _println(const T &v) const {
		printf("%s\n", v);
	}

	template<typename T1, typename T2> void _print(const T1 &u,
			const T2 &v) const {
		printf(u, v);
	}

	template<typename T1, typename T2> void _println(const T1 &u,
			const T2 &v) const {
		printf(u, v);
	}

	template<typename T1> void _printf(const char *str, const T1 &a) const {
		printf(str, a);
	}

	template<typename T1, typename T2> void _printf(const char *str,
			const T1 &a, const T2 &b) const {
		printf(str, a, b);
	}

	template<typename T1, typename T2, typename T3> void _printf(
			const char *str, const T1 &a, const T2 &b, const T3 &c) const {
		printf(str, a, b, c);
	}

	template<typename T1, typename T2, typename T3, typename T4> void _printf(
			const char *str, const T1 &a, const T2 &b, const T3 &c,
			const T4 &d) const {
		printf(str, a, b, c, d);
	}

	/**********************************************************************************************************
	 * About buffering / update mode.
	 ***********************************************************************************************************/

	volatile int _diff_gap;                     // gap when creating diffs.
	volatile int _vsync_spacing;         // update stategy / framerate divider. 
	volatile float _late_start_ratio; // late start parameter (by how much we can miss the first sync line and still start the frame without waiting for the next refresh).
	volatile bool _late_start_ratio_override; // if true the next frame upload will wait for the scanline to start a next frame. 
	volatile uint16_t _compare_mask; // the compare mask used to compare pixels when doing a diff

	//DiffBuffStatic<ILI9341_T4_NB_PIXELS>* _diff1;
	//uint16_t* _fb1;

	/**
	 * Update part of the screen using a diff buffer object representing the changes between
	 * the old framebuffer and the new one 'fb'.
	 * - return only when update completed.
	 * - uses the _vsync_spacing parameter to choose the vsync stategy.
	 **/
	void _updateNow(const uint16_t *fb, DiffBuffBase *diff);

	/**
	 * Update a rectangular region of the screen directly.
	 * no diff buffer (the whole region is updated)
	 * no vsync (i.e. as fast as possible)
	 * no dma.
	 **/
	void _updateRectNow(const uint16_t *sub_fb, int xmin, int xmax, int ymin,
			int ymax, int stride);

	void _pushRect(uint16_t color, int xmin, int xmax, int ymin, int ymax);

	void _pushpixels(const uint16_t *fb, int x, int y, int len)
			ILI9341_T4_ALWAYS_INLINE
			{
		switch (_rotation) {
		case 0:
			_pushpixels_mode0(fb, x, y, len);
			return;
		case 1:
			_pushpixels_mode1(fb, x, y, len);
			return;
		case 2:
			_pushpixels_mode2(fb, x, y, len);
			return;
		case 3:
			_pushpixels_mode3(fb, x, y, len);
			return;
		}
		// hum...
	}

	void _pushpixels_mode0(const uint16_t *fb, int x, int y, int len);

	void _pushpixels_mode1(const uint16_t *fb, int x, int y, int len);

	void _pushpixels_mode2(const uint16_t *fb, int x, int y, int len);

	void _pushpixels_mode3(const uint16_t *fb, int x, int y, int len);

	/** clip val to [min,max] */
	template<typename T> static T _clip(T val, T min, T max) {
		if (val < min)
			val = min;
		if (val > max)
			val = max;
		return val;
	}

	/**********************************************************************************************************
	 * Drawing characters
	 * (adapted from the tgx library)
	 ***********************************************************************************************************/

	static uint16_t _blend32(uint32_t bg_col, uint32_t fg_col, uint32_t a)
			ILI9341_T4_ALWAYS_INLINE
			{
		const uint32_t bg = (bg_col | (bg_col << 16))
				& 0b00000111111000001111100000011111;
		const uint32_t fg = (fg_col | (fg_col << 16))
				& 0b00000111111000001111100000011111;
		const uint32_t result = ((((fg - bg) * a) >> 5) + bg)
				& 0b00000111111000001111100000011111;
		return (uint16_t) ((result >> 16) | result); // contract result
	}

	static uint32_t _fetchbit(const uint8_t *p, uint32_t index)
			ILI9341_T4_ALWAYS_INLINE {
		return (p[index >> 3] & (0x80 >> (index & 7)));
	}

	static uint32_t _fetchbits_unsigned(const uint8_t *p, uint32_t index,
			uint32_t required);

	static uint32_t _fetchbits_signed(const uint8_t *p, uint32_t index,
			uint32_t required);

	static bool _clipit(int &x, int &y, int &sx, int &sy, int &b_left,
			int &b_up, int lx, int ly);

	static void _measureChar(char c, int pos_x, int pos_y, int &min_x,
			int &max_x, int &min_y, int &max_y, const void *pfont,
			int &xadvance);

	static void _measureText(const char *text, int pos_x, int pos_y, int &min_x,
			int &max_x, int &min_y, int &max_y, const void *pfont,
			bool start_newline_at_0);

	static void _drawTextILI(const char *text, int pos_x, int pos_y,
			uint16_t col, const void *pfont, bool start_newline_at_0, int lx,
			int ly, int stride, uint16_t *buffer, float opacity);

	static void _drawCharILI(char c, int &pos_x, int &pos_y, uint16_t col,
			const void *pfont, int lx, int ly, int stride, uint16_t *buffer,
			float opacity);

	static void _drawCharBitmap_4BPP(const uint8_t *bitmap, int rsx, int b_up,
			int b_left, int sx, int sy, int x, int y, uint16_t col, int stride,
			uint16_t *buffer, float opacity);

	static void _fillRect(int xmin, int xmax, int ymin, int ymax, int lx,
			int ly, int stride, uint16_t *buffer, uint16_t color,
			float opacity);

	void _uploadText(const char *text, int pos_x, int pos_y, uint16_t col,
			uint16_t col_bg, const void *pfont, bool start_newline_at_0);

	void _uploadChar(char c, int &pos_x, int &pos_y, uint16_t col,
			uint16_t col_bg, const void *pfont);

};

}

#endif 

#endif
/** end of file */

