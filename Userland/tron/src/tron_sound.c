#include <stdint.h>
#include "../include/tron_sound.h"
#include "../include/types.h"     
#include "../../c/include/userlib.h" 

// Local helper to play a sequence of (freq, ms) pairs (blocking)
typedef struct { uint16_t f; uint16_t d; } Note;

static void play_notes(const Note *seq, int len){
	for(int i = 0; i < len; i++){
		sys_beep(seq[i].f, seq[i].d);
	}
}

void tron_play_intro_song(void){
	static const Note s[] = {
		{ NOTE_A4, 80 }, { NOTE_C5, 80 }, { NOTE_E5, 120 }, { 0, 60 },
		{ NOTE_E5, 100 }, { NOTE_C5, 80 }, { NOTE_A4, 160 }
	};
	play_notes(s, (int)(sizeof(s)/sizeof(s[0])));
}

void tron_play_game_over_song(Winner w){
	if(w == WIN_P1){
		static const Note s1[] = {{ NOTE_A4, 80 }, { NOTE_C5, 80 }, { NOTE_E5, 160 }, { 0, 60 }, { NOTE_A4, 200 }};
		play_notes(s1, (int)(sizeof(s1)/sizeof(s1[0])));
	} else if(w == WIN_P2){
		static const Note s2[] = {{ NOTE_G4, 80 }, { NOTE_B4, 80 }, { NOTE_D5, 160 }, { 0, 60 }, { NOTE_G4, 200 }};
		play_notes(s2, (int)(sizeof(s2)/sizeof(s2[0])));
	} else { // empate
		static const Note s3[] = {{ NOTE_C4, 200 }, { 0, 60 }, { NOTE_C4, 200 }};
		play_notes(s3, (int)(sizeof(s3)/sizeof(s3[0])));
	}
}


typedef struct {
	const Note *seq;
	int len;
	int idx;
	int looping;
	uint64_t note_start_ticks;
	int playing;
} MusicState;

static MusicState g_music = {0};

static inline uint64_t ms_to_ticks(uint64_t ms){
	return (ms + 9) / 10; // redondeo hacia arriba
}

static void music_apply_note(const Note *n){
	if(!n) return;
	if(n->f == 0){
		sys_speaker_off();
	} else {
		sys_speaker_start(n->f);
	}
}

void tron_music_play_menu(void){
	static const Note MENU_BG[] = {
		{ NOTE_A4, 120 }, { NOTE_C5, 120 }, { NOTE_E5, 180 }, { 0, 60 },
		{ NOTE_C5, 120 }, { NOTE_E5, 120 }, { NOTE_A4, 240 }, { 0, 60 }
	};
	g_music.seq = MENU_BG;
	g_music.len = (int)(sizeof(MENU_BG)/sizeof(MENU_BG[0]));
	g_music.idx = 0;
	g_music.looping = 1;
	g_music.playing = 1;
	g_music.note_start_ticks = sys_ticks();
	music_apply_note(&g_music.seq[0]);
}

void tron_music_play_game(void){
	static const Note GAME_BG[] = {
		{ NOTE_A4, 90 }, { NOTE_E5, 90 }, { NOTE_A4, 90 }, { 0, 30 },
		{ NOTE_G4, 90 }, { NOTE_D5, 90 }, { NOTE_G4, 120 }, { 0, 30 }
	};
	g_music.seq = GAME_BG;
	g_music.len = (int)(sizeof(GAME_BG)/sizeof(GAME_BG[0]));
	g_music.idx = 0;
	g_music.looping = 1;
	g_music.playing = 1;
	g_music.note_start_ticks = sys_ticks();
	music_apply_note(&g_music.seq[0]);
}

void tron_music_stop(void){
	g_music.playing = 0;
	sys_speaker_off();
}

void tron_music_update(void){
	if(!g_music.playing || g_music.seq == 0 || g_music.len <= 0) return;
	uint64_t now = sys_ticks();
	const Note *cur = &g_music.seq[g_music.idx];
	uint64_t dur_ticks = ms_to_ticks(cur->d);
	if((uint64_t)(now - g_music.note_start_ticks) >= dur_ticks){
		// avanzar a la próxima nota
		g_music.idx++;
		if(g_music.idx >= g_music.len){
			if(g_music.looping){
				g_music.idx = 0;
			} else {
				tron_music_stop();
				return;
			}
		}
		g_music.note_start_ticks = now;
		music_apply_note(&g_music.seq[g_music.idx]);
	}
}