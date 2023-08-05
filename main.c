#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define WIDTH  400
#define HEIGHT 400

#define UPDATES_PER_SECOND 30.f

#define RED   0xFF000000ul
#define GREEN 0x00FF0000ul
#define BLUE  0x0000FF00ul

static void draw_pixel(
  uint32_t* pixels,
  int       x,
  int       y,
  uint32_t  color
) {
  if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT)
    pixels[x + WIDTH * y] = color;
}

static void draw_circle(
  uint32_t* pixels,
  int       center_x,
  int       center_y,
  int       radius,
  uint32_t  color
) {
  int x = radius;
  int y = 0;
  int p = 1 - radius;

  draw_pixel(pixels, center_x + radius, center_y         , color);
  draw_pixel(pixels, center_x - radius, center_y         , color);
  draw_pixel(pixels, center_x         , center_y + radius, color);
  draw_pixel(pixels, center_x         , center_y - radius, color);

  while (x > y) {
    y++;

    p += y + y + 1;

    if (p > 0)
      x--, p -= x + x;

    if (x < y)
      break;

    draw_pixel(pixels, center_x + x, center_y + y, color);
    draw_pixel(pixels, center_x + x, center_y - y, color);
    draw_pixel(pixels, center_x - x, center_y + y, color);
    draw_pixel(pixels, center_x - x, center_y - y, color);
    draw_pixel(pixels, center_x + y, center_y + x, color);
    draw_pixel(pixels, center_x + y, center_y - x, color);
    draw_pixel(pixels, center_x - y, center_y + x, color);
    draw_pixel(pixels, center_x - y, center_y - x, color);
  }
}

static void sdl_error(char* operation) {
  fprintf(stderr, "Failed to %s: %s.\n", operation, SDL_GetError());
}

static void sdl_error_exit(char* operation) {
  sdl_error(operation);
  exit(EXIT_FAILURE);
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO))
    sdl_error_exit("initialize sdl");

  SDL_Window* window = SDL_CreateWindow(
    "Circle Demo",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    WIDTH,
    HEIGHT,
    0
  );
  if (window == NULL)
    sdl_error_exit("create window");

  SDL_Renderer* renderer = SDL_CreateRenderer(
    window, -1, SDL_RENDERER_ACCELERATED
  );
  if (renderer == NULL)
    sdl_error_exit("create renderer");

  SDL_Texture* texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBX8888,
    SDL_TEXTUREACCESS_STREAMING,
    WIDTH,
    HEIGHT
  );
  if (texture == NULL)
    sdl_error_exit("create texture");

  int      x, y;
  bool     visible = false;
  int      min_radius;
  int      max_radius;
  int      radius;
  bool     expanding;
  uint32_t color;
  
  SDL_Event event;
  while (1) {
    while (SDL_PollEvent(&event))
      if (event.type == SDL_QUIT)
        goto CLEANUP;
      else if (event.type == SDL_MOUSEBUTTONDOWN) {
        x          = event.button.x;
        y          = event.button.y;
        visible    = true;
        min_radius = rand() % 50;
        max_radius = min_radius + rand() % 50;
        radius     = min_radius;
        expanding  = true;
        color      =
          ((rand() & 0xFF) << 24) +
          ((rand() & 0xFF) << 16) +
          ((rand() & 0xFF) << 8 );
      }

    int speed = (max_radius - min_radius) / 10;
    radius      += expanding ? speed : -speed;
    if (radius > max_radius)
      expanding = false;
    if (radius < min_radius)
      visible = false;
    
    if (SDL_RenderClear(renderer))
      sdl_error("clear renderer");

    void* pixels;
    int   pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    memset(pixels, 0, HEIGHT * pitch);
    if (visible)
      draw_circle(pixels, x, y, radius, color);

    SDL_UnlockTexture(texture);

    if (SDL_RenderCopy(renderer, texture, NULL, NULL))
      sdl_error("render texture");

    SDL_RenderPresent(renderer);

    SDL_Delay(1000 / UPDATES_PER_SECOND);
  }

 CLEANUP:
  SDL_DestroyWindow(window);
  SDL_Quit();
}
