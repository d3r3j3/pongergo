#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

// TTF Setup constants
SDL_Texture *text_Texture;
SDL_Rect text_rect;
SDL_Surface *text_surface;
TTF_Font *FONT;
const SDL_Color TEXT_COLOR = {255, 0, 0, 255};

// Gamecontroller setup constants
int controller_count = 0;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;

// Controller button handler, or NULL if none has been configured.
controller_handler_t controller_handler = NULL;

// Controller axis handler, or NULL if none has been configured.
axis_handler_t axis_handler = NULL;

/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;

uint32_t button_start_timestamp;

/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

// Game music constants
Mix_Music *bg_music = NULL;
Mix_Chunk *powerup_hit = NULL;
Mix_Chunk *paddle_hit = NULL;
Mix_Chunk *wall_hit = NULL;

// Game image constants
SDL_Surface *bg_img = NULL;
SDL_Texture *tex = NULL;
SDL_Rect pic_rec;
const int IMG_W = 1000;
const int IMG_H = 500;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_a:
    return A;
  case SDLK_w:
    return W;
  case SDLK_d:
    return D;
  case SDLK_s:
    return S;
  case SDLK_q:
    return Q;
  case SDLK_e:
    return E;
  case SDLK_j:
    return J;
  case SDLK_i:
    return I;
  case SDLK_l:
    return L;
  case SDLK_k:
    return K;
  case SDLK_u:
    return U;
  case SDLK_o:
    return O;
  case SDLK_SPACE:
    return SPACE;
  case SDLK_z:
    return Z;
  case SDLK_m:
    return M;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

char get_controllerkey(SDL_GameControllerButton button) {
  switch (button) {
  case SDL_CONTROLLER_BUTTON_A:
    return C_A;
  case SDL_CONTROLLER_BUTTON_B:
    return C_B;
  case SDL_CONTROLLER_BUTTON_X:
    return C_X;
  case SDL_CONTROLLER_BUTTON_Y:
    return C_Y;
  case SDL_CONTROLLER_BUTTON_BACK:
    return C_S;
  case SDL_CONTROLLER_BUTTON_START:
    return C_M;
  case SDL_CONTROLLER_BUTTON_LEFTSTICK:
    return C_LJC;
  case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
    return C_RJC;
  case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
    return C_LB;
  case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
    return C_RB;
  case SDL_CONTROLLER_BUTTON_DPAD_UP:
    return C_DU;
  case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
    return C_DD;
  case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
    return C_DL;
  case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
    return C_DR;
  default:
    return '\0';
  }
}

char get_axiskey(SDL_GameControllerAxis axis) {
  switch (axis) {
  case SDL_CONTROLLER_AXIS_LEFTX:
    return AXIS_LEFTX;
  case SDL_CONTROLLER_AXIS_LEFTY:
    return AXIS_LEFTY;
  case SDL_CONTROLLER_AXIS_RIGHTX:
    return AXIS_RIGHTX;
  case SDL_CONTROLLER_AXIS_RIGHTY:
    return AXIS_RIGHTY;
  case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
    return AXIS_LT;
  case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
    return AXIS_RT;
  default:
    return '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO |
               SDL_INIT_TIMER) < 0) {
    printf("Missing Initializations\n");
  }
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
}

void add_controller(int device_id) {
  if (SDL_IsGameController(device_id)) {
    printf("Supported \n");
  } else {
    printf("Not Supported \n");
  }
}

