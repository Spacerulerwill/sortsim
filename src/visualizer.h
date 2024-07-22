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

enum VisualizerMode {
    Staircase,
    Pyramid,
    Spiral,
    Circle,
    NumModes,
};

enum SortType {
    BubbleSort,
    SelectionSort,
    InsertionSort,
    CocktailShakerSort,
    Quicksort,
    MergeSort,
    BogoSort,
};

struct SortStats {
    size_t swaps;
    size_t comparisons;
    size_t arrayAccesses;
    size_t arrayWrites;
};

// Set all sort stats to zero
void sort_stats_reset(struct SortStats *sortStats);

struct Visualizer
{
    SortValueType *values;
    size_t count;
    struct SortStats sortStats;
    enum VisualizerMode mode;
    float speed;
    _Atomic bool isSorting;
    _Atomic bool cancelSort;
    enum SortType selectedSort;
};

void visualizer_init(struct Visualizer *visualizer);
void visualizer_free(struct Visualizer *visualizer);
void visualizer_resize(struct Visualizer *visualizer, size_t count);
void visualizer_start_sort(struct Visualizer *visualizer);
void visualizer_draw(struct Visualizer *visualizer);
void visualizer_draw_gui(struct Visualizer *visualizer);

#endif // !VISUALIZER_H
