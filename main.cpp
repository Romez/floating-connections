#include <stdio.h>
#include <raylib.h>
#include <vector>

typedef struct
{
    Vector2 pos;
    Vector2 dir;
} Point;


int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int windowWidth = 1200;
    int windowHeight = 900;

    InitWindow(windowWidth, windowHeight, "Floating dots");

    SetTargetFPS(60);

    float radius = 50;

    std::vector<Point> points;
    points.push_back({
        .pos = {.x = 500, .y = 500},
        .dir = {.x = 1, .y = 1},
    });

    points.push_back({
        .pos = {.x = 800, .y = 600},
        .dir = {.x = 1, .y = 1},
    });

    while (!WindowShouldClose()) {
        for (size_t i = 0; i < points.size(); i++) {
            Point point = points[i];

            point.pos.x += point.dir.x * 10;
            point.pos.y += point.dir.y * 10;

            if (point.pos.x - radius < 0) {
                point.pos.x = radius;
                point.dir.x = -point.dir.x;
            }

            if (point.pos.y - radius < 0) {
                point.pos.y = radius;
                point.dir.y = -point.dir.y;
            }

            if (point.pos.x + radius > GetScreenWidth()) {
                point.pos.x = GetScreenWidth() - radius;
                point.dir.x = -point.dir.x;
            }

            if (point.pos.y + radius > GetScreenHeight()) {
                point.pos.y = GetScreenHeight() - radius;
                point.dir.y = -point.dir.y;
            }

            points[i] = point;
        }

        

        BeginDrawing();
        ClearBackground(DARKGRAY);

        for (size_t i = 0; i < points.size(); i++) {
            Point point = points[i];
            DrawCircleV(point.pos, radius, YELLOW);
        }

        EndDrawing();
    }

    return 0;
}