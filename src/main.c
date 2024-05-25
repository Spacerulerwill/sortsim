#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#include "raylib.h"
#include "sorts.h"
#include "visualizer.h"

static void handle_keys(Visualizer *visualizer, _Atomic bool *isSorting, _Atomic bool *continueSorting,
                        size_t *currentSort)
{
    if (!atomic_load(isSorting))
    {
        if (IsKeyPressed(KEY_ENTER) && !is_already_sorted(visualizer->values, visualizer->count, NULL))
        {
            thrd_t sortThreadID;
            PerformSortParameter *param = malloc(sizeof(PerformSortParameter));
            if (param == NULL)
            {
                fputs("Memory allocation failed\n", stderr);
                exit(EXIT_FAILURE);
            }
            param->visualizer = visualizer;
            param->sort = sortFunctions[*currentSort];
            param->isSorting = isSorting;
            param->continueSorting = continueSorting;
            if (thrd_create(&sortThreadID, perform_sort, (void *)param) != thrd_success)
            {
                fputs("Error creating thread\n", stderr);
                exit(EXIT_FAILURE);
            }
        }
        else if (IsKeyPressed(KEY_SPACE))
        {
            shuffle(visualizer->values, visualizer->count, NULL);
        }
    }
    else if (IsKeyPressed(KEY_BACKSPACE))
    {
        atomic_store(continueSorting, false);
    }

    if (IsKeyPressed(KEY_LEFT))
    {
        *currentSort = *currentSort == 0 ? totalSorts - 1 : *currentSort - 1;
    }
    else if (IsKeyPressed(KEY_RIGHT))
    {
        *currentSort = *currentSort == totalSorts - 1 ? 0 : *currentSort + 1;
    }

    if (IsKeyPressed(KEY_UP))
    {
        visualizer->mode = visualizer->mode == NumModes - 1 ? 0 : visualizer->mode + 1;
    }
    else if (IsKeyPressed(KEY_DOWN))
    {
        visualizer->mode = visualizer->mode == 0 ? NumModes - 1 : visualizer->mode - 1;
    }
}

static void draw_text(SortStats *stats, size_t currentSort)
{
    const char *text;
    switch (currentSort)
    {
    case 0:
        text = "Bubble Sort O(n^2)";
        break;
    case 1:
        text = "Selection Sort O(n^2)";
        break;
    case 2:
        text = "Insertion Sort O(n^2)";
        break;
    case 3:
        text = "Cocktail Shaker Sort O(n^2)";
        break;
    case 4:
        text = "Bogo Sort O(Infinity)";
        break;
    default:
        return;
    }
    char formatted[512];
    int result = snprintf(formatted, sizeof(formatted),
                          "%s\nSwaps Made: %zu\nComparisons Made: %zu\nArray "
                          "Accesses: %zu\nArray Writes: %zu",
                          text, stats->swaps, stats->comparisons, stats->array_accesses, stats->array_writes);
    if (result == -1)
    {
        fputs("Failed to format string\n", stderr);
        exit(EXIT_FAILURE);
    }
    DrawText(formatted, 20, 20, 20, GREEN);
}

int main()
{
    // Seed rng
    srand((unsigned int)time(NULL));
    size_t currentSort = 0;
    // Array to sort
    Visualizer visualizer;
    visualizer_init(&visualizer);
    visualizer_resize(&visualizer, 512);
    shuffle(visualizer.values, visualizer.count, NULL);
    // Configuration
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Sorting Simulator");
    SetTargetFPS(60);
    // atomic boolean flag to determine whether the sort is running
    _Atomic bool isSorting = false;
    _Atomic bool continueSorting = false;
    while (!WindowShouldClose())
    {
        handle_keys(&visualizer, &isSorting, &continueSorting, &currentSort);
        BeginDrawing();
        ClearBackground(BLACK);
        visualizer_draw(&visualizer);
        draw_text(&visualizer.sortStats, currentSort);
        EndDrawing();
    }
    // Cleanup
    CloseWindow();
    visualizer_free(&visualizer);
    return EXIT_SUCCESS;
}
