#pragma once
#include "ili9341.h"
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include "color.h"
#include "linalg.h"

#include "ILI9341Wrapper.h"
#include "BaseAnimation.h"
#include "3dPrimitives.h"
#include <string>
#include <optional>

class Render: public BaseAnimation {
public:
	Render() : BaseAnimation() {
		_scene.push_back(new Cube({-3, -3, 6}));
		_scene.push_back(new Cube({0, -3, 6}));
		_scene.push_back(new Cube({3, -3, 6}));
		_scene.push_back(new Cube({-3, 0, 6}));
		_scene.push_back(new Cube({0, 0, 6}));
		_scene.push_back(new Cube({3, 0, 6}));
		_scene.push_back(new Cube({-3, 3, 6}));
		_scene.push_back(new Cube({0, 3, 6}));
		_scene.push_back(new Cube({3, 3, 6}));
	}

	void init(ILI9341Wrapper &tft);
	uint_fast16_t bgColor(void);
	std::string title();
	void perFrame(ILI9341Wrapper &tft, FrameParams frameParams);

private:
	uint_fast16_t _bgColor;
	uint32_t _time = 0;
	std::vector<Object*> _scene;
};

void Render::init(ILI9341Wrapper &tft) {
	_bgColor = color565(0, 0, 0);
}

uint_fast16_t Render::bgColor() {
	return _bgColor;
}

std::string Render::title() {
	return "Render";
}

void Render::perFrame(ILI9341Wrapper &tft, FrameParams frameParams) {
	tft.fillScreen(_bgColor);
	_time++;

	Vec3d camera{};
	camera[0] = 4.0 * sin(_time * M_PI/180.0);
	camera[1] = 4.0 * sin(5e8 + 0.77 * _time * M_PI/180.0);
	camera[2] = 2.0 + 2.0 * cos(0.3 * _time * M_PI/180.0);

	std::vector<Triangle> triangles;
	int time_offset = 0;
	for(Object* o : _scene) {
		o->update(_time + time_offset);
		//time_offset += 400;
		std::vector<Triangle> newTri = o->getTriangles(camera);
		triangles.insert(triangles.end(), newTri.begin(), newTri.end());
	}

	std::sort(triangles.begin(), triangles.end(),
			[](Triangle const &t1, Triangle const &t2) {
				return t1.distFromCamera > t2.distFromCamera;
			});

	// Draw the triangles
	for (const Triangle t : triangles) {
		// the virtual screen -1->1 maps to the LCD screen
		auto toLCD = [&](double x, double y) -> Vec2d {
			return Vec2d { static_cast<int>(tft.width() * (1 + x) / 2.0),
					static_cast<int>(tft.height() * (1 + y) / 2.0)};
		};

		Vec2d p1 = toLCD(t.p1[0], t.p1[1]);
		Vec2d p2 = toLCD(t.p2[0], t.p2[1]);
		Vec2d p3 = toLCD(t.p3[0], t.p3[1]);
		tft.drawFilledTriangle(p1[0], p1[1], p2[0], p2[1], p3[0], p3[1], t.col);
	}
//	tft.drawFastHLine(0, tft.height()/2, tft.width(), 0xff00);
	//tft.drawFastVLine(tft.width()/2, 0, tft.height(), 0x00ff);
}

