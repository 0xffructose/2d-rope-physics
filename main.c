#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>

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

typedef struct Rope {
    Particle PARTICLES[MAX_PARTICLE_COUNT + 1];
} Rope;

typedef struct Ropes {
    Rope *items;
    size_t count;
    size_t capacity;
} Ropes;

#define append_rope(ropes , rope) \
    do {\
        if (ropes.count >= ropes.capacity) {\
            if (ropes.capacity == 0) ropes.capacity = 1024;\
            else ropes.capacity *= 2;\
            ropes.items = realloc(ropes.items , ropes.capacity * sizeof(*ropes.items));\
        }\
        ropes.items[ropes.count++] = rope;\
    } while (0)

// Particle PARTICLES[MAX_PARTICLE_COUNT + 1]; // 672 BYTE
Ropes ROPES = { .items = NULL , .count = 0 , .capacity = 0 };

// bool ROPE_INITIALIZED = false;

int ACTIVE_ROPE = -1;
int PINNABLE = 0;

bool DEBUG_CIRCLES = false;

void DisplayRope() {
    for (int i = 0; i < ROPES.count; i++) {
        for (int j = 0; j < MAX_PARTICLE_COUNT + 1; j++) {
            if (DEBUG_CIRCLES) DrawCircle(ROPES.items[i].PARTICLES[j].position.x , ROPES.items[i].PARTICLES[j].position.y , 10 , WHITE);
            if (j != MAX_PARTICLE_COUNT + 1 - 1) DrawLineEx(ROPES.items[i].PARTICLES[j].position , ROPES.items[i].PARTICLES[j+1].position , 5.0 , ROPES.items[i].PARTICLES[j].color);
            else DrawLineEx(ROPES.items[i].PARTICLES[j].position , ROPES.items[i].PARTICLES[j].position , 5 , ROPES.items[i].PARTICLES[j].color);
        }
    }
}

void UpdateRope(float deltaTime) {
    for (int i = 0; i < ROPES.count; i++) {
        for (int j = 0; j < MAX_PARTICLE_COUNT + 1; j++) {
            if (!ROPES.items[i].PARTICLES[j].pinned) {
                if (j == PINNABLE) {
                    ROPES.items[i].PARTICLES[j].position = (Vector2) { GetMousePosition().x , GetMousePosition().y };
                    ROPES.items[i].PARTICLES[j].previous_position = ROPES.items[i].PARTICLES[j].position;
                    continue;
                }

                ROPES.items[i].PARTICLES[j].acceleration.y += GRAVITY;

                Vector2 velocity = { ROPES.items[i].PARTICLES[j].position.x - ROPES.items[i].PARTICLES[j].previous_position.x , ROPES.items[i].PARTICLES[j].position.y - ROPES.items[i].PARTICLES[j].previous_position.y };
            
                velocity.x *= DAMPING;
                velocity.y *= DAMPING;

                ROPES.items[i].PARTICLES[j].previous_position = ROPES.items[i].PARTICLES[j].position;
                ROPES.items[i].PARTICLES[j].position = Vector2Add(Vector2Add(ROPES.items[i].PARTICLES[j].position , velocity) , Vector2Scale(ROPES.items[i].PARTICLES[j].acceleration , deltaTime * deltaTime));
                ROPES.items[i].PARTICLES[j].acceleration = (Vector2) { 0 , 0 };

            }
        }
    }
}

void UpdateConstraintRope() {
    for (int i = 0; i < ROPES.count; i++) {
        for (int j = 0; j < MAX_PARTICLE_COUNT + 1 - 1; j++) {
            Vector2 d = {   
                ROPES.items[i].PARTICLES[j+1].position.x - ROPES.items[i].PARTICLES[j].position.x,
                ROPES.items[i].PARTICLES[j+1].position.y - ROPES.items[i].PARTICLES[j].position.y
            };

            float l = sqrt(pow(d.x , 2) + pow(d.y , 2));
            if (l < 0.0001f) continue;

            Vector2 n = { d.x / l , d.y / l };

            float difference = l - MAX_PARTICLE_DISTANCE;
            
            Vector2 c = {
                n.x * difference * 0.5f,
                n.y * difference * 0.5f
            };

            if (!ROPES.items[i].PARTICLES[j].pinned) {
                ROPES.items[i].PARTICLES[j].position.x += c.x;
                ROPES.items[i].PARTICLES[j].position.y += c.y;
            }
            // PARTICLES[i].previous_position.x += c.x;
            // PARTICLES[i].previous_position.y += c.y;

            if (!ROPES.items[i].PARTICLES[j+1].pinned) {
                ROPES.items[i].PARTICLES[j+1].position.x -= c.x;
                ROPES.items[i].PARTICLES[j+1].position.y -= c.y;
            }
            // PARTICLES[i+1].previous_position.x -= c.x;
            // PARTICLES[i+1].previous_position.y -= c.y;
        }
    }
}

int main(void) {

    InitWindow(WIDTH , HEIGHT , "2D Rope Physics");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ACTIVE_ROPE == -1) {

            Rope ROPE = { 0 };

            for (int i = 0; i < MAX_PARTICLE_COUNT + 1; i++) {              

                Vector2 position = { GetMousePosition().x , GetMousePosition().y + MAX_PARTICLE_DISTANCE * i };
                Vector2 previous_position = position;

                Color color = { rand() % 256 , rand() % 256 , rand() % 256 , 255 };

                ROPE.PARTICLES[i] = (Particle){ false , position , previous_position , (Vector2){ 0 , 0 } , color};
            }

            append_rope(ROPES , ROPE);
            
            ACTIVE_ROPE = ROPES.count - 1;
            PINNABLE = 0;
            // ROPE_INITIALIZED = true;
        }

        if (IsKeyPressed(KEY_D)) {
            DEBUG_CIRCLES = !DEBUG_CIRCLES;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (!ROPES.items[ACTIVE_ROPE].PARTICLES[0].pinned) { ROPES.items[ACTIVE_ROPE].PARTICLES[0].pinned = true; PINNABLE = MAX_PARTICLE_COUNT; }
            else { ROPES.items[ACTIVE_ROPE].PARTICLES[MAX_PARTICLE_COUNT].pinned = true; ACTIVE_ROPE = -1; }
        }
 
        if (ROPES.count > 0) {
            DisplayRope();
            UpdateRope(GetFrameTime());
            for (int i = 0; i < MAX_CONSTRAINT_ITERATION; i++) {
                UpdateConstraintRope();
            }
        }

        EndDrawing();
    }

    CloseWindow();

    if (ROPES.items != NULL) {
        free(ROPES.items);
        ROPES.items = NULL;
    }

    ROPES.count = 0;
    ROPES.capacity = 0;

    return 0;
}