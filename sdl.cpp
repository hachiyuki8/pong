#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

bool init();
void kill();
bool loop();

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture, *text_texture;
SDL_Surface *text;
TTF_Font* font;
SDL_Rect rect;
Uint32 lastUpdate;


SDL_Color text_color = { 0, 0, 0 };
bool started = false;

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

void renderStartupText() {
  text = TTF_RenderText_Solid(font, "Press SPACEBAR to start", text_color);
  if (!text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }

  text_texture = SDL_CreateTextureFromSurface(renderer, text);
  SDL_Rect dest = {(SCREEN_WIDTH - text->w)/2, (SCREEN_HEIGHT - text->h)/2, text->w, text->h};
  SDL_RenderCopy(renderer, text_texture, NULL, &dest);

  SDL_DestroyTexture(text_texture);
  SDL_FreeSurface(text);
}

bool loop() {
  SDL_Event evt;

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  if (!started) {
    renderStartupText();
  }

  Uint32 current = SDL_GetTicks();
  float dT = (current - lastUpdate) / 1000.0f;

  const Uint8* keys = SDL_GetKeyboardState(NULL);
  if (keys[SDL_SCANCODE_A]) {
    rect.x = max(0.0f, rect.x - RECT_VELOCITY_H * dT);
  }
  if (keys[SDL_SCANCODE_D]) {
    rect.x = min((float)SCREEN_WIDTH - RECT_WIDTH, rect.x + RECT_VELOCITY_H * dT);
  }
  if (keys[SDL_SCANCODE_W]) {
    rect.y = max(0.0f, rect.y - RECT_VELOCITY_V * dT);
  }
  if (keys[SDL_SCANCODE_S]) {
    rect.y = min((float)SCREEN_HEIGHT - RECT_HEIGHT, rect.y + RECT_VELOCITY_V * dT);
  }

  while (SDL_PollEvent(&evt) != 0) {
    switch (evt.type) {
      case SDL_QUIT:
        return false;
      case SDL_KEYDOWN:
        switch (evt.key.keysym.sym) {
          case SDLK_SPACE:
            started = true;
            rect.x = (SCREEN_WIDTH - RECT_WIDTH) / 2;
            rect.y = SCREEN_HEIGHT - RECT_HEIGHT;
            rect.w = RECT_WIDTH;
            rect.h = RECT_HEIGHT;
            SDL_SetTextureColorMod(texture, 255, 255, 255);
            break;
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
    cout << "Error initializing SDL: " << SDL_GetError() << endl;
    return false;
  }

  window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window)
  {
    cout << "Error creating window: " << SDL_GetError() << endl;
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (!renderer)
  {
    cout << "Error creating renderer: " << SDL_GetError() << endl;
    return false;
  }

  SDL_Surface *image = SDL_LoadBMP("1.bmp");
  if (!image) {
    cout << "Error loading image 1.bmp: " << SDL_GetError() << endl;
    return false;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_FreeSurface(image);
  if (!texture) {
    cout << "Error creating texture: " << SDL_GetError() << endl;
    return false;
  }

  if (TTF_Init() < 0) {
		cout << "Error intializing SDL_ttf: " << TTF_GetError() << endl;
		return false;
	}

  font = TTF_OpenFont("font.ttf", 48);
	if (!font) {
		cout << "Error loading font: " << TTF_GetError() << endl;
		return false;
	}

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  return true;
}

void kill() {
  TTF_CloseFont(font);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}