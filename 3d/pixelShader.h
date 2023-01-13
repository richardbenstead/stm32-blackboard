#pragma once

#include <math.h>
#include "linalg.h"

// Constants for the Perlin noise algorithm
const int p[512] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68,
    175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
    102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109,
    198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
    189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108,
    110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235,
    249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114,
    67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};


Vec4d taylorInvSqrt(Vec4d r)
{
    return 1.79284291400159 - 0.85373472095314 * r.array();
}

/*
Vec2d fade(Vec2d t)
{
    return t * t * t * (t * ((t * 6.0).array() - 15.0) + 10.0);
}
*/

Vec4d permute(Vec4d x)
{
    Vec4d y;
    y[0] = p[int(x(0)) % 256];
    y[1] = p[int(x(1)) % 256];
    y[2] = p[int(x(2)) % 256];
    y[3] = p[int(x(3)) % 256];
    return y;
}

float cnoise(Vec2d P)
{
    Vec4d Pi = Vec4d{floor(P[0]), floor(P[1]), floor(P[0]), floor(P[1])} + Vec4d{0.0, 0.0, 1.0, 1.0};
    Vec4d Pf = Vec4d{floor(P[0]), floor(P[1]), floor(P[0]), floor(P[1])} - Vec4d{0.0, 0.0, 1.0, 1.0};
    Pi = Pi.array().unaryExpr([](double x) { return (double)((int)x % 289); });

    Vec4d ix = {Pi[0], Pi[2], Pi[0], Pi[2]};
    Vec4d iy = {Pi[1], Pi[1], Pi[3], Pi[3]};
    Vec4d fx = {Pf[0], Pf[2], Pf[0], Pf[2]};
    Vec4d fy = {Pf[1], Pf[1], Pf[3], Pf[3]};
    Vec4d i = permute(permute(ix) + iy);

    auto fract = [](Vec4d x) {
    	return x - Vec4d(x.array().floor());
    };
    Vec4d gx = (fract(i * (1.0 / 41.0)) * 2.0).array() - 1.0;
    Vec4d gy = gx.array().abs() - 0.5;
    Vec4d tx = (gx.array() + 0.5).array().floor();
    gx = gx - tx;
    Vec2d g00 = Vec2d(gx[0], gy[0]);
    Vec2d g10 = Vec2d(gx[1], gy[1]);
    Vec2d g01 = Vec2d(gx[2], gy[2]);
    Vec2d g11 = Vec2d(gx[3], gy[3]);
    Vec4d norm = taylorInvSqrt(Vec4d(g00.dot(g00), g01.dot(g01), g10.dot(g10), g11.dot(g11)));
    g00 *= norm[0];
    g01 *= norm[1];
    g10 *= norm[2];
    g11 *= norm[3];
    float n00 = g00.dot(Vec2d(fx[0], fy[0]));
    float n10 = g10.dot(Vec2d(fx[1], fy[1]));
    float n01 = g01.dot(Vec2d(fx[2], fy[2]));
    float n11 = g11.dot(Vec2d(fx[3], fy[3]));
    Vec2d fade_xy = {1, 0.9999};//fade(Vec2d{Pf[0], Pf[1]});
    Vec2d n_x = lerp(Vec2d(n00, n01), Vec2d(n10, n11), fade_xy[0]);
    float n_xy = lerp(n_x[0], n_x[1], fade_xy[1]);
    return 2.3 * n_xy;
}

