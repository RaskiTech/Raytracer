#pragma once

//  PROGRAM  //
#define WINDOW_WIDTH 1280                  /* Positive intiger */
#define WINDOW_HEIGHT 720                  /* Positive intiger */
#define EXTRA_THREAD_COUNT 4               /* Positive intiger */
#define FPS 24                             /* Positive intiger */
#define PIXEL_CALCULATING_OREDER_SPREAD 37 /* Positive intiger */

//  QUALITY  //
#define FIELD_OF_VIEW 1.5f
#define DEPTH_OF_FIELD_INTENSITY 0.2f
#define FOCUS_DISTANCE 12.0f
#define SELF_SHADOW_INTENSITY 0.5f /* 0.0 - 1.0 */
#define LIGHT_BOUNCE_AMOUNT 3
#define SAMPLES_PER_PIXEL_AXIS 3 /* This number squared, since it is looped for both axis */

//  GAMEPLAY  //
#define CAN_MOVE_CAMERA true
#define MOVEMENT_SPEED 0.25f
#define MOUSE_SENSITIVITY 0.0005f