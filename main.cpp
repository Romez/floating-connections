#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <string>
#include <format>
#include <iostream>
#include <fstream>

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

void getPairsIter(size_t pointsSize, size_t l, std::vector<std::vector<size_t>>& pairs, std::pair<size_t, size_t> pair, size_t pairSize) {
    if (pairSize == 2) {
        pairs.push_back(std::vector<size_t>{pair.first, pair.second});

        return;
    }

    for (size_t i = l; i < pointsSize; i++) {
        if (pairSize == 0) {
            pair.first = i;
        } else {
            pair.second = i;
        }

        getPairsIter(pointsSize, i + 1, pairs, pair, pairSize + 1);
    }
}

std::vector<std::vector<size_t>> getPointPairs(std::vector<Point> points) {
    std::vector<std::vector<size_t>> pairs;
    std::pair<size_t, size_t> pair;

    getPairsIter(points.size(), 0, pairs, pair, 0);

    return pairs;
}

std::vector<std::string> split(std::string& s, char delim) {
    std::vector<std::string> result;

    size_t j = 0;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == delim) {
            result.push_back(s.substr(j, i - j));
            j = i + 1;
        }
    }

    result.push_back(s.substr(j));

    return result;
}

std::vector<Point> readPoints(const char* filePath) {
    std::vector<Point> points;

    std::fstream pFile(filePath);
    if (!pFile.is_open()) {
        std::cerr << "Can't open points file" << std::endl;
        exit(1);
    }

    std::string line;

    while (std::getline(pFile, line)) {
        Point point = {};

        std::vector<std::string> parts = split(line, ',');

        point.pos.x = std::stof(parts[0]);
        point.pos.y = std::stof(parts[1]);

        point.dir.x = std::stof(parts[2]);
        point.dir.y = std::stof(parts[3]);

        point.color = HEX_COLOR(std::stoul(parts[4], nullptr, 16));

        point.radius = std::stof(parts[5]);

        points.push_back(point);
    }

    return points;
}

typedef struct {
    int state;
    Point p;
} NewPoint;

NewPoint newPointState = { 0 };

void initPoint(Vector2 v) {
    newPointState.state = AddNewPoint;
    newPointState.p.pos = v;
    newPointState.p.color = LIME;
    newPointState.p.dir.x = 0.1;
    newPointState.p.dir.y = 0.1;
}

std::vector<Point> addNewPoint(std::vector<Point> &points) {
    points.push_back(newPointState.p);
    newPointState.state = IdleNewPoint;
    return points;
}

void movePoints(std::vector<Point> &points, std::vector<std::vector<size_t>>& pairs) {
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

        if (dist == 0.0f) {
            delta = (Vector2){0.001f, 0.0f};
            dist = 0.001f;
        }

        if (dist < (p1.radius + p2.radius)) {
            Vector2 n = Vector2Scale(delta, 1.0f / dist);
            Vector2 t = (Vector2){-n.y, n.x};

            float m1 = p1.radius * p1.radius;
            float m2 = p2.radius * p2.radius;

            float v1n = Vector2DotProduct(p1.dir, n);
            float v1t = Vector2DotProduct(p1.dir, t);
            float v2n = Vector2DotProduct(p2.dir, n);
            float v2t = Vector2DotProduct(p2.dir, t);

            Vector2 relVel = Vector2Subtract(p2.dir, p1.dir);
            if (Vector2DotProduct(relVel, n) < 0.0f) {
                float v1n_new = (v1n * (m1 - m2) + 2.0f * m2 * v2n) / (m1 + m2);
                float v2n_new = (v2n * (m2 - m1) + 2.0f * m1 * v1n) / (m1 + m2);

                Vector2 v1n_vec = Vector2Scale(n, v1n_new);
                Vector2 v1t_vec = Vector2Scale(t, v1t);
                Vector2 v2n_vec = Vector2Scale(n, v2n_new);
                Vector2 v2t_vec = Vector2Scale(t, v2t);

                p1.dir = Vector2Add(v1n_vec, v1t_vec);
                p2.dir = Vector2Add(v2n_vec, v2t_vec);
            }

            float overlap = (p1.radius + p2.radius) - dist;
            float totalMass = m1 + m2;

            p1.pos = Vector2Subtract(p1.pos, Vector2Scale(n, overlap * (m2 / totalMass)));
            p2.pos = Vector2Add(p2.pos, Vector2Scale(n, overlap * (m1 / totalMass)));

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

    if (newPointState.state == AddNewPoint) {
        DrawCircleV(newPointState.p.pos, newPointState.p.radius, (Color) { 0, 0xff, 0, 0x30 });
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
    std::vector<Point> points = readPoints("resources/points.txt");

    std::vector<std::vector<size_t>> pairs = getPointPairs(points);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int windowWidth = 1200;
    int windowHeight = 900;

    SetTraceLogLevel(LOG_ERROR);

    InitWindow(windowWidth, windowHeight, "Floating dots");

    font = LoadFontEx("resources/font.ttf", 14, 0, 0);
    if (font.texture.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load font!");
    }

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        movePoints(points, pairs);

        if (IsKeyPressed(KEY_SPACE)) {
            if (newPointState.state == IdleNewPoint) {
                Vector2 pos = GetMousePosition();
                initPoint(pos);
            }
            else if (newPointState.state == AddNewPoint) {
                if (newPointState.p.radius > 3) {
                    points = addNewPoint(points);
                    pairs = getPointPairs(points);
                }
            }
        }

        if (newPointState.state == AddNewPoint) {
            Vector2 pos = GetMousePosition();
            float radius = Vector2Distance(newPointState.p.pos, pos);
            newPointState.p.radius = radius;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (newPointState.state == AddNewPoint) {
                newPointState.state = IdleNewPoint;
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
