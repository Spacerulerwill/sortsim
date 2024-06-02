#include "visualizer.h"
#include "sorts.h"
#include <math.h>
#include <raygui.h>
#include <raylib.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#define MAX_VISUALIZER_SIZE 256
#define MIN_VISUALIZER_SIZE 8
#define TOOLBAR_HEIGHT 45

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

void sort_stats_reset(SortStats *sortStats)
{
    sortStats->swaps = 0;
    sortStats->comparisons = 0;
    sortStats->arrayAccesses = 0;
    sortStats->arrayWrites = 0;
}

void visualizer_init(Visualizer *visualizer)
{
    visualizer->values = NULL;
    visualizer->count = 0;
    SortStats sortStats = {0, 0, 0, 0};
    visualizer->sortStats = sortStats;
    visualizer->mode = Staircase;
    visualizer->speed = 0.5f;
    visualizer->isSorting = false;
    visualizer->cancelSort = false;
    visualizer->selectedSort = BubbleSort;
}

void visualizer_free(Visualizer *visualizer)
{
    free(visualizer->values);
    visualizer->values = NULL;
    visualizer->count = 0;
}

void visualizer_resize(Visualizer *visualizer, size_t count)
{
    free(visualizer->values);
    visualizer->values = malloc(count * sizeof(SortValueType));
    if (visualizer->values == NULL)
    {
        fputs("Failed to allocate memory for visualizer\n", stderr);
        exit(EXIT_FAILURE);
    }
    visualizer->count = count;
    for (size_t i = 0; i < count; i++)
    {
        visualizer->values[i] = (SortValueType)(i + 1);
    }
}

