#pragma once

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define WORLD_OBJECT_COUNT 8
#define FIELD_OF_VIEW 1.5f

#define LIGHT_BOUNCE_AMOUNT 3
#define SAMPLES_PER_PIXEL_AXIS 4 /* This number squared, since it is looped for both axis */
#define SELF_SHADOW_INTENSITY 0.5f /* 0.0 - 1.0 */
