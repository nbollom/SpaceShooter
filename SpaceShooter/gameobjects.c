#include <Allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "gameobjects.h"

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a > b ? b : a)

#define MAX_STARS (int)((float)MAP_MAX / 1.2)
#define MAX_TURRETS (MAP_MAX / 100)
#define FORCE 0.5
#define MAX_SPEED 20
#define DECAY 0.01
#define MAX_SHIELDS 1
#define MAX_HEALTH 1
#define MAX_LAZERS 100
#define LAZER_FORCE 30
#define LAZER_MAX_DISTANCE 1000
#define MAX_RADUINS 6.283185
#define QUARTER_RADIUNS (MAX_RADUINS / 4)
#define PLAYER_SHOT_RATE 3
#define TURRET_SHOT_RATE 6
#define LAZER_DAMAGE 0.1

ALLEGRO_BITMAP *starImage = NULL;
ALLEGRO_COLOR colors[4] = {{1,1,1,1},{0.6,0.2,0.2,1},{0.2,1,0.2,1},{0.2,0.2,1,1}};
ALLEGRO_BITMAP *shipImages[2] = {NULL};
ALLEGRO_BITMAP *shieldImage = NULL;
ALLEGRO_BITMAP *lazerImage = NULL;
ALLEGRO_BITMAP *turretImage = NULL;

ALLEGRO_SAMPLE *lazerSound = NULL;
ALLEGRO_SAMPLE *explosionSound = NULL;
ALLEGRO_SAMPLE *bgMusic = NULL;

struct Star stars[MAX_STARS] = {{{0,0},{0,0},{1,1,1,1},NULL}};
struct Turret turrets[MAX_TURRETS];
struct SpaceShip player = {{0,0},{0,0},{0,0},0,0,{NULL,NULL},0,0,0,0};
struct Size windowSize;
struct Point camera = {0,0};
struct Lazer lazers[MAX_LAZERS];
int lazerCount;
bool showTextures = true;
bool playSounds = true;
bool gameOver = false;
bool win = false;

