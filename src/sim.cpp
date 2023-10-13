#include <iostream>
#include <raylib.h>
#include "verilated.h"
#include "../bin/Vtop.h"
#include "raylib.h"

#define NUM_FRAMES  3 

const int width = 960;
const int height = 540;
const int cells_x = 32;
const int cells_y = cells_x / 16 * 9;
const int cell_width = width / cells_x;

VerilatedContext context;
Vtop top;

// state to be copied into hardware
// when top.overwrite is high.
char cells_in[cells_y][cells_x] = {0};

int main(void) {
    InitWindow(width, height, "Conway's Game Of Life");

    // Define frame rectangle for drawing
    float frameHeight = (float)button.height/NUM_FRAMES;
    Rectangle sourceRec = { 0, 0, (float)button.width, frameHeight };

    // Define button bounds on screen
    Rectangle btnBounds = { screenWidth/2.0f - button.width/2.0f, screenHeight/2.0f - button.height/NUM_FRAMES/2.0f, (float)button.width, frameHeight };

    int btnState = 0;               // Button state: 0-NORMAL, 1-MOUSE_HOVER, 2-PRESSED
    bool btnAction = false;         // Button action should be activated

    Vector2 mousePoint = { 0.0f, 0.0f };
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        mousePoint = GetMousePosition();
        btnAction = false;

        // Check button state
        if (CheckCollisionPointRec(mousePoint, btnBounds))
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) btnState = 2;
            else btnState = 1;

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) btnAction = true;
        }
        else btnState = 0;


        // Calculate button frame rectangle to draw depending on button state
        sourceRec.y = btnState*frameHeight;
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTextureRec(button, sourceRec, (Vector2){ btnBounds.x, btnBounds.y }, WHITE); // Draw button frame

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(button);  // Unload button texture

    CloseWindow();          // Close window and OpenGL context
    

    top.clk = 0;
    top.overwrite = 0;
    size_t frame_count = 0;
    enum State { SET, SIM } state = SET;

    // evaluate once to register top.overwrite to 0
    // such that a positive edge is registered when
    // overwrite is set to 1 from here on.
    top.eval();

    while (!context.gotFinish() && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        if (mouse_x < 0 || mouse_x >= width) mouse_x = -1;
        if (mouse_y < 0 || mouse_y >= height) mouse_y = -1;

        if (state == SET) {
            // set simulation state using mouse.
            // left click to breathe life into a cell,
            // right click to drain its life force.

            if (GetKeyPressed() == KEY_SPACE) {
                top.overwrite = 1;
                for (int i = 0; i < cells_y; i++) {
                    for (int j = 0; j < cells_x; j++) {
                        top.cells_in[i][j] = cells_in[i][j];
                    }
                }
                top.eval();
                top.overwrite = 0;
                state = SIM;
            }

            if (mouse_x != -1 && mouse_y != -1) {
                int hover_x = mouse_x / cell_width;
                int hover_y = mouse_y / cell_width;
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    cells_in[hover_y][hover_x] = 1;
                } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                    cells_in[hover_y][hover_x] = 0;
                } else {
                    DrawRectangle(hover_x * cell_width + 2, hover_y * cell_width + 2, cell_width - 4, cell_width - 4, LIGHTGRAY);
                }
            }

            for (int i = 0; i < cells_y; i++) {
                for (int j = 0; j < cells_x; j++) {
                    if (cells_in[i][j] == 1) {
                        DrawRectangle(j * cell_width + 2, i * cell_width + 2, cell_width - 4, cell_width - 4, RED);
                    } else {
                        DrawRectangleLines(j * cell_width + 2, i * cell_width + 2, cell_width - 4, cell_width - 4, GRAY);
                    }
                }
            }
        } else if (state == SIM) {
            // run the simulation.
            // user will not be able to alter simulation state here.

            if (GetKeyPressed() == KEY_SPACE) {
                for (int i = 0; i < cells_y; i++) {
                    for (int j = 0; j < cells_x; j++) {
                        cells_in[i][j] = top.cells[i][j];
                    }
                }
                state = SET;
            }

            // 60 fps is too fast for running simulation logic.
            // this should do the trick :D
            if (frame_count % 10 == 0) {
                top.eval();
                top.clk = !top.clk;
            }

            for (int i = 0; i < cells_y; i++) {
                for (int j = 0; j < cells_x; j++) {
                    if (top.cells[i][j] == 1) {
                        DrawRectangle(j * cell_width + 2, i * cell_width + 2, cell_width - 4, cell_width - 4, RED);
                    } else {
                        DrawRectangleLines(j * cell_width + 2, i * cell_width + 2, cell_width - 4, cell_width - 4, GRAY);
                    }
                }
            }
        }

        frame_count++;
        EndDrawing();
    }

    top.final();
    CloseWindow();
    return 0;
}
