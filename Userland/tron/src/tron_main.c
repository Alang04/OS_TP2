#include <stdint.h>
#include "../include/tron_main.h"
#include "../include/game.h"
#include "../include/input.h"
#include "../include/render.h"
#include "../include/ai.h"
#include "../include/menu.h"
#include "../include/tron_sound.h"
#include "../../c/include/userlib.h"

int tron_main(void){
    sys_clear();

    // Bucle: menú -> juego -> (salir o volver al menú)
    for(;;){
        MenuConfig cfg;
        // Inicia música de fondo del menú (no bloqueante)
        tron_music_play_menu();
        if(!menu_show_and_get_config(&cfg)){
            // Usuario eligió salir desde el menú
            return 0;
        }

        TronGame *g = game_create(cfg.mode);
        if(!g){
            sys_write(STDOUT, "Error iniciando Tron\n", 22);
            return -1;
        }

        // Detiene música de menú y opcional: fanfarria de inicio
        tron_music_stop();
        tron_play_intro_song();
        // Arranca música de juego en loop
        tron_music_play_game();

        // Configuración elegida
        game_set_speed(g, cfg.ticks_per_step);
        game_enable_ai(g, (cfg.mode == MODE_ONE_PLAYER) ? 1 : 0); // P2 como IA en 1P

        input_init();
    render_init(TRON_GRID_W, TRON_GRID_H);

        int running = 1;
        int paused = 0;

        int last_over = 0;
        while(running){
            uint64_t now = sys_ticks();

            InputEvents ev; 
            input_poll(&ev);

            if(ev.quit_requested){
                // Salir del juego actual y volver al menú principal
                running = 0; 
                break;
            }

            if(ev.pause_toggled){
                if(game_is_over(g)){
                    game_reset_round(g);
                    paused = 0;
                } else {
                    paused = !paused;
                }
            }

            if(!paused && !game_is_over(g)){
                if(ev.has_p1_turn){
                    game_queue_turn(g, 1, ev.p1_turn);
                }

                // IA para P2 cuando corresponde
                GameSnapshot snap;
                game_get_snapshot(g, &snap);
                Direction ai_dir;
                if(snap.mode == MODE_ONE_PLAYER && snap.p2.alive){
                    if(ai_decide_next_dir(&snap, 2, 0, &ai_dir)){
                        game_queue_turn(g, 2, ai_dir);
                    }
                } else if(ev.has_p2_turn){
                    game_queue_turn(g, 2, ev.p2_turn);
                }

                game_update(g, now);
            }

            // Si la ronda termina en este frame, reproducir jingle de fin
            int is_over = game_is_over(g);
            if(is_over && !last_over){
                // Pausa música de fondo durante el jingle
                tron_music_stop();
                tron_play_game_over_song(game_winner(g));
                // Reinicia música de juego si se va a continuar
                tron_music_play_game();
            }
            last_over = is_over;

            // Actualiza música de fondo
            tron_music_update();

            // Render: no limpiar cada frame en modo video para preservar el trail
#if TRON_RENDER_VIDEO
            /* En modo video hacemos render incremental; no limpiar el framebuffer
               en cada iteración para que el cache de celdas coincida con lo dibujado. */
#else
            render_clear();
#endif
            GameSnapshot snap2; 
            game_get_snapshot(g, &snap2);
            if(paused && snap2.state == STATE_PLAYING){
                snap2.state = STATE_PAUSED; // reflect pause in HUD
            }
            render_begin();
            render_draw(&snap2);
            render_end();
        }

        game_destroy(g);
        tron_music_stop();
        // vuelve al menú principal (no salir del programa) a menos que el usuario lo haga desde menú
    }
}
