#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <raylib.h>

#define WIDTH 640
#define HEIGHT 640

#define MAX_PARTICLE_COUNT 20
#define MAX_PARTICLE_DISTANCE 20

#define MAX_CONSTRAINT_ITERATION 5

#define GRAVITY 980.0
#define DAMPING 0.98

typedef struct Particle {
    bool pinned; 

    Vector2 position;
    Vector2 previous_position;

    Vector2 acceleration;

    Color color;
} Particle;

Particle PARTICLES[MAX_PARTICLE_COUNT + 1];
bool ROPE_INITIALIZED = false;
int PINNABLE = 0;

bool DEBUG_CIRCLES = false;

Vector2 vec2sum(Vector2 a , Vector2 b) {
    return (Vector2) { a.x + b.x , a.y + b.y }; 
}

Vector2 vec2sub(Vector2 a , Vector2 b) {
    return (Vector2) { a.x - b.x , a.y - b.y }; 
}

Vector2 vec2mulf(Vector2 a , float b) {
    return (Vector2) { a.x * b , a.y * b };
}

void DisplayRope() {
    for (int i = 0; i < MAX_PARTICLE_COUNT + 1; i++) {
        if (DEBUG_CIRCLES) DrawCircle(PARTICLES[i].position.x , PARTICLES[i].position.y , 10 , WHITE);
        if (i != MAX_PARTICLE_COUNT + 1 - 1) DrawLineEx(PARTICLES[i].position , PARTICLES[i+1].position , 5.0 , PARTICLES[i].color);
        else DrawLineEx(PARTICLES[i].position , PARTICLES[i].position , 5 , PARTICLES[i].color);
    }
}

void UpdateRope(float deltaTime) {
    for (int i = 0; i < MAX_PARTICLE_COUNT + 1; i++) {
        
        if (!PARTICLES[i].pinned) {
            if (i == PINNABLE) {
                PARTICLES[i].position = (Vector2) { GetMousePosition().x , GetMousePosition().y };
                PARTICLES[i].previous_position = PARTICLES[i].position;
                continue;
            }

            PARTICLES[i].acceleration.y += GRAVITY;

            Vector2 velocity = { PARTICLES[i].position.x - PARTICLES[i].previous_position.x , PARTICLES[i].position.y - PARTICLES[i].previous_position.y };
        
            velocity.x *= DAMPING;
            velocity.y *= DAMPING;

            PARTICLES[i].previous_position = PARTICLES[i].position;
            PARTICLES[i].position = vec2sum(vec2sum(PARTICLES[i].position , velocity) , vec2mulf(PARTICLES[i].acceleration , deltaTime * deltaTime));
            PARTICLES[i].acceleration = (Vector2) { 0 , 0 };

        }
    }
}

void UpdateConstraintRope() {
    for (int i = 0; i < MAX_PARTICLE_COUNT + 1 - 1; i++) {

        Vector2 d = {
            PARTICLES[i+1].position.x - PARTICLES[i].position.x,
            PARTICLES[i+1].position.y - PARTICLES[i].position.y
        };

        float l = sqrt(pow(d.x , 2) + pow(d.y , 2));
        if (l < 0.0001f) continue;

        Vector2 n = { d.x / l , d.y / l };

        float difference = l - MAX_PARTICLE_DISTANCE;
        
        Vector2 c = {
            n.x * difference * 0.5f,
            n.y * difference * 0.5f
        };

        if (!PARTICLES[i].pinned) {
            PARTICLES[i].position.x += c.x;
            PARTICLES[i].position.y += c.y;
        }
        // PARTICLES[i].previous_position.x += c.x;
        // PARTICLES[i].previous_position.y += c.y;

        if (!PARTICLES[i+1].pinned) {
            PARTICLES[i+1].position.x -= c.x;
            PARTICLES[i+1].position.y -= c.y;
        }
        // PARTICLES[i+1].previous_position.x -= c.x;
        // PARTICLES[i+1].previous_position.y -= c.y;

    }
}

int main(void) {

    InitWindow(WIDTH , HEIGHT , "2D Rope Physics");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !ROPE_INITIALIZED) {
            for (int i = 0; i < MAX_PARTICLE_COUNT + 1; i++) {              

                Vector2 position = { GetMousePosition().x , GetMousePosition().y + MAX_PARTICLE_DISTANCE * i };
                Vector2 previous_position = position;

                Color color = { rand() % 256 , rand() % 256 , rand() % 256 , 255 };

                PARTICLES[i] = (Particle){ false , position , previous_position , (Vector2){ 0 , 0 } , color};
            }
            ROPE_INITIALIZED = true;
        }

        if (IsKeyPressed(KEY_D)) {
            DEBUG_CIRCLES = !DEBUG_CIRCLES;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (!PARTICLES[0].pinned) { PARTICLES[0].pinned = true; PINNABLE = MAX_PARTICLE_COUNT; }
            else PARTICLES[MAX_PARTICLE_COUNT].pinned = true; 
        }
 
        if (ROPE_INITIALIZED) {
            DisplayRope();
            UpdateRope(GetFrameTime());
            for (int i = 0; i < MAX_CONSTRAINT_ITERATION; i++) {
                UpdateConstraintRope();
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}