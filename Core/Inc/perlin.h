#pragma once
#include "ili9341.h"
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include "color.h"
#include "linalg.h"
#include "pixelShader.h"

#include "ILI9341Wrapper.h"
#include "BaseAnimation.h"
#include "3dPrimitives.h"
#include <string>
#include <optional>

class Perlin: public BaseAnimation {
public:
	void init(ILI9341Wrapper &tft);
	uint_fast16_t bgColor(void);
	std::string title();
	void perFrame(ILI9341Wrapper &tft, FrameParams frameParams);

private:
	uint_fast16_t _bgColor;
	uint32_t _time = 0;
	std::vector<Object*> _scene;
};


void Perlin::init(ILI9341Wrapper &tft) {
	_bgColor = color565(0, 0, 0);
}

uint_fast16_t Perlin::bgColor() {
	return _bgColor;
}

std::string Perlin::title() {
	return "Render";
}

void Perlin::perFrame(ILI9341Wrapper &tft, FrameParams frameParams) {
	tft.fillScreen(_bgColor);
	_time++;

	for (int x = 0; x < tft.width(); ++x) {
		for (int y = 0; y < tft.height(); ++y) {
			tft.drawPixel(x, y, cnoise(Vec2d{50+x,50+y}));
		}
	}
}
