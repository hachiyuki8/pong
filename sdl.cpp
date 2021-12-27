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
SDL_Texture *rect_texture, *circ_texture, *text_texture;
SDL_Surface *text;
TTF_Font* font;
SDL_Rect rect, circ;
int circ_direction_x = 1;
int circ_direction_y = 1; // 1 for right/down, -1 for left/up
Uint32 lastUpdate;

const Uint8* keys = SDL_GetKeyboardState(NULL);

SDL_Color text_color = { 0, 0, 0 };
bool started = false;
bool ended = false;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int RECT_WIDTH = 240;
const int RECT_HEIGHT = 40;
const int RECT_VELOCITY_H = 1000;
// const int RECT_VELOCITY_V = 600;
const int CIRC_RADIUS = 20;
const int CIRC_VELOCITY_BASELINE = 200;
const int CIRC_VELOCITY_RANGE = 100;

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
  SDL_Rect dest = {(SCREEN_WIDTH - text->w)/2, (SCREEN_HEIGHT - text->h)/2, text->w, text->h};
  SDL_RenderCopy(renderer, text_texture, NULL, &dest);
}

void updateCircle(float dT) {
  int velocity_h = rand() % CIRC_VELOCITY_RANGE + CIRC_VELOCITY_BASELINE;
  int velocity_v = rand() % CIRC_VELOCITY_RANGE + CIRC_VELOCITY_BASELINE;

  circ.y += circ_direction_y * velocity_v * dT;
  if (circ.y > SCREEN_HEIGHT) {
    started = false;
    ended = true;
    return;
  }
  if (circ.y < 0) {
    circ.y = 0;
    circ_direction_y *= -1;
  }

  circ.x += circ_direction_x * velocity_h * dT;
  if (circ.x < 0) {
    circ.x = 0;
    circ_direction_x *= -1;
  }
  if (circ.x > SCREEN_WIDTH - CIRC_RADIUS * 2) {
    circ.x = SCREEN_WIDTH - CIRC_RADIUS * 2;
    circ_direction_x *= -1;
  }

  if (circ.x + CIRC_RADIUS >= rect.x && circ.x + CIRC_RADIUS <= rect.x + RECT_WIDTH) {
    if (circ_direction_y == 1 && circ.y + CIRC_RADIUS * 2 >= rect.y && circ.y <= rect.y) {
      circ.y = rect.y - CIRC_RADIUS * 2;
      circ_direction_y *= -1;
    } else if (circ_direction_y == -1 && circ.y < rect.y + RECT_HEIGHT && circ.y + CIRC_RADIUS * 2 > rect.y + RECT_HEIGHT) {
      circ.y = rect.y + RECT_HEIGHT;
      circ_direction_y *= -1;
    }
  }
  if (circ.y + CIRC_RADIUS >= rect.y && circ.y + CIRC_RADIUS <= rect.y + RECT_HEIGHT) {
    if (circ_direction_x == 1 && circ.x + CIRC_RADIUS * 2 >= rect.x && circ.x <= rect.x) {
      circ.x = rect.x - CIRC_RADIUS * 2;
      circ_direction_x *= -1;
    }
    else if (circ_direction_x == -1 && circ.x <= rect.x + RECT_WIDTH && circ.x + CIRC_RADIUS * 2 >= rect.x + RECT_WIDTH) {
      circ.x = rect.x + RECT_WIDTH;
      circ_direction_x *= -1;
    }
  }
}

bool loop() {
  SDL_Event evt;

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  if (!started) {
    renderStartupText();
  }

  while (SDL_PollEvent(&evt) != 0) {
    switch (evt.type) {
      case SDL_QUIT:
        return false;
      case SDL_KEYDOWN:
        switch (evt.key.keysym.sym) {
          case SDLK_SPACE:
            if (!started) {
              started = true;
              ended = false;
              rect.x = (SCREEN_WIDTH - RECT_WIDTH) / 2;
              rect.y = SCREEN_HEIGHT - RECT_HEIGHT;
              rect.w = RECT_WIDTH;
              rect.h = RECT_HEIGHT;
              
              circ.x = (SCREEN_WIDTH - CIRC_RADIUS) / 2;
              circ.y = 0;
              circ.w = CIRC_RADIUS * 2;
              circ.h = CIRC_RADIUS * 2;
            }
            break;
        }
    }
  }

  Uint32 current = SDL_GetTicks();
  float dT = (current - lastUpdate) / 1000.0f;

  if (keys[SDL_SCANCODE_A]) {
    rect.x = max(0.0f, rect.x - RECT_VELOCITY_H * dT);
    if (circ.y + CIRC_RADIUS >= rect.y && circ.y + CIRC_RADIUS <= rect.y + RECT_HEIGHT) {
      rect.x = max(rect.x, circ.x + CIRC_RADIUS * 2);
    }
  }
  if (keys[SDL_SCANCODE_D]) {
    rect.x = min((float)SCREEN_WIDTH - RECT_WIDTH, rect.x + RECT_VELOCITY_H * dT);
    if (circ.y + CIRC_RADIUS >= rect.y && circ.y + CIRC_RADIUS <= rect.y + RECT_HEIGHT) {
      rect.x = min(rect.x, circ.x - RECT_WIDTH);
    }
  }
  // if (keys[SDL_SCANCODE_W]) {
  //   rect.y = max(0.0f, rect.y - RECT_VELOCITY_V * dT);
  //   if (circ.x + CIRC_RADIUS >= rect.x && circ.x + CIRC_RADIUS <= rect.x + RECT_WIDTH) {
  //     rect.y = max(rect.y, circ.y + CIRC_RADIUS * 2);
  //   }
  // }
  // if (keys[SDL_SCANCODE_S]) {
  //   rect.y = min((float)SCREEN_HEIGHT - RECT_HEIGHT, rect.y + RECT_VELOCITY_V * dT);
  //   if (circ.x + CIRC_RADIUS >= rect.x && circ.x + CIRC_RADIUS <= rect.x + RECT_WIDTH) {
  //     rect.y = min(rect.y, circ.y - RECT_HEIGHT);
  //   }
  // }

  if (!ended) {
    updateCircle(dT);
  } else {
    renderStartupText();
  }

  lastUpdate = current;
  SDL_RenderCopy(renderer, rect_texture, NULL, &rect);
  SDL_RenderCopy(renderer, circ_texture, NULL, &circ);
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

  SDL_Surface *image = SDL_LoadBMP("rect.bmp");
  if (!image) {
    cout << "Error loading image rect.bmp: " << SDL_GetError() << endl;
    return false;
  }

  rect_texture = SDL_CreateTextureFromSurface(renderer, image);
  SDL_FreeSurface(image);
  if (!rect_texture) {
    cout << "Error creating texture: " << SDL_GetError() << endl;
    return false;
  }

  SDL_Surface *image2 = SDL_LoadBMP("circle.bmp");
  if (!image2) {
    cout << "Error loading image circle.bmp: " << SDL_GetError() << endl;
    return false;
  }

  circ_texture = SDL_CreateTextureFromSurface(renderer, image2);
  SDL_FreeSurface(image2);
  if (!circ_texture) {
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

  text = TTF_RenderText_Solid(font, "Press SPACEBAR to start", text_color);
  if (!text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }
  text_texture = SDL_CreateTextureFromSurface(renderer, text);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  return true;
}

void kill() {
  TTF_CloseFont(font);
  SDL_FreeSurface(text);
  SDL_DestroyTexture(text_texture);
  SDL_DestroyTexture(rect_texture);
  SDL_DestroyTexture(circ_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}