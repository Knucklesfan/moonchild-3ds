#pragma once

#include <SDL.h>

#include <cstdint>

void gameTick(uint8_t *pixels, int width, int height, int pitch, Uint8 *keyboardState);
void keyUp(int key);
void keyDown(int key);
void initMoonChild(unsigned char *pixelBuffer, int width, int height);
void resetProgress();
void enableCheat();