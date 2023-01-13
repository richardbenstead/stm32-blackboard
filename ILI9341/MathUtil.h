#pragma once

#include <stdint.h>
#include <type_traits>
#include <algorithm>
#include <cstdarg>

template <typename T>
constexpr T min(T x)
{
    return x;
}

template <typename T, typename... Ts>
constexpr T min(T x, Ts... xs)
{
    return std::min(x, min(xs...));
}

template <typename... Ts>
constexpr auto min(Ts... xs) -> std::common_type_t<Ts...>
{
    //static_assert((std::is_arithmetic_v<Ts> && ...), "All arguments must be arithmetic types");
    return min(static_cast<std::common_type_t<Ts...>>(xs)...);
}

struct Point16 {
	int_fast16_t x;
	int_fast16_t y;
};

struct PointU16 {
	uint_fast16_t x;
	uint_fast16_t y;
};

struct PointU8 {
	uint_fast8_t x;
	uint_fast8_t y;
};

struct PointF {
	float x;
	float y;
};

uint_fast8_t lerp8(uint_fast8_t a, uint_fast8_t b, float progress) {
	// Cast to int, avoid horrible values when b-a is less than zero
	return a + (int_fast8_t) ((int_fast8_t) b - (int_fast8_t) a) * progress;
}

template<typename T>
T lerp(T a, T b, float progress) {
	return a + (b - a) * progress;
}

//float lerp(float a, float b, float progress) {
//	return a + (b - a) * progress;
//}

// x=0,y=0 returns center of screen.
// z=1: Normalized coordinates with screen (x==-1 is left, x==1 is right, etc.)
// screenW_2 and screenH_2 are __half__ the screen size, and it's your job to pass in these numbers
// because you're probably re-using them for several points on the other end :P
// Assumes landscape orientation, so screenW_2 is greater than screenH_2.
Point16 xyz2screen(float x, float y, float z, uint_fast16_t screenW_2,
		uint_fast16_t screenH_2) {
	float invZ = 1.0f / z;

	return Point16 { (int_fast16_t) (screenW_2 + x * invZ * screenW_2),
			(int_fast16_t) (screenH_2 + y * invZ * screenH_2) };
}

static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

template <typename T>
T clamp(T value, T min, T max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

uint16_t lerpCol(uint16_t col1, uint16_t col2, float progress) {
	uint8_t baseR = (col1 & 0xf800) >> 8;
	uint8_t baseG = (col1 & 0x07e0) >> 3;
	uint8_t baseB = (col1 & 0x001f) << 3;

	uint8_t tgtR = (col2 & 0xf800) >> 8;
	uint8_t tgtG = (col2 & 0x07e0) >> 3;
	uint8_t tgtB = (col2 & 0x001f) << 3;

	uint8_t r = lerp8(baseR, tgtR, progress);
	uint8_t g = lerp8(baseG, tgtG, progress);
	uint8_t b = lerp8(baseB, tgtB, progress);
	return color565(r,g,b);
}

template<typename T>
T fract(T x)
{
    return x - floor(x);
}
