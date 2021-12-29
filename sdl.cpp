#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

using namespace std;

SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font, *help_font;
SDL_Texture *paddle_texture, *ball_texture, *rect_texture, *text_texture,
    *end_text_texture, *help_text_texture;
SDL_Surface *text;
// game 1
SDL_Surface *lose_text, *win_text, *help_text;
// game 2
SDL_Surface *win1_text, *win2_text, *help1_text, *help2_text;

const Uint8 *keys = SDL_GetKeyboardState(NULL);
int curGame = 0; // 1 for game 1, -1 for game 2

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// game 1
const int PADDLE_WIDTH = 240;
const int PADDLE_HEIGHT = 30;
const int PADDLE_XVELOCITY = 600;

const int BALL_RADIUS = 15;
const int BALL_YVELOCITY = 400;
const int BALL_XVELOCITY_BASELINE = 200;
const int BALL_XVELOCITY_RANGE = 400;

const int NUM_ROWS = 3;
const int NUM_COLUMNS = 10;
const int RECT_BORDER = 10;
const int RECT_WIDTH = SCREEN_WIDTH / NUM_COLUMNS - RECT_BORDER;
const int RECT_HEIGHT = 50;

// game 2
const int PADDLE2_WIDTH = 30;
const int PADDLE2_HEIGHT = 180;
const int PADDLE2_YVELOCITY = 360;

const int BALL2_RADIUS = 15;
const int BALL2_XVELOCITY = 600;
const int BALL2_YVELOCITY_BASELINE = 100;
const int BALL2_YVELOCITY_RANGE = 200;

struct Paddle {
  float x, y, w, h, xvelocity, yvelocity;
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
  int state; // 0 for not started, 1 for in progress, -1 for lost, 999 for won
  int numRows;
};

struct GameState2 {
  Paddle paddle1;
  Paddle paddle2;
  Ball ball;
  int state; // 0 for not started, 1 for in progress, -1 for player 1 won, -2
             // for player 2 won
};

bool init();
void kill();
bool loop(GameState *game, GameState2 *game2);

// initialize game states
// game 1
void resetGameState(GameState *game);
Paddle initializePaddle(GameState *game);
Ball initializeBall(GameState *game);
Rectangle initializeRectangle(float x, float y);
vector<Rectangle> initializeRectangles(int numRows);
// game 2
void resetGameState(GameState2 *game);
pair<Paddle, Paddle> initializePaddles(GameState2 *game);
Ball initializeBall(GameState2 *game);

// dealing with collision
float pointDistance(float x1, float y1, float x2, float y2);
pair<float, float> checkCollision(Ball b, float x, float y, float w, float h);
bool eliminateBlocks(GameState *game);

// update object states
// game 1
void updatePaddle(GameState *game);
void updateCircle(GameState *game);
// game 2
void updatePaddle(GameState2 *game);
void updateCircle(GameState2 *game);

// render
void renderStartupText();
// game 1
void renderHelpText();
void renderEndText(SDL_Surface *text);
void renderGameState(GameState *game);
// game 2
void renderHelpText2();
void renderEndText2(SDL_Surface *text);
void renderGameState(GameState2 *game);

/*
 * game
 */
int main(int argc, char **args) {
  if (!init()) {
    system("pause");
    return 1;
  }

  GameState game;
  GameState2 game2;
  game.state = 0;
  game2.state = 0;

  while (loop(&game, &game2)) {
  }

  // End the program
  kill();
  return 0;
}

bool loop(GameState *game, GameState2 *game2) {
  SDL_Event evt;

  // clear the screen to white
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  if (game->state == 0 && game2->state == 0) {
    renderStartupText();
  }

  // event loop
  while (SDL_PollEvent(&evt) != 0) {
    switch (evt.type) {
    case SDL_QUIT:
      return false;
    case SDL_KEYDOWN:
      switch (evt.key.keysym.sym) {
      // game 1
      case SDLK_SPACE:
        resetGameState(game);
        curGame = 1;
        break;
      // game 2
      case SDLK_RETURN:
        resetGameState(game2);
        curGame = -1;
        break;
      }
    }
  }

  if (curGame == 1) {
    updatePaddle(game);
    switch (game->state) {
    case 1:
      updateCircle(game);
      renderGameState(game);
      renderHelpText();
      break;
    case -1:
      renderEndText(lose_text);
      break;
    case 999:
      renderEndText(win_text);
      break;
    case 0:
      renderStartupText();
      break;
    }
  } else if (curGame == -1) {
    updatePaddle(game2);
    switch (game2->state) {
    case 1:
      updateCircle(game2);
      renderGameState(game2);
      renderHelpText2();
      break;
    case -1:
      renderEndText2(win1_text);
      break;
    case -2:
      renderEndText2(win2_text);
      break;
    case 0:
      renderStartupText();
      break;
    }
  }

  SDL_RenderPresent(renderer);
  return true;
}