void visualizer_draw(Visualizer *visualizer)
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    float drawHeight = (screenHeight - TOOLBAR_HEIGHT) / (float)screenHeight;

    switch (visualizer->mode)
    {
    case Staircase: {
        float barWidth = (float)screenWidth / visualizer->count;
        float barUnitHeight = (float)screenHeight / visualizer->count;

        for (size_t i = 0; i < visualizer->count; i++)
        {
            float barHeight = barUnitHeight * visualizer->values[i] * drawHeight;
            int x = (int)(i * barWidth);
            int y = screenHeight - (int)barHeight;
            int width = (int)barWidth;
            int height = (int)barHeight;
            if (width < 1)
                width = 1;
            if (height < 1)
                height = 1;
            DrawRectangle(x, y - TOOLBAR_HEIGHT, width, height, RAYWHITE);
            if (barWidth > 4.0f)
            {
                DrawRectangleLines(x, y - TOOLBAR_HEIGHT, width, height, BLACK);
            }
        }
        break;
    }
    case Pyramid: {
        float barUnitWidth = (float)screenWidth / visualizer->count;
        float barHeight = ((float)screenHeight / visualizer->count) * drawHeight;
        for (size_t i = 0; i < visualizer->count; i++)
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
                DrawRectangleLines(x, y, width, (int)(height * drawHeight), BLACK);
            }
        }
        break;
    }
    case Spiral: {
        float theta = 0.0f; 
        float deltaTheta = (360.0f / visualizer->count) * 3.0f;
        Vector2 center = {(float)screenWidth / 2.0f, (float)(screenHeight / 2.0f) - TOOLBAR_HEIGHT / 2.0f};
        float radius = ((float)screenHeight * drawHeight) / 2.0f;

        for (size_t i = 0; i < visualizer->count; i++) {
            float length = visualizer->values[i] / (float)visualizer->count;
            float x = center.x + radius * length * cosf(theta * DEG2RAD);
            float y = center.y + radius * length * sinf(theta * DEG2RAD);
            DrawCircle(x, y, 5.0f, BLUE);
            theta += deltaTheta;
        }
        break;
    }
    case Circle: {
        float theta = 360.0f / visualizer->count;
        Vector2 center = {(float)screenWidth / 2.0f, (float)(screenHeight / 2.0f) - TOOLBAR_HEIGHT / 2.0f};
        float radius = ((float)screenHeight * drawHeight) / 2.0f;
        for (size_t i = 0; i < visualizer->count; i++)
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

void visualizer_draw_gui(Visualizer *visualizer)
{
    // Top left text
    SortStats* sortStats = &visualizer->sortStats;
    char formatted[256];
    int result = snprintf(formatted, sizeof(formatted),
                          "Swaps Made : %zu\nComparisons Made : %zu\n"
                          "Array Accesses: %zu\nArray Writes: %zu",
                          sortStats->swaps,
                          sortStats->comparisons,
                          sortStats->arrayAccesses,
                          sortStats->arrayWrites
                          );
    if (result == -1)
    {
        fputs("Failed to format string\n", stderr);
        exit(EXIT_FAILURE);
    }
    DrawText(formatted, 20, 20, 20, GREEN);
    // Toolbar
    DrawRectangle(0, GetScreenHeight() - TOOLBAR_HEIGHT, GetScreenWidth(), TOOLBAR_HEIGHT,
                  GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    GuiGroupBox((Rectangle){0, (float)(GetScreenHeight() - TOOLBAR_HEIGHT + 5), (float)GetScreenWidth(),
                            (float)(TOOLBAR_HEIGHT - 5)},
                "Toolbar");
    float widgetY = (float)GetScreenHeight() - 30.0f;
    // Sort select dropdown
    static bool sortDropdownEditMode = false;
    if (atomic_load(&visualizer->isSorting)) {
        GuiLock();
    }
    if (GuiRollupBox((Rectangle){10, widgetY, 150, 20},
                       "Bubble Sort;Selection Sort;Insertion Sort;Cocktail Shaker Sort;Quick Sort;Merge Sort;Bogo Sort",
                       (int*)&visualizer->selectedSort, sortDropdownEditMode))
    {
        sortDropdownEditMode = !sortDropdownEditMode;
    }
    GuiUnlock();
    // Mode select dropdown
    static bool modeDropdwonEditMode = false;
    if (GuiRollupBox((Rectangle){170, widgetY, 110, 20}, "Staircase;Pyramid;Spiral;Color Wheel", (int*)&visualizer->mode,
                       modeDropdwonEditMode))
    {
        modeDropdwonEditMode = !modeDropdwonEditMode;
    }
    // Speed slider
    GuiSliderBar((Rectangle){290, widgetY, 140, 20}, NULL, "Delay", &visualizer->speed, 0.0f, 1.0f);
    // Size slider
    if (atomic_load(&visualizer->isSorting)) {
        GuiLock();
    }
    static float x = (float)DEFAULT_VISUALIZER_SIZE / (float)(MAX_VISUALIZER_SIZE - MIN_VISUALIZER_SIZE);
    if (GuiSliderBar((Rectangle){480, widgetY, 140, 20}, NULL, "Size", &x, 0.0f, 1.0f)) {
        float new_size = (float)MIN_VISUALIZER_SIZE + x * (float)(MAX_VISUALIZER_SIZE - MIN_VISUALIZER_SIZE);
        visualizer_resize(visualizer, (size_t)new_size);
    };
    GuiUnlock();
    // Sort and shuffle button
    if (atomic_load(&visualizer->isSorting))
    {
        if (GuiButton((Rectangle){660, widgetY, 50, 20}, "Cancel"))
        {
            atomic_store(&visualizer->cancelSort, true);
        }
        GuiLock();
        GuiButton((Rectangle){720, widgetY, 50, 20}, "Shuffle");
        GuiUnlock();
    }
    else
    {
        if (GuiButton((Rectangle){660, widgetY, 50, 20}, "Shuffle"))
        {
            shuffle(visualizer->values, visualizer->count, NULL);
        }
        if (GuiButton((Rectangle){720, widgetY, 50, 20}, "Sort"))
        {
            visualizer_start_sort(visualizer);
        }
    }
}

void visualizer_start_sort(Visualizer *visualizer)
{
    if (is_already_sorted(visualizer->values, visualizer->count, NULL))
        return;
    thrd_t sortThreadID;
    atomic_store(&visualizer->isSorting, true);
    atomic_store(&visualizer->cancelSort, false);
    if (thrd_create(&sortThreadID, perform_sort, (void *)visualizer) != thrd_success)
    {
        fputs("Error creating thread\n", stderr);
        exit(EXIT_FAILURE);
    }
}
