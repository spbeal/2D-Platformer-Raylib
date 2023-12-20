# include <raylib.h>
# include <raymath.h>
# include <iostream>
# include <vector>


static const float TILE_SIZE = 100.0f;
static const int ROOM_WIDTH = 7;
static const int ROOM_HEIGHT = 50;

static const float BOMB_SIZE = 50.0f;
static const float BOMB_DELAY = 1.0f;
static const float BOMB_RADIUS = 180;

struct Collision
{
    int type;
    Rectangle collider;
};

struct Bomb
{
    float time; 
    Vector2 position; 
};

struct Room
{
    Room(int index)
    {
        zone = index / 2;
        startPos = { TILE_SIZE * index * ROOM_WIDTH, 0.0f };
        tiles.resize(ROOM_WIDTH * ROOM_HEIGHT);
        for (int& tile : tiles) tile = -1;
    }

    void Generate(bool start = false)
    {
        if (start)
        {
            for (int i = 0; i < ROOM_WIDTH * ROOM_HEIGHT; ++i)
            {
                int x = i % ROOM_WIDTH;
                int y = i / ROOM_WIDTH;

                // Initialize border of room
                if (y == 0 || y == ROOM_HEIGHT - 1)
                {
                    tiles[i] = 47;
                }
                else
                {
                    tiles[i] = -1;
                }
            }
        }
        else
        {
            for (int i = 0; i < ROOM_WIDTH * ROOM_HEIGHT; ++i)
            {
                int x = i % ROOM_WIDTH;
                int y = i / ROOM_WIDTH;

                // Initialize border of room
                if (y == 0 || y == ROOM_HEIGHT - 1)
                {
                    tiles[i] = 47;
                }
                else
                {
                    
                    tiles[i] = rand() % std::max(2, (5-zone)) == 0 ? 47 : -1;
                }
            }

            for (int x = 0; x < ROOM_WIDTH; ++x)
            {
                for (int y = 1; y < ROOM_HEIGHT - 1; ++y)
                {
                    int i = y * ROOM_WIDTH + x;
                    int belowI = (y+1) * ROOM_WIDTH + x;

                    if (tiles[i] == -1 && tiles[belowI] >= 0)
                    {
                        if (rand() % 12 == 0)
                        {
                            tiles[i] = 68;
                        }
                    }
                }
            }
        }
    }

    void Draw(Texture2D& tileTexture, Texture2D& bgTexture)
    {
        // Background
        if (zone == 0)
        {
            Rectangle destBg = { startPos.x, startPos.y, TILE_SIZE * ROOM_WIDTH, TILE_SIZE * ROOM_HEIGHT };
            DrawTexturePro(bgTexture, { 0.0f, 0.0f, 24.0 * 2, 24.0f * 3 }, destBg, { 0.0f, 0.0f }, 0.0f, WHITE);
        }
        else if (zone == 1)
        {
            Rectangle destBg = { startPos.x, startPos.y, TILE_SIZE * ROOM_WIDTH, TILE_SIZE * ROOM_HEIGHT };
            DrawTexturePro(bgTexture, { 24.0f * 2, 0.0f, 24.0 * 2, 24.0f * 3 }, destBg, { 0.0f, 0.0f }, 0.0f, WHITE);
        }
        else if (zone == 2)
        {
            Rectangle destBg = { startPos.x, startPos.y, TILE_SIZE * ROOM_WIDTH, TILE_SIZE * ROOM_HEIGHT };
            DrawTexturePro(bgTexture, { 24.0f * 4, 0.0f, 24.0 * 2, 24.0f * 3 }, destBg, { 0.0f, 0.0f }, 0.0f, WHITE);
        }
        else if (zone == 3)
        {
            Rectangle destBg = { startPos.x, startPos.y, TILE_SIZE * ROOM_WIDTH, TILE_SIZE * ROOM_HEIGHT };
            DrawTexturePro(bgTexture, { 24.0f * 6, 0.0f, 24.0 * 2, 24.0f * 3 }, destBg, { 0.0f, 0.0f }, 0.0f, WHITE);
        }
        else
        {
            Rectangle destBg = { startPos.x, startPos.y, TILE_SIZE * ROOM_WIDTH, TILE_SIZE * ROOM_HEIGHT };
            DrawTexturePro(bgTexture, { 24.0f * 6, 0.0f, 24.0 * 2, 24.0f * 3}, destBg, {0.0f, 0.0f}, 0.0f, RED);
        }

        for (int i = 0; i < ROOM_WIDTH * ROOM_HEIGHT; ++i)
        {
            if (tiles[i] < 0) continue;
            int tileX = i % ROOM_WIDTH;
            int tileY = i / ROOM_WIDTH;
            Rectangle dest = { startPos.x + tileX * TILE_SIZE, startPos.y + tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE  };
            DrawTexturePro(tileTexture, { (tiles[i] % 20) * 18.0f, (tiles[i] / 20 ) * 18.0f, 18.0, 18.0f }, dest, { 0.0f, 0.0f }, 0.0f, WHITE);
        }
    }
    int zone; 
    std::vector<int> tiles;
    Vector2 startPos;
};

