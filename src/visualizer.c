#include "visualizer.h"

#include <math.h>
#include <stdlib.h>

#include "raylib.h"

const uint64_t MAX_DELAY = 10000;
const uint64_t DEFAULT_DELAY = 2000;

void sort_stats_reset(SortStats *sortStats)
{
    sortStats->comparisons = 0;
    sortStats->swaps = 0;
    sortStats->array_accesses = 0;
    sortStats->array_writes = 0;
}

void visualizer_init(Visualizer *visualizer)
{
    visualizer->values = NULL;
    visualizer->count = 0;
    SortStats stats = {0, 0, 0, 0};
    visualizer->sortStats = stats;
    visualizer->mode = Bars;
    visualizer->delay = DEFAULT_DELAY;
}

void visualizer_resize(Visualizer *visualizer, SortType count)
{
    free(visualizer->values);
    visualizer->values = (SortType *)calloc(count, sizeof(SortType));
    if (visualizer->values == NULL)
    {
        fputs("Failed to allocate memory for visualizer\n", stderr);
        exit(EXIT_FAILURE);
    }
    visualizer->count = count;
    for (SortType i = 0; i < count; i++)
    {
        visualizer->values[i] = count - i;
    }
}

void visualizer_free(Visualizer *visualizer)
{
    free(visualizer->values);
    visualizer->values = NULL;
    visualizer->count = 0;
}

static Color hsv_to_rgb(float h, float s, float v)
{
    float r, g, b;

    int i = (int)floor(h * 6);
    float f = h * 6.0f - (float)i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6)
    {
    case 0:
        r = v, g = t, b = p;
        break;
    case 1:
        r = q, g = v, b = p;
        break;
    case 2:
        r = p, g = v, b = t;
        break;
    case 3:
        r = p, g = q, b = v;
        break;
    case 4:
        r = t, g = p, b = v;
        break;
    case 5:
        r = v, g = p, b = q;
        break;
    default:
        fputs("There is a bug with the hsv_to_rgb function, tell a programmer!", stderr); // Should never happen
        exit(EXIT_FAILURE);
    }

    Color color;
    color.r = (unsigned char)(r * 255);
    color.g = (unsigned char)(g * 255);
    color.b = (unsigned char)(b * 255);
    color.a = 255;

    return color;
}

void visualizer_draw(Visualizer *visualizer)
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    switch (visualizer->mode)
    {
    case Bars: {
        float barWidth = (float)screenWidth / visualizer->count;
        float barUnitHeight = (float)screenHeight / visualizer->count;

        for (SortType i = 0; i < visualizer->count; i++)
        {
            float barHeight = barUnitHeight * visualizer->values[i];
            int x = (int)(i * barWidth);
            int y = screenHeight - (int)barHeight;
            int width = (int)barWidth;
            int height = (int)barHeight;
            if (width < 1)
                width = 1;
            if (height < 1)
                height = 1;
            DrawRectangle(x, y, width, height, RAYWHITE);
            if (barWidth > 4.0f)
            {
                DrawRectangleLines(x, y, width, height, BLACK);
            }
        }
        break;
    }
    case Pyramid: {
        float barUnitWidth = (float)screenWidth / visualizer->count;
        float barHeight = (float)screenHeight / visualizer->count;
        for (SortType i = 0; i < visualizer->count; i++)
        {
            float barWidth = barUnitWidth * visualizer->values[i];
            int x = (int)(((float)screenWidth - barWidth) / 2);
            int y = (int)(barHeight * i);
            int width = (int)(barWidth);
            int height = (int)(barHeight);
            if (width < 1)
                width = 1;
            if (height < 1)
                height = 1;
            DrawRectangle(x, y, width, height, RAYWHITE);
            if (barHeight > 4.0f)
            {
                DrawRectangleLines(x, y, width, height, BLACK);
            }
        }
        break;
    }
    case Circle: {
        float theta = 360.0f / visualizer->count;
        Vector2 center = {(float)screenWidth / 2.0f, (float)screenHeight / 2.0f};
        float radius = (float)screenHeight / 2.0f;
        for (SortType i = 0; i < visualizer->count; i++)
        {
            float startAngle = theta * i;
            float endAngle = theta * (i + 1);
            float hue = (visualizer->values[i] / (float)visualizer->count);
            Color color = hsv_to_rgb(hue, 1.0f, 1.0f);
            DrawCircleSector(center, radius, startAngle, endAngle, 10, color);
        }
        break;
    }
    default:
        fputs("Error: Current selected visualizer mode is somehow invalid, tell a programmer!\n", stderr);
        exit(EXIT_FAILURE);
    }
}
