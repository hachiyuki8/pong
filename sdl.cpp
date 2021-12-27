#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

using namespace std;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *paddle_texture, *ball_texture, *rect_texture, *text_texture;
SDL_Surface *text;
TTF_Font* font;

const Uint8* keys = SDL_GetKeyboardState(NULL);

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const int PADDLE_WIDTH = 240;
const int PADDLE_HEIGHT = 40;
const int PADDLE_XVELOCITY = 600;

const int BALL_RADIUS = 20;
const int BALL_YVELOCITY = 300;
const int BALL_XVELOCITY_BASELINE = 100;
const int BALL_XVELOCITY_RANGE = 400;

const int NUM_ROWS = 5;
const int NUM_COLUMNS = 10;
const int RECT_BORDER = 10;
const int RECT_WIDTH = SCREEN_WIDTH / NUM_COLUMNS - RECT_BORDER;
const int RECT_HEIGHT = 50;

struct Paddle {
  float x, y, w, h, xvelocity;
  Uint32 lastUpdate;
};

struct Ball {
  float x, y, w, h, r, xvelocity, yvelocity, xdirection, ydirection;
  Uint32 lastUpdate;
};

struct Rectangle {
  float x, y, w, h;
};

struct GameState {
  Paddle paddle;
  Ball ball;
  vector<Rectangle> rectangles;
  int state; // 0 for not started, 1 for in progress, -1 for lost, 999 for success
  int numRows;
};

bool init();
void kill();
bool loop(GameState *game);

// initialize game states
void resetGameState(GameState *game);
Paddle initializePaddle();
Ball initializeBall();
Rectangle initializeRectangle(float x, float y);
vector<Rectangle> initializeRectangles(int numRows);

// dealing with collision
float pointDistance(float x1, float y1, float x2, float y2);
bool checkCollision(Ball b, float x, float y, float w, float h); 
bool eliminateBlocks(GameState *game);

// update object states
void updatePaddle(GameState *game);
void updateCircle(GameState *game);

// render
void renderStartupText();
void renderGameState(GameState *game);


/*
 * game
 */
int main(int argc, char **args)
{
  if (!init()) {
		system("pause");
		return 1;
	}

  GameState game;
  game.state = 0;

  while(loop(&game)) {
  }

  // End the program
  kill();
  return 0;
}

bool loop(GameState *game) {
  SDL_Event evt;

  // clear the screen to white
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  if (game->state == 0) {
    renderStartupText();
  }

  // event loop
  while (SDL_PollEvent(&evt) != 0) {
    switch (evt.type) {
      case SDL_QUIT:
        return false;
      case SDL_KEYDOWN:
        switch (evt.key.keysym.sym) {
          // start/restart game
          case SDLK_SPACE:
            resetGameState(game); 
            break;
        }
    }
  }

  updatePaddle(game);
  if (game->state == 1) {
    updateCircle(game);
    renderGameState(game);
  } else {
    renderStartupText();
  }

  SDL_RenderPresent(renderer);
  return true;
}

void renderStartupText() {
  SDL_Rect dest = {(SCREEN_WIDTH - text->w)/2, (SCREEN_HEIGHT - text->h)/2, text->w, text->h};
  SDL_RenderCopy(renderer, text_texture, NULL, &dest);
}

void renderGameState(GameState *game) {
  SDL_Rect p, b;
  p.x = game->paddle.x;
  p.y = game->paddle.y;
  p.w = game->paddle.w;
  p.h = game->paddle.h;
  b.x = game->ball.x;
  b.y = game->ball.y;
  b.w = game->ball.w;
  b.h = game->ball.h;
  for (auto rec: game->rectangles) {
    SDL_Rect r;
    r.x = rec.x;
    r.y = rec.y;
    r.w = rec.w;
    r.h = rec.h;
    SDL_RenderCopy(renderer, rect_texture, NULL, &r);
  }
  SDL_RenderCopy(renderer, paddle_texture, NULL, &p);
  SDL_RenderCopy(renderer, ball_texture, NULL, &b);
}

void updatePaddle(GameState *game) {
  Uint32 current = SDL_GetTicks();
  float dT = (current - game->paddle.lastUpdate) / 1000.0f;

  if (keys[SDL_SCANCODE_A]) {
    game->paddle.x -= game->paddle.xvelocity * dT;
    if (game->paddle.x <= 0 || checkCollision(game->ball, game->paddle.x, game->paddle.y, game->paddle.w, game->paddle.h)) {
      game->paddle.x += game->paddle.xvelocity * dT;
    }
  }
  if (keys[SDL_SCANCODE_D]) {
    game->paddle.x += game->paddle.xvelocity * dT;
    if (game->paddle.x + game->paddle.w >= SCREEN_WIDTH || checkCollision(game->ball, game->paddle.x, game->paddle.y, game->paddle.w, game->paddle.h)) {
      game->paddle.x -= game->paddle.xvelocity * dT;
    }
  }

  game->paddle.lastUpdate = current;
}

void updateCircle(GameState *game) {
  Uint32 current = SDL_GetTicks();
  float dT = (current - game->ball.lastUpdate) / 1000.0f;

  game->ball.xvelocity = rand() % BALL_XVELOCITY_RANGE + BALL_XVELOCITY_BASELINE;

  // move up/down
  game->ball.y += game->ball.ydirection * game->ball.yvelocity * dT;
  if (game->ball.y >= SCREEN_HEIGHT) { // lost
    game->state = -1;
    return;
  }
  if (game->ball.y <= 0 || checkCollision(game->ball, game->paddle.x, game->paddle.y, game->paddle.w, game->paddle.h) || eliminateBlocks(game)) {
    game->ball.y -= game->ball.ydirection * game->ball.yvelocity * dT;
    game->ball.ydirection *= -1;
  }

  // move left/right
  game->ball.x += game->ball.xdirection * game->ball.xvelocity * dT;
  if (game->ball.x <= 0 || game->ball.x + game->ball.w >= SCREEN_WIDTH || checkCollision(game->ball, game->paddle.x, game->paddle.y, game->paddle.w, game->paddle.h) || eliminateBlocks(game)) {
    game->ball.x -= game->ball.xdirection * game->ball.xvelocity * dT;
    game->ball.xdirection *= -1;
  }

  game->ball.lastUpdate = current;
}

