#include <Allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include "gameobjects.h"

#ifdef _WIN32

static double round(double val) {
	return floor(val + 0.5);
}

#endif

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a > b ? b : a)
#define FORCE 0.5
#define MAX_SPEED 20
#define DECAY 0.01
#define MAX_SHIELDS 0.5

ALLEGRO_BITMAP *starImage = NULL;
ALLEGRO_COLOR colors[4] = {{1,1,1,1},{1,0.5,0.5,1},{0.5,1,0.5,1},{0.5,0.5,1,1}};
ALLEGRO_BITMAP *shipImages[2] = {NULL};

struct Sprite stars[MAX_STARS] = {{{0,0},{0,0},{1,1,1,1},NULL}};
struct SpaceShip player = {{0,0},{0,0},{0,0},0,0,{1,1,1,1},{NULL,NULL}};
struct Size windowSize;
struct Point camera = {0,0};

static float r(float max) {
    return (float)(rand() / ((float)RAND_MAX / max));
}

static bool loadImage(char name[50], ALLEGRO_BITMAP **img) {
    *img = al_load_bitmap(name);
    if (!*img) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to load required image", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return false;
    }
    return true;
}

bool loadImages(void) {
    if (!loadImage("star.png", &starImage)) {
        unloadImages();
        return false;
    }
    if (!loadImage("RocketOff.png", &shipImages[0]) || !loadImage("RocketOn.png", &shipImages[1])) {
        unloadImages();
        return false;
    }
    return true;
}

void unloadImages(void) {
	int i;
    if (starImage) {
        al_destroy_bitmap(starImage);
    }
    for (i = 0; i < 2; i++) {
        if (shipImages[i]) {
            al_destroy_bitmap(shipImages[i]);
        }
    }
}

static void fixCamera(void) {
    struct Point middle = {windowSize.width / 2, windowSize.height / 2};
    camera.x = MAX(middle.x, MIN(MAP_MAX - middle.x, camera.x));
    camera.y = MAX(middle.y, MIN(MAP_MAX - middle.y, camera.y));
}

void buildMap(struct Size winSize) {
	int maxStars = MAX_STARS;
	int i;
    windowSize = winSize;
    for (i = 0; i < maxStars; i++) {
        float s = r(0.4) + 0.1;
		stars[i].size.width = 20 * s;
        stars[i].size.height = 20 * s;
        stars[i].pos.x = r(MAP_MAX - stars[i].size.width);
        stars[i].pos.y = r(MAP_MAX - stars[i].size.height);
        stars[i].tint = colors[(int)round(r(4))];
        stars[i].image = starImage;
    }
    { //player scope
        float w = 110;
        float h = 246;
        player.pos.x = r(MAP_MAX - (w * 2)) + w;
        player.pos.y = r(MAP_MAX - (h * 2)) + h;
        player.rotation = r(6.2);
		player.centre.x = 55;
		player.centre.y = 95;
		player.tint = al_map_rgba_f(1,1,1,1);
		player.image[0] = shipImages[0];
		player.image[1] = shipImages[1];
        player.shield = MAX_SHIELDS;
		camera = player.pos;
        fixCamera();
    }
}

void moveObjects(void) {
	ALLEGRO_KEYBOARD_STATE keyState;
    player.accel.x -= player.accel.x * DECAY;
    player.accel.y -= player.accel.y * DECAY;
    if (player.accel.x < 0.01 && player.accel.x > -0.01) {
        player.accel.x = 0;
    }
    if (player.accel.y < 0.01 && player.accel.y > -0.01) {
        player.accel.y = 0;
    }
    al_get_keyboard_state(&keyState);
    if (al_key_down(&keyState, ALLEGRO_KEY_UP)) {
//        player.a.x += sin(player.r) * FORCE;
//        player.a.y += -cos(player.r) * FORCE;
//        player.a.x = MAX(-MAX_SPEED, MIN(MAX_SPEED, player.a.x));
//        player.a.y = MAX(-MAX_SPEED, MIN(MAX_SPEED, player.a.y));
        struct Point force = {(sin(player.rotation) * FORCE) + player.accel.x, (-cos(player.rotation) * FORCE) + player.accel.y};
        struct Point maxForce = {sin(player.rotation) * MAX_SPEED, -cos(player.rotation) * MAX_SPEED};
        if (maxForce.x < 0 && player.accel.x > maxForce.x) {
            player.accel.x = MAX(maxForce.x, force.x);
        }
        else if (maxForce.x > 0 && player.accel.x < maxForce.x) {
            player.accel.x = MIN(maxForce.x, force.x);
        }
        if (maxForce.y < 0 && player.accel.y > maxForce.y) {
            player.accel.y = MAX(maxForce.y, force.y);
        }
        else if (maxForce.y > 0 && player.accel.y < maxForce.y) {
            player.accel.y = MIN(maxForce.y, force.y);
        }
        player.imageIndex = true;
    }
    else {
        player.imageIndex = false;
    }
    if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
        player.rotation -= 0.05;
    }
    else if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
        player.rotation += 0.05;
    }
    if (player.rotation > 6.2) {
        player.rotation -= 6.2;
    }
    else if (player.rotation < 0) {
        player.rotation += 6.2;
    }
    player.pos.x += player.accel.x;
    player.pos.y += player.accel.y;
    camera = player.pos;
    player.pos.x = MAX(0, MIN(MAP_MAX, player.pos.x));
    player.pos.y = MAX(0, MIN(MAP_MAX, player.pos.y));
    fixCamera();
}

void renderObjects(void) {
    struct Point offset = {(camera.x - (windowSize.width / 2)), (camera.y - (windowSize.height / 2))};
    struct Point beg = {(camera.x - (windowSize.width / 2)) - 200, (camera.y - (windowSize.height / 2)) - 200};
    struct Point end = {(camera.x + (windowSize.width / 2)) + 200, (camera.y + (windowSize.height / 2)) + 200};
	int i;
    int maxStars = MAX_STARS;
    for (i = 0; i < maxStars; i++) {
        struct Sprite s = stars[i];
        if (s.pos.x > beg.x && s.pos.x < end.x && s.pos.y > beg.y && s.pos.y < end.y) {
            al_draw_tinted_scaled_bitmap(s.image, s.tint, 0, 0, 20, 20, s.pos.x - offset.x, s.pos.y - offset.y, s.size.width, s.size.height, 0);
        }
    }
    { //player scope
        struct SpaceShip s = player;
        if (s.pos.x > beg.x && s.pos.x < end.x && s.pos.y > beg.y && s.pos.y < end.y) {
            al_draw_filled_circle(s.pos.x - offset.x, s.pos.y - offset.y, 100, al_map_rgba_f(0, MAX_SHIELDS, 0, 0.1));
            al_draw_tinted_rotated_bitmap(s.image[s.imageIndex], s.tint, s.centre.x, s.centre.y, s.pos.x - offset.x, s.pos.y - offset.y, s.rotation, 0);
        }
    }
}