void renderStartupText() {
  text_texture = SDL_CreateTextureFromSurface(renderer, text);
  SDL_Rect dest = {(SCREEN_WIDTH - text->w) / 2, (SCREEN_HEIGHT - text->h) / 2,
                   text->w, text->h};
  SDL_RenderCopy(renderer, text_texture, NULL, &dest);
}

void renderHelpText() {
  help_text_texture = SDL_CreateTextureFromSurface(renderer, help_text);
  SDL_Rect dest = {(SCREEN_WIDTH - help_text->w) / 2,
                   (SCREEN_HEIGHT - help_text->h) / 2, help_text->w,
                   help_text->h};
  SDL_RenderCopy(renderer, help_text_texture, NULL, &dest);
}

void renderEndText(SDL_Surface *end_text) {
  end_text_texture = SDL_CreateTextureFromSurface(renderer, end_text);
  SDL_Rect dest = {(SCREEN_WIDTH - end_text->w) / 2,
                   SCREEN_HEIGHT / 2 - end_text->h, end_text->w, end_text->h};
  SDL_RenderCopy(renderer, end_text_texture, NULL, &dest);

  text_texture = SDL_CreateTextureFromSurface(renderer, text);
  dest = {(SCREEN_WIDTH - text->w) / 2, SCREEN_HEIGHT / 2, text->w, text->h};
  SDL_RenderCopy(renderer, text_texture, NULL, &dest);
}

void renderHelpText2() {
  help_text_texture = SDL_CreateTextureFromSurface(renderer, help1_text);
  SDL_Rect dest = {0, 0, help1_text->w, help1_text->h};
  SDL_RenderCopy(renderer, help_text_texture, NULL, &dest);

  help_text_texture = SDL_CreateTextureFromSurface(renderer, help2_text);
  dest = {SCREEN_WIDTH - help2_text->w, 0, help2_text->w, help2_text->h};
  SDL_RenderCopy(renderer, help_text_texture, NULL, &dest);
}

