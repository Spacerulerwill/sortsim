#include <raylib.h>
#include <time.h>
#define RAYGUI_IMPLEMENTATION
#include "visualizer.h"
#include <raygui.h>
#include <style_cyber.h>

int main()
{
    srand((unsigned int)time(NULL));
    Visualizer visualizer;
    visualizer_init(&visualizer);
    visualizer_resize(&visualizer, 128);
    // Raylib configuration
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Sorting Simulator");
    SetTargetFPS(60);
    GuiLoadStyleCyber();
    // Mainloop
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        visualizer_draw(&visualizer);
        visualizer_draw_gui(&visualizer);
        EndDrawing();
    }
    // Cleanup
    CloseWindow();
    visualizer_free(&visualizer);
    return EXIT_SUCCESS;
}
