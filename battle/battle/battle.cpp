#include "raylib.h"
#include "raymath.h"
#include <vector>

const int screenWidth = 1280;
const int screenHeight = 720;

struct Character {
    Rectangle rect;
    int health;
    int maxHealth;
    int speed;
    Color color;
};

struct Projectile {
    Vector2 position;
    Vector2 velocity;
    float radius;
    bool active;
};

bool CheckCollision(Character a, Character b) {
    return CheckCollisionRecs(a.rect, b.rect);
}

Character CreateMonster(int index) {
    int x = 1000 + (index % 5) * 80;
    int y = 200 + (index / 5) * 80;
    return { {(float)x, (float)y, 60, 60}, 100, 100, 2 + index / 5, RED };
}

int main() {
    InitWindow(screenWidth, screenHeight, "Battle with Monster");
    InitAudioDevice();

    Music bgm = LoadMusicStream("resources/music.mp3");
    Sound attackSound = LoadSound("resources/attack.wav");

    Texture2D playerTex = LoadTexture("resources/player.png");
    Texture2D monsterTex = LoadTexture("resources/monster.png");
    Texture2D backgroundTex = LoadTexture("resources/background.png");

    SetTargetFPS(60);

    std::vector<Projectile> projectiles;
    std::vector<Character> monsters;
    float spawnTimer = 0.0f;
    float spawnInterval = 5.0f;
    int wave = 1;

    float volume = 0.2f;

    Character player;
    bool gameOver = false;
    bool gameWin = false;
    bool gameStarted = false;

    auto ResetGame = [&]() {
        player = { {100, 100, 100, 100}, 100, 100, 5, BLUE };
        monsters.clear();
        projectiles.clear();
        monsters.push_back(CreateMonster(0));
        gameOver = false;
        gameWin = false;
        wave = 1;
        spawnTimer = 0.0f;
        };

    while (!WindowShouldClose()) {
        if (!gameStarted) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                Rectangle playBtn = { screenWidth / 2 - 100, screenHeight / 2 - 60, 200, 50 };
                Rectangle exitBtn = { screenWidth / 2 - 100, screenHeight / 2 + 20, 200, 50 };

                if (CheckCollisionPointRec(mousePos, playBtn)) {
                    gameStarted = true;
                    PlayMusicStream(bgm);
                    SetMusicVolume(bgm, volume);
                    ResetGame();
                }
                else if (CheckCollisionPointRec(mousePos, exitBtn)) {
                    break;
                }
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("MAIN MENU", screenWidth / 2 - 100, screenHeight / 2 - 150, 40, DARKGRAY);

            Rectangle playBtn = { screenWidth / 2 - 100, screenHeight / 2 - 60, 200, 50 };
            Rectangle exitBtn = { screenWidth / 2 - 100, screenHeight / 2 + 20, 200, 50 };

            DrawRectangleRec(playBtn, LIGHTGRAY);
            DrawRectangleRec(exitBtn, LIGHTGRAY);

            DrawText("PLAY", playBtn.x + 65, playBtn.y + 10, 30, BLACK);
            DrawText("EXIT", exitBtn.x + 65, exitBtn.y + 10, 30, BLACK);

            EndDrawing();

            continue;
        }

        UpdateMusicStream(bgm);

        if (IsKeyPressed(KEY_UP)) {
            volume = Clamp(volume + 0.1f, 0.0f, 1.0f);
            SetMusicVolume(bgm, volume);
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            volume = Clamp(volume - 0.1f, 0.0f, 1.0f);
            SetMusicVolume(bgm, volume);
        }

        if (!gameOver && !gameWin) {
            float moveSpeed = player.speed;

            if (IsKeyDown(KEY_D)) player.rect.x += moveSpeed;
            if (IsKeyDown(KEY_A)) player.rect.x -= moveSpeed;
            if (IsKeyDown(KEY_S)) player.rect.y += moveSpeed;
            if (IsKeyDown(KEY_W)) player.rect.y -= moveSpeed;

            player.rect.x = Clamp(player.rect.x, 0, screenWidth - player.rect.width);
            player.rect.y = Clamp(player.rect.y, 0, screenHeight - player.rect.height);

            for (auto& monster : monsters) {
                if (monster.health <= 0) continue;
                if (monster.rect.x < player.rect.x) monster.rect.x += monster.speed;
                if (monster.rect.x > player.rect.x) monster.rect.x -= monster.speed;
                if (monster.rect.y < player.rect.y) monster.rect.y += monster.speed;
                if (monster.rect.y > player.rect.y) monster.rect.y -= monster.speed;

                monster.rect.x = Clamp(monster.rect.x, 0, screenWidth - monster.rect.width);
                monster.rect.y = Clamp(monster.rect.y, 0, screenHeight - monster.rect.height);
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                Projectile proj;
                proj.position = { player.rect.x + player.rect.width / 2, player.rect.y + player.rect.height / 2 };
                Vector2 direction = Vector2Normalize(Vector2Subtract(mousePos, proj.position));
                proj.velocity = Vector2Scale(direction, 8.0f);
                proj.radius = 8;
                proj.active = true;
                projectiles.push_back(proj);
            }

            for (auto& proj : projectiles) {
                if (!proj.active) continue;
                proj.position = Vector2Add(proj.position, proj.velocity);
                for (auto& monster : monsters) {
                    if (monster.health > 0 && CheckCollisionCircleRec(proj.position, proj.radius, monster.rect)) {
                        monster.health -= 10;
                        proj.active = false;
                        PlaySound(attackSound);
                        break;
                    }
                }
            }

            for (auto& monster : monsters) {
                if (monster.health > 0 && CheckCollision(player, monster)) {
                    player.health -= 1;
                }
            }

            bool allMonstersDead = true;
            for (auto& monster : monsters) {
                if (monster.health > 0) {
                    allMonstersDead = false;
                    break;
                }
            }

            if (player.health <= 0) gameOver = true;
            if (allMonstersDead) gameWin = true;

            spawnTimer += GetFrameTime();
            if (spawnTimer >= spawnInterval && monsters.size() < 10) {
                for (int i = 0; i < wave; i++) {
                    monsters.push_back(CreateMonster(monsters.size()));
                }
                wave++;
                spawnTimer = 0;
            }
        }
        else {
            if (IsKeyPressed(KEY_ENTER)) ResetGame();
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(backgroundTex, 0, 0, WHITE);
        DrawRectangleLines(0, 0, screenWidth, screenHeight, DARKGRAY);

        if (gameOver) {
            DrawText("YOU LOST - Press ENTER to Restart", screenWidth / 2 - 200, screenHeight / 2, 30, RED);
        }
        else if (gameWin) {
            DrawText("YOU WIN! - Press ENTER to Continue", screenWidth / 2 - 200, screenHeight / 2, 30, GREEN);
        }
        else {
            float playerScale = 0.12f;
            Vector2 playerDrawPos = {
                player.rect.x + (player.rect.width - player.rect.width * playerScale) / 2,
                player.rect.y + (player.rect.height - player.rect.height * playerScale) / 2
            };
            DrawTextureEx(playerTex, playerDrawPos, 0.0f, playerScale, WHITE);

            float monsterScale = 0.02f;
            for (auto& monster : monsters) {
                if (monster.health > 0) {
                    Vector2 monsterDrawPos = {
                        monster.rect.x + (monster.rect.width - monster.rect.width * monsterScale) / 2,
                        monster.rect.y + (monster.rect.height - monster.rect.height * monsterScale) / 2
                    };
                    DrawTextureEx(monsterTex, monsterDrawPos, 0.0f, monsterScale, WHITE);
                }
            }

            for (auto& proj : projectiles) {
                if (proj.active)
                    DrawCircleV(proj.position, proj.radius, DARKBLUE);
            }

            DrawText(TextFormat("Player HP: %d", player.health), 10, 10, 20, BLACK);
            DrawText(TextFormat("Wave: %d", wave), 10, 40, 20, BLACK);
            DrawText(TextFormat("Volume: %0.1f (UP/DOWN)", volume), 10, 70, 20, GRAY);
        }

        EndDrawing();
    }

    UnloadTexture(playerTex);
    UnloadTexture(monsterTex);
    UnloadTexture(backgroundTex);
    UnloadMusicStream(bgm);
    UnloadSound(attackSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