void renderEndText2(SDL_Surface *end_text) {
  end_text_texture = SDL_CreateTextureFromSurface(renderer, end_text);
  SDL_Rect dest = {(SCREEN_WIDTH - end_text->w) / 2,
                   SCREEN_HEIGHT / 2 - end_text->h, end_text->w, end_text->h};
  SDL_RenderCopy(renderer, end_text_texture, NULL, &dest);

  text_texture = SDL_CreateTextureFromSurface(renderer, text);
  dest = {(SCREEN_WIDTH - text->w) / 2, SCREEN_HEIGHT / 2, text->w, text->h};
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
  for (auto rec : game->rectangles) {
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

void renderGameState(GameState2 *game) {
  SDL_Rect p1, p2, b;
  p1.x = game->paddle1.x;
  p1.y = game->paddle1.y;
  p1.w = game->paddle1.w;
  p1.h = game->paddle1.h;
  p2.x = game->paddle2.x;
  p2.y = game->paddle2.y;
  p2.w = game->paddle2.w;
  p2.h = game->paddle2.h;
  b.x = game->ball.x;
  b.y = game->ball.y;
  b.w = game->ball.w;
  b.h = game->ball.h;
  SDL_RenderCopy(renderer, paddle_texture, NULL, &p1);
  SDL_RenderCopy(renderer, paddle_texture, NULL, &p2);
  SDL_RenderCopy(renderer, ball_texture, NULL, &b);
}

void updatePaddle(GameState *game) {
  Uint32 current = SDL_GetTicks();
  float dT = (current - game->paddle.lastUpdate) / 1000.0f;

  if (keys[SDL_SCANCODE_A]) {
    game->paddle.x -= game->paddle.xvelocity * dT;
    if (game->paddle.x <= 0 ||
        checkCollision(game->ball, game->paddle.x, game->paddle.y,
                       game->paddle.w, game->paddle.h)
                .first >= 0) {
      game->paddle.x += game->paddle.xvelocity * dT;
    }
  }
  if (keys[SDL_SCANCODE_D]) {
    game->paddle.x += game->paddle.xvelocity * dT;
    if (game->paddle.x + game->paddle.w >= SCREEN_WIDTH ||
        checkCollision(game->ball, game->paddle.x, game->paddle.y,
                       game->paddle.w, game->paddle.h)
                .first >= 0) {
      game->paddle.x -= game->paddle.xvelocity * dT;
    }
  }

  game->paddle.lastUpdate = current;
}

void updatePaddle(GameState2 *game) {
  Uint32 current = SDL_GetTicks();
  float dT1 = (current - game->paddle1.lastUpdate) / 1000.0f;
  float dT2 = (current - game->paddle2.lastUpdate) / 1000.0f;

  // paddle 1
  if (keys[SDL_SCANCODE_W]) {
    game->paddle1.y -= game->paddle1.yvelocity * dT1;
    if (game->paddle1.y <= 0 ||
        checkCollision(game->ball, game->paddle1.x, game->paddle1.y,
                       game->paddle1.w, game->paddle1.h)
                .second >= 0) {
      game->paddle1.y += game->paddle1.yvelocity * dT1;
    }
  }
  if (keys[SDL_SCANCODE_S]) {
    game->paddle1.y += game->paddle1.yvelocity * dT1;
    if (game->paddle1.y + game->paddle1.h >= SCREEN_HEIGHT ||
        checkCollision(game->ball, game->paddle1.x, game->paddle1.y,
                       game->paddle1.w, game->paddle1.h)
                .second >= 0) {
      game->paddle1.y -= game->paddle1.yvelocity * dT1;
    }
  }

  // paddle 2
  if (keys[SDL_SCANCODE_UP]) {
    game->paddle2.y -= game->paddle2.yvelocity * dT2;
    if (game->paddle2.y <= 0 ||
        checkCollision(game->ball, game->paddle2.x, game->paddle2.y,
                       game->paddle2.w, game->paddle2.h)
                .second >= 0) {
      game->paddle2.y += game->paddle2.yvelocity * dT2;
    }
  }
  if (keys[SDL_SCANCODE_DOWN]) {
    game->paddle2.y += game->paddle2.yvelocity * dT2;
    if (game->paddle2.y + game->paddle2.h >= SCREEN_HEIGHT ||
        checkCollision(game->ball, game->paddle2.x, game->paddle2.y,
                       game->paddle2.w, game->paddle2.h)
                .second >= 0) {
      game->paddle2.y -= game->paddle2.yvelocity * dT2;
    }
  }

  game->paddle1.lastUpdate = current;
  game->paddle2.lastUpdate = current;
}

void updateCircle(GameState *game) {
  Uint32 current = SDL_GetTicks();
  float dT = (current - game->ball.lastUpdate) / 1000.0f;

  game->ball.y += game->ball.ydirection * game->ball.yvelocity * dT;
  float cX = checkCollision(game->ball, game->paddle.x, game->paddle.y,
                            game->paddle.w, game->paddle.h)
                 .first;
  if (game->ball.y >= SCREEN_HEIGHT) { // lost
    game->state = -1;
    return;
  }
  if (game->ball.y <= 0 || eliminateBlocks(game)) {
    game->ball.y -= game->ball.ydirection * game->ball.yvelocity * dT;
    game->ball.ydirection *= -1;
  } else if (cX >= 0) {
    game->ball.y -= game->ball.ydirection * game->ball.yvelocity * dT;
    game->ball.ydirection *= -1;

    // new xvelocity proportional to distance between collision point and paddle
    // center
    float scale =
        abs(cX - (game->paddle.x + game->paddle.w / 2)) / (game->paddle.w / 2);
    game->ball.xvelocity =
        BALL_XVELOCITY_BASELINE + BALL_XVELOCITY_RANGE * scale;
  }

  game->ball.x += game->ball.xdirection * game->ball.xvelocity * dT;
  if (game->ball.x <= 0 || game->ball.x + game->ball.w >= SCREEN_WIDTH ||
      checkCollision(game->ball, game->paddle.x, game->paddle.y, game->paddle.w,
                     game->paddle.h)
              .first >= 0 ||
      eliminateBlocks(game)) {
    game->ball.x -= game->ball.xdirection * game->ball.xvelocity * dT;
    game->ball.xdirection *= -1;
  }

  game->ball.lastUpdate = current;
}

void updateCircle(GameState2 *game) {
  Uint32 current = SDL_GetTicks();
  float dT = (current - game->ball.lastUpdate) / 1000.0f;

  game->ball.x += game->ball.xdirection * game->ball.xvelocity * dT;
  float cY1 = checkCollision(game->ball, game->paddle1.x, game->paddle1.y,
                             game->paddle1.w, game->paddle1.h)
                  .second;
  float cY2 = checkCollision(game->ball, game->paddle2.x, game->paddle2.y,
                             game->paddle2.w, game->paddle2.h)
                  .second;
  if (game->ball.x <= 0) { // player 2 won
    game->state = -2;
    return;
  } else if (game->ball.x >= SCREEN_WIDTH) { // player 1 won
    game->state = -1;
    return;
  } else if (cY1 >= 0) {
    game->ball.x -= game->ball.xdirection * game->ball.xvelocity * dT;
    game->ball.xdirection *= -1;

    // new yvelocity proportional to distance between collision point and paddle
    // center
    float scale = abs(cY1 - (game->paddle1.y + game->paddle1.h / 2)) /
                  (game->paddle1.h / 2);
    game->ball.yvelocity =
        BALL2_YVELOCITY_BASELINE + BALL2_YVELOCITY_RANGE * scale;
  } else if (cY2 >= 0) {
    game->ball.x -= game->ball.xdirection * game->ball.xvelocity * dT;
    game->ball.xdirection *= -1;

    // new yvelocity proportional to distance between collision point and paddle
    // center
    float scale = abs(cY2 - (game->paddle2.y + game->paddle2.h / 2)) /
                  (game->paddle2.h / 2);
    game->ball.yvelocity =
        BALL2_YVELOCITY_BASELINE + BALL2_YVELOCITY_RANGE * scale;
  }

  game->ball.y += game->ball.ydirection * game->ball.yvelocity * dT;
  if (game->ball.y <= 0 || game->ball.y + game->ball.h >= SCREEN_HEIGHT ||
      checkCollision(game->ball, game->paddle1.x, game->paddle1.y,
                     game->paddle1.w, game->paddle1.h)
              .second >= 0 ||
      checkCollision(game->ball, game->paddle2.x, game->paddle2.y,
                     game->paddle2.w, game->paddle2.h)
              .second >= 0) {
    game->ball.y -= game->ball.ydirection * game->ball.yvelocity * dT;
    game->ball.ydirection *= -1;
  }

  game->ball.lastUpdate = current;
}

float pointDistance(float x1, float y1, float x2, float y2) {
  float x = x1 - x2;
  float y = y1 - y2;
  return sqrt(x * x + y * y);
}

pair<float, float> checkCollision(Ball b, float x, float y, float w, float h) {
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

  if (pointDistance(centerX, centerY, cX, cY) <= b.r) {
    return make_pair(cX, cY);
  } else {
    return make_pair(-1, -1); // no collision
  }
}

bool eliminateBlocks(GameState *game) {
  bool flag = false;
  for (int i = 0; i < game->rectangles.size(); i++) {
    Rectangle &r = game->rectangles[i];
    if (checkCollision(game->ball, r.x, r.y, r.w, r.h).first >= 0) {
      flag = true;
      game->rectangles.erase(game->rectangles.begin() + i);
    }
  }
  if (!game->rectangles.size()) {
    game->state = 999; // won
  }
  return flag;
}

void resetGameState(GameState *game) {
  if (game->state != 1) {
    game->state = 1;
    game->paddle = initializePaddle(game);
    game->ball = initializeBall(game);
    game->numRows = NUM_ROWS;
    game->rectangles = initializeRectangles(game->numRows);
  }
}

void resetGameState(GameState2 *game) {
  if (game->state != 1) {
    game->state = 1;
    pair<Paddle, Paddle> paddles = initializePaddles(game);
    game->paddle1 = paddles.first;
    game->paddle2 = paddles.second;
    game->ball = initializeBall(game);
  }
}

Paddle initializePaddle(GameState *game) {
  Paddle p;
  p.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
  p.y = SCREEN_HEIGHT - PADDLE_HEIGHT;
  p.w = PADDLE_WIDTH;
  p.h = PADDLE_HEIGHT;
  p.xvelocity = PADDLE_XVELOCITY;
  p.lastUpdate = SDL_GetTicks();
  return p;
}

pair<Paddle, Paddle> initializePaddles(GameState2 *game) {
  Paddle p1, p2;
  p1.x = 0;
  p1.y = (SCREEN_HEIGHT - PADDLE2_HEIGHT) / 2;
  p1.w = PADDLE2_WIDTH;
  p1.h = PADDLE2_HEIGHT;
  p1.yvelocity = PADDLE2_YVELOCITY;
  p1.lastUpdate = SDL_GetTicks();
  p2.x = SCREEN_WIDTH - PADDLE2_WIDTH;
  p2.y = (SCREEN_HEIGHT - PADDLE2_HEIGHT) / 2;
  p2.w = PADDLE2_WIDTH;
  p2.h = PADDLE2_HEIGHT;
  p2.yvelocity = PADDLE2_YVELOCITY;
  p2.lastUpdate = SDL_GetTicks();
  return make_pair(p1, p2);
}

Ball initializeBall(GameState *game) {
  Ball b;
  b.x = (SCREEN_WIDTH - BALL_RADIUS) / 2;
  b.y = SCREEN_HEIGHT - PADDLE_HEIGHT - BALL_RADIUS * 2;
  b.w = BALL_RADIUS * 2;
  b.h = BALL_RADIUS * 2;
  b.r = BALL_RADIUS;
  b.xvelocity = BALL_XVELOCITY_BASELINE;
  b.yvelocity = BALL_YVELOCITY;
  b.xdirection = 1;  // 1 for right, -1 for left
  b.ydirection = -1; // 1 for down, -1 for up
  b.lastUpdate = SDL_GetTicks();
  return b;
}

Ball initializeBall(GameState2 *game) {
  Ball b;
  b.x = SCREEN_WIDTH / 2 - BALL2_RADIUS;
  b.y = SCREEN_HEIGHT / 2 - BALL2_RADIUS;
  b.w = BALL2_RADIUS * 2;
  b.h = BALL2_RADIUS * 2;
  b.r = BALL2_RADIUS;
  b.xvelocity = BALL2_XVELOCITY;
  b.yvelocity = BALL2_YVELOCITY_BASELINE;
  int d[2] = {-1, 1};
  int i1 = rand() % 2;
  int i2 = rand() % 2;
  b.xdirection = d[i1]; // 1 for right, -1 for left
  b.ydirection = d[i2]; // 1 for down, -1 for up
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
      Rectangle r = initializeRectangle(col * (RECT_WIDTH + RECT_BORDER),
                                        row * (RECT_HEIGHT + RECT_BORDER));
      rs.push_back(r);
    }
  }
  return rs;
}

