#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <string>

using namespace std;


Color sky_blue = {135, 206, 235, 255};
Color darker_skyblue = {75, 135, 175, 255};
Color button_color = {100, 100, 100, 255};


// Making a GRID for the game window.
int cellSize = 40;
int cellCount = 25;

int offset = 75;

double lastUpdateTime = 0;

enum GameScreen {MENU, PLAYING, GAME_OVER};
GameScreen currentScreen = MENU;

enum Difficulty {EASY, MEDIUM, HARD};
Difficulty currentDifficulty = MEDIUM;
float gameSpeeds[] = {0.2, 0.1, 0.05}; // ms 0.05s = 50 ms / 0.1 s = 100 ms / 0.2 s = 200 ms
const char* difficultyNames[] = {"EASY", "MEDIUM", "HARD"};

bool ElementInDeque(Vector2 element, deque<Vector2> deque){
    for (unsigned int i = 0 ; i < deque.size(); i++){
        if (Vector2Equals(deque[i], element)){
        return true;
        }
    }
    return false;
}

bool eventTriggered(double interval){
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

bool DrawButton (Rectangle bounds, const char* text, Color baseColor, Color hoverColor, Color textColor){
    Vector2 mousePoint = GetMousePosition();
    bool hovered = CheckCollisionPointRec (mousePoint, bounds);

    Color drawColor;
    if (hovered){
        drawColor = hoverColor;
    }else{
        drawColor = baseColor;
    }
    DrawRectangleRounded (bounds, 0.2, 8, drawColor);

    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, bounds.x + bounds.width / 2 - textWidth / 2, bounds.y + bounds.height /2 - fontSize /2, fontSize, textColor);

    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

class Snake{

    public:
        deque<Vector2> body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
        Vector2 direction = {1, 0};
        bool addSegment = false;

        void Draw(){
            for (unsigned int i=0; i < body.size(); i++){
                float x = body[i].x;
                float y = body[i].y;
                Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
                DrawRectangleRounded(segment, 0.5, 6, darker_skyblue);
            }
        }
        void Update(){
            body.push_front(Vector2Add(body[0], direction));
            if (addSegment == true){
                addSegment = false;
            }else{
                body.pop_back();
            }
        }
        void Reset(){
            body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
            direction = {1,0};
        }
};

// This class is made for the food. We use a few methods for this class(random positioning, texture, saving memory by unloading the useless images).
class Food{

    public:
    Vector2 position;
    Texture2D texture;

    Food() : position({}){}

    void LoadTextureAndGeneratePos(deque<Vector2> snakeBody){
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenRandomPos(snakeBody);
    }

    ~Food(){
        UnloadTexture(texture);
    }

    void Draw(){
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell(){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x,y}; 
    }

    Vector2 GenRandomPos(deque<Vector2> snakeBody){
        Vector2 position = GenerateRandomCell();
        while(ElementInDeque(position, snakeBody)){
            position = GenerateRandomCell();
        }
        return position;
    }
};


class Game{
    public:
        Snake snake = Snake();
        Food food;
        bool running = true;
        int score = 0;


        void InitializeGameResources(){
            food.LoadTextureAndGeneratePos(snake.body);
        }

        void Draw(){
            food.Draw();
            snake.Draw();
        }

        void Update(){
            if (running == true){
            snake.Update();
            checkCollisionWithFood();
            checkCollisionWithEdges();
            checkCollisionWithTail();
            }
        }

        void checkCollisionWithFood(){
            if (Vector2Equals (snake.body[0], food.position)){
                food.position = food.GenRandomPos(snake.body);
                snake.addSegment = true;
                score++;
            }
        }

        void checkCollisionWithEdges(){
            if(snake.body[0].x == cellCount || snake.body[0].x == -1){
                GameOver();
            }

            if(snake.body[0].y == cellCount || snake.body[0].y == -1){
                GameOver();
            }
        }

        void GameOver(){
            snake.Reset();
            food.position = food.GenRandomPos(snake.body);
            running = false;
            currentScreen = GAME_OVER;
        }

        void checkCollisionWithTail(){
            deque<Vector2> headlessBody = snake.body;
            headlessBody.pop_front();
            if (ElementInDeque(snake.body[0], headlessBody)){
                GameOver();
            }
        }

        void ResetGame(){
            snake.Reset();
            food.position = food.GenRandomPos(snake.body);
            score = 0;
            running = true;
            currentScreen = PLAYING;
        }
};



int main () {
    cout << "Starting the game..." << '\n';

    int screenWidth = 2 * offset + cellSize * cellCount;
    int screenHeight = 2 * offset + cellSize * cellCount;

    InitWindow(screenWidth, screenHeight, "Docker Snake");
    SetTargetFPS(60);

    Game game;
    game.InitializeGameResources();

    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(sky_blue);

        switch(currentScreen){
            case MENU:{
                int titleFontSize = 60;
                int titleWidth = MeasureText("Docker Snake", titleFontSize);
                DrawText("Docker Snake", screenWidth / 2 - titleWidth / 2, screenHeight / 4, titleFontSize, darker_skyblue);

                int buttonWidth = 200;
                int buttonHeight = 50;
                int buttonX = screenWidth / 2 - buttonWidth / 2;
                int startY = screenHeight / 2 - buttonHeight * 1.5;

                Rectangle playButton = {(float) buttonX, (float)startY, (float)buttonWidth, (float)buttonHeight};
                if (DrawButton(playButton, "PLAY", button_color, DARKGRAY, WHITE)){
                    game.ResetGame();
                    currentScreen = PLAYING;
                }

                Rectangle difficultyButton = { (float)buttonX, (float)startY + buttonHeight + 20, (float)buttonWidth, (float)buttonHeight };
                string difficultyText = "DIFFICULTY: " + string(difficultyNames[currentDifficulty]);
                if (DrawButton(difficultyButton, difficultyText.c_str(), button_color, DARKGRAY, WHITE)){
                    currentDifficulty = (Difficulty) ((currentDifficulty + 1) % 3);
                    lastUpdateTime = GetTime();
                }

                Rectangle quitButton = {(float)buttonX, (float)startY + 2 * (buttonHeight + 20), (float)buttonWidth, (float)buttonHeight};
                if (DrawButton (quitButton, "QUIT GAME", button_color, DARKGRAY, WHITE)){
                    CloseWindow();
                }
                break;
            }

            case PLAYING:{
                DrawRectangleLinesEx(Rectangle{(float)offset-5, (float)offset-5, (float)cellSize*cellCount + 10, (float)cellSize*cellCount + 10}, 5, darker_skyblue);
                DrawText("Docker Snake", offset-5, 20, 40, darker_skyblue);
                DrawText(TextFormat("%i", game.score), offset-5, offset + cellCount * cellSize + 10 , 40, darker_skyblue);

                if (eventTriggered(gameSpeeds[currentDifficulty])){
                    game.Update();
                }

                if (game.running){
                    if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1){
                        game.snake.direction = {0, -1};
                    } else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1){
                        game.snake.direction = {0, 1};
                    } else if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1){
                        game.snake.direction = {1, 0};
                    } else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1){
                        game.snake.direction = {-1, 0};
                    }
                }
                game.Draw();
                break;
            }

            case GAME_OVER:{
                int gameOverFontSize = 50;
                int gameOverWidth = MeasureText("GAME OVER!", gameOverFontSize);
                DrawText("GAME OVER!", screenWidth / 2 - gameOverWidth / 2, screenHeight / 4, gameOverFontSize, DARKGRAY);

                int finalScoreFontSize = 30;
                string finalScoreText = "YOUR SCORE: " + to_string(game.score);
                int finalScoreWidth = MeasureText(finalScoreText.c_str(), finalScoreFontSize);
                DrawText(finalScoreText.c_str(), screenWidth / 2 - finalScoreWidth / 2, screenHeight / 2 - 40, finalScoreFontSize, DARKGRAY);

                Rectangle retryButton = { (float)screenWidth / 2 - 100, (float)screenHeight / 2 + 30, 200, 50 };
                if (DrawButton(retryButton, "RETRY", button_color, DARKGRAY, WHITE)) {
                    game.ResetGame();
                    game.running = true;
                    currentScreen = PLAYING;
                }

                Rectangle menuButton = { (float)screenWidth / 2 - 100, (float)screenHeight / 2 + 100, 200, 50 };
                if (DrawButton(menuButton, "BACK TO MENU", button_color, DARKGRAY, WHITE)) {
                    currentScreen = MENU;
                    game.running = false;
                }
                break;
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}