struct Point shieldCollisionPath[64] = {{74,21},{69,53},{67,55},{66,62},{64,62},{60,77},{51,91},{52,94},{50,93},{49,96},{44,100},{44,102},{28,117},{10,124},{-10,124},{-23,120},{-26,117},{-28,117},{-31,113},{-33,113},{-44,102},{-44,100},{-49,96},{-50,93},{-52,94},{-51,91},{-60,77},{-64,62},{-66,62},{-67,55},{-69,53},{-74,21},{-73,-31},{-69,-53},{-67,-55},{-66,-62},{-64,-62},{-60,-77},{-51,-91},{-52,-94},{-50,-93},{-49,-96},{-44,-100},{-44,-102},{-28,-117},{-14,-123},{10,-124},{23,-120},{26,-117},{28,-117},{31,-113},{33,-113},{44,-102},{44,-100},{49,-96},{50,-93},{52,-94},{51,-91},{60,-77},{64,-62},{66,-62},{67,-55},{69,-53},{74,-21}};
struct Point shipCollisionPath[68] = {{55,43.00000044},{46,70.00000044},{44,72.00000044},{44,75.00000044},{47,78.00000044},{48,82.00000044},{31,82.00000044},{31,53.00000044},{25,45.00000044},{16,64.00000044},{4,64.00000044},{4,74.00000044},{6,75.00000044},{7,82.00000044},{-7,82.00000044},{-6,75.00000044},{-3,73.00000044},{-3,64.00000044},{-15,64.00000044},{-16,58.00000044},{-19,57.00000044},{-24,45.00000044},{-30,50.00000044},{-31,54.00000044},{-31,82.00000044},{-47,82.00000044},{-47,78.00000044},{-44,76.00000044},{-44,71.00000044},{-49,61.00000044},{-49,57.00000044},{-51,55.00000044},{-51,51.00000044},{-53,49.00000044},{-55,41.00000044},{-55,13.00000044},{-51,3.00000044000001},{-46,-1.99999955999999},{-46,-3.99999955999999},{-43,-3.99999955999999},{-43,8.00000044000001},{-41,13.00000044},{-37,14.00000044},{-33,12.00000044},{-36,-12.99999956},{-36,-30.99999956},{-30,-58.99999956},{-22,-72.99999956},{-19,-74.99999956},{-18,-78.99999956},{-2,-93.99999956},{3,-93.99999956},{18,-79.99999956},{19,-75.99999956},{22,-73.99999956},{27,-63.99999956},{29,-62.99999956},{30,-56.99999956},{32,-54.99999956},{36,-37.99999956},{37,-25.99999956},{36,-2.99999955999999},{33,12.00000044},{37,14.00000044},{42,12.00000044},{43,-3.99999955999999},{47,-2.99999955999999},{55,11.00000044}};
struct Point turretCollisionPath[95] = {{6,-50},{9,-48},{8,-43},{12,-42},{15,-43},{17,-48},{22,-47},{22,-38},{27,-36},{29,-33},{31,-33},{34,-36},{37,-36},{37,-33},{40,-33},{40,-30},{36,-27},{36,-25},{38,-24},{38,-22},{44,-23},{49,-17},{49,-7},{48,-5},{44,-5},{44,1},{50,1},{50,9},{43,9},{43,13},{41,16},{41,19},{46,23},{44,25},{44,28},{41,29},{38,26},{35,26},{35,28},{32,30},{35,31},{35,36},{28,43},{20,44},{19,40},{13,42},{14,50},{7,50},{5,44},{-5,44},{-7,50},{-14,50},{-13,42},{-18,40},{-20,44},{-30,42},{-35,36},{-35,31},{-32,31},{-35,28},{-35,26},{-41,29},{-44,28},{-45,21},{-41,19},{-43,9},{-50,9},{-50,1},{-44,1},{-44,-5},{-48,-5},{-49,-7},{-49,-17},{-44,-23},{-38,-22},{-38,-24},{-36,-25},{-36,-27},{-40,-30},{-40,-33},{-37,-33},{-37,-36},{-34,-36},{-31,-33},{-29,-33},{-27,-36},{-22,-38},{-23,-46},{-20,-46},{-20,-48},{-17,-48},{-14,-42},{-8,-43},{-9,-48},{-6,-50}};
struct Point lazerCollisionPath[4] = {{-4,-12},{4,-12},{4,12},{-4,12}};

struct Point lastCollisionPath[95] = {{0,0}};

static float r(float max) {
    return (float)(rand() / ((float)RAND_MAX / max));
}

