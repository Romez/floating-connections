#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <vector>

typedef struct {
    Vector2 pos;
    Vector2 dir;
    Color color;
    float radius;
} Point;

void printPoint(Point p) {
    printf("(%02f;%02f)\n", p.dir.x, p.dir.y);
}

void gen_pairs_iter(size_t points_size, size_t l, std::vector<std::vector<size_t>>* pairs, std::vector<size_t> pair) {
    if (pair.size() == 2) {
        pairs->push_back(std::vector<size_t>{pair[0], pair[1]});

        return;
    }

    for (size_t i = l; i < points_size; i++) {
        pair.push_back(i);
        gen_pairs_iter(points_size, i + 1, pairs, pair);
        pair.pop_back();
    }
}

std::vector<std::vector<size_t>> gen_pairs(std::vector<Point> points) {
    std::vector<std::vector<size_t>> pairs;
    std::vector<size_t> pair;
    gen_pairs_iter(points.size(), 0, &pairs, pair);

    return pairs;
}

int main() {
    // Vector2 v = {.x = 4, .y = 3};

    // printf("N, %f, %f", Vector2Normalize(v).x, Vector2Normalize(v).y);
    // return 1;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int windowWidth = 1200;
    int windowHeight = 900;

    InitWindow(windowWidth, windowHeight, "Floating dots");

    SetTargetFPS(60);

    std::vector<Point> points;
    points.push_back({
        .pos = {.x = 200, .y = 800},
        .dir = {.x = 1, .y = -1},
        .color = YELLOW,
        .radius = 50.0,
	});

    points.push_back({
        .pos = {.x = 800, .y = 200},
        .dir = {.x = -1, .y = 1},
        .color = YELLOW,
        .radius = 50.0,
	});

    points.push_back({
        .pos = {.x = 1000, .y = 800},
        .dir = {.x = 1, .y = 1},
        .color = YELLOW,
        .radius = 50.0,
	});

    points.push_back({
        .pos = {.x = 1100, .y = 200},
        .dir = {.x = 1, .y = 1},
        .color = YELLOW,
        .radius = 30.0,
	});

    points.push_back({
        .pos = {.x = 1100, .y = 400},
        .dir = {.x = 1, .y = 1},
        .color = YELLOW,
        .radius = 20.0,
	});

    std::vector<std::vector<size_t>> pairs = gen_pairs(points);

    while (!WindowShouldClose()) {
        for (size_t i = 0; i < points.size(); i++) {
            Point point = points[i];

            point.pos.x += point.dir.x * 10;
            point.pos.y += point.dir.y * 10;

            if (point.pos.x - point.radius < 0) {
                point.pos.x = point.radius;
                point.dir.x = -point.dir.x;
            }

            if (point.pos.y - point.radius < 0) {
                point.pos.y = point.radius;
                point.dir.y = -point.dir.y;
            }

            if (point.pos.x + point.radius > GetScreenWidth()) {
                point.pos.x = GetScreenWidth() - point.radius;
                point.dir.x = -point.dir.x;
            }

            if (point.pos.y + point.radius > GetScreenHeight()) {
                point.pos.y = GetScreenHeight() - point.radius;
                point.dir.y = -point.dir.y;
            }

            points[i] = point;
        }

        for (std::vector<size_t> pair : pairs) {
            Point p1 = points[pair[0]];
            Point p2 = points[pair[1]];

            Vector2 delta = Vector2Subtract(p2.pos, p1.pos);
            float dist = Vector2Length(delta);

            if (dist == 0.0f || dist < (p1.radius + p2.radius)) {
                // 1) Normalized vector & tangent vector
                Vector2 n = Vector2Normalize(delta);
                Vector2 t = (Vector2){ .x = -n.y, .y = n.x };

                // 2) Projections of the dir onto the normal n and the tangent t
                float v1n = Vector2DotProduct(p1.dir, n);
                float v1t = Vector2DotProduct(p1.dir, t);

                float v2n = Vector2DotProduct(p2.dir, n);
                float v2t = Vector2DotProduct(p2.dir, t);

                // 3) Build new dir
                Vector2 v1n_vec = Vector2Scale(n, v2n);
                Vector2 v1t_vec = Vector2Scale(t, v1t);
                Vector2 v2n_vec = Vector2Scale(n, v1n);
                Vector2 v2t_vec = Vector2Scale(t, v2t);

                p1.dir = Vector2Add(v1n_vec, v1t_vec);
                p2.dir = Vector2Add(v2n_vec, v2t_vec);

                // 4) Separete the balls so they don't overlap
                float overlap = 0.5f * ((p1.radius + p2.radius) - dist);
                p1.pos = Vector2Subtract(p1.pos, Vector2Scale(n, overlap));
                p2.pos = Vector2Add(p2.pos, Vector2Scale(n, overlap));

                points[pair[0]] = p1;
                points[pair[1]] = p2;
            }
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        for (size_t i = 0; i < points.size(); i++) {
            Point point = points[i];
            DrawCircleV(point.pos, point.radius, point.color);
        }

        EndDrawing();
    }

    return 0;
}
