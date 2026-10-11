#include <stdint.h>
#include "../include/render_video.h"
#include "../include/config.h"
#include "../include/types.h"
#include "../../c/include/userlib.h"

static int g_grid_w = 0, g_grid_h = 0;
static uint64_t g_scr_w = 0, g_scr_h = 0;
static uint64_t g_cell = 0;                  // tamaño de celda en píxeles (lado)
static uint64_t g_off_x = 0, g_off_y = 0;    // offset para centrar tablero
static int g_cache_valid = 0;
static int g_last_step = -1;
static uint8_t g_drawn_cells[TRON_GRID_H][TRON_GRID_W];

static inline uint64_t umin64(uint64_t a, uint64_t b){ return (a < b) ? a : b; }

static int update_layout_if_needed(void){
	if(g_grid_w <= 0 || g_grid_h <= 0){
		g_cell = 0; g_off_x = g_off_y = 0; return 0;
	}
	uint64_t new_w = sys_screen_width();
	uint64_t new_h = sys_screen_height();
	if(new_w == 0 || new_h == 0){
		return 0;
	}
	uint64_t old_cell = g_cell;
	uint64_t old_offx = g_off_x;
	uint64_t old_offy = g_off_y;
	uint64_t old_w = g_scr_w, old_h = g_scr_h;

	g_scr_w = new_w; g_scr_h = new_h;
	uint64_t cw = g_scr_w / (uint64_t)g_grid_w;
	uint64_t ch = g_scr_h / (uint64_t)g_grid_h;
	g_cell = umin64(cw, ch);
	if(g_cell == 0) g_cell = 1;
	uint64_t used_w = (uint64_t)g_grid_w * g_cell;
	uint64_t used_h = (uint64_t)g_grid_h * g_cell;
	g_off_x = (g_scr_w > used_w) ? (g_scr_w - used_w) / 2 : 0;
	g_off_y = (g_scr_h > used_h) ? (g_scr_h - used_h) / 2 : 0;

	int changed = (old_cell != g_cell) || (old_offx != g_off_x) || (old_offy != g_off_y) || (old_w != g_scr_w) || (old_h != g_scr_h);
	return changed;
}

void render_video_init(int grid_w, int grid_h){
	g_grid_w = grid_w; g_grid_h = grid_h;
	(void)update_layout_if_needed();
	// reset cache
	for(int y=0;y<TRON_GRID_H;y++) for(int x=0;x<TRON_GRID_W;x++) g_drawn_cells[y][x] = CELL_EMPTY;
	g_cache_valid = 1;
	g_last_step = -1;
	render_video_clear();
}

void render_video_begin(void){

}

static inline void draw_cell(int gx, int gy, uint32_t color){
	if(g_cell == 0) return;
	uint64_t x = g_off_x + (uint64_t)gx * g_cell;
	uint64_t y = g_off_y + (uint64_t)gy * g_cell;
	sys_fill_rect(x, y, g_cell, g_cell, color);
}

// Cada entrada es 7 filas, 5 bits usados de MSB a LSB (bit 4..0)
typedef struct { char ch; uint8_t rows[7]; } Glyph5x7;