namespace
{
    bool dead = false;
    const float gravity = 1000.0f;
    const float jumpHeight = 210.0f;

    float playerSpeed = 700.0f;
    Vector2 playerSize = { 80.0f, 80.0f };
    Vector2 playerVel = { 0.0f, 0.0f };
    Vector2 playerPos = { 100.0f, 0.0f };
    int jumps = 0;

    Vector2 wallPos = { 0.0f, 0.0f };
    Vector2 wallSize = { 10000.0f, 10000.0f };

    std::vector<Room> rooms;
    std::vector<Bomb> bombs;
}

Collision CheckRoomCollisions(Rectangle& playerRec)
{
    for (Room& room : rooms)
    {
        for (int i = 0; i < ROOM_WIDTH * ROOM_HEIGHT; ++i)
        {
            if (room.tiles[i] < 0) continue;

            int tileX = i % ROOM_WIDTH;
            int tileY = i / ROOM_WIDTH;
            Rectangle wallRec;

            // Better spike collision
            if (room.tiles[i] != 68) wallRec = { room.startPos.x + tileX * TILE_SIZE, room.startPos.y + tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            else wallRec = { room.startPos.x + tileX * TILE_SIZE + TILE_SIZE * 0.1f, room.startPos.y + tileY * TILE_SIZE + TILE_SIZE * 0.6f, TILE_SIZE - TILE_SIZE * 0.2f, TILE_SIZE * 0.4f};

            if (CheckCollisionRecs(playerRec, wallRec))
            {
                if (room.tiles[i] == 68)
                {
                    dead = true;
                }
                return { room.tiles[i], wallRec };
            }
        }
    }
    return { -1, {0.0f, 0.0f, 0.0f, 0.0f} };
}


int main()
{
    srand(time(0));
    InitWindow(1280, 720, "2D Platformer");
    SetTargetFPS(144);

    Camera2D camera = { {0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f };
    camera.offset = { GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };

    Image tileImage = LoadImage("resources/kenny_pixel_platformer/Tilemap/tilemap_packed.png");
    Texture2D tileTexture = LoadTextureFromImage(tileImage);

    Image charImage = LoadImage("resources/kenny_pixel_platformer/Tilemap/tilemap-characters_packed.png");
    Texture2D charTexture = LoadTextureFromImage(charImage);

    Image bgImage = LoadImage("resources/kenny_pixel_platformer/Tilemap/tilemap-backgrounds_packed.png");
    Texture2D bgTexture = LoadTextureFromImage(bgImage);

    float time = 0.0f;

    // Generate rooms. Keep 3 going at once, delete as you go
    Room room1 = Room(rooms.size());
    room1.Generate(true);
    rooms.push_back(room1);

    Room room2 = Room(rooms.size());
    room2.Generate();
    rooms.push_back(room2);

    Room room3 = Room(rooms.size());
    room3.Generate();
    rooms.push_back(room3);




    while (!WindowShouldClose())
    {
        time += GetFrameTime();

        if (playerPos.x >= rooms[rooms.size() - 1].startPos.x)
        {
            Room infiniteRoom = Room(rooms.size());
            infiniteRoom.Generate();
            rooms.push_back(infiniteRoom);
        }

        // Bomb update
        for (Bomb& bomb : bombs)
        {
            bomb.position.y += 50.0f * GetFrameTime();
            bomb.time += GetFrameTime();
            Rectangle bombRec = { bomb.position.x, bomb.position.y, BOMB_SIZE, BOMB_SIZE };
            Collision collision = CheckRoomCollisions(bombRec);
            if (collision.type >= 0)
            {
                bomb.position.y = collision.collider.y - BOMB_SIZE;
            }
            if (bomb.time >= BOMB_DELAY)
            {
                bomb.time = -10.0e8;
                Vector2 bombCenter = { bomb.position.x + BOMB_SIZE * 0.5f, bomb.position.y + BOMB_SIZE * 0.5f };

                if (Vector2Distance( bombCenter, { playerPos.x + playerSize.x * 0.5f, playerPos.y + playerSize.y * 0.5f }) < BOMB_RADIUS )
                {
                    dead = true;
                }
                for (Room& room : rooms)
                {
                    for (int i = 0 ; i < ROOM_WIDTH * ROOM_HEIGHT; i++)
                    {
                        if (room.tiles[i] < 0) continue;

                        int tileX = i % ROOM_WIDTH;
                        int tileY = i / ROOM_WIDTH;
                        Vector2 tileCenter = { room.startPos.x + tileX * TILE_SIZE + TILE_SIZE * 0.5f, room.startPos.y + tileY * TILE_SIZE + TILE_SIZE * 0.5f};

                        if (Vector2Distance(bombCenter, tileCenter) < 200.0f)
                        {
                            room.tiles[i] = -1;
                        }
                    }
                }
            }
        }

        // Endless black Wall update
        wallPos.x = -wallSize.x + 2.0f * time + 4.0f * time * time; // x + vt + at^2 kinematic equation for constant acceleration

        // Update
        playerVel.y += gravity * GetFrameTime();
        playerVel.x = 0.0f;
        if (IsKeyDown(KEY_D))
        {
            playerVel.x += playerSpeed;
        }
        if (IsKeyDown(KEY_A))
        {
            playerVel.x -= playerSpeed;
        }
        if (IsKeyPressed(KEY_SPACE) && jumps)
        {
            playerVel.y = -sqrtf(2.0f * gravity * jumpHeight);
            jumps--;
        }

        if (IsKeyPressed(KEY_F))
        {
            Vector2 bombPos = Vector2Add(playerPos, Vector2Scale(playerSize, 0.5f));
            bombPos = Vector2Add(bombPos, Vector2Scale({ BOMB_SIZE, BOMB_SIZE }, -0.5f));
            Bomb b = { 0.0f, bombPos };
            bombs.push_back(b);
        }

        // Collisions
        playerPos.x += playerVel.x * GetFrameTime();
        Rectangle playerRec = { playerPos.x, playerPos.y, playerSize.x, playerSize.y };
        Collision collision = CheckRoomCollisions(playerRec);
        if (collision.type >= 0)
        {
            //playerPos.x -= playerVel.x * GetFrameTime();
            //if (jumps == 0) ++jumps; // wall jump
            if (playerVel.x > 0.0f)
            {
                playerPos.x = collision.collider.x - playerSize.x;
            }
            else {
                playerPos.x = collision.collider.x + collision.collider.width;
            }
        }

        playerPos.y += playerVel.y * GetFrameTime();
        playerRec = { playerPos.x, playerPos.y, playerSize.x, playerSize.y };
        collision = CheckRoomCollisions(playerRec);
        if (collision.type >= 0)
        {
            if (playerVel.y >= 0.0f)
            {
                jumps = 2;
            }

            if (playerVel.y > 0.0f)
            {
                playerPos.y = collision.collider.y - playerSize.y;
            }
            else {
                playerPos.y = collision.collider.y + collision.collider.height;
            }
            playerVel.y = 0.0f;
        }
        else
        {
            // we are on air
            if (jumps >= 2) jumps = 1;
        }

        // Camera Update
        camera.target = { playerPos.x + playerSize.x * 0.5f, playerPos.y + playerSize.y * 0.5f };

        // Render
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // World Space
        BeginMode2D(camera);

        // Random rooms
        for (Room& room : rooms)
        {
            room.Draw(tileTexture, bgTexture);
        }

        // Player
        int playerIndex = 6;
        if (fmodf(time, 1.0f) <= 0.5f) playerIndex = 7; // first half of second
        Rectangle destPlayer = { playerPos.x, playerPos.y, playerSize.x, playerSize.y };
        DrawTexturePro(charTexture, { (playerIndex % 9) * 24.0f + 2.0f, (playerIndex / 9) * 24.0f, 20.0, 24.0f }, destPlayer, { 0.0f, 0.0f }, 0.0f, WHITE);

        //Bomb
        int bombIndex = 145;
        for (Bomb& bomb : bombs)
        {
            if (bomb.time < 0.0f) continue; // delete later, vectors move
            Rectangle destBomb = { bomb.position.x, bomb.position.y, BOMB_SIZE, BOMB_SIZE };
            Vector2 bombCenter = { bomb.position.x + BOMB_SIZE * 0.5f, bomb.position.y + BOMB_SIZE * 0.5f };
            DrawCircleV(bombCenter, BOMB_RADIUS, { 200, 0, 120, (unsigned char)(bomb.time / BOMB_DELAY * 127.0f)});
            DrawTexturePro(tileTexture, { (bombIndex % 20) * 18.0f, (bombIndex / 20) * 18.0f, 18.0, 18.0f }, destBomb, { 0.0f, 0.0f }, 0.0f, WHITE);
        }

        // Endless black wall
        DrawRectanglePro({ wallPos.x, wallPos.y, wallSize.x, wallSize.y }, { 0.0f, 0.0f, }, 0.0f, BLACK);

        //int tileX = 2;
        //int tileY = 3;
        //Rectangle dest = { wallPos.x, wallPos.y, wallSize.x, wallSize.y };
        //DrawTexturePro(tileTexture, { tileX * 18.0f, tileY * 18.0f, 18.0, 18.0f }, dest, { 0.0f, 0.0f }, 0.0f, WHITE);

        EndMode2D();

        // Screen Space 
        char posText[32] = { 0 };
        sprintf(posText, "X: %.2f\nY: %.2f", playerPos.x, playerPos.y);
        DrawText(posText, 4, 80, 16, BLACK);

        char wallText[64] = { 0 };
        sprintf(wallText, "Wall X: %.2f", wallPos.x + wallSize.x);
        DrawText(wallText, 4, 48, 16, BLACK);

        char fpsText[32] = { 0 };
        sprintf(fpsText, "FPS: %i", GetFPS());
        DrawText(fpsText, 4, 64, 16, BLACK);

        if (playerPos.x < wallPos.x + wallSize.x || playerPos.y > TILE_SIZE * ROOM_HEIGHT || dead)
        {
            playerSpeed = 0.0f;
            wallSize.x = 0;
            ClearBackground(BLACK);
            char endText[64] = { 0 };
            sprintf(endText, "GAME OVER", 100, 100, 100, GRAY);
            DrawText(endText, 64, 100, 100, RED);

            char scoreText[64] = { 0 };
            sprintf(scoreText, "Score: %.0f", playerPos.x * 0.01f);
            DrawText(scoreText, 64, 364, 100,  GREEN);
        }

        if (playerPos.x >= 100000)
        {
            ClearBackground(BLACK);
            char endText[64] = { 0 };
            sprintf(endText, "YOU WIN", 100, 100, 100, GRAY);
            DrawText(endText, 64, 100, 100, GREEN);
        }

        EndDrawing();
    }

    // Unload
    UnloadTexture(tileTexture);
    UnloadImage(tileImage);

    UnloadTexture(bgTexture);
    UnloadImage(bgImage);

    UnloadTexture(charTexture);
    UnloadImage(charImage);


    CloseWindow();

    //InitAudioDevice();      // Initialize audio device

    //font = LoadFont("resources/mecha.png");
    //music = LoadMusicStream("resources/ambient.ogg");
    //fxCoin = LoadSound("resources/coin.wav");

    //SetMusicVolume(music, 1.0f);
    //PlayMusicStream(music);

    // Setup and init first screen

}