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
int screenHeight = 240;
int bytesPerPixel = 4;
int ticksPerSecond = 60;

Uint64 performanceFrequency = 0;
Uint64 tickIntervalTicks = 0;
Uint64 nextTickTime = 0;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *frameTexture = nullptr;

uint8_t *pixelBuffer = nullptr;
int pixelBufferPitch = 0;

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
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) != 0) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return false;
  }
  performanceFrequency = SDL_GetPerformanceFrequency();
  tickIntervalTicks = performanceFrequency / (Uint64)ticksPerSecond;
  nextTickTime = SDL_GetPerformanceCounter();

  window = SDL_CreateWindow("Moon Child", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_RESIZABLE);
  if (!window) {
    fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());

    return false;
  }
  SDL_RenderSetLogicalSize(renderer, screenWidth, screenHeight);
  SDL_RenderSetVSync(renderer, 1);

  frameTexture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING,
                        screenWidth, screenHeight);
  if (!frameTexture) {
    fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());

    return false;
  }
  SDL_SetTextureBlendMode(frameTexture, SDL_BLENDMODE_NONE);
  
  pixelBufferPitch = screenWidth * bytesPerPixel;
  pixelBuffer = new uint8_t[pixelBufferPitch * screenHeight];
  memset(pixelBuffer, 0, pixelBufferPitch * screenHeight);
	consoleInit(GFX_BOTTOM, NULL);
  SDL_GameController* c = SDL_GameControllerOpen(0);

  return true;
}

void shutdownSDL() {
  delete[] pixelBuffer;
  pixelBuffer = nullptr;

  if (frameTexture) {
    SDL_DestroyTexture(frameTexture);
    frameTexture = nullptr;
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }
  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }
  SDL_Quit();
}

void presentFrame() {
  SDL_UpdateTexture(frameTexture, nullptr, pixelBuffer, pixelBufferPitch);
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, frameTexture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
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
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = false;
      }
      if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.scancode) {
          case SDL_SCANCODE_UP:
            keyDown(SDL_SCANCODE_UP);
            break;
          case SDL_SCANCODE_DOWN:
            keyDown(SDL_SCANCODE_DOWN);
          break;
          case SDL_SCANCODE_LEFT:
            keyDown(SDL_SCANCODE_LEFT);
            break;
          case SDL_SCANCODE_RIGHT:
            keyDown(SDL_SCANCODE_RIGHT);
            break;
          case SDL_SCANCODE_SPACE:
            keyDown(SDL_SCANCODE_SPACE);
            break;
          case SDL_SCANCODE_ESCAPE:
            keyDown(SDL_SCANCODE_ESCAPE);
            break;
          case SDL_SCANCODE_E:
            keyDown(SDL_SCANCODE_E);
            break;
          case SDL_SCANCODE_P:
            keyDown(SDL_SCANCODE_P);
          break;
          case SDL_SCANCODE_RETURN:
            if (e.key.keysym.mod & KMOD_ALT) {
              SDL_SetWindowFullscreen(window, SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
            break;
          default:
            break;
        }
      }
      if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        switch (e.key.keysym.scancode) {
          case SDL_SCANCODE_UP:
            keyUp(SDL_SCANCODE_UP);
            break;
          case SDL_SCANCODE_DOWN:
            keyUp(SDL_SCANCODE_DOWN);
            break;
          case SDL_SCANCODE_LEFT:
            keyUp(SDL_SCANCODE_LEFT);
            break;
          case SDL_SCANCODE_RIGHT:
            keyUp(SDL_SCANCODE_RIGHT);
            break;
          case SDL_SCANCODE_SPACE:
            keyUp(SDL_SCANCODE_SPACE);
            break;
          case SDL_SCANCODE_ESCAPE:
            keyUp(SDL_SCANCODE_ESCAPE);
            break;
          case SDL_SCANCODE_E:
            keyUp(SDL_SCANCODE_E);
            break;
          case SDL_SCANCODE_P:
            keyUp(SDL_SCANCODE_P);
            break;
          default:
            break;
        }
      }
      if (e.type == SDL_JOYBUTTONDOWN ) {
        printf("%d\n",e.cbutton.button);
        switch (e.cbutton.button) {
          case JOY_UP:
            printf("SDL_SCANCODE_UP\n");
            keyDown(SDL_SCANCODE_UP);
            break;
          case JOY_DOWN:
            printf("SDL_SCANCODE_DOWN\n");
            keyDown(SDL_SCANCODE_DOWN);
            break;
          case JOY_LEFT:
            printf("SDL_SCANCODE_LEFT\n");
            keyDown(SDL_SCANCODE_LEFT);
            break;
          case JOY_RIGHT:
            printf("SDL_SCANCODE_RIGHT\n");
            keyDown(SDL_SCANCODE_RIGHT);
            break;
          case JOY_A:
            printf("SDL_SCANCODE_UP\n");
            keyDown(SDL_SCANCODE_UP);
            break;
          case JOY_X:
            printf("SDL_SCANCODE_SPACE\n");
            keyDown(SDL_SCANCODE_SPACE);
            break;
          case JOY_PLUS:
            printf("SDL_SCANCODE_SPACE\n");
            keyDown(SDL_SCANCODE_SPACE);
            break;
          case JOY_L:
            printf("SDL_SCANCODE_ESCAPE\n");
            keyDown(SDL_SCANCODE_ESCAPE);

            break;
          default:
            break;
        }
      }
      if (e.type == SDL_JOYBUTTONUP) {
        switch (e.cbutton.button) {
          case JOY_UP:
            keyUp(SDL_SCANCODE_UP);
            break;
          case JOY_DOWN:
            keyUp(SDL_SCANCODE_DOWN);
            break;
          case JOY_LEFT:
            keyUp(SDL_SCANCODE_LEFT);
            break;
          case JOY_RIGHT:
            keyUp(SDL_SCANCODE_RIGHT);
            break;
          case JOY_A:
            keyUp(SDL_SCANCODE_UP);
            break;
          case JOY_X:
            keyUp(SDL_SCANCODE_SPACE);
            break;
          case JOY_PLUS:
            keyUp(SDL_SCANCODE_SPACE);
            break;
          case JOY_L:
            keyUp(SDL_SCANCODE_ESCAPE);
            break;
          default:
            break;
        }
      }
    }

    syncMouse();

    waitUntilNextTickBoundary();

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
