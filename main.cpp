#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <format>

#define HEX_COLOR(c) ((Color) { \
    (unsigned char) (c >> 24), \
    (unsigned char) ((c >> 16) & 0xff)  , \
    (unsigned char) ((c >> 8) & 0xff), \
    (unsigned char) (c & 0xff) \
})

#define CONNECTION_DIST 400

enum NewPointState {
    IdleNewPoint,
    AddNewPoint,
};

typedef struct {
    Vector2 pos;
    Vector2 dir;
    Color color;
    float radius;
} Point;

Font font;

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

size_t take_until(char* text, char* buf, size_t off, char stop_char) {
    size_t i = 0;
    while (text[off + i] != stop_char && text[off + i] != '\0') {
        buf[i] = text[off + i];
        i++;
    }
    return i + 1;
}

void print_point(Point point) {
    printf("Point{\n");
    printf("  .pos.x = %.2f\n", point.pos.x);
    printf("  .pos.y = %.2f\n", point.pos.y);
    printf("  .dir.x = %.2f\n", point.dir.x);
    printf("  .dir.y = %.2f\n", point.dir.y);
    printf("  .color = 0x%.2x%.2x%.2x%.2x\n", point.color.r, point.color.g, point.color.b, point.color.a);
    printf("  .radius = %.2f\n", point.radius);
    printf("}\n");
}

std::vector<Point> read_points(const char* file_path) {
    FILE* points_fd = fopen(file_path, "r");
    if (points_fd == NULL) {
        perror("File error");
        exit(1);
    }

    fseek(points_fd, 0, SEEK_END);

    size_t file_size = ftell(points_fd);

    fseek(points_fd, 0, SEEK_SET);

    char* points_text = (char*)calloc(1, file_size + 1);
    if (points_text == NULL) {
        perror("Malloc failed");
        exit(1);
    }

    size_t bytes_read = fread(points_text, 1, file_size, points_fd);
    if (bytes_read != file_size) {
        fprintf(stderr, "read %ld/%ld\n", bytes_read, file_size);
        exit(1);
    }

    fclose(points_fd);

    std::vector<Point> points;

    size_t i = 0;
    while (i < file_size) {
        Point point = {};

        char buf[256];

        // ---- point.pos.x

        memset(buf, 0, 256);

        i += take_until(points_text, buf, i, ',');

        point.pos.x = atof(buf);

        // ---- point.pos.y

        memset(buf, 0, 256);

        i += take_until(points_text, buf, i, ',');
        point.pos.y = atof(buf);

        // ---- point.dir.x

        memset(buf, 0, 256);

        i += take_until(points_text, buf, i, ',');
        point.dir.x = atof(buf);

        // ---- point.dir.y

        memset(buf, 0, 256);

        i += take_until(points_text, buf, i, ',');
        point.dir.y = atof(buf);

        // ---- point.color

        memset(buf, 0, 256);

        i += take_until(points_text, buf, i, ',');
        point.color = HEX_COLOR(strtol(buf, 0, 16));

        // ---- point.radius
        memset(buf, 0, 256);

        i += take_until(points_text, buf, i, '\n');
        point.radius = atof(buf);

        points.push_back(point);

    }

    free(points_text);

    return points;
}

typedef struct {
    int state;
    Point p;
} NewPoint;

NewPoint new_point_state = { 0 };

void init_point(Vector2 v) {
    new_point_state.state = AddNewPoint;
    new_point_state.p.pos = v;
    new_point_state.p.color = LIME;
    new_point_state.p.dir.x = 0.1;
    new_point_state.p.dir.y = 0.1;
}

std::vector<Point> add_new_point(std::vector<Point> &points) {
    points.push_back(new_point_state.p);
    new_point_state.state = IdleNewPoint;
    return points;
}

void move_points(std::vector<Point> &points, std::vector<std::vector<size_t>>& pairs) {
    for (size_t i = 0; i < points.size(); i++) {
        Point point = points[i];

        point.pos.x += point.dir.x * 6;
        point.pos.y += point.dir.y * 6;

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
}

void drawPoints(const std::vector<Point> &points, const std::vector<std::vector<size_t>> &pairs) {
    for (std::vector<size_t> pair : pairs) {
        Point p1 = points[pair[0]];
        Point p2 = points[pair[1]];

        Vector2 delta = Vector2Subtract(p2.pos, p1.pos);
        float dist = Vector2Length(delta);

        if (dist <= CONNECTION_DIST) {
            DrawLineV(p1.pos, p2.pos, LIME);
        }
    }

    for (size_t i = 0; i < points.size(); i++) {
        Point point = points[i];
        DrawCircleV(point.pos, point.radius, point.color);
    }

    if (new_point_state.state == AddNewPoint) {
        DrawCircleV(new_point_state.p.pos, new_point_state.p.radius, (Color) { 0, 0xff, 0, 0x30 });
    }
}

void drawInfo(std::vector<Point>& points) {
    Vector2 textPos = {2, 4};

    {
        std::string text = std::format("Screen w={}, h={}", GetScreenWidth(), GetScreenHeight());
        DrawTextEx(font, text.c_str(), textPos, (float)font.baseSize, 2, LIME);
        textPos.y += font.baseSize;
    }

    std::string countText = "Count: " + std::to_string(points.size());
    
    DrawTextEx(font, countText.c_str(), textPos, (float)font.baseSize, 2, LIME);

    for (size_t i = 0; i < points.size(); i++) {
        Point p = points[i];

        textPos.y += font.baseSize;
        
        std::string pointText = std::format("({:d} ; {:d})", (int)p.pos.x, (int)p.pos.y);

        DrawTextEx(font, pointText.c_str(), textPos, (float)font.baseSize, 2, LIME);
    }
}

int main() {
    std::vector<Point> points = read_points("resources/points.txt");

    std::vector<std::vector<size_t>> pairs = gen_pairs(points);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int windowWidth = 1200;
    int windowHeight = 900;

    SetTraceLogLevel(LOG_ERROR);

    InitWindow(windowWidth, windowHeight, "Floating dots");

    if (FileExists("resources/font.ttf")) {
        TraceLog(LOG_INFO, "font exists");
    } else {
        TraceLog(LOG_ERROR, "font does not exist");
    }

    font = LoadFontEx("resources/font.ttf", 14, 0, 0);
    if (font.texture.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load font!");
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        move_points(points, pairs);

        if (IsKeyPressed(KEY_SPACE)) {
            if (new_point_state.state == IdleNewPoint) {
                Vector2 pos = GetMousePosition();
                init_point(pos);
            }
            else if (new_point_state.state == AddNewPoint) {
                if (new_point_state.p.radius > 3) {
                    points = add_new_point(points);
                    pairs = gen_pairs(points);
                }
            }
        }

        if (new_point_state.state == AddNewPoint) {
            Vector2 pos = GetMousePosition();
            float radius = Vector2Distance(new_point_state.p.pos, pos);
            new_point_state.p.radius = radius;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (new_point_state.state == AddNewPoint) {
                new_point_state.state = IdleNewPoint;
            }
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        drawPoints(points, pairs);

        // Draw Info
        drawInfo(points);

        EndDrawing();
    }

    UnloadFont(font);

    return 0;
}
