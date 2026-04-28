// #include "globals.hpp"
#include "Audio.h"
#include "Game.h"
#include "Util.h"
#include <SDL.h>

#include <filesystem>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#define _IN_MAIN
#include "frm_int.hpp"

// #include "livesplit.hpp"
// #include "zed_net.h"

namespace {

int screenWidth = 400;
int realHeight = 240;
int screenHeight = 480; //this is the height to render at for both the touch screen and the regular screen
int bytesPerPixel = 4;
int ticksPerSecond = 60;

Uint64 performanceFrequency = 0;
Uint64 tickIntervalTicks = 0;
Uint64 nextTickTime = 0;

// SDL_Window *window = nullptr;
// SDL_Renderer *renderer = nullptr;
// SDL_Texture *frameTexture = nullptr;

u8 *pixelBuffer = nullptr;
// u8 *rightBuffer = nullptr;
// u8 *bottomBuffer = nullptr;

int pixelBufferPitch = 0;
// int bottomBufferPitch = 0;
#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define JOY_PLUS  10
#define JOY_L 20
#define JOY_LEFT  5
#define JOY_UP    6
#define JOY_RIGHT 4
#define JOY_DOWN  7
enum {
  OFF_A = 0, OFF_B, OFF_SELECT, OFF_START,
  OFF_DRIGHT, OFF_DLEFT, OFF_DUP, OFF_DDOWN,
  OFF_R, OFF_L, OFF_X, OFF_Y,
  OFF_12, OFF_13, OFF_ZL, OFF_ZR,
  OFF_16, OFF_17, OFF_18, OFF_19,
  OFF_TOUCH, OFF_21, OFF_22, OFF_23,
  OFF_CSTICK_RIGHT, OFF_CSTICK_LEFT, OFF_CSTICK_UP, OFF_CSTICK_DOWN,
  OFF_CPAD_RIGHT, OFF_CPAD_LEFT, OFF_CPAD_UP, OFF_CPAD_DOWN
};

void waitUntilNextTickBoundary() {
  for (;;) {
    Uint64 now = SDL_GetPerformanceCounter();
    if (now >= nextTickTime) {
      break;
    }
    Uint64 remaining = nextTickTime - now;
    Uint64 remainingNs = (remaining * 1000000000ULL) / performanceFrequency;
    if (remainingNs > 2000000ULL) {
      SDL_Delay(1);
    }
  }
}

void advanceTickSchedule() {
  Uint64 now = SDL_GetPerformanceCounter();
  nextTickTime += tickIntervalTicks;
  if (now > nextTickTime + tickIntervalTicks) {
    nextTickTime = now + tickIntervalTicks;
  }
}

bool initSDL() {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    performanceFrequency = SDL_GetPerformanceFrequency();
    tickIntervalTicks = performanceFrequency / (Uint64)ticksPerSecond;
    nextTickTime = SDL_GetPerformanceCounter();

    pixelBufferPitch = screenWidth * bytesPerPixel;
    // bottomBufferPitch = bottomWidth * bytesPerPixel;
    
    pixelBuffer = new uint8_t[pixelBufferPitch * screenHeight];
    // rightBuffer = new uint8_t[pixelBufferPitch * screenHeight];
    // bottomBuffer = new uint8_t[bottomBufferPitch * screenHeight];

    memset(pixelBuffer, 0, pixelBufferPitch * screenHeight);
    // memset(rightBuffer, 0, pixelBufferPitch * screenHeight);
    // memset(bottomBuffer, 0, bottomBufferPitch * screenHeight);

    gfxInitDefault();
    // gfxSet3D(true); // Activate stereoscopic 3D

	  gfxSetDoubleBuffering(GFX_TOP, true);
	  gfxSetDoubleBuffering(GFX_BOTTOM, true);

    // consoleInit(GFX_BOTTOM, NULL);

    return true;
}

void shutdownSDL() {
  delete[] pixelBuffer;
  pixelBuffer = nullptr;

  // if (frameTexture) {
  //   SDL_DestroyTexture(frameTexture);
  //   frameTexture = nullptr;
  // }
  // if (renderer) {
  //   SDL_DestroyRenderer(renderer);
  //   renderer = nullptr;
  // }
  // if (window) {
  //   SDL_DestroyWindow(window);
  //   window = nullptr;
  // }
  SDL_Quit();
}
void renderFb(u8* pixelBuffer, gfx3dSide_t screen, gfxScreen_t position, int width, int height) {
  // u8* btm = gfxGetFramebuffer(GFX_BOTTOM, screen, NULL, NULL);
  // u8* top = gfxGetFramebuffer(GFX_TOP, screen, NULL, NULL);

  // const int inStride  = screenWidth * 4;
  // const int outStride = realHeight * 3;
  // for (int x = 0; x < width; ++x) {
  //     unsigned char *out = fb + (x+(position==GFX_BOTTOM?(screenWidth-width)/2:0)) * outStride;
  //     const unsigned char *inRow = pixelBuffer + ((position==GFX_BOTTOM?(screenHeight):realHeight) - 1) * inStride + (x+(position==GFX_BOTTOM?(realHeight):0)) * 4;
  //     for (int y = 0; y < realHeight; ++y) {
  //         out[0] = inRow[0];
  //         out[1] = inRow[1];
  //         out[2] = inRow[2];
  //         out += 3;
  //         inRow -= inStride; // move up one input row since we're at the bottom
  //     }
  // }  // memcpy(fb, pixelBuffer, screenWidth * bytesPerPixel * screenHeight);
  // for(int x= 0; x < screenWidth; x++) {
  //   int othery = 0;
  //   for(int y = screenHeight; y > 0; y--) {
  //     if(y > realHeight) {
  //       if(x > (screenWidth-320)/2 && x-(screenWidth-320)/2 < 320) {
  //         btm[(x-(screenWidth-320)/2)*(realHeight*3)+(othery)*3] = pixelBuffer[(y*(screenWidth*4))+(x)*4]; 
  //         btm[(x-(screenWidth-320)/2)*(realHeight*3)+(othery)*3+1] = pixelBuffer[y*(screenWidth*4)+(x)*4+1]; 
  //         btm[(x-(screenWidth-320)/2)*(realHeight*3)+(othery)*3+2] = pixelBuffer[y*(screenWidth*4)+(x)*4+2];
  //       }
  //     }
  //     else {
  //       top[x*(realHeight*3)+(othery%realHeight)*3] = pixelBuffer[(y*(screenWidth*4))+x*4]; 
  //       top[x*(realHeight*3)+(othery%realHeight)*3+1] = pixelBuffer[y*(screenWidth*4)+x*4+1]; 
  //       top[x*(realHeight*3)+(othery%realHeight)*3+2] = pixelBuffer[y*(screenWidth*4)+x*4+2];
  //     }
  //     othery++;
  //   }
  // }
}
void presentFrame() {
  // renderFb(rightBuffer,GFX_RIGHT,GFX_TOP, screenWidth, screenHeight);
  // renderFb(pixelBuffer,GFX_LEFT,GFX_TOP, screenWidth, realHeight);
  // renderFb(pixelBuffer,GFX_LEFT,GFX_BOTTOM, 320, realHeight);

  gfxFlushBuffers();
  gfxSwapBuffers();

  //Wait for VBlank
  gspWaitForVBlank();

}

void syncMouse() {
  static int prevLeft = 0;
  int mx = 0;
  int my = 0;
  Uint32 buttons = SDL_GetMouseState(&mx, &my);
  g_MouseXCurrent = mx;
  g_MouseYCurrent = my;
  int left = (buttons & SDL_BUTTON_LMASK) ? 1 : 0;
  g_MouseFlg = left;
  g_MouseActualFlg = left;
  if (left != 0 && prevLeft == 0) {
    g_MouseXDown = mx;
    g_MouseYDown = my;
  }
  prevLeft = left;
}

}  // namespace

