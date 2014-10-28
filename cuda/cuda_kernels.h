#pragma once

typedef unsigned char Pixel;

extern enum SobelDisplayMode g_SobelDisplayMode;

extern "C" void sobelFilter(Pixel *odata, int iw, int ih); //, enum SobelDisplayMode mode, float fScale);
extern "C" void setupTexture(int iw, int ih, Pixel *data);
