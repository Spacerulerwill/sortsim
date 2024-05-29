#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_VISUALIZER_SIZE 64

typedef uint16_t SortValueType;

typedef enum
{
    Staircase,
    Pyramid,
    Circle,
    NumModes,
} VisualizerMode;

typedef enum
{
    BubbleSort,
    SelectionSort,
    InsertionSort,
    CocktailShakerSort,
    Quicksort,
    MergeSort,
    BogoSort,
} SortType;

typedef struct
{
    size_t swaps;
    size_t comparisons;
    size_t arrayAccesses;
    size_t arrayWrites;
} SortStats;

// Set all sort stats to zero
void sort_stats_reset(SortStats *sortStats);

typedef struct
{
    SortValueType *values;
    size_t count;
    SortStats sortStats;
    VisualizerMode mode;
    float speed;
    _Atomic bool isSorting;
    _Atomic bool cancelSort;
    SortType selectedSort;
} Visualizer;

void visualizer_init(Visualizer *visualizer);
void visualizer_free(Visualizer *visualizer);
void visualizer_resize(Visualizer *visualizer, size_t count);
void visualizer_start_sort(Visualizer *visualizer);
void visualizer_draw(Visualizer *visualizer);
void visualizer_draw_gui(Visualizer *visualizer);

#endif // !VISUALIZER_H
