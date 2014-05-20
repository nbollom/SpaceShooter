#include <allegro5/allegro.h>
#include <allegro5/keyboard.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>

#include "gameobjects.h"

#define APPNAME "Allegro Templete"
#define DESIGNSIZE {1680, 1050}
#define USEMOUSE false
#define SHOWSTATS true

const float FPS = 60;

float getAspectRatio(struct Size size) {
    return size.width / size.height;
}

int main(int argc, char **argv) {
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_FONT *fpsfont;
    ALLEGRO_MONITOR_INFO monitor;
	struct Size designSize = DESIGNSIZE;
	struct Size winSize;
    bool run = true;
    int fps = 0, fps_accum = 0;
    double fps_time = 0.0;
	float sx;
	float sy;
	ALLEGRO_TRANSFORM trans;
	ALLEGRO_COLOR color_black = al_map_rgb_f(0, 0, 0);
    ALLEGRO_COLOR color_white = al_map_rgb_f(1, 1, 1);
    
    if (!al_init()) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to initialize allegro", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    
    if (!al_install_keyboard()) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to initialize keyboard input", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    
    if (USEMOUSE && !al_install_mouse()) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to initialize mouse input", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    
#ifndef _WIN32
    if (!al_init_font_addon()) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to initialize font addon", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
#else
	al_init_font_addon();
#endif
    
    if (!al_init_ttf_addon()) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to initialize ttf font addon", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    
    if (!al_init_image_addon()) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to initialize image addon", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    
    if (!al_init_primitives_addon()) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to initialize primitives", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    
    event_queue = al_create_event_queue();
    if (!event_queue) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to create event_queue", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        al_destroy_display(display);
        return -1;
    }
    
    al_get_monitor_info(0, &monitor);
	winSize.width = (float)(monitor.x2 - monitor.x1);
	winSize.height = (float)(monitor.y2 - monitor.x1);
    
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
    display = al_create_display(designSize.width, designSize.height);
    if (!display) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to create display", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    al_set_window_title(display, APPNAME);
    if (!USEMOUSE) {
        al_hide_mouse_cursor(display);
    }
    sx = winSize.width / designSize.width;
    sy = winSize.height / designSize.height;
    al_identity_transform(&trans);
    al_scale_transform(&trans, sx, sy);
    al_use_transform(&trans);
    
    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to create game timer", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    
    fpsfont = al_load_ttf_font("Menlo.ttf", 18, 0);
    if (!fpsfont) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to load required font", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return -1;
    }
    
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    
    al_clear_to_color(color_black);
    al_flip_display();
    
    srand((unsigned int)time(0));
    
    if (!loadImages()) {
        return -1;
    }
    
    buildMap(designSize);
    
    al_start_timer(timer);
    
    while (run)
    {
		double t;
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);
        
        switch (ev.type) {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                run = false;
                break;
                
            case ALLEGRO_EVENT_KEY_DOWN:
                if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                    run = false;
                }
                break;
                
            case ALLEGRO_EVENT_TIMER:
                al_clear_to_color(color_black);
                
                moveObjects();
                renderObjects();
                
                if (SHOWSTATS) {
                    al_draw_textf(fpsfont, color_white, designSize.width, 0, ALLEGRO_ALIGN_RIGHT, "FPS: %02d", fps);
                    al_draw_textf(fpsfont, color_white, designSize.width, 20, ALLEGRO_ALIGN_RIGHT, "R: %0.2f", player.rotation);
                    al_draw_textf(fpsfont, color_white, designSize.width, 40, ALLEGRO_ALIGN_RIGHT, "AX: %06.2f", player.accel.x);
                    al_draw_textf(fpsfont, color_white, designSize.width, 60, ALLEGRO_ALIGN_RIGHT, "AY: %06.2f", player.accel.y);
                    al_draw_textf(fpsfont, color_white, designSize.width, 80, ALLEGRO_ALIGN_RIGHT, "X: %04.0f/%04d", player.pos.x, MAP_MAX);
                    al_draw_textf(fpsfont, color_white, designSize.width, 100, ALLEGRO_ALIGN_RIGHT, "Y: %04.0f/%04d", player.pos.y, MAP_MAX);
                    al_draw_textf(fpsfont, color_white, designSize.width, 120, ALLEGRO_ALIGN_RIGHT, "Shield: %0.4f", player.shield);
                    al_draw_textf(fpsfont, color_white, designSize.width, 140, ALLEGRO_ALIGN_RIGHT, "Shot: %0.4f", player.shotCooldown);
                }
                al_wait_for_vsync();
                al_flip_display();
                
                fps_accum++;
                t = al_get_time();
                if (t - fps_time >= 1) {
                    fps = fps_accum;
                    fps_accum = 0;
                    fps_time = t;
                }
                
                break;
        }
    }
    unloadImages();
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return 0;
}