bool init() {
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    cout << "Error initializing SDL: " << SDL_GetError() << endl;
    return false;
  }

  window = SDL_CreateWindow("Example", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                            SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    cout << "Error creating window: " << SDL_GetError() << endl;
    return false;
  }
  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (!renderer) {
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

  // texts
  if (TTF_Init() < 0) {
    cout << "Error intializing SDL_ttf: " << TTF_GetError() << endl;
    return false;
  }
  font = TTF_OpenFont("font.ttf", 36);
  if (!font) {
    cout << "Error loading font: " << TTF_GetError() << endl;
    return false;
  }
  help_font = TTF_OpenFont("font.ttf", 24);
  if (!help_font) {
    cout << "Error loading font: " << TTF_GetError() << endl;
    return false;
  }

  SDL_Color text_color = {0, 0, 0};
  text = TTF_RenderText_Solid(
      font, "Press SPACEBAR to start game 1, RETURN to start game 2",
      text_color);
  if (!text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }
  lose_text = TTF_RenderText_Solid(font, "You lost Sadge", text_color);
  if (!lose_text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }
  win_text = TTF_RenderText_Solid(font, "You won Pog", text_color);
  if (!win_text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }

  win1_text = TTF_RenderText_Solid(font, "Player 1 won peepoClap", text_color);
  if (!win1_text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }
  win2_text = TTF_RenderText_Solid(font, "Player 2 won peepoClap", text_color);
  if (!win2_text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }

  SDL_Color help_text_color = {255, 0, 0};
  help_text = TTF_RenderText_Solid(
      help_font, "Press A and D to control the paddle", help_text_color);
  if (!help_text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }
  help1_text = TTF_RenderText_Solid(
      help_font, "Player 1: Press W and S to control the paddle",
      help_text_color);
  if (!help1_text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }
  help2_text = TTF_RenderText_Solid(
      help_font, "Player 2: Press UP and DOWN to control the paddle",
      help_text_color);
  if (!help2_text) {
    cout << "Failed to render text: " << TTF_GetError() << endl;
  }

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  return true;
}

void kill() {
  TTF_CloseFont(font);
  TTF_CloseFont(help_font);
  SDL_FreeSurface(text);
  SDL_FreeSurface(lose_text);
  SDL_FreeSurface(win_text);
  SDL_FreeSurface(help_text);
  SDL_FreeSurface(win1_text);
  SDL_FreeSurface(win2_text);
  SDL_FreeSurface(help1_text);
  SDL_FreeSurface(help2_text);
  SDL_DestroyTexture(text_texture);
  SDL_DestroyTexture(end_text_texture);
  SDL_DestroyTexture(help_text_texture);
  SDL_DestroyTexture(paddle_texture);
  SDL_DestroyTexture(ball_texture);
  SDL_DestroyTexture(rect_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  TTF_Quit();
  SDL_Quit();
}