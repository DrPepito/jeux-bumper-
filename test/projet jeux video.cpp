#pragma warning( disable : 4996 )

#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include "G2D.h"
using namespace std;

// ===================== STRUCTURES =====================

struct Bullet
{
    V2 pos;
    V2 velocity;
    bool active = true;
};

struct Enemy
{
    V2 pos;
    bool active = true;
};

struct Player
{
    V2 pos;
    float speed = 6.0f;
};

// ===================== GAME DATA =====================

struct GameData
{
    int WidthPix = 600;
    int HeightPix = 800;

    Player player{ V2(300, 50) };

    vector<Bullet> bullets;
    vector<Enemy> enemies;

    int score = 0;

    GameData()
    {
        // spawn ennemis
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                enemies.push_back({ V2(100 + i * 80, 600 + j * 60) });
            }
        }
    }
};

// ===================== LOGIC =====================

void Logic(GameData& G)
{
    // ===== PLAYER MOVE =====
    if (G2D::isKeyPressed('q')) G.player.pos.x -= G.player.speed;
    if (G2D::isKeyPressed('d')) G.player.pos.x += G.player.speed;
    if (G2D::isKeyPressed('z')) G.player.pos.y += G.player.speed;
    if (G2D::isKeyPressed('s')) G.player.pos.y -= G.player.speed;

    // ===== SHOOT =====
    static float lastShotTime = 0;
    float t = G2D::elapsedTimeFromStartSeconds();

    if (G2D::isKeyPressed(VK_SPACE) && (t - lastShotTime > 0.2f))
    {
        Bullet b;
        b.pos = G.player.pos;
        b.velocity = V2(0, 10);
        G.bullets.push_back(b);

        lastShotTime = t;
    }

    // ===== UPDATE BULLETS =====
    for (Bullet& b : G.bullets)
    {
        b.pos = b.pos + b.velocity;

        if (b.pos.y > G.HeightPix)
            b.active = false;
    }

    // ===== COLLISION BULLETS / ENEMIES =====
    for (Bullet& b : G.bullets)
    {
        if (!b.active) continue;

        for (Enemy& e : G.enemies)
        {
            if (!e.active) continue;

            V2 diff = b.pos - e.pos;
            if (diff.norm() < 20)
            {
                e.active = false;
                b.active = false;
                G.score += 100;
            }
        }
    }

    // ===== CLEAN =====
    G.bullets.erase(remove_if(G.bullets.begin(), G.bullets.end(),
        [](Bullet& b) { return !b.active; }), G.bullets.end());

    // ===== ENEMY MOVE (descend doucement) =====
    for (Enemy& e : G.enemies)
    {
        if (e.active)
            e.pos.y -= 0.2f;
    }
}

// ===================== RENDER =====================

void render(const GameData& G)
{
    G2D::clearScreen(Color::Black);

    // SCORE
    G2D::drawStringFontMono(V2(20, 750), "Score: " + to_string(G.score), 20, 2, Color::White);

    // PLAYER
    G2D::drawCircle(G.player.pos, 10, Color::White, true);

    // BULLETS
    for (const Bullet& b : G.bullets)
    {
        G2D::drawCircle(b.pos, 3, Color::Red, true);
    }

    // ENEMIES
    for (const Enemy& e : G.enemies)
    {
        if (e.active)
            G2D::drawCircle(e.pos, 15, Color::Green, true);
    }

    G2D::Show();
}

// ===================== MAIN =====================

int main(int argc, char* argv[])
{
    GameData G;
    G2D::initWindow(V2(G.WidthPix, G.HeightPix), V2(200, 200), "Shooter 2D");

    int fps = 60;
    G2D::Run(Logic, render, G, fps, true);
}