float pointDistance(float x1, float y1, float x2, float y2) {
  float x = x1 - x2;
  float y = y1 - y2;
  return sqrt(x * x + y * y);
}

bool checkCollision(Ball b, float x, float y, float w, float h) {
  float cX, cY; // closest points on the other object

  // center of the ball
  float centerX = b.x + b.r;
  float centerY = b.y + b.r;

  // find closest x
  if (centerX < x) {
    cX = x;
  } else if (centerX > x + w) {
    cX = x + w;
  } else {
    cX = centerX;
  }

  // find closest y
  if (centerY < y) {
    cY = y;
  } else if (centerY > y + h) {
    cY = y + h;
  } else {
    cY = centerY;
  }

  return pointDistance(centerX, centerY, cX, cY) <= b.r;
}

bool eliminateBlocks(GameState *game) {
  bool flag = false;
  for (int i = 0; i < game->rectangles.size(); i++) {
    Rectangle& r = game->rectangles[i];
    if (checkCollision(game->ball, r.x, r.y, r.w, r.h)) {
      flag = true;
      game->rectangles.erase(game->rectangles.begin() + i);
    }
  }
  if (!game->rectangles.size()) {
    game->state = 999;
    cout << "SUCCESS" << endl; // TO-DO: add ending screen
  }
  return flag;
}

void resetGameState(GameState *game) {
  if (game->state != 1) {
    game->state = 1;
    game->paddle = initializePaddle();
    game->ball = initializeBall();
    game->numRows = NUM_ROWS;
    game->rectangles = initializeRectangles(game->numRows);
  }
}

Paddle initializePaddle() {
  Paddle p;
  p.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
  p.y = SCREEN_HEIGHT - PADDLE_HEIGHT;
  p.w = PADDLE_WIDTH;
  p.h = PADDLE_HEIGHT;
  p.xvelocity = PADDLE_XVELOCITY;
  p.lastUpdate = SDL_GetTicks();
  return p;
}

Ball initializeBall() {
  Ball b;
  b.x = (SCREEN_WIDTH - BALL_RADIUS) / 2;
  b.y = SCREEN_HEIGHT - PADDLE_HEIGHT - BALL_RADIUS * 2;
  b.w = BALL_RADIUS * 2;
  b.h = BALL_RADIUS * 2;
  b.r = BALL_RADIUS;
  b.xvelocity = BALL_XVELOCITY_BASELINE;
  b.yvelocity = BALL_YVELOCITY;
  b.xdirection = 1; // 1 for right, -1 for left
  b.ydirection = -1; // 1 for down, -1 for up
  b.lastUpdate = SDL_GetTicks();
  return b;
}

Rectangle initializeRectangle(float x, float y) {
  Rectangle r;
  r.x = x;
  r.y = y;
  r.w = RECT_WIDTH;
  r.h = RECT_HEIGHT;
  return r;
}

vector<Rectangle> initializeRectangles(int numRows) {
  vector<Rectangle> rs;
  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < SCREEN_WIDTH / (RECT_WIDTH + RECT_BORDER); col++) {
      Rectangle r = initializeRectangle(col * (RECT_WIDTH + RECT_BORDER), row * (RECT_HEIGHT + RECT_BORDER));
        rs.push_back(r);
    }
  }
  return rs;
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

  // paddle
  SDL_Surface *image = SDL_LoadBMP("images/paddle.bmp");
  if (!image) {
    cout << "Error loading image paddle.bmp: " << SDL_GetError() << endl;
    return false;
  }
  paddle_texture = SDL_CreateTextureFromSurface(renderer, image);
  SDL_FreeSurface(image);
  if (!paddle_texture) {
    cout << "Error creating texture: " << SDL_GetError() << endl;
    return false;
  }

  // ball
  SDL_Surface *image2 = SDL_LoadBMP("images/circle.bmp");
  if (!image2) {
    cout << "Error loading image circle.bmp: " << SDL_GetError() << endl;
    return false;
  }
  ball_texture = SDL_CreateTextureFromSurface(renderer, image2);
  SDL_FreeSurface(image2);
  if (!ball_texture) {
    cout << "Error creating texture: " << SDL_GetError() << endl;
    return false;
  }

  // rectangles
  SDL_Surface *image3 = SDL_LoadBMP("images/rectangle.bmp");
  if (!image3) {
    cout << "Error loading image rectangle.bmp: " << SDL_GetError() << endl;
    return false;
  }
  rect_texture = SDL_CreateTextureFromSurface(renderer, image3);
  SDL_FreeSurface(image3);
  if (!rect_texture) {
    cout << "Error creating texture: " << SDL_GetError() << endl;
    return false;
  }

  // startup text
  if (TTF_Init() < 0) {
		cout << "Error intializing SDL_ttf: " << TTF_GetError() << endl;
		return false;
	}
  font = TTF_OpenFont("font.ttf", 48);
	if (!font) {
		cout << "Error loading font: " << TTF_GetError() << endl;
		return false;
	}
  SDL_Color text_color = { 0, 0, 0 };
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
  SDL_DestroyTexture(paddle_texture);
  SDL_DestroyTexture(ball_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}