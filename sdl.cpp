
#include <SDL2/SDL.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

bool init();
void kill();
bool loop();

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Rect rect;
Uint32 lastUpdate;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int RECT_WIDTH = 240;
const int RECT_HEIGHT = 40;
const int RECT_VELOCITY_H = 1200;
const int RECT_VELOCITY_V = 675;

int main(int argc, char **args)
{
  if (!init()) {
		system("pause");
		return 1;
	}

  lastUpdate = SDL_GetTicks();
  while(loop()) {
  }

  // End the program
  kill();
  return 0;
}

bool loop() {
  SDL_Event evt;

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  Uint32 current = SDL_GetTicks();
  float dT = (current - lastUpdate) / 1000.0f;

  while (SDL_PollEvent(&evt) != 0) {
    switch (evt.type) {
      case SDL_QUIT:
        return false;
      case SDL_KEYDOWN:
        switch (evt.key.keysym.sym) {
          case SDLK_SPACE:
            rect.x = (SCREEN_WIDTH - RECT_WIDTH) / 2;
            rect.y = SCREEN_HEIGHT - RECT_HEIGHT;
            rect.w = RECT_WIDTH;
            rect.h = RECT_HEIGHT;
            SDL_SetTextureColorMod(texture, 255, 255, 255);
            break;
          case SDLK_a:
            rect.x = max(0.0f, rect.x - RECT_VELOCITY_H * dT);
            break;
          case SDLK_d:
            rect.x = min((float)SCREEN_WIDTH - RECT_WIDTH, rect.x + RECT_VELOCITY_H * dT);
            break;
          // case SDLK_w:
          //   rect.y = max(0.0f, rect.y - RECT_VELOCITY_V * dT);
          //   break;
          // case SDLK_s:
          //   rect.y = min((float)SCREEN_HEIGHT - RECT_HEIGHT, rect.y + RECT_VELOCITY_V * dT);
          //   break;
        }
    }

  }

  lastUpdate = current;
  SDL_RenderCopy(renderer, texture, NULL, &rect);
  SDL_RenderPresent(renderer);

  return true;
}

bool init() {
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
    return false;
  }

  window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window)
  {
    std::cout << "Error creating window: " << SDL_GetError() << std::endl;
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (!renderer)
  {
    std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
    return false;
  }

  SDL_Surface *image = SDL_LoadBMP("1.bmp");
  if (!image) {
    std::cout << "Error loading image 1.bmp: " << SDL_GetError() << std::endl;
    return false;
  }

  // texture = SDL_CreateTextureFromSurface(renderer, image);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 1024, 1024);
  SDL_FreeSurface(image);
  if (!texture) {
    std::cout << "Error creating texture: " << SDL_GetError() << std::endl;
    return false;
  }

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
  return true;
}

void kill() {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
}