#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct {
  float r;
  float g;
  float b;
} rgb_color_t;

/**
 * Gets red color value
 * @param rgb_color object
 * @return float red value
 */
float get_red(rgb_color_t color);

/**
 * Gets green color value
 * @param rgb_color object
 * @return float green value
 */
float get_green(rgb_color_t color);

/**
 * Gets blue color value
 * @param rgb_color object
 * @return float blue value
 */
float get_blue(rgb_color_t color);

#endif // #ifndef __COLOR_H__
