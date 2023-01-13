#pragma once
#include <complex>
#include "ili9341.h"
#include "color.h"

void drawFractal(double x1, double x2, double y1, double y2,
		int maxIterations) {
	int const HEIGHT = lcdGetHeight();
	int const WIDTH = lcdGetWidth();
	lcdSetWindow(0, 0, WIDTH - 1, HEIGHT - 1);
	// Iterate over each pixel

	double x_step = (x2 - x1) / WIDTH;
	double y_step = (y2 - y1) / HEIGHT;

	for (int y = 0; y < HEIGHT; y++) {
		std::complex<double> c(x1, y1 + y * y_step);
		for (int x = 0; x < WIDTH; x++) {
			// Map the pixel coordinates to the complex plane
			std::complex<double> z(0.0, 0.0);

			// Iterate until we escape the circle of radius 2 centered at the origin
			int iterations = 0;
			for (; std::abs(z) < 2.0 && iterations < maxIterations;
					++iterations) {
				z = z * z + c;
			}

			// Set the pixel color based on the number of iterations
			LCD_DataWrite(mapColor((float )iterations / maxIterations));
			c += x_step;
		}
	}
}

void test() {
	int const HEIGHT = lcdGetHeight();
	int const WIDTH = lcdGetWidth();
	lcdSetWindow(0, 0, WIDTH - 1, HEIGHT - 1);

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			LCD_DataWrite(mapColor((float )y / HEIGHT));
		}
	}
}