bool sdl_is_done(void *scene_tup) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_CONTROLLERDEVICEADDED:
      controller_count++;
      int index = event->cdevice.which;
      add_controller(index);
      break;
    case SDL_CONTROLLERDEVICEREMOVED:
      if (controller_count != 0) {
        controller_count--;
      }
      SDL_GameControllerClose(
          SDL_GameControllerFromInstanceID(event->cdevice.which));
      printf("DEVICE REMOVED\n");
      break;
    case SDL_CONTROLLERAXISMOTION: {
      char axis_key = get_axiskey(event->caxis.axis);
      if (axis_key == '\0') {
        break;
      }
      double held_time = SDL_GetTicks() / MS_PER_S;
      int value = event->caxis.value;
      axis_event_type_t type = event->caxis.type;
      int which = event->caxis.which;
      axis_handler(axis_key, type, value, which, held_time, scene_tup);
      break;
    }
    case SDL_KEYDOWN:
    case SDL_KEYUP: {
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, scene_tup);
      break;
    }
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
      if (controller_handler == NULL)
        break;
      char button_key = get_controllerkey(event->cbutton.button);
      if (button_key == '\0') {
        break;
      }
      uint32_t timestamp = event->cbutton.timestamp;
      if (event->cbutton.state) {
        button_start_timestamp = timestamp;
      }
      double held_time = (timestamp - button_start_timestamp) / MS_PER_S;
      button_event_type_t type = event->type == SDL_CONTROLLERBUTTONDOWN
                                     ? BUTTON_PRESSED
                                     : BUTTON_RELEASED;
      controller_handler(button_key, type, held_time, event->cbutton.which,
                         scene_tup);
      break;
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);
  SDL_RenderPresent(renderer);
}

void init_img(char *path) {
  IMG_Init(2);
  bg_img = IMG_Load(path);
  pic_rec.x = (WINDOW_WIDTH - IMG_W) * 0.5;
  pic_rec.y = (WINDOW_HEIGHT - IMG_H) * 0.5;
  pic_rec.w = IMG_W;
  pic_rec.h = IMG_H;
  tex = SDL_CreateTextureFromSurface(renderer, bg_img);
  if (tex == NULL) {
    printf("Texture Null \n");
  }
}

void free_img(void) {
  SDL_FreeSurface(bg_img);
  bg_img = NULL;
  tex = NULL;
}

void init_audio(char *hit, char *powerup, char *wall, char *bg) {
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    printf("SOUND NOT SET");
    return;
  }
  paddle_hit = Mix_LoadWAV(hit);
  powerup_hit = Mix_LoadWAV(powerup);
  wall_hit = Mix_LoadWAV(wall);
  bg_music = Mix_LoadMUS(bg);
}

void free_audio(void) {
  Mix_FreeChunk(paddle_hit);
  Mix_FreeChunk(wall_hit);
  Mix_FreeChunk(powerup_hit);
  Mix_FreeMusic(bg_music);

  paddle_hit = NULL;
  wall_hit = NULL;
  powerup_hit = NULL;
  bg_music = NULL;
}

void init_text(char *path, int font_size) {
  TTF_Init();
  FONT = TTF_OpenFont(path, font_size);
  if (!FONT) {
    printf("FONT NOT SET \n");
  }
}

void free_text(void) {
  TTF_CloseFont(FONT);
  FONT = NULL;
  text_surface = NULL;
  text_Texture = NULL;
}

void play_audio(void) { Mix_PlayChannel(-1, paddle_hit, 0); }

void play_wall_audio(void) { Mix_PlayChannel(-1, wall_hit, 0); }

void play_powerup_audio(void) { Mix_PlayChannel(-1, powerup_hit, 0); }

void play_bg(void) { Mix_PlayMusic(bg_music, -1); }

void create_text(char *text) {
  if (FONT != NULL) {
    TTF_Init();
    text_surface = TTF_RenderText_Solid(FONT, text, TEXT_COLOR);
    text_Texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    text_rect.x = (WINDOW_WIDTH - text_surface->w) * 0.5;
    text_rect.y = (WINDOW_HEIGHT - text_surface->h) * 0.5;
    text_rect.w = text_surface->w;
    text_rect.h = text_surface->h;
    SDL_FreeSurface(text_surface);
    TTF_Quit();
  }
}

void render_text(void) {
  if (text_Texture != NULL) {
    SDL_RenderCopy(renderer, text_Texture, NULL, &text_rect);
  }
}

void render_img(void) { SDL_RenderCopy(renderer, tex, NULL, &pic_rec); }

void sdl_render_scene(scene_t *scene, char *message) {
  SDL_DestroyTexture(text_Texture);
  sdl_clear();
  render_img();
  create_text(message);
  render_text();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    sdl_draw_polygon(shape, body_get_color(body));
    list_free(shape);
  }
  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }
void sdl_on_axis(axis_handler_t handler) { axis_handler = handler; }
void sdl_on_button(controller_handler_t handler) {
  controller_handler = handler;
}

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}