#include "visualizer.h"

#include <math.h>

#include "raylib.h"

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
    float f = h * 6 - i;
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
        return (Color){255, 255, 255, 255}; // Should never happen
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
            DrawRectangle((int)(i * barWidth), (int)(screenHeight - barHeight), (int)barWidth, (int)(barHeight), GRAY);
        }
        break;
    }
    case Pyramid: {
        float barUnitWidth = ((float)screenWidth / visualizer->count);
        float barHeight = (float)screenHeight / visualizer->count;
        for (SortType i = 0; i < visualizer->count; i++)
        {
            float barWidth = barUnitWidth * visualizer->values[i];
            DrawRectangle((int)(screenWidth - barWidth) / 2, (int)(barHeight * i), (int)barWidth, (int)barHeight, GRAY);
        }
        break;
    }
    case Circle: {
        float theta = 360.0f / visualizer->count;
        // Define the center of the circle
        Vector2 center = {screenWidth / 2.0f, screenHeight / 2.0f};
        float radius = screenHeight / 2.0f;
        for (SortType i = 0; i < visualizer->count; i++)
        {
            // Calculate the start and end angles of the current segment
            float startAngle = theta * i;
            float endAngle = theta * (i + 1);
            float hue = (visualizer->values[i] / (float)visualizer->count);
            ;                                          // Convert to degrees
            Color color = hsv_to_rgb(hue, 1.0f, 1.0f); // Max saturation and value
            DrawCircleSector(center, radius, startAngle, endAngle, 10, color);
        }
    }
    default:
        return;
    }
}
