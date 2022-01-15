#pragma once

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define EXTRA_THREAD_COUNT 4
#define FPS 30

#define FIELD_OF_VIEW 1.5f
#define DEPTH_OF_FIELD_INTENSITY 0.2f
#define FOCUS_DISTANCE 12.0f
#define SELF_SHADOW_INTENSITY 0.5f /* 0.0 - 1.0 */

#define LIGHT_BOUNCE_AMOUNT 3
#define SAMPLES_PER_PIXEL_AXIS 3 /* This number squared, since it is looped for both axis */
