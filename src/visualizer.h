#ifndef BAR_ARRAY_H
#define BAR_ARRAY_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint16_t SortType;

typedef enum
{
    Bars,
    Pyramid,
    Circle,
    NumModes,
} VisualizerMode;

typedef struct
{
    size_t swaps;
    size_t comparisons;
    size_t array_accesses;
    size_t array_writes;
} SortStats;

void sort_stats_reset(SortStats *sorrtStats);

typedef struct
{
    SortType *values;
    SortType count;
    SortStats sortStats;
    VisualizerMode mode;
} Visualizer;

void visualizer_init(Visualizer *visualizer);
void visualizer_resize(Visualizer *visualizer, SortType count);
void visualizer_free(Visualizer *visualizer);
void visualizer_draw(Visualizer *visualizer);

#endif // !VISUALIZER_H
