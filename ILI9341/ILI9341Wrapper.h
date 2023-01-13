#pragma once

/**
 * Minimal wrapper for the ILI9341Driver class that implement the
 * needed drawing primitives (line / circle / rectangle...). 
 **/
class ILI9341Wrapper {

public:

	ILI9341Wrapper(uint16_t *fb, int lx, int ly) {
		_buffer = fb;
		_lx = lx;
		_ly = ly;
		_stride = lx;
	}

	inline void drawPixel(int x, int y, uint16_t color) {
		if ((x < 0) || (y < 0) || (x >= _lx) || (y >= _ly))
			return;
		_buffer[x + _stride * y] = color;
	}

	inline uint16_t readPixel(int x, int y) {
		if ((x < 0) || (y < 0) || (x >= _lx) || (y >= _ly))
			return 0;
		return _buffer[x + _stride * y];
	}

	void fillScreen(uint16_t color) {
		fillRect(0, 0, _lx, _ly, color);
	}

	void fillRect(int x, int y, int w, int h, uint16_t color) {
		for (int j = y; j < y + h; j++) {
			drawFastHLine(x, j, w, color);
		}
	}

	inline void drawFastVLine(int x, int y, int h, uint16_t color) {
		if ((x < 0) || (x >= _lx) || (y >= _ly))
			return;
		if (y < 0) {
			h += y;
			y = 0;
		}
		if (y + h > _ly) {
			h = _ly - y;
		}
		uint16_t *p = _buffer + x + y * _stride;
		while (h-- > 0) {
			(*p) = color;
			p += _stride;
		}
	}

	inline void drawFastHLine(int x, int y, int w, uint16_t color) {
		if ((y < 0) || (y >= _ly) || (x >= _lx))
			return;
		if (x < 0) {
			w += x;
			x = 0;
		}
		if (x + w > _lx) {
			w = _lx - x;
		}
		uint16_t *p = _buffer + x + y * _stride;
		while (w-- > 0) {
			(*p) = color;
			p++;
		}
	}

	inline void drawHLine(int a, int b, int y, uint16_t color) {
		if (b > a) {
			drawFastHLine(a, y, b - a, color);
		} else {
			drawFastHLine(b, y, a - b, color);
		}
	}

	inline void drawVLine(int a, int b, int x, uint16_t color) {
		if (b > a) {
			drawFastVLine(a, x, b - a, color);
		} else {
			drawFastVLine(b, x, a - b, color);
		}
	}
	inline void drawFilledTriangle(int16_t x0, int16_t y0, int16_t x1,
			int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
		// Sort the y-coordinates in ascending order
		if (y0 > y1) {
			swap(y0, y1);
			swap(x0, x1);
		}
		if (y1 > y2) {
			swap(y1, y2);
			swap(x1, x2);
		}
		if (y0 > y1) {
			swap(y0, y1);
			swap(x0, x1);
		}

		// Calculate the slope and y-intercept for each line
		int16_t m1 = (x1 - x0) * 256 / (y1 - y0 + 1);
		int16_t b1 = x0 - m1 * y0 / 256;
		int16_t m2 = (x2 - x0) * 256 / (y2 - y0 + 1);
		int16_t b2 = x0 - m2 * y0 / 256;

		// Draw the horizontal lines between the pairs of points with the same y value
		int16_t curx1 = x0;
		int16_t curx2 = x0;
		for (int16_t scanlineY = y0; scanlineY <= y1; scanlineY++) {
			curx1 = m1 * scanlineY / 256 + b1;
			curx2 = m2 * scanlineY / 256 + b2;
			drawHLine(curx1, curx2, scanlineY, color);
		}

		m1 = (x2 - x1) * 256 / (y2 - y1 + 1);
		b1 = x1 - m1 * y1 / 256;
		for (int16_t scanlineY = y1; scanlineY <= y2; scanlineY++) {
			curx1 = m1 * scanlineY / 256 + b1;
			curx2 = m2 * scanlineY / 256 + b2;
			drawHLine(curx1, curx2, scanlineY, color);
		}
	}

	inline void drawRect(int x, int y, int w, int h, uint16_t color) {
		drawFastHLine(x, y, w, color);
		drawFastHLine(x, y + h - 1, w, color);
		drawFastVLine(x, y, h, color);
		drawFastVLine(x + w - 1, y, h, color);
	}

