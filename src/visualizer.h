#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// The type of the element in our sort array that is being sorted
typedef uint16_t SortType;

extern const uint64_t MAX_DELAY;
extern const uint64_t DEFAULT_DELAY;

typedef enum
{
    Bars,
    Pyramid,
    Circle,
    NumModes,
} VisualizerMode;

// Struct containing information to display about the sort, updated as the sort progresses
typedef struct
{
    size_t swaps;
    size_t comparisons;
    size_t array_accesses;
    size_t array_writes;
} SortStats;

// Set all sort stats to zero
void sort_stats_reset(SortStats *sorrtStats);

typedef struct
{
    SortType *values;
    SortType count;
    SortStats sortStats;
    VisualizerMode mode;
    uint64_t delay;
} Visualizer;

void visualizer_init(Visualizer *visualizer);
void visualizer_resize(Visualizer *visualizer, SortType count);
void visualizer_free(Visualizer *visualizer);
void visualizer_draw(Visualizer *visualizer);

#endif // !VISUALIZER_H