static const Glyph5x7 FONT5x7[] = {
	// Dígitos
	{'0',{0x1E,0x11,0x13,0x15,0x19,0x11,0x1E}},
	{'1',{0x04,0x0C,0x14,0x04,0x04,0x04,0x1F}},
	{'2',{0x1E,0x01,0x01,0x1E,0x10,0x10,0x1F}},
	{'3',{0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E}},
	{'4',{0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}},
	{'5',{0x1F,0x10,0x10,0x1E,0x01,0x01,0x1E}},
	{'6',{0x0E,0x10,0x10,0x1E,0x11,0x11,0x1E}},
	{'7',{0x1F,0x01,0x02,0x04,0x08,0x08,0x08}},
	{'8',{0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}},
	{'9',{0x1E,0x11,0x11,0x1F,0x01,0x01,0x1E}},
	// Letras necesarias (mayúsculas)
	{'P',{0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}},
	{'A',{0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}},
	{'I',{0x0E,0x04,0x04,0x04,0x04,0x04,0x0E}},
	{'L',{0x10,0x10,0x10,0x10,0x10,0x10,0x1F}},
	{'U',{0x11,0x11,0x11,0x11,0x11,0x11,0x1E}},
	{'S',{0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}},
	{'E',{0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}},
	{'R',{0x1E,0x11,0x11,0x1E,0x12,0x11,0x11}},
	{'G',{0x0E,0x10,0x10,0x17,0x11,0x11,0x0E}},
	{'M',{0x11,0x1B,0x15,0x11,0x11,0x11,0x11}},
	{'O',{0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}},
	{'V',{0x11,0x11,0x11,0x11,0x0A,0x0A,0x04}},
	{'W',{0x11,0x11,0x11,0x15,0x15,0x1B,0x11}},
	{'D',{0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}},
	{'J',{0x01,0x01,0x01,0x01,0x11,0x11,0x0E}},
	{'K',{0x11,0x12,0x14,0x18,0x14,0x12,0x11}},
	{'/',{0x01,0x02,0x04,0x08,0x10,0x00,0x00}},
	{'Q',{0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}},
	// Símbolos mínimos
	{':',{0x00,0x04,0x00,0x00,0x00,0x04,0x00}},
	{' ',{0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{'1',{0x04,0x0C,0x14,0x04,0x04,0x04,0x1F}},
	{'2',{0x1E,0x01,0x01,0x1E,0x10,0x10,0x1F}},
};

static const Glyph5x7* glyph_lookup(char c){
	for(unsigned i = 0; i < (sizeof(FONT5x7)/sizeof(FONT5x7[0])); ++i){
		if(FONT5x7[i].ch == c) return &FONT5x7[i];
	}
	return 0;
}

static void draw_glyph(uint64_t x, uint64_t y, uint32_t color, const Glyph5x7 *g, uint64_t scale){
	if(!g || scale == 0) return;
	for(int row = 0; row < 7; ++row){
		uint8_t m = g->rows[row];
		for(int col = 0; col < 5; ++col){
			if(m & (1u << (4 - col))){
				sys_fill_rect(x + (uint64_t)col * scale,
							  y + (uint64_t)row * scale,
							  scale, scale, color);
			}
		}
	}
}

static void draw_text(uint64_t x, uint64_t y, uint32_t color, const char *s, uint64_t scale){
	if(!s) return;
	uint64_t cx = x;
	while(*s){
		const Glyph5x7 *g = glyph_lookup(*s);
		if(g){
			draw_glyph(cx, y, color, g, scale);
		}
		cx += (uint64_t)(6 * scale); // 5 cols + 1 espaciado
		++s;
	}
}

static void draw_scores_and_state(const GameSnapshot *snap){
	// Escala de fuente para HUD
	uint64_t scale = (g_cell >= 12 ? g_cell/4 : 2);
	if(scale < 2) scale = 2;
	uint64_t text_h = 7*scale;
	// Calcula altura de barra para 2 líneas (scores + ayuda)
	uint64_t desired = 2*text_h + scale;
	uint64_t bar_h = (g_cell > 4 ? g_cell * 2 : 12);
	if(bar_h < desired) bar_h = desired;

	uint32_t bar_col = 0x101010;
	sys_fill_rect(0, 0, g_scr_w, bar_h, bar_col);

	// Línea 1: P1 y P2
	char num[32];
	uint64_t pad = scale; // margen
	uint64_t y1 = (bar_h > 2*text_h + scale) ? (bar_h - (2*text_h + scale))/2 : 0;

	draw_text(pad, y1, 0xCCCCCC, "P1:", scale);
	num_to_str((uint64_t)snap->score_p1, num, 10);
	draw_text(pad + 4*(6*scale), y1, TRON_COLOR_P1, num, scale);

	char num2[32];
	num_to_str((uint64_t)snap->score_p2, num2, 10);
	int len2 = 0; while(num2[len2]) len2++;
	uint64_t block_w = (uint64_t)(3 + len2) * (6*scale);
	uint64_t start_x = (g_scr_w > pad + block_w) ? (g_scr_w - pad - block_w) : 0;
	draw_text(start_x, y1, 0xCCCCCC, "P2:", scale);
	draw_text(start_x + 3*(6*scale), y1, TRON_COLOR_P2, num2, scale);

	// Mensaje de estado centrado en la línea 1 si corresponde
	if(snap->state == STATE_PAUSED || snap->state == STATE_GAME_OVER){
		const char *msg = (snap->state == STATE_PAUSED) ? "PAUSA" : "GAME OVER";
		int len = 0; while(msg[len]) len++;
		uint64_t msg_w = (uint64_t)len * (6*scale);
		uint64_t msg_x = (g_scr_w > msg_w) ? (g_scr_w - msg_w)/2 : 0;
		draw_text(msg_x, y1, 0xFFFFFF, msg, scale);
	}

	// Línea 2: ayuda de controles, centrada
	const char *help = "P: PAUSA  Q: SALIR   WASD P1 / IJKL P2";
	int hl = 0; while(help[hl]) hl++;
	uint64_t help_w = (uint64_t)hl * (6*scale);
	uint64_t xh = (g_scr_w > help_w) ? (g_scr_w - help_w)/2 : 0;
	uint64_t y2 = y1 + text_h + scale;
	draw_text(xh, y2, 0xAAAAAA, help, scale);
}

void render_video_draw(const GameSnapshot *snap){
	if(!snap) return;
	// Actualizar layout sólo si cambió (para evitar full redraw)
	int layout_changed = update_layout_if_needed();

	// Dibuja grilla
	const int w = snap->grid.width;
	const int h = snap->grid.height;
	const uint8_t *cells = snap->grid.cells;

	// Determinar si corresponde redibujar todo (layout, cache inválida o nueva ronda)
	int full = layout_changed || !g_cache_valid || (g_last_step >= 0 && snap->step_count < g_last_step);
	if(full){
		// Clear total y reconstruir cache
		render_video_clear();
		for(int yy=0; yy<TRON_GRID_H; ++yy){
			for(int xx=0; xx<TRON_GRID_W; ++xx){
				g_drawn_cells[yy][xx] = CELL_EMPTY;
			}
		}
	}

	// Dibujo incremental por diferencias de celdas
	for(int y = 0; y < h; ++y){
		for(int x = 0; x < w; ++x){
			uint8_t cur = cells[y * w + x];
			if(full || cur != g_drawn_cells[y][x]){
				if(cur == CELL_EMPTY){
					// limpiar la celda al color de fondo
					uint64_t px = g_off_x + (uint64_t)x * g_cell;
					uint64_t py = g_off_y + (uint64_t)y * g_cell;
					sys_fill_rect(px, py, g_cell, g_cell, TRON_COLOR_BG);
				} else {
					uint32_t color = (cur == CELL_WALL) ? TRON_COLOR_WALL : (cur == CELL_TRAIL_P1 ? TRON_COLOR_P1 : TRON_COLOR_P2);
					draw_cell(x, y, color);
				}
				g_drawn_cells[y][x] = cur;
			}
		}
	}

	// Dibuja cabezas (por encima)
	if(snap->p1.alive){
		draw_cell(snap->p1.x, snap->p1.y, TRON_COLOR_P1);
	}
	if(snap->p2.alive){
		draw_cell(snap->p2.x, snap->p2.y, TRON_COLOR_P2);
	}
	// HUD con puntajes, estado y ayuda de controles
	draw_scores_and_state(snap);

	g_last_step = snap->step_count;
	g_cache_valid = 1;
}

void render_video_end(void){

}

void render_video_clear(void){
	// Limpia toda la pantalla con color de fondo
	if(g_scr_w == 0 || g_scr_h == 0){
		g_scr_w = sys_screen_width();
		g_scr_h = sys_screen_height();
	}
	if(g_scr_w && g_scr_h){
		sys_fill_rect(0, 0, g_scr_w, g_scr_h, TRON_COLOR_BG);
	}
	// Invalida cache para forzar redibujo completo en el próximo frame
	g_cache_valid = 0;
}