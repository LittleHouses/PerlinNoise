#include <random>
#include <ctime>
#include <iostream>
#include <math.h>
#include "FreeImage.h"

const int gridDef = 16;
const int width = 512;
const int height = 512;
const int gradHeight = height / gridDef;
const int gradWidth = width / gridDef;
float gradient[gradHeight][gradWidth][2];

//Clamping and Hermite Interpolation. 
float clamp(float x, float lowerlim, float upperlim) {
	if (x < lowerlim) x = lowerlim;
	if (x > upperlim) x = upperlim;
	return x;
}
float smoothstep(float edge0, float edge1, float x) {
	x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
	return x * x * (3 - 2 * x);
}

float fade(float edge0, float edge1, float x) {
	x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
	return x * x * x * (10 - 15 * x + 6 * x * x);
}

void initRandom() {
	std::srand(time(NULL) * time(NULL) + 3.14);
}
float getRandom() {
	int parity = (float)std::rand() / (float)RAND_MAX > 0.5 ? -1 : 1;
	return parity * (float)std::rand() / (float)RAND_MAX;
}

void setValue(RGBQUAD* color, float value) {
	color->rgbBlue = value * 255;
	color->rgbRed = value * 255;
	color->rgbGreen = value * 255;
}

float lerp(float a, float b, float w) {	
	return (1.0f - w) * a + w * b;
}

void normalize(float* vector) {
	float norm = std::sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
	vector[0] /= norm;
	vector[1] /= norm;
}

float dotGridGradient(int gx, int gy, int px, int py) {
	
	float dx = (float)px - (float)((gx + (gridDef / 2)) * gridDef);
	float dy = (float)py - (float)((gy + (gridDef / 2)) * gridDef);
	float vec[2] = { dx, dy };
	normalize(vec);
	return vec[0] * gradient[gy][gx][0] + vec[1] * gradient[gy][gx][1];
}

float perlin(int x, int y, int gridDef) {
	int x0 = x / gridDef;
	int x1 = x0 + 1;
	int y0 = y / gridDef;
	int y1 = y0 + 1;

	float sx = x % gridDef / (float)gridDef;
	float sy = y % gridDef / (float)gridDef;

	float n0 = dotGridGradient(x0, y0, x, y);

	float n1 = dotGridGradient(x1, y0, x, y);
	float ix0 = lerp(n0, n1, sx);

	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	float ix1 = lerp(n0, n1, sx);

	return lerp(ix0, ix1, sy);
}

int main() {

	initRandom();

	const int BPP = 24; // 8 bit per color -> [0,255]

	FIBITMAP* bitmap = FreeImage_Allocate(width, height, BPP);
	RGBQUAD color;

	for (int i = 0; i < gradHeight; ++i) {
		for (int j = 0; j < gradWidth; ++j) {
			float vec[2] = { getRandom(), getRandom() };
			normalize(vec);
			gradient[i][j][0] = vec[0];
			gradient[i][j][1] = vec[1];
		}
	}

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			float value = perlin(i, j, gridDef);
			value = (value + 1) / 2;
			value = smoothstep(0, 1, value);
			setValue(&color, value);
			FreeImage_SetPixelColor(bitmap, j, i, &color);
		}
	}


	FreeImage_Save(FIF_PNG, bitmap, "perlin.png", 0);

	FreeImage_DeInitialise();

}