int SDL_main(int argc, char **argv) {
  Result rc = romfsInit();

  printf("initializing game");
  if (!initSDL()) {
    printf("unable to init sdl");
    shutdownSDL();
    return 1;
  }

  if (!initAudio()) {
    printf("unable to init audio");

    shutdownAudio();
    shutdownSDL();
    return 1;

  }
  // if (zed_net_init() < 0) {
  //   fprintf(stderr, "zed_net init failed: %s\n", zed_net_get_error());
  // }
  printf("initializing moonchild\n");
	if (rc)
		printf("romfsInit: %08lX\n", rc);
	else
	{
    printf("romfs initialized successfully\n");
	FILE* f = fopen("romfs:/mc.txt", "r");
	if (f)
	{
		char mystring[100];
		while (fgets(mystring, sizeof(mystring), f))
		{
			int a = strlen(mystring);
			if (mystring[a-1] == '\n')
			{
				mystring[a-1] = 0;
				if (mystring[a-2] == '\r')
					mystring[a-2] = 0;
			}
			puts(mystring);
		}
		fclose(f);
	}

    printf("dir closed\n");

  }

  initMoonChild(pixelBuffer, screenWidth, screenHeight);

  bool running = true;
  while (running) {
    // printf("looping");
    // SDL_Event e;
    // while (SDL_PollEvent(&e)) {
    //   if (e.type == SDL_QUIT) {
    //     running = false;
    //   }
    //   if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
    //     switch (e.key.keysym.scancode) {
    //       case SDL_SCANCODE_UP:
    //         keyDown(SDL_SCANCODE_UP);
    //         break;
    //       case SDL_SCANCODE_DOWN:
    //         keyDown(SDL_SCANCODE_DOWN);
    //       break;
    //       case SDL_SCANCODE_LEFT:
    //         keyDown(SDL_SCANCODE_LEFT);
    //         break;
    //       case SDL_SCANCODE_RIGHT:
    //         keyDown(SDL_SCANCODE_RIGHT);
    //         break;
    //       case SDL_SCANCODE_SPACE:
    //         keyDown(SDL_SCANCODE_SPACE);
    //         break;
    //       case SDL_SCANCODE_ESCAPE:
    //         keyDown(SDL_SCANCODE_ESCAPE);
    //         break;
    //       case SDL_SCANCODE_E:
    //         keyDown(SDL_SCANCODE_E);
    //         break;
    //       case SDL_SCANCODE_P:
    //         keyDown(SDL_SCANCODE_P);
    //       break;
    //       case SDL_SCANCODE_RETURN:
    //         if (e.key.keysym.mod & KMOD_ALT) {
    //           // SDL_SetWindowFullscreen(window, SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
    //         }
    //         break;
    //       default:
    //         break;
    //     }
    //   }
    //   if (e.type == SDL_KEYUP && e.key.repeat == 0) {
    //     switch (e.key.keysym.scancode) {
    //       case SDL_SCANCODE_UP:
    //         keyUp(SDL_SCANCODE_UP);
    //         break;
    //       case SDL_SCANCODE_DOWN:
    //         keyUp(SDL_SCANCODE_DOWN);
    //         break;
    //       case SDL_SCANCODE_LEFT:
    //         keyUp(SDL_SCANCODE_LEFT);
    //         break;
    //       case SDL_SCANCODE_RIGHT:
    //         keyUp(SDL_SCANCODE_RIGHT);
    //         break;
    //       case SDL_SCANCODE_SPACE:
    //         keyUp(SDL_SCANCODE_SPACE);
    //         break;
    //       case SDL_SCANCODE_ESCAPE:
    //         keyUp(SDL_SCANCODE_ESCAPE);
    //         break;
    //       case SDL_SCANCODE_E:
    //         keyUp(SDL_SCANCODE_E);
    //         break;
    //       case SDL_SCANCODE_P:
    //         keyUp(SDL_SCANCODE_P);
    //         break;
    //       default:
    //         break;
    //     }
    //   }
    //   if (e.type == SDL_JOYBUTTONDOWN ) {
    //     printf("%d\n",e.cbutton.button);
    //     switch (e.cbutton.button) {
    //       case JOY_UP:
    //         printf("SDL_SCANCODE_UP\n");
    //         keyDown(SDL_SCANCODE_UP);
    //         break;
    //       case JOY_DOWN:
    //         printf("SDL_SCANCODE_DOWN\n");
    //         keyDown(SDL_SCANCODE_DOWN);
    //         break;
    //       case JOY_LEFT:
    //         printf("SDL_SCANCODE_LEFT\n");
    //         keyDown(SDL_SCANCODE_LEFT);
    //         break;
    //       case JOY_RIGHT:
    //         printf("SDL_SCANCODE_RIGHT\n");
    //         keyDown(SDL_SCANCODE_RIGHT);
    //         break;
    //       case JOY_A:
    //         printf("SDL_SCANCODE_UP\n");
    //         keyDown(SDL_SCANCODE_UP);
    //         break;
    //       case JOY_X:
    //         printf("SDL_SCANCODE_SPACE\n");
    //         keyDown(SDL_SCANCODE_SPACE);
    //         break;
    //       case JOY_PLUS:
    //         printf("SDL_SCANCODE_SPACE\n");
    //         keyDown(SDL_SCANCODE_SPACE);
    //         break;
    //       case JOY_L:
    //         printf("SDL_SCANCODE_ESCAPE\n");
    //         keyDown(SDL_SCANCODE_ESCAPE);

    //         break;
    //       default:
    //         break;
    //     }
    //   }
    //   if (e.type == SDL_JOYBUTTONUP) {
    //     switch (e.cbutton.button) {
    //       case JOY_UP:
    //         keyUp(SDL_SCANCODE_UP);
    //         break;
    //       case JOY_DOWN:
    //         keyUp(SDL_SCANCODE_DOWN);
    //         break;
    //       case JOY_LEFT:
    //         keyUp(SDL_SCANCODE_LEFT);
    //         break;
    //       case JOY_RIGHT:
    //         keyUp(SDL_SCANCODE_RIGHT);
    //         break;
    //       case JOY_A:
    //         keyUp(SDL_SCANCODE_UP);
    //         break;
    //       case JOY_X:
    //         keyUp(SDL_SCANCODE_SPACE);
    //         break;
    //       case JOY_PLUS:
    //         keyUp(SDL_SCANCODE_SPACE);
    //         break;
    //       case JOY_L:
    //         keyUp(SDL_SCANCODE_ESCAPE);
    //         break;
    //       default:
    //         break;
    //     }
    //   }
    // }

    // syncMouse();
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();
		//hidKeysHeld returns information about which buttons have are held down in this frame
		u32 kHeld = hidKeysHeld();
		//hidKeysUp returns information about which buttons have been just released
		u32 kUp = hidKeysUp();

    if (kDown & (1u << OFF_DUP) || kDown & (1u << OFF_CPAD_UP)) {
      printf("SDL_SCANCODE_UP\n");
      keyDown(SDL_SCANCODE_UP);
    } else if (kDown & (1u << OFF_DDOWN) || kDown & (1u << OFF_CPAD_DOWN)) {
      printf("SDL_SCANCODE_DOWN\n");
      keyDown(SDL_SCANCODE_DOWN);
    } else if (kDown & (1u << OFF_DLEFT) || kDown & (1u << OFF_CPAD_LEFT)) {
      printf("SDL_SCANCODE_LEFT\n");
      keyDown(SDL_SCANCODE_LEFT);
    } else if (kDown & (1u << OFF_DRIGHT) || kDown & (1u << OFF_CPAD_RIGHT)) {
      printf("SDL_SCANCODE_RIGHT\n");
      keyDown(SDL_SCANCODE_RIGHT);
    } else if (kDown & ((1u << OFF_B) | (1u << OFF_A))) {
      printf("SDL_SCANCODE_UP\n");
      keyDown(SDL_SCANCODE_UP);
    } else if (kDown & ((1u << OFF_X) | (1u << OFF_Y))) {
      printf("SDL_SCANCODE_SPACE\n");
      keyDown(SDL_SCANCODE_SPACE);
    } else if (kDown & (1u << OFF_START)) {
      printf("SDL_SCANCODE_SPACE\n");
      keyDown(SDL_SCANCODE_SPACE);
    } else if (kDown & (1u << OFF_TOUCH)) {
      printf("SDL_SCANCODE_ESCAPE\n");
      keyDown(SDL_SCANCODE_ESCAPE);
    }

    /* Key-up handling (map bit -> keyUp), including C-stick */
    if (kUp & ((1u << OFF_DUP) | (1u << OFF_CPAD_UP))) {
      keyUp(SDL_SCANCODE_UP);
    }
    if (kUp & ((1u << OFF_DDOWN) | (1u << OFF_CPAD_DOWN))) {
      keyUp(SDL_SCANCODE_DOWN);
    }
    if (kUp & ((1u << OFF_DLEFT) | (1u << OFF_CPAD_LEFT))) {
      keyUp(SDL_SCANCODE_LEFT);
    }
    if (kUp & ((1u << OFF_DRIGHT) | (1u << OFF_CPAD_RIGHT))) {
      keyUp(SDL_SCANCODE_RIGHT);
    }
    if (kUp & ((1u << OFF_B) | (1u << OFF_A))) {
      keyUp(SDL_SCANCODE_UP);
    }
    if (kUp & ((1u << OFF_X) | (1u << OFF_Y))) {
      keyUp(SDL_SCANCODE_SPACE);
    }
    if (kUp & (1u << OFF_START)) {
      keyUp(SDL_SCANCODE_SPACE);
    }
    if (kUp & (1u << OFF_TOUCH)) {
      keyUp(SDL_SCANCODE_ESCAPE);
    }    
    // waitUntilNextTickBoundary();

    int keyCount = 0;
    Uint8 *keyboardState = (Uint8 *)SDL_GetKeyboardState(&keyCount);
    (void)keyCount;

    gameTick(pixelBuffer, screenWidth, screenHeight, pixelBufferPitch, keyboardState);

    presentFrame();

    advanceTickSchedule();
  }

  // livesplit::Quit();
  // zed_net_shutdown();
  shutdownAudio();
  shutdownSDL();
  return 0;
}
