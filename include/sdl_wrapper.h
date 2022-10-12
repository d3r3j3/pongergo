#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  A = 1,
  W = 2,
  D = 3,
  S = 4,
  Q = 5,
  E = 6,
  J = 7,
  I = 8,
  L = 9,
  K = 10,
  U = 11,
  O = 12,
  Z = 13,
  M = 14,
  SPACE = 15

} arrow_key_t;

typedef enum {
  C_A = 0,
  C_B = 1,
  C_X = 2,
  C_Y = 3,
  C_S = 4,
  C_M = 6,
  C_LJC = 7,
  C_RJC = 8,
  C_LB = 9,
  C_RB = 10,
  C_DU = 11,
  C_DD = 12,
  C_DL = 13,
  C_DR = 14
} button_key_t;

typedef enum {
  AXIS_LEFTX = 0,
  AXIS_LEFTY = 1,
  AXIS_RIGHTX = 2,
  AXIS_RIGHTY = 3,
  AXIS_LT = 4,
  AXIS_RT = 5,
} axis_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

typedef enum { BUTTON_PRESSED, BUTTON_RELEASED } button_event_type_t;

typedef enum { AXIS_PRESSED, AXIS_RELEASED } axis_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(char key, key_event_type_t type, double held_time,
                              scene_tuple_t *scene_tup);

typedef void (*controller_handler_t)(char key, button_event_type_t type,
                                     double held_time, int which,
                                     scene_tuple_t *scene_tup);

typedef void (*axis_handler_t)(char key, axis_event_type_t type, int value,
                               int which, double held_time,
                               scene_tuple_t *scene_tup);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(list_t *points, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 */
void sdl_render_scene(scene_t *scene, char *message);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

void sdl_on_axis(axis_handler_t handler);

void sdl_on_button(controller_handler_t handler);
/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

void init_img(char *path);

void init_text(char *path, int font_size);

void init_audio(char *hit, char *powerup, char *wall, char *bg);

void play_audio(void);

void play_wall_audio(void);

void play_powerup_audio(void);

void play_bg(void);

void play_powerup_audio(void);

void create_text(char *text);

void render_text(void);

#endif // #ifndef __SDL_WRAPPER_H__
