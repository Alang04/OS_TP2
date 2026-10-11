#ifndef TRON_RENDER_H
#define TRON_RENDER_H

#include "types.h"

// Seleccionar backend de render: por defecto, video.
#ifndef TRON_RENDER_VIDEO
#define TRON_RENDER_VIDEO 1
#endif

#if TRON_RENDER_VIDEO
#include "render_video.h"
#define render_init  render_video_init
#define render_begin render_video_begin
#define render_draw  render_video_draw
#define render_end   render_video_end
#define render_clear render_video_clear
#else
#include "render_text.h"
#define render_init  render_text_init
#define render_begin render_text_begin
#define render_draw  render_text_draw
#define render_end   render_text_end
#define render_clear render_text_clear
#endif

#endif
