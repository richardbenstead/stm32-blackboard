#pragma once

uint16_t mapColor(float value) {
	// Map the value to the range 0-6
	float hue = value * 5;

	// Calculate the red, green, and blue values
	float r, g, b;
	if (hue < 1) {
		r = hue;
		g = 0;
		b = 0;
	} else if (hue < 2) {
		r = 1;
		g = hue - 1;
		b = 0;
	} else if (hue < 3) {
		r = 3 - hue;
		g = 1;
		b = hue - 2;
	} else if (hue < 4) {
		r = hue - 3;
		g = 4 - hue;
		b = 1;
	} else {
		r = 1;
		g = hue - 4;
		b = 1;
	}

	// Return the color as a 16-bit value
	return (((uint16_t)(r * 31)) << 11) | (((uint16_t)(g * 63)) << 5)
			| ((uint16_t)(b * 31));
}
