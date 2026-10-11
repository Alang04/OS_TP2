#ifndef TRON_RENDER_VIDEO_H
#define TRON_RENDER_VIDEO_H

#include <stdint.h>
#include "types.h"

void render_video_init(int grid_w, int grid_h);
void render_video_begin(void);
void render_video_draw(const GameSnapshot *snap);
void render_video_end(void);
void render_video_clear(void);

#endif
