/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2022 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include <stdlib.h>
#include <time.h>

#include "raylib.h"
#include "screens.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------

static int framesCounter = 0;
static int finishScreen = 0;

#define COLS 15
#define ROWS 15

#define screen_w 600
#define screen_h 600

#define cellWidth (screen_w / COLS)
#define cellHeight (screen_h / ROWS)

const char* youLose = "YOU LOSE!";
const char* youWin = "YOU WIN!";
const char* pressRToRestart = "Press 'r' to play again!";

typedef struct Cell
{
    int i;
    int j;
    bool containsMine;
    bool revealed;
    bool flagged;
    int nearbyMines;
} Cell;

Cell grid[COLS][ROWS];

Texture2D flagSprite;
int tilesRevealed;
int minesPresent;

typedef enum GameState
{
    PLAYING,
    LOSE,
    WIN
} GameState;

GameState state;

float timeGameStarted;
float timeGameEnded;

void CellDraw(Cell);
bool IndexIsValid(int, int);
void CellReveal(int, int);
void CellFlag(int, int);
int CellCountMines(int, int);
void GridInit(void);
void GridFloodClearFrom(int, int);
void GameInit(void);

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    // TODO: Initialize GAMEPLAY screen variables here!
    framesCounter = 0;
    finishScreen = 0;

    flagSprite = LoadTexture("resources/flag.png");

    GameInit();
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    // TODO: Update GAMEPLAY screen variables here!
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mPos = GetMousePosition();
        int indexI = mPos.x / cellWidth;
        int indexJ = mPos.y / cellHeight;

        if (state == PLAYING && IndexIsValid(indexI, indexJ))
        {
            CellReveal(indexI, indexJ);
            PlaySound(digSound);
        }
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        Vector2 mPos = GetMousePosition();
        int indexI = mPos.x / cellWidth;
        int indexJ = mPos.y / cellHeight;

        if (state == PLAYING && IndexIsValid(indexI, indexJ))
        {
            CellFlag(indexI, indexJ);
            PlaySound(flagSound);
        }
    }

    if (IsKeyPressed(KEY_R))
    {
        GameInit();
    }
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
    // TODO: Draw GAMEPLAY screen here!
    for (int i = 0; i < COLS; ++i)
    {
        for (int j = 0; j < ROWS; ++j)
        {
            CellDraw(grid[i][j]);
        }
    }

    if (state == LOSE)
    {
        DrawRectangle(0, 0, screen_w, screen_h, Fade(WHITE, 0.8f));
        DrawText(youLose, screen_w / 2 - MeasureText(youLose, 40) / 2, screen_h / 2 - 10, 40, DARKGRAY);
        DrawText(pressRToRestart, screen_w / 2 - MeasureText(pressRToRestart, 20) / 2, screen_h * 0.75f - 10, 20, DARKGRAY);
        
        int minutes = (int)(timeGameEnded - timeGameStarted) / 60;
        int seconds = (int)(timeGameEnded - timeGameStarted) % 60;
        DrawText(TextFormat("Time played: %d minutes, %d seconds.", minutes, seconds), 20, screen_h - 40, 20, DARKGRAY);

    }

    if (state == WIN)
    {
        DrawRectangle(0, 0, screen_w, screen_h, Fade(WHITE, 0.8f));
        DrawText(youWin, screen_w / 2 - MeasureText(youWin, 40) / 2, screen_h / 2 - 10, 40, DARKGRAY);
        DrawText(pressRToRestart, screen_w / 2 - MeasureText(pressRToRestart, 20) / 2, screen_h * 0.75f - 10, 20, DARKGRAY);
        
        int minutes = (int)(timeGameEnded - timeGameStarted) / 60;
        int seconds = (int)(timeGameEnded - timeGameStarted) % 60;
        DrawText(TextFormat("Time played: %d minutes, %d seconds.", minutes, seconds), 20, screen_h - 40, 20, DARKGRAY);

    }
}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}