	void drawLine(int x0, int y0, int x1, int y1, uint16_t color) {
		if (y0 == y1) {
			if (x1 > x0) {
				drawFastHLine(x0, y0, x1 - x0 + 1, color);
			} else if (x1 < x0) {
				drawFastHLine(x1, y0, x0 - x1 + 1, color);
			} else {
				drawPixel(x0, y0, color);
			}
			return;
		} else if (x0 == x1) {
			if (y1 > y0) {
				drawFastVLine(x0, y0, y1 - y0 + 1, color);
			} else {
				drawFastVLine(x0, y1, y0 - y1 + 1, color);
			}
			return;
		}

		bool steep = abs(y1 - y0) > abs(x1 - x0);
		if (steep) {
			swap(x0, y0);
			swap(x1, y1);
		}
		if (x0 > x1) {
			swap(x0, x1);
			swap(y0, y1);
		}

		int dx, dy;
		dx = x1 - x0;
		dy = abs(y1 - y0);

		int err = dx / 2;
		int ystep;

		if (y0 < y1) {
			ystep = 1;
		} else {
			ystep = -1;
		}
		int xbegin = x0;
		if (steep) {
			for (; x0 <= x1; x0++) {
				err -= dy;
				if (err < 0) {
					int len = x0 - xbegin;
					if (len) {
						drawFastVLine(y0, xbegin, len + 1, color);
					} else {
						drawPixel(y0, x0, color);
					}
					xbegin = x0 + 1;
					y0 += ystep;
					err += dx;
				}
			}
			if (x0 > xbegin + 1) {
				drawFastVLine(y0, xbegin, x0 - xbegin, color);
			}
		} else {
			for (; x0 <= x1; x0++) {
				err -= dy;
				if (err < 0) {
					int len = x0 - xbegin;
					if (len) {
						drawFastHLine(xbegin, y0, len + 1, color);
					} else {
						drawPixel(x0, y0, color);
					}
					xbegin = x0 + 1;
					y0 += ystep;
					err += dx;
				}
			}
			if (x0 > xbegin + 1) {
				drawFastHLine(xbegin, y0, x0 - xbegin, color);
			}
		}
	}

	template<bool OUTLINE, bool FILL> void drawFilledCircle(int xm, int ym,
			int r, uint16_t color, uint16_t fillcolor) {
		if (r <= 0)
			return;
		if (r > 2) { // circle is large enough to check first if there is something to draw.
			if ((xm + r < 0) || (xm - r >= _lx) || (ym + r < 0)
					|| (ym - r >= _ly))
				return; // outside of image. 
			// TODO : check if the circle completely fills the image, in this case use FillScreen()
		}
		switch (r) {
		case 0: {
			if (OUTLINE) {
				drawPixel(xm, ym, color);
			} else if (FILL) {
				drawPixel(xm, ym, fillcolor);
			}
			return;
		}
		case 1: {
			if (FILL) {
				drawPixel(xm, ym, fillcolor);
			}
			drawPixel(xm + 1, ym, color);
			drawPixel(xm - 1, ym, color);
			drawPixel(xm, ym - 1, color);
			drawPixel(xm, ym + 1, color);
			return;
		}
		}
		int x = -r, y = 0, err = 2 - 2 * r;
		do {
			if (OUTLINE) {
				drawPixel(xm - x, ym + y, color);
				drawPixel(xm - y, ym - x, color);
				drawPixel(xm + x, ym - y, color);
				drawPixel(xm + y, ym + x, color);
			}
			r = err;
			if (r <= y) {
				if (FILL) {
					drawFastHLine(xm, ym + y, -x, fillcolor);
					drawFastHLine(xm + x + 1, ym - y, -x - 1, fillcolor);
				}
				err += ++y * 2 + 1;
			}
			if (r > x || err > y) {
				err += ++x * 2 + 1;
				if (FILL) {
					if (x) {
						drawFastHLine(xm - y + 1, ym - x, y - 1, fillcolor);
						drawFastHLine(xm, ym + x, y, fillcolor);
					}
				}
			}
		} while (x < 0);
	}

	uint16_t width() { return _lx; }
	uint16_t height() { return _ly; }
private:

	template<typename T> inline static void swap(T &a, T &b) {
		T c = a;
		a = b;
		b = c;
	}

	uint16_t *_buffer;
	int _lx;
	int _ly;
	int _stride;

};