void toggleSound(void) {
    playSounds = !playSounds;
    if (playSounds) {
        al_play_sample(bgMusic, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
    }
    else {
        al_stop_samples();
    }
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
    if (!loadImage("shield.png", &shieldImage)) {
        unloadImages();
        return false;
    }
    if (!loadImage("lazer.png", &lazerImage)) {
        unloadImages();
        return false;
    }
    if (!loadImage("turret.png", &turretImage)) {
        unloadImages();
        return false;
    }
    return true;
}

bool loadSounds(void) {
    if (!al_reserve_samples(10)) {
        return false;
    }
    lazerSound = al_load_sample("laser.wav");
    if (!lazerSound) {
        unloadSounds();
        return false;
    }
    explosionSound = al_load_sample("explosion.wav");
    if (!explosionSound) {
        unloadSounds();
        return false;
    }
    bgMusic = al_load_sample("Afterburner.ogg");
    if (!bgMusic) {
        unloadSounds();
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
    if (shieldImage) {
        al_destroy_bitmap(shieldImage);
    }
    if (lazerImage) {
        al_destroy_bitmap(lazerImage);
    }
    if (turretImage) {
        al_destroy_bitmap(turretImage);
    }
}

void unloadSounds(void) {
    al_stop_samples();
    if (lazerSound) {
        al_destroy_sample(lazerSound);
    }
    if (explosionSound) {
        al_destroy_sample(explosionSound);
    }
    if (bgMusic) {
        al_destroy_sample(bgMusic);
    }
}

static void fixCamera(void) {
    struct Point middle = {windowSize.width / 2, windowSize.height / 2};
    camera.x = MAX(middle.x, MIN(MAP_MAX - middle.x, camera.x));
    camera.y = MAX(middle.y, MIN(MAP_MAX - middle.y, camera.y));
}

void buildMap(struct Size winSize) {
    windowSize = winSize;
    int maxStars = MAX_STARS;
    for (int i = 0; i < maxStars; i++) {
        float s = r(0.4) + 0.1;
		stars[i].size.width = 20 * s;
        stars[i].size.height = 20 * s;
        stars[i].pos.x = r(MAP_MAX - stars[i].size.width);
        stars[i].pos.y = r(MAP_MAX - stars[i].size.height);
        stars[i].tint = colors[(int)round(r(4))];
        stars[i].image = starImage;
    }
    { //player scope
        player.accel.x = 0;
        player.accel.y = 0;
        player.pos.x = MAP_MAX / 2;
        player.pos.y = MAP_MAX / 2;
        player.rotation = r(MAX_RADUINS);
		player.centre.x = 55;
		player.centre.y = 95;
		player.image[0] = shipImages[0];
		player.image[1] = shipImages[1];
        player.imageIndex = 0;
        player.shield = MAX_SHIELDS;
        player.health = MAX_HEALTH;
        player.shieldRecharge = 0;
        player.shotCooldown = 0;
		camera = player.pos;
        fixCamera();
    }
    int maxTurrets = MAX_TURRETS;
    for (int i = 0; i < maxTurrets; i++) {
        turrets[i].size.width = 100;
        turrets[i].size.height = 100;
        do {
            turrets[i].pos.x = r(MAP_MAX - turrets[i].size.width);
            turrets[i].pos.y = r(MAP_MAX - turrets[i].size.height);
        } while (distanceBetween(player.pos, turrets[i].pos) < LAZER_MAX_DISTANCE);
        turrets[i].image = turretImage;
        turrets[i].rotation = r(MAX_RADUINS);
        turrets[i].rotationIncrement = round(r(1.0)) ? -0.05 : 0.05;
        turrets[i].shotCooldown = TURRET_SHOT_RATE;
        turrets[i].dead = false;
    }
    if (playSounds) {
        al_play_sample(bgMusic, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);
    }
}

void removeLazers(int count, int indexes[]) {
    for (int i = count; i > 0 ; i--) {
        int index = indexes[i - 1];
        for (int j = index; j < lazerCount - 1; j++) {
            lazers[j] = lazers[j + 1];
        }
        lazerCount--;
    }
}

void updateLazers() {
    int removeIndexes[MAX_LAZERS] = {0};
    int removeCount = 0;
    for (int i = 0; i < lazerCount; i++) {
        lazers[i].distance = distanceBetween(lazers[i].pos, lazers[i].start);
        if (lazers[i].pos.x < 0 || lazers[i].pos.x > MAP_MAX || lazers[i].pos.y < 0 || lazers[i].pos.y > MAP_MAX || lazers[i].distance > LAZER_MAX_DISTANCE) {
            removeIndexes[removeCount] = i;
            removeCount++;
        }
        else {
            lazers[i].pos.x += lazers[i].accel.x;
            lazers[i].pos.y += lazers[i].accel.y;
        }
    }
    if (removeCount > 0) {
        removeLazers(removeCount, removeIndexes);
    }
}

void updateTurrets() {
    int maxTurrets = MAX_TURRETS;
    for (int i = 0; i < maxTurrets; i++) {
        if (!turrets[i].dead) {
            turrets[i].rotation += turrets[i].rotationIncrement;
            if (turrets[i].rotation > MAX_RADUINS) {
                turrets[i].rotation -= MAX_RADUINS;
            }
            if (turrets[i].rotation < 0) {
                turrets[i].rotation += MAX_RADUINS;
            }
            if (turrets[i].shotCooldown <= 0) {
                if (player.health > 0 && lazerCount < MAX_LAZERS && distanceBetween(turrets[i].pos, player.pos) < 700) {
                    struct Lazer lazer;
                    lazer.rotation = -atan2f(turrets[i].pos.x - player.pos.x, turrets[i].pos.y - player.pos.y) + (r(0.2) - 0.1);
                    lazer.start.x = lazer.pos.x = turrets[i].pos.x + (sin(lazer.rotation) * 50);
                    lazer.start.y = lazer.pos.y = turrets[i].pos.y + (-cos(lazer.rotation) * 50);
                    lazer.accel.x = (sin(lazer.rotation) * LAZER_FORCE);
                    lazer.accel.y = (-cos(lazer.rotation) * LAZER_FORCE);
                    lazer.players = false;
                    lazer.distance = 0;
                    lazers[lazerCount] = lazer;
                    lazerCount++;
                    turrets[i].shotCooldown = TURRET_SHOT_RATE;
                    if (playSounds) {
                        al_play_sample(lazerSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                    }
                }
            }
            else {
                turrets[i].shotCooldown -= 0.2;
            }
        }
    }
}

bool pointInPath(struct Point p, struct Point *points, int count) {
    int j = count - 1;
    bool oddNodes = false;
    for (int i = 0; i < count; i++) {
        if (((points[i].y < p.y && points[j].y >= p.y) || (points[j].y < p.y && points[i].y >= p.y)) && (points[i].x <= p.x || points[j].x <= p.x)) {
            oddNodes ^= (points[i].x + (p.y - points[i].y) / (points[j].y - points[i].y) * (points[j].x - points[i].x) < p.x);
        }
        j=i;
    }
    return oddNodes;
}

void rotateOffsetPath(int count, struct Point *srcPoints, struct Point *destPoints, float rotation, struct Point offset) {
    for (int i = 0; i < count; i++) {
        destPoints[i] = addPoints(rotatePoint(srcPoints[i], rotation), offset);
    }
}

void checkCollisions() {
    struct Point playerPath[68];
    bool shieldActive = player.shield > 0;
    if (shieldActive) {
        rotateOffsetPath(64, shieldCollisionPath, playerPath, player.rotation, player.pos);
    }
    else {
        rotateOffsetPath(68, shipCollisionPath, playerPath, player.rotation, player.pos);
    }
    int removeIndexes[MAX_LAZERS] = {0};
    int removeCount = 0;
    for (int i = 0; i < lazerCount; i++) {
        struct Point lazerPath[4];
        rotateOffsetPath(4, lazerCollisionPath, lazerPath, lazers[i].rotation, lazers[i].pos);
        if (lazers[i].players) {
            //check turrets
            int turretsCount = MAX_TURRETS;
            for (int j = 0; j < turretsCount; j++) {
                if (!turrets[j].dead && distanceBetween(turrets[j].pos, lazers[i].pos) < 65) {
                    rotateOffsetPath(95, turretCollisionPath, lastCollisionPath, turrets[j].rotation, turrets[j].pos);
                    for (int k = 0; k < 4; k++) {
                        if (pointInPath(lazerPath[k], lastCollisionPath, 95)) {
                            turrets[j].dead = true;
                            if (playSounds) {
                                al_play_sample(explosionSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            }
                            removeIndexes[removeCount] = i;
                            removeCount++;
                            break;
                        }
                    }
                }
            }
        }
        else {
            //check player
            if (distanceBetween(player.pos, lazers[i].pos) < 140) {
                if (shieldActive) {
                    for (int j = 0; j < 4; j++) {
                        if (pointInPath(lazerPath[j], playerPath, 64)) {
                            player.shield -= LAZER_DAMAGE;
                            player.shieldRecharge = 0;
                            removeIndexes[removeCount] = i;
                            removeCount++;
                            break;
                        }
                    }
                }
                else {
                    for (int j = 0; j < 4; j++) {
                        if (pointInPath(lazerPath[j], playerPath, 68)) {
                            player.shieldRecharge = 0;
                            player.health -= LAZER_DAMAGE;
                            removeIndexes[removeCount] = i;
                            removeCount++;
                            break;
                        }
                    }
                }
            }
        }
    }
    int turretsCount = MAX_TURRETS;
    bool allTurretsDead = true;
    for (int i = 0; i < turretsCount; i++) {
        if (!turrets[i].dead) {
            allTurretsDead = false;
            //check collision with player
            //if collide calculate force on angle between player and turret and then apply opposing force * 2 (reverse it)
            if (distanceBetween(turrets[i].pos, player.pos) < 180) {
                rotateOffsetPath(95, turretCollisionPath, lastCollisionPath, turrets[i].rotation, turrets[i].pos);
                for (int j = 0; j < 95; j++) {
                    if (shieldActive) {
                        if (pointInPath(lastCollisionPath[j], playerPath, 64)) {
                            float angle = -atan2f(turrets[i].pos.x - player.pos.x, turrets[i].pos.y - player.pos.y);
                            float currentXAngleForce = abs(sin(angle) * player.accel.x);
                            float currentYAngleForce = abs(-cos(angle) * player.accel.y);
                            player.accel.x += (sin(angle) * MAX(currentXAngleForce, 0.5)) * 2;
                            player.accel.y += (-cos(angle) * MAX(currentYAngleForce, 0.5)) * 2;
                            player.shield -= LAZER_DAMAGE * 1.5;
                            player.shieldRecharge = 0;
                            turrets[i].dead = true;
                            if (playSounds) {
                                al_play_sample(explosionSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            }
                            break;
                        }
                    }
                    else {
                        if (pointInPath(lastCollisionPath[j], playerPath, 68)) {
                            float angle = -atan2f(turrets[i].pos.x - player.pos.x, turrets[i].pos.y - player.pos.y);
                            float currentXAngleForce = abs(sin(angle) * player.accel.x);
                            float currentYAngleForce = abs(-cos(angle) * player.accel.y);
                            player.accel.x += (sin(angle) * MAX(currentXAngleForce, 0.5)) * 2;
                            player.accel.y += (-cos(angle) * MAX(currentYAngleForce, 0.5)) * 2;
                            player.shieldRecharge = 0;
                            player.health -= LAZER_DAMAGE * 1.5;
                            turrets[i].dead = true;
                            if (playSounds) {
                                al_play_sample(explosionSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    if (removeCount > 0) {
        removeLazers(removeCount, removeIndexes);
    }
    if (player.health < 0) {
        if (playSounds) {
            al_play_sample(explosionSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }
        gameOver = true;
    }
    if (allTurretsDead) {
        gameOver = true;
        win = true;
    }
}

void moveObjects(void) {
    if (gameOver) {
        return;
    }
    checkCollisions();
	ALLEGRO_KEYBOARD_STATE keyState;
    player.accel.x -= player.accel.x * DECAY;
    player.accel.y -= player.accel.y * DECAY;
    if (player.accel.x < 0.01 && player.accel.x > -0.01) {
        player.accel.x = 0;
    }
    if (player.accel.y < 0.01 && player.accel.y > -0.01) {
        player.accel.y = 0;
    }
    if (player.shotCooldown > 0) {
        player.shotCooldown -= 0.2;
        player.shotCooldown = MAX(player.shotCooldown, 0);
    }
    if (player.shieldRecharge > 180) {
        if (player.shield < MAX_SHIELDS) {
            player.shield += 0.01;
            player.shield = MIN(player.shield, MAX_SHIELDS);
        }
    }
    else {
        player.shieldRecharge += 1;
    }
    updateLazers();
    updateTurrets();
    al_get_keyboard_state(&keyState);
    if (al_key_down(&keyState, ALLEGRO_KEY_UP)) {
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
    player.pos.x += player.accel.x;
    player.pos.y += player.accel.y;
    camera = player.pos;
    player.pos.x = MAX(0, MIN(MAP_MAX, player.pos.x));
    player.pos.y = MAX(0, MIN(MAP_MAX, player.pos.y));
    fixCamera();
    if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
        player.rotation -= 0.05;
    }
    if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
        player.rotation += 0.05;
    }
    if (player.rotation > MAX_RADUINS) {
        player.rotation -= MAX_RADUINS;
    }
    else if (player.rotation < 0) {
        player.rotation += MAX_RADUINS;
    }
    if (al_key_down(&keyState, ALLEGRO_KEY_SPACE) && player.shotCooldown <= 0 && lazerCount < MAX_LAZERS) {
        player.shotCooldown = PLAYER_SHOT_RATE;
        struct Lazer lazer;
        lazer.rotation = player.rotation + (r(0.06) - 0.03);
        lazer.start.x = lazer.pos.x = player.pos.x + (sin(lazer.rotation) * 100);
        lazer.start.y = lazer.pos.y = player.pos.y + (-cos(lazer.rotation) * 100);
        float shipXAngleForce = sin(player.rotation) * player.accel.x;
        float shipYAngleForce = -cos(player.rotation) * player.accel.y;
        lazer.accel.x = (sin(lazer.rotation) * (LAZER_FORCE + shipXAngleForce));
        lazer.accel.y = (-cos(lazer.rotation) * (LAZER_FORCE + shipYAngleForce));
        lazer.distance = 0;
        lazer.players = true;
        lazers[lazerCount] = lazer;
        lazerCount++;
        if (playSounds) {
            al_play_sample(lazerSound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }
    }
}

void renderObjects(void) {
    struct Point beg = {camera.x - (windowSize.width / 2), camera.y - (windowSize.height / 2)};
    struct Point end = {camera.x + (windowSize.width / 2), camera.y + (windowSize.height / 2)};
    int maxStars = MAX_STARS;
    for (int i = 0; i < maxStars; i++) {
        struct Star s = stars[i];
        if (s.pos.x > beg.x - 10 && s.pos.x < end.x + 10 && s.pos.y > beg.y - 10 && s.pos.y < end.y + 10) {
            al_draw_tinted_scaled_bitmap(s.image, s.tint, 0, 0, 20, 20, s.pos.x - beg.x, s.pos.y - beg.y, s.size.width, s.size.height, 0);
        }
    }
    int maxTurrets = MAX_TURRETS;
    for (int i = 0; i < maxTurrets; i++) {
        struct Turret t = turrets[i];
        if (!t.dead) {
            if (t.pos.x > beg.x - 50 && t.pos.x < end.x + 50 && t.pos.y > beg.y - 50 && t.pos.y < end.y + 50) {
                if (showTextures) {
                    al_draw_rotated_bitmap(t.image, 50, 50, t.pos.x - beg.x, t.pos.y - beg.y, t.rotation, 0);
                }
                else {
                    struct Point negOffset = makePoint(-beg.x, -beg.y);
                    rotateOffsetPath(95, turretCollisionPath, lastCollisionPath, t.rotation, addPoints(t.pos, negOffset));
                    al_draw_polygon((float*)lastCollisionPath, 95, ALLEGRO_LINE_JOIN_MITER, al_map_rgb_f(1, 0, 0), 1, 1);
                }
            }
            else {
                //draw radar dot
                float a1 = turrets[i].pos.y - camera.y;
                float b1 = camera.x - turrets[i].pos.x;
                float c1 = (a1 * camera.x) + (b1 * camera.y);
                float a2;
                float b2;
                float c2;
                if (turrets[i].pos.y < camera.y) {
                    a2 = 0;
                    b2 = beg.x - end.x;
                    c2 = (a2 * beg.x) + (b2 * beg.y);
                    double x = ((b2 * c1) - (b1 * c2)) / ((a1 * b2) - (a2 * b1));
                    if (x >= beg.x && x <= end.x) {
                        al_draw_filled_circle(x - beg.x, beg.y - beg.y, 3, al_map_rgb_f(1, 0, 0));
                    }
                }
                else {
                    a2 = 0;
                    b2 = beg.x - end.x;
                    c2 = (a2 * beg.x) + (b2 * end.y);
                    double x = ((b2 * c1) - (b1 * c2)) / ((a1 * b2) - (a2 * b1));
                    if (x >= beg.x && x <= end.x) {
                        al_draw_filled_circle(x - beg.x, end.y - beg.y, 3, al_map_rgb_f(1, 0, 0));
                    }
                }
                if (turrets[i].pos.x < camera.x) {
                    a2 = end.y - beg.y;
                    b2 = 0;
                    c2 = (a2 * beg.x) + (b2 * end.y);
                    double y = ((a1 * c2) - (a2 * c1)) / ((a1 * b2) - (a2 * b1));
                    if (y >= beg.y && y <= end.y) {
                        al_draw_filled_circle(beg.x - beg.x, y - beg.y, 3, al_map_rgb_f(1, 0, 0));
                    }
                }
                else {
                    a2 = end.y - beg.y;
                    b2 = 0;
                    c2 = (a2 * end.x) + (b2 * end.y);
                    double y = ((a1 * c2) - (a2 * c1)) / ((a1 * b2) - (a2 * b1));
                    if (y >= beg.y && y <= end.y) {
                        al_draw_filled_circle(end.x - beg.x, y - beg.y, 3, al_map_rgb_f(1, 0, 0));
                    }
                }
            }
        }
    }
    for (int i = 0; i < lazerCount; i++) {
        struct Lazer l = lazers[i];
        if (l.pos.x > beg.x - 18 && l.pos.x < end.x + 18 && l.pos.y > beg.y - 18 && l.pos.y < end.y + 18) {
            if (showTextures) {
                float tint = (((LAZER_MAX_DISTANCE - l.distance) / LAZER_MAX_DISTANCE) * 0.3) + 0.7;
                al_draw_tinted_rotated_bitmap(lazerImage, al_map_rgba_f(tint, tint, tint, tint), 10, 18, l.pos.x - beg.x, l.pos.y - beg.y, l.rotation, 0);
            }
            else {
                struct Point negOffset = makePoint(-beg.x, -beg.y);
                rotateOffsetPath(4, lazerCollisionPath, lastCollisionPath, l.rotation, addPoints(l.pos, negOffset));
                al_draw_polygon((float*)lastCollisionPath, 4, ALLEGRO_LINE_JOIN_MITER, al_map_rgb_f(1, 0, 0), 1, 1);
            }
        }
    }
    if (player.health > 0) { //player scope
        struct SpaceShip s = player;
        if (s.shield > 0) {
            if (showTextures) {
                float tint = ((s.shield / MAX_SHIELDS) * 0.7) + 0.25;
                al_draw_tinted_rotated_bitmap(shieldImage, al_map_rgba_f(tint, tint, tint, tint), 75, 125, s.pos.x - beg.x, s.pos.y - beg.y, s.rotation, 0);
            }
            else {
                struct Point negOffset = makePoint(-beg.x, -beg.y);
                rotateOffsetPath(64, shieldCollisionPath, lastCollisionPath, s.rotation, addPoints(s.pos, negOffset));
                al_draw_polygon((float*)lastCollisionPath, 64, ALLEGRO_LINE_JOIN_MITER, al_map_rgb_f(1, 0, 0), 1, 1);
            }
        }
        if (showTextures) {
            float tint = (player.health / MAX_HEALTH * 0.5) + 0.5;
            al_draw_tinted_rotated_bitmap(s.image[s.imageIndex], al_map_rgb_f(tint, tint, tint), s.centre.x, s.centre.y, s.pos.x - beg.x, s.pos.y - beg.y, s.rotation, 0);
        }
        else {
            struct Point negOffset = makePoint(-beg.x, -beg.y);
            rotateOffsetPath(68, shipCollisionPath, lastCollisionPath, s.rotation, addPoints(s.pos, negOffset));
            al_draw_polygon((float*)lastCollisionPath, 68, ALLEGRO_LINE_JOIN_MITER, al_map_rgb_f(1, 0, 0), 1, 1);
        }
    }
}
