
#include <SDL2/SDL.h>
#include <iostream>
#include <stdlib.h>

int main(int argc, char **args)
{
  // Pointers to our window and surface
  SDL_Surface *winSurface = NULL;
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  // Initialize SDL. SDL_Init will return -1 if it fails.
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
    system("pause");
    // End the program
    return 1;
  }

  // Create our window
  window =
      SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);

  // Make sure creating the window succeeded
  if (!window)
  {
    std::cout << "Error creating window: " << SDL_GetError() << std::endl;
    system("pause");
    // End the program
    return 1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (!renderer)
  {
    std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Surface *image = SDL_LoadBMP("1.bmp");
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  bool running = true;
  // Wait
  while (running)
  {
    SDL_Event evt;

    while (SDL_PollEvent(&evt) != 0)
    {
      switch (evt.type)
      {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:
        SDL_RenderClear(renderer);

        SDL_Rect r;
        int red;
        int green;
        int blue;
        r.x = rand() % 1280;
        r.y = rand() % 720;
        r.w = rand() % (1280 - r.x);
        r.h = rand() % (720 - r.y);
        red = rand() % 255;
        green = rand() % 255;
        blue = rand() % 255;
        switch (evt.key.keysym.sym)
        {
        case SDLK_w:
        case SDLK_a:
        case SDLK_s:
        case SDLK_d:
          SDL_SetTextureColorMod(texture, red, green, blue);
          SDL_RenderCopy(renderer, texture, NULL, &r);
        }
        SDL_RenderPresent(renderer);
        break;
      }
    }
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);

  // Destroy the window. This will also destroy the surface
  SDL_DestroyWindow(window);

  // Quit SDL
  SDL_Quit();

  // End the program
  return 0;
}
