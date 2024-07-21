#include "heap_sort.h"

#define HEAP_SORT_SLEEP sleep_microseconds((uint64_t)(*args->speed * 50000.0f));

static void heapify(SortFunctionArgs* args, SortValueType root, int n) {
    if (atomic_load(args->cancelSort))
        return;
    int largest = root; // Initialize largest as root
    int l = 2 * root + 1; // left = 2*i + 1
    int r = 2 * root + 2; // right = 2*i + 2

    // If left child is larger than root
    if (l < n && args->values[l] > args->values[largest])
        largest = l;

    // If right child is larger than largest so far
    if (r < n && args->values[r] > args->values[largest])
        largest = r;

    // If largest is not root
    if (largest != root) {
        swap(args->sortStats, &args->values[root], &args->values[largest]);
        HEAP_SORT_SLEEP

        // Recursively heapify the affected sub-tree
        heapify(args, largest, n);
    }
}

static void impl_heap_sort(SortFunctionArgs* args) {
    int n = args->count;

    // Build max heap
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(args, i, n); 
        if (atomic_load(args->cancelSort))
                return;
    }

    // One by one extract an element from heap
    for (int i = n - 1; i > 0; i--) {
        // Move current root to end
        swap(args->sortStats, &args->values[0], &args->values[i]);
        if (atomic_load(args->cancelSort))
                return;
        HEAP_SORT_SLEEP

        // Heapify again
        heapify(args, 0, i);
        if (atomic_load(args->cancelSort))
            return;
    }
}

void heap_sort(SortFunctionArgs args) {
    impl_heap_sort(&args);
}