void CellDraw(Cell cell)
{
    if (cell.revealed)
    {
        if (cell.containsMine)
        {
            DrawRectangle(cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight, RED);
        }
        else
        {
            DrawRectangle(cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight, LIGHTGRAY);
            
            if (cell.nearbyMines > 0)
            {
                DrawText(TextFormat("%d", cell.nearbyMines), cell.i * cellWidth + 12, cell.j * cellHeight + 4, cellHeight - 8, DARKGRAY);
            }
        }
    }
    else if (cell.flagged)
    {
        // draw flag
        Rectangle source = {0, 0, flagSprite.width, flagSprite.height};
        Rectangle dest = {cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight};
        Vector2 origin = {0, 0};

        DrawTexturePro(flagSprite, source, dest, origin, 0.0f, Fade(WHITE, 0.5f));
    }

    DrawRectangleLines(cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight, BLACK);
}

bool IndexIsValid(int i, int j)
{
    return (i >= 0 && i < COLS && j >= 0 && j < ROWS);
}

void CellReveal(int i, int j)
{
    if (grid[i][j].flagged)
    {
        return;
    }

    grid[i][j].revealed = true;

    if(grid[i][j].containsMine)
    {
        // lose!
        state = LOSE;
        PlaySound(loseSound);
        timeGameEnded = GetTime();
    }
    else
    {
        // play sound

        if (grid[i][j].nearbyMines == 0)
        {
            GridFloodClearFrom(i, j);
        }

        tilesRevealed++;

        if (tilesRevealed >= ROWS * COLS - minesPresent)
        {
            state = WIN;
            PlaySound(winSound);
            timeGameEnded = GetTime();
        }
    }
}

void CellFlag(int i, int j)
{
    if (grid[i][j].revealed)
    {
        return;
    }

    grid[i][j].flagged = !grid[i][j].flagged;
}

int CellCountMines(int i, int j)
{
    int count = 0;
    for (int iOffset = -1; iOffset <= 1; ++iOffset)
    {
        for (int jOffset = -1; jOffset <= 1; ++jOffset)
        {
            if (iOffset == 0 && jOffset == 0)
            {
                continue;
            }
            if (!IndexIsValid(i + iOffset, j + jOffset))
            {
                continue;
            }
            if (grid[i + iOffset][j + jOffset].containsMine)
            {
                count++;
            }
        }
    }
    return count;
}

void GridInit(void)
{
    for (int i = 0; i < COLS; ++i)
    {
        for (int j = 0; j < ROWS; ++j)
        {
            grid[i][j] = (Cell) 
            {
                .i = i, 
                .j = j,
                .containsMine = false,
                .revealed = false,
                .flagged = false,
                .nearbyMines = -1
            };
        }
    }

    minesPresent = (int)(ROWS * COLS * 0.1f);
    int minesToPlace = minesPresent;
    while (minesToPlace > 0)
    {
        int i = rand() % COLS;
        int j = rand() % ROWS;

        if (!grid[i][j].containsMine)
        {
            grid[i][j].containsMine = true;
            minesToPlace--;
        }
    }

    for (int i = 0; i < COLS; ++i)
    {
        for (int j = 0; j < ROWS; ++j)
        {
            if (!grid[i][j].containsMine)
            {
                grid[i][j].nearbyMines = CellCountMines(i, j);
            }
        }
    }
}

void GridFloodClearFrom(int i, int j)
{
    for (int iOffset = -1; iOffset <= 1; ++iOffset)
    {
        for (int jOffset = -1; jOffset <= 1; ++jOffset)
        {
            if (iOffset == 0 && jOffset == 0)
            {
                continue;
            }
            if (!IndexIsValid(i + iOffset, j + jOffset))
            {
                continue;
            }
            if (!grid[i + iOffset][j + jOffset].revealed)
            {
                CellReveal(i + iOffset, j + jOffset);
            }
        }
    }
}

void GameInit(void)
{
    GridInit();
    state = PLAYING;
    tilesRevealed = 0;
    timeGameStarted = GetTime();
}