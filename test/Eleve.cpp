#pragma warning(disable : 4996)

#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include "G2D.h"
using namespace std;

// ===================== LEADERBOARD =====================

struct ScoreEntry
{
    string name;
    int    score;
    int    wave;
};

// ===================== GAME STATE =====================

enum class GameState { MENU, PLAYING, GAME_OVER };

// ===================== SPRITES =====================

struct Sprites
{
    V2  pos;
    V2  size;
    int IdSpriteBoss;
    int IdSpritePlayer;
    int IdSpriteEnemyBullet;
    int IdSpritePlayerBullet;
    int IdSpriteLifeBar_1;
    int IdSpriteLifeBar_2;
    int IdSpriteLifeBar_3;
    int IdSpriteLifeBar_4;
    int IdSpriteLifeBar_5;
    int IdSpriteAsteroid;
    int IdSpriteStar;
    int IdSpriteBasic;
    int IdSpriteFast;
    int IdSpriteKamikaze;
    int IdSpriteSplitter;
    int IdSpriteSniper;
    int IdSpriteTank;

    Sprites()
    {
        pos = V2(300, 100);
        size = V2(32, 27);
    }

    void InitTextures()
    {
        IdSpritePlayer = G2D::ExtractTextureFromPNG(".//Sprites//Spaceship//Spaceship.png", Transparency::UpperLeft);
        printf("Loading Player...\n");
        IdSpriteBoss = G2D::ExtractTextureFromPNG(".//Sprites//Enemy_spaceships//enemy_spaceship.png", Transparency::UpperLeft);
        printf("Loading Boss...\n");
        IdSpritePlayerBullet = G2D::ExtractTextureFromPNG(".//Sprites//Spaceships_bullets//spaceship_bullet.png", Transparency::UpperLeft);
        printf("Loading player bullet...\n");
        IdSpriteEnemyBullet = G2D::ExtractTextureFromPNG(".//Sprites//Enemy_bullets//enemy_bullet.png", Transparency::UpperLeft);
        printf("Loading enemy bullet...\n");
        IdSpriteAsteroid = G2D::ExtractTextureFromPNG(".//Sprites//Asteroids//asteroid.png", Transparency::UpperLeft);
        printf("Loading asteroid...\n");
        IdSpriteStar = G2D::ExtractTextureFromPNG(".//Sprites//Star//star.png", Transparency::UpperLeft);
        printf("Loading star...\n");
        IdSpriteBasic = G2D::ExtractTextureFromPNG(".//Sprites//Enemies//basic.png", Transparency::UpperLeft);
        printf("Loading basic enemy...\n");
        IdSpriteFast = G2D::ExtractTextureFromPNG(".//Sprites//Enemies//fast.png", Transparency::UpperLeft);
        printf("Loading fast enemy...\n");
        IdSpriteKamikaze = G2D::ExtractTextureFromPNG(".//Sprites//Enemies//kamikaze.png", Transparency::UpperLeft);
        printf("Loading kamikaze enemy...\n");
        IdSpriteSplitter = G2D::ExtractTextureFromPNG(".//Sprites//Enemies//splitter.png", Transparency::UpperLeft);
        printf("Loading splitter enemy...\n");
        IdSpriteSniper = G2D::ExtractTextureFromPNG(".//Sprites//Enemies//sniper.png", Transparency::UpperLeft);
        printf("Loading sniper enemy...\n");
        IdSpriteTank = G2D::ExtractTextureFromPNG(".//Sprites//Enemies//tank.png", Transparency::UpperLeft);
        printf("Loading tank enemy...\n");

        size = size * 2;
    }
};

// ===================== STRUCTS INTERNES =====================

struct Bullet
{
    V2   pos;
    V2   velocity;
    bool active = true;
    bool pierce = false;
};

// Fantome laisse pendant le dash Sandevistan
struct AfterImage
{
    V2    pos;
    float r;
    float life;
    float maxLife;
    int   colorIndex;
};

// Missile a trajectoire courbe limitee.
// Le missile est tire dans une direction initiale figee (initialDir).
// Il peut devier au maximum de maxAngle radians de cet axe.
// Au-dela il vole droit — le joueur peut donc l'esquiver en s'ecartant.
struct Missile
{
    V2    pos;
    V2    velocity;
    V2    initialDir;         // direction de tir figee a la creation
    float currentAngle = 0.f; // deviation courante / initialDir (radians)
    float maxAngle = 0.44f; // ~25 degres max de deviation
    float turnSpeed = 0.018f; // radians/frame — plus faible = plus lent a reagir
    float speed = 5.f;
    float life = 6.f;
    bool  active = true;
};

struct Explosion
{
    V2    pos;
    float radius = 0.f;
    float maxRadius = 40.f;
    float speed = 2.f;
    bool  active = true;
};

struct Star
{
    V2    pos;
    float speed;
};

struct FloatingText
{
    V2     pos;
    string text;
    float  life = 1.5f;
    float  maxLife = 1.5f;
    bool   active = true;
};

struct Meteor
{
    V2    pos;
    V2    velocity;
    float radius = 12.f;
    bool  active = true;
};

// ===================== ENEMY =====================

enum class EnemyType { BASIC, FAST, TANK, KAMIKAZE, BOSS, SNIPER, SPLITTER };

struct Enemy
{
    V2        pos;
    EnemyType type = EnemyType::BASIC;
    int       hp = 3;
    int       maxHp = 3;
    bool      active = true;

    float lastShotTime = 0;
    float lastRegenTime = 0;
    float lastTeleportTime = 0;
    float lastDodgeTime = 0;
    float aimStartTime = 0;

    bool teleporter = false;
    bool regenerates = false;
    bool shielded = false;
    bool dodger = false;
    bool aiming = false;

    // Sandevistan dash
    bool  sandevistanActive = false;
    V2    sandevistanTarget;
    float sandevistanSpeed = 28.f;
    float lastSandevistanTime = -10.f;
    float sandevistanCooldown = 5.f;
    float afterImageTimer = 0.f;
    V2    lastAfterImagePos;
    int   colorToggle = 0;

    int bossPhase = 0;
};

enum class DropType { SPEED, DAMAGE, HEALTH, FIRERATE, MULTISHOT, SHIELD, BOMB, PIERCE };

struct Drop
{
    V2       pos;
    DropType type;
    bool     active = true;
    float    blinkTime = 0.f;
};

struct Player
{
    V2    pos;
    float speed = 6.0f;
    float bulletSpeed = 10.f;
    float fireRate = 0.2f;
    int   shotCount = 1;
    int   hp = 5;
    int   shield = 0;
    int   bombs = 2;
    bool  pierce = false;
    bool  dashing = false;
    float dashTime = 0.f;
    float dashDuration = 0.2f;
    float dashCooldown = 1.0f;
    float lastDashTime = -10.f;
    V2    dashVelocity;
    float lastHitTime = -10.f;
    float invincDuration = 1.0f;
};

// ===================== GAME DATA =====================

struct GameData
{
    int  WidthPix = 600;
    int  HeightPix = 800;
    bool gameOver = false;
    bool paused = false;
    bool inpause = false; 

    GameState state = GameState::MENU;

    vector<ScoreEntry> leaderboard;
    string             inputName = "";
    bool               enteringName = false;

    Sprites sprites;

    float waveTransitionTime = -10.f;
    bool  inTransition = false;

    Player player{ V2(300, 50) };

    vector<Bullet>       bullets;
    vector<Bullet>       enemyBullets;
    vector<Missile>      missiles;
    vector<AfterImage>   afterImages;
    vector<Enemy>        enemies;
    vector<Drop>         drops;
    vector<Explosion>    explosions;
    vector<Star>         stars;
    vector<FloatingText> floatingTexts;
    vector<Meteor>       meteors;
    vector<Enemy>        toSpawn;

    int   score = 0;
    int   highScore = 0;
    int   combo = 0;
    float lastKillTime = 0.f;
    int   wave = 1;
    bool  bossSpawned = false;

    float waveStartTime = 0.f;
    bool  enraged = false;
    float waveAnnounceTime = -10.f;
    float lastMeteorTime = 0.f;
    float timeOffset = 0.f;
    float lastShotTime = 0.f;

    GameData()
    {
        for (int i = 0; i < 80; i++)
        {
            Star s;
            s.pos = V2(float(rand() % 600), float(rand() % 800));
            s.speed = 0.5f + (rand() % 20) * 0.1f;
            stars.push_back(s);
        }
        spawnWave(1);
    }

    void spawnWave(int w)
    {
        enemies.clear();
        drops.clear();
        missiles.clear();
        afterImages.clear();
        bossSpawned = false;
        enraged = false;

        if (w % 5 == 0)
        {
            Enemy boss;
            boss.pos = V2(300.f, 700.f);
            boss.type = EnemyType::BOSS;
            boss.hp = 50 + w * 5;
            boss.maxHp = boss.hp;
            boss.regenerates = true;
            boss.dodger = true;
            enemies.push_back(boss);
            bossSpawned = true;
        }
        else
        {
            int count = min(2 + w, 14);
            for (int i = 0; i < count; i++)
            {
                Enemy e;
                e.pos = V2(50.f + (i % 8) * 70.f, 600.f + (i / 8) * 60.f);

                int r = rand() % 6;
                if (r == 0) { e.type = EnemyType::BASIC;    e.hp = 3 + w; }
                else if (r == 1) { e.type = EnemyType::FAST;     e.hp = 2 + w; }
                else if (r == 2) { e.type = EnemyType::TANK;     e.hp = 6 + w * 2; }
                else if (r == 3) { e.type = EnemyType::KAMIKAZE; e.hp = 1; }
                else if (r == 4) { e.type = EnemyType::SNIPER;   e.hp = 2 + w; }
                else { e.type = EnemyType::SPLITTER; e.hp = 2 + w; }

                e.maxHp = e.hp;

                if (w >= 2 && rand() % max(2, 4 - w / 3) == 0) e.shielded = true;
                if (w >= 2 && rand() % max(2, 4 - w / 3) == 0) e.teleporter = true;
                if (w >= 3 && rand() % max(2, 5 - w / 3) == 0) e.regenerates = true;
                if (w >= 3 && rand() % 5 == 0)                  e.dodger = true;

                enemies.push_back(e);
            }
        }
    }
};

// ===================== HELPERS =====================

void spawnExplosion(GameData& G, V2 pos, float maxR = 40.f)
{
    Explosion ex;
    ex.pos = pos;
    ex.maxRadius = maxR;
    G.explosions.push_back(ex);
}

void spawnFloatingText(GameData& G, V2 pos, string text)
{
    FloatingText ft;
    ft.pos = pos;
    ft.text = text;
    G.floatingTexts.push_back(ft);
}

float enemyRadius(EnemyType t)
{
    switch (t)
    {
    case EnemyType::BASIC:    return 10.f;
    case EnemyType::FAST:     return 10.f;
    case EnemyType::TANK:     return 30.f;
    case EnemyType::KAMIKAZE: return 10.f;
    case EnemyType::BOSS:     return 40.f;
    case EnemyType::SNIPER:   return 20.f;
    case EnemyType::SPLITTER: return 22.f;
    default:                  return 15.f;
    }
}

void spawnAfterImage(GameData& G, V2 pos, float r, int colorIndex)
{
    AfterImage ai;
    ai.pos = pos;
    ai.r = r;
    ai.maxLife = 0.35f;
    ai.life = ai.maxLife;
    ai.colorIndex = colorIndex;
    G.afterImages.push_back(ai);
}

V2 EnemyShootPattern(const Enemy& e, const Player& player)
{
    V2 dir = player.pos - e.pos;
    dir.normalize();
    switch (e.type)
    {
    case EnemyType::BASIC:  return dir * 3;
    case EnemyType::FAST:   return dir * 8;
    case EnemyType::TANK:   return dir * 4;
    case EnemyType::BOSS:   return dir * 6;
    case EnemyType::SNIPER: return dir * 20;
    default:                return dir * 5;
    }
}

void useBomb(GameData& G)
{
    if (G.player.bombs <= 0) return;
    G.player.bombs--;
    for (auto& b : G.enemyBullets) b.active = false;
    for (auto& m : G.missiles)     m.active = false;
    for (auto& e : G.enemies)
    {
        if (!e.active) continue;
        if ((e.pos - G.player.pos).norm() < 200.f)
        {
            e.hp -= 5;
            if (e.hp <= 0)
            {
                e.active = false;
                G.score += 200;
                spawnExplosion(G, e.pos);
                spawnFloatingText(G, e.pos, "+200");
            }
        }
    }
    spawnExplosion(G, G.player.pos, 200.f);
}

void killEnemy(GameData& G, Enemy& e, float t)
{
    e.active = false;
    G.combo++;
    int bonus = 200 * min(G.combo, 10);
    G.score += bonus;
    G.lastKillTime = t;
    spawnExplosion(G, e.pos, e.type == EnemyType::BOSS ? 80.f : 40.f);
    spawnFloatingText(G, e.pos, "+" + to_string(bonus) + (G.combo > 1 ? " x" + to_string(G.combo) : ""));

    if (e.type == EnemyType::SPLITTER)
    {
        int aliveCount = 0;
        for (auto& en : G.enemies) if (en.active) aliveCount++;
        int maxEnemies = min(2 + G.wave, 12);
        if (aliveCount > 0 && aliveCount < maxEnemies)
        {
            for (int s = 0; s < 2; s++)
            {
                Enemy small;
                small.pos = V2(e.pos.x + (s == 0 ? -20.f : 20.f), e.pos.y);
                small.type = EnemyType::FAST;
                small.hp = 1;
                small.maxHp = 1;
                G.toSpawn.push_back(small);
            }
        }
    }

    int dropChance = max(10, 35 - (int)G.wave * 2);
    if (rand() % 100 < dropChance)
    {
        Drop d;
        d.pos = e.pos;
        int r = rand() % 10;
        d.type = (r < 3) ? DropType::HEALTH : static_cast<DropType>(rand() % 8);
        G.drops.push_back(d);
    }
}

// ===================== LOGIQUE =====================

void Logic(GameData& G)
{
    float t = G2D::elapsedTimeFromStartSeconds() - G.timeOffset;

    // ===== MENU =====
    if (G.state == GameState::MENU)
    {
        if (G2D::isKeyPressed(Key::ENTER))
            G.state = GameState::PLAYING;
        return;
    }

    // ===== SAISIE NOM =====
    if (G.state == GameState::GAME_OVER && G.enteringName)
    {
        static float lastInputTime = 0;
        const float  inputDelay = 0.15f;

        if (G2D::elapsedTimeFromStartSeconds() - lastInputTime > inputDelay)
        {
            for (char c = 'A'; c <= 'Z'; c++)
            {
                Key key = (Key)((int)Key::A + (c - 'A'));
                if (G2D::isKeyPressed(key) && G.inputName.size() < 3)
                {
                    G.inputName += c;
                    lastInputTime = G2D::elapsedTimeFromStartSeconds();
                }
            }
            if (G2D::isKeyPressed(Key::BACKSPACE) && !G.inputName.empty())
            {
                G.inputName.pop_back();
                lastInputTime = G2D::elapsedTimeFromStartSeconds();
            }
            if (G2D::isKeyPressed(Key::ENTER) && !G.inputName.empty())
            {
                ScoreEntry se{ G.inputName, G.score, G.wave };
                G.leaderboard.push_back(se);
                sort(G.leaderboard.begin(), G.leaderboard.end(),
                    [](const ScoreEntry& a, const ScoreEntry& b) { return a.score > b.score; });
                if (G.leaderboard.size() > 8) G.leaderboard.resize(8);
                G.enteringName = false;
            }
        }
        return;
    }

    // ===== RETRY =====
    if (G.state == GameState::GAME_OVER && G2D::isKeyPressed(Key::R))
    {
        int                best = G.highScore;
        Sprites            savedSpr = G.sprites;
        vector<ScoreEntry> savedLB = G.leaderboard;
        float              curTime = G2D::elapsedTimeFromStartSeconds();
        G = GameData();
        G.highScore = best;
        G.sprites = savedSpr;
        G.leaderboard = savedLB;
        G.timeOffset = curTime;
        G.state = GameState::PLAYING;
        return;
    }
    if (G.state == GameState::GAME_OVER) return;

    // gestion de la pause 



    static bool pWasDown = false;
    bool pIsDown = G2D::isKeyPressed(Key::P);
    bool pJustPressed = (pIsDown && !pWasDown);

    pWasDown = pIsDown; 
    if (pJustPressed)
    {
        G.paused = !G.paused;
        G.inpause = G.paused;
        printf("Changement de pause ! Nouvel etat : %s\n", G.paused ? "PAUSE" : "PLAY"); // enfinnn 

    }

    
    if (G.paused)
    {
        G.timeOffset += 0.0166f;
        return;
    }


    ///  pause a refaire car jsp pk on doit appuyé 4fois sur p pour entrer sortir du pause 0-0




    // ===== ETOILES =====
    for (auto& s : G.stars)
    {
        s.pos.y -= s.speed;
        if (s.pos.y < 0)
        {
            s.pos.y = float(G.HeightPix);
            s.pos.x = float(rand() % (2 * G.WidthPix));
        }
    }

    // ===== EXPLOSIONS =====
    for (auto& ex : G.explosions)
    {
        ex.radius += ex.speed;
        if (ex.radius >= ex.maxRadius) ex.active = false;
    }
    G.explosions.erase(
        remove_if(G.explosions.begin(), G.explosions.end(), [](const Explosion& e) { return !e.active; }),
        G.explosions.end());

    // ===== FLOATING TEXTS =====
    for (auto& ft : G.floatingTexts)
    {
        ft.pos.y += 1.f;
        ft.life -= 0.016f;
        if (ft.life <= 0) ft.active = false;
    }
    G.floatingTexts.erase(
        remove_if(G.floatingTexts.begin(), G.floatingTexts.end(), [](const FloatingText& f) { return !f.active; }),
        G.floatingTexts.end());

    // ===== AFTERIMAGES =====
    for (auto& ai : G.afterImages) ai.life -= 0.016f;
    G.afterImages.erase(
        remove_if(G.afterImages.begin(), G.afterImages.end(), [](const AfterImage& ai) { return ai.life <= 0.f; }),
        G.afterImages.end());

    // ===== METEORITES =====
    if (t - G.lastMeteorTime > 3.f)
    {
        Meteor m;
        m.pos = V2(float(rand() % G.WidthPix), float(G.HeightPix));
        m.velocity = V2((rand() % 3 - 1) * 1.f, -4.f);
        G.meteors.push_back(m);
        G.lastMeteorTime = t;
    }
    for (auto& m : G.meteors)
    {
        m.pos = m.pos + m.velocity;
        if (m.pos.y < 0) { m.active = false; continue; }

        bool invincCheck = G.player.dashing || (t - G.player.lastHitTime < G.player.invincDuration);
        if (!invincCheck && (m.pos - G.player.pos).norm() < 20.f)
        {
            G.player.hp -= 2;
            G.player.lastHitTime = t;
            m.active = false;
            spawnExplosion(G, m.pos, 50.f);
        }
    }
    G.meteors.erase(
        remove_if(G.meteors.begin(), G.meteors.end(), [](const Meteor& m) { return !m.active; }),
        G.meteors.end());

    // ===== MISSILES =====
   // on va sur un principe de tete chercheuse , bon quelques zones a verifier ex  parfois trop precis et d autre ne follow pas correctement a voir 
    {
        bool invincible = G.player.dashing || (t - G.player.lastHitTime < G.player.invincDuration);
        for (auto& m : G.missiles)
        {
            if (!m.active) continue;
            m.life -= 0.016f;
            if (m.life <= 0.f) { m.active = false; continue; }

            // Angle entre initialDir et la direction vers le joueur
            V2 toPlayer = G.player.pos - m.pos;
            toPlayer.normalize();
            float dot = m.initialDir.x * toPlayer.x + m.initialDir.y * toPlayer.y;
            float cross = m.initialDir.x * toPlayer.y - m.initialDir.y * toPlayer.x;
            float angleToPlayer = atan2f(cross, dot);

            // Clamp : le missile ne peut pas depasser maxAngle de son axe
            float targetAngle = max(-m.maxAngle, min(m.maxAngle, angleToPlayer));

            // Rotation progressive (turnSpeed par frame)
            float delta = targetAngle - m.currentAngle;
            if (delta > m.turnSpeed) delta = m.turnSpeed;
            else if (delta < -m.turnSpeed) delta = -m.turnSpeed;
            m.currentAngle += delta;

            // Reconstruire velocity depuis initialDir + currentAngle
            float c = cosf(m.currentAngle);
            float s = sinf(m.currentAngle);
            m.velocity = V2(
                m.initialDir.x * c - m.initialDir.y * s,
                m.initialDir.x * s + m.initialDir.y * c
            ) * m.speed;

            m.pos = m.pos + m.velocity;

            if (m.pos.x < -50 || m.pos.x > G.WidthPix + 50 ||
                m.pos.y < -50 || m.pos.y > G.HeightPix + 50)
            {
                m.active = false; continue;
            }
            if (!invincible && (m.pos - G.player.pos).norm() < 14.f)
            {
                if (G.player.shield > 0) G.player.shield--;
                else                     G.player.hp--;
                G.player.lastHitTime = t;
                m.active = false;
                spawnExplosion(G, m.pos, 30.f);
                G.combo = 0;
            }
        }
        G.missiles.erase(
            remove_if(G.missiles.begin(), G.missiles.end(), [](const Missile& m) { return !m.active; }),
            G.missiles.end());
    }

    // ===== VAGUE SUIVANTE =====
     // oeee bon la quand on avance un peu trop dans le jeu sa devient quasi trop galere a mod
    {
        int alive = 0;
        for (auto& e : G.enemies) if (e.active) alive++;
        if (alive == 0 && !G.inTransition) { G.inTransition = true; G.waveTransitionTime = t; }
    }
    if (G.inTransition && t - G.waveTransitionTime >= 3.f)
    {
        G.inTransition = false;
        G.wave++;
        G.waveStartTime = t;
        G.waveAnnounceTime = t;
        G.enraged = false;
        if (G.wave % 3 == 0)
        {
            G.player.bombs = min(3, G.player.bombs + 1);
            spawnFloatingText(G, V2(180, 420), "BOMBE +1 !");
        }
        G.spawnWave(G.wave);
    }

    // ===== ENRAGEMENT =====
    if (!G.inTransition && !G.enraged && t - G.waveStartTime > 10.f)
    {
        G.enraged = true;
        spawnFloatingText(G, V2(150, 400), "ENRAGES !");
    }

    // ===== PLAYER MOVE =====
    bool moving = false;
    V2   moveDir = { 0, 0 };
    if (G2D::isKeyPressed(Key::Q)) { moveDir.x -= 1; moving = true; }
    if (G2D::isKeyPressed(Key::D)) { moveDir.x += 1; moving = true; }
    if (G2D::isKeyPressed(Key::Z)) { moveDir.y += 1; moving = true; }
    if (G2D::isKeyPressed(Key::S)) { moveDir.y -= 1; moving = true; }

    bool canDash = (t - G.player.lastDashTime > G.player.dashCooldown);
    if (G2D::isKeyPressed(Key::LEFT) && canDash && moving)
    {
        G.player.dashing = true;
        G.player.dashTime = t;
        G.player.lastDashTime = t;
        moveDir.normalize();
        G.player.dashVelocity = moveDir * min(30.f, 15.f + G.player.speed);
    }

    if (G.player.dashing)
    {
        G.player.pos = G.player.pos + G.player.dashVelocity;
        if (t - G.player.dashTime > G.player.dashDuration) G.player.dashing = false;
    }
    else
    {
        G.player.pos.x += moveDir.x * G.player.speed;
        G.player.pos.y += moveDir.y * G.player.speed;
    }

    const float pRadius = 10.f;
    G.player.pos.x = max(pRadius, min(float(G.WidthPix) - pRadius, G.player.pos.x));
    G.player.pos.y = max(pRadius, min(float(G.HeightPix) - pRadius, G.player.pos.y));

    // ===== BOMBE =====
    {
        static bool bombPressed = false;
        if (G2D::isKeyPressed(Key::DOWN)) { if (!bombPressed) { useBomb(G); bombPressed = true; } }
        else bombPressed = false;
    }

    // ===== PLAYER SHOOT =====
    if (G2D::isKeyPressed(Key::UP) && (t - G.lastShotTime > G.player.fireRate))
    {
        for (int k = 0; k < G.player.shotCount; k++)
        {
            Bullet b;
            b.pos = G.player.pos;
            b.pierce = G.player.pierce;
            float spread = (G.player.shotCount > 1) ? (k - G.player.shotCount / 2) * 15.f : 0.f;
            b.velocity = V2(spread, G.player.bulletSpeed);
            G.bullets.push_back(b);
        }
        G.lastShotTime = t;
    }

    for (auto& b : G.bullets)
    {
        b.pos = b.pos + b.velocity;
        if (b.pos.y > G.HeightPix) b.active = false;
    }

    bool invincible = G.player.dashing || (t - G.player.lastHitTime < G.player.invincDuration);
    for (auto& b : G.enemyBullets)
    {
        b.pos = b.pos + b.velocity;
        if (b.pos.y < 0 || b.pos.y > G.HeightPix ||
            b.pos.x < 0 || b.pos.x > G.WidthPix)
        {
            b.active = false; continue;
        }
        if (!invincible && b.active && (b.pos - G.player.pos).norm() < 10.f)
        {
            if (G.player.shield > 0) G.player.shield--;
            else                     G.player.hp--;
            G.player.lastHitTime = t;
            b.active = false;
            G.combo = 0;
        }
    }

    // ===== COLLISION BULLETS / ENEMIES =====


// penser a augmenter le radius de notre perso en fonction du sprite 
    for (int i = 0; i < (int)G.bullets.size(); i++)
    {
        if (!G.bullets[i].active) continue;
        for (int j = 0; j < (int)G.enemies.size(); j++)
        {
            Enemy& e = G.enemies[j];
            if (!e.active || e.sandevistanActive) continue;

            float hitRadius = (e.type == EnemyType::BOSS) ? 40.f : 20.f;
            if ((G.bullets[i].pos - e.pos).norm() < hitRadius)
            {
                if (e.shielded) e.shielded = false;
                else            e.hp--;
                if (!G.bullets[i].pierce) G.bullets[i].active = false;
                if (e.hp <= 0) killEnemy(G, e, t);
            }
        }
    }

    for (auto& s : G.toSpawn) G.enemies.push_back(s);
    G.toSpawn.clear();

    if (t - G.lastKillTime > 3.f && G.combo > 0) G.combo = 0;

    G.bullets.erase(
        remove_if(G.bullets.begin(), G.bullets.end(), [](const Bullet& b) { return !b.active; }),
        G.bullets.end());
    G.enemyBullets.erase(
        remove_if(G.enemyBullets.begin(), G.enemyBullets.end(), [](const Bullet& b) { return !b.active; }),
        G.enemyBullets.end());

    // ===== BULLETS vs METEORITES =====
    for (auto& b : G.bullets)
    {
        if (!b.active) continue;
        for (auto& m : G.meteors)
        {
            if (!m.active) continue;
            if ((b.pos - m.pos).norm() < m.radius + 2.f)
            {
                m.active = false;
                if (!b.pierce) b.active = false;
                spawnExplosion(G, m.pos, 30.f);
            }
        }
    }

    // ===================== ENEMY AI =====================
    float enrageMultiplier = G.enraged ? 2.f : 1.f;

    for (int i = 0; i < (int)G.enemies.size(); i++)
    {
        Enemy& e = G.enemies[i];
        if (!e.active) continue;

        float r = enemyRadius(e.type);

        // ===== SANDEVISTAN : declenchement =====
        if (e.teleporter && !e.sandevistanActive
            && t - e.lastSandevistanTime > e.sandevistanCooldown)
        {
            e.sandevistanActive = true;
            e.lastSandevistanTime = t;
            e.afterImageTimer = 0.f;

            e.sandevistanTarget.x = max(40.f, min(float(G.WidthPix - 40), float(rand() % G.WidthPix)));
            e.sandevistanTarget.y = max(400.f, min(float(G.HeightPix - 40), float(rand() % G.HeightPix)));

            spawnFloatingText(G, e.pos, "!!");
        }

        // ===== SANDEVISTAN : execution =====

        // franchement c grave stylé le rendu t en pense quoi ??  bref tu me redis si tu trouve ça trop flashy
        if (e.sandevistanActive)
        {
            V2    toTarget = e.sandevistanTarget - e.pos;
            float dist = toTarget.norm();

            if (dist <= e.sandevistanSpeed)
            {
                e.pos = e.sandevistanTarget;
                e.sandevistanActive = false;
                e.afterImageTimer = 0.f;
                spawnExplosion(G, e.pos, 25.f);
                spawnAfterImage(G, e.pos, r * 1.4f, e.colorToggle % 2);
            }
            else
            {
                toTarget.normalize();
                e.pos = e.pos + toTarget * e.sandevistanSpeed;
                e.afterImageTimer += e.sandevistanSpeed;

                if (e.afterImageTimer >= 12.f)
                {
                    e.afterImageTimer = 0.f;
                    e.colorToggle = (e.colorToggle + 1) % 2;
                    spawnAfterImage(G, e.pos, r, e.colorToggle);
                }
            }
            continue;
        }

        // ===== MOUVEMENT NORMAL =====
        V2 dir = G.player.pos - e.pos;
        dir.normalize();

        float speed = 0.08f;
        switch (e.type)
        {
        case EnemyType::BASIC:    speed = (0.4f + G.wave * 0.05f) * enrageMultiplier; break;
        case EnemyType::FAST:     speed = (0.6f + G.wave * 0.08f) * enrageMultiplier; break;
        case EnemyType::TANK:     speed = (0.2f + G.wave * 0.03f) * enrageMultiplier; break;
        case EnemyType::KAMIKAZE: speed = (0.4f + G.wave * 0.1f) * enrageMultiplier; break;
        case EnemyType::BOSS:     speed = (0.4f + G.wave * 0.03f) * enrageMultiplier; break;
        case EnemyType::SNIPER:   speed = 0.1f;                                        break;
        case EnemyType::SPLITTER: speed = (0.5f + G.wave * 0.04f) * enrageMultiplier; break;
        }

        if (e.type == EnemyType::BOSS)
        {
            float dist = (G.player.pos - e.pos).norm();
            float idealDist = 300.f;
            float margin = 60.f;
            if (dist < idealDist - margin) e.pos = e.pos + dir * (-speed);
            else if (dist > idealDist + margin) e.pos = e.pos + dir * speed;
            else { V2 perp = { -dir.y, dir.x }; e.pos = e.pos + perp * speed; }
            e.pos.x = max(r + 5.f, min(float(G.WidthPix) - r - 5.f, e.pos.x));
            e.pos.y = max(r + 5.f, min(float(G.HeightPix) - r - 5.f, e.pos.y));
        }
        else if (e.type == EnemyType::SNIPER)
        {
            float dist = (G.player.pos - e.pos).norm();
            e.pos = e.pos + dir * (dist < 250.f ? -speed : speed);
        }
        else
        {
            e.pos = e.pos + dir * speed;
        }

        if (e.pos.x < -100.f || e.pos.x > G.WidthPix + 100.f ||
            e.pos.y < -100.f || e.pos.y > G.HeightPix + 100.f)
        {
            e.active = false; continue;
        }

        // ===== KAMIKAZE =====
        if (e.type == EnemyType::KAMIKAZE)
        {
            if (!invincible && (e.pos - G.player.pos).norm() < 20.f)
            {
                G.player.hp--;
                G.player.lastHitTime = t;
                killEnemy(G, e, t);
                G.combo = 0;
            }
            continue;
        }

        // ===== TIR =====
        float shotDelay = max(2.4f, (e.type == EnemyType::BOSS ? 0.5f : 1.5f) - G.wave * 0.05f);
        shotDelay /= enrageMultiplier;

        if (t - e.lastShotTime > shotDelay)
        {
            if (e.type == EnemyType::SNIPER)
            {
                if (!e.aiming) { e.aiming = true; e.aimStartTime = t; }
                else if (t - e.aimStartTime > 1.5f)
                {
                    Bullet b; b.pos = e.pos; b.velocity = EnemyShootPattern(e, G.player);
                    G.enemyBullets.push_back(b);
                    e.lastShotTime = t;
                    e.aiming = false;
                }
            }
            else if (e.type == EnemyType::BOSS)
            {
                e.bossPhase = (e.bossPhase + 1) % 2;
                if (e.bossPhase == 0)
                {
                    // Salve large
                    for (int k = -5; k <= 5; k++)
                    {
                        Bullet b; b.pos = e.pos;
                        V2 d = { (G.player.pos - e.pos).x + 40.f * k, (G.player.pos - e.pos).y };
                        d.normalize();
                        b.velocity = d * 5.f;
                        G.enemyBullets.push_back(b);
                    }
                }
                else
                {
                    // Missiles en eventail — deviation limitee, peuvent rater
                    for (int k = 0; k < 5; k++)
                    {
                        Missile m;
                        m.pos = e.pos;

                        float angle = (k - 2) * 0.4f;
                        V2 baseDir = G.player.pos - e.pos;
                        baseDir.normalize();

                        // Direction initiale figee au moment du tir (eventail)
                        m.initialDir = V2(
                            baseDir.x * cosf(angle) - baseDir.y * sinf(angle),
                            baseDir.x * sinf(angle) + baseDir.y * cosf(angle)
                        );
                        m.velocity = m.initialDir * m.speed;
                        m.currentAngle = 0.f;
                        // Agile progressivement selon la vague, mais jamais trop
                        m.turnSpeed = min(0.03f, 0.015f + G.wave * 0.002f);
                        m.maxAngle = 0.44f; // ~25 degres — reglable ici
                        G.missiles.push_back(m);
                    }
                }
                e.lastShotTime = t;
            }
            else if (e.type == EnemyType::TANK)
            {
                for (int k = -1; k <= 1; k++)
                {
                    Bullet b; b.pos = e.pos;
                    V2 d = { (G.player.pos - e.pos).x + 30.f * k, (G.player.pos - e.pos).y };
                    d.normalize();
                    b.velocity = d * 4.f;
                    G.enemyBullets.push_back(b);
                }
                e.lastShotTime = t;
            }
            else
            {
                Bullet b; b.pos = e.pos; b.velocity = EnemyShootPattern(e, G.player);
                G.enemyBullets.push_back(b);
                e.lastShotTime = t;
            }
        }

        // ===== REGENERATION =====
        if (e.regenerates && e.hp < e.maxHp && t - e.lastRegenTime > 2.f)
        {
            e.hp++;
            e.lastRegenTime = t;
        }

        // ===== DODGER =====
        if (e.dodger)
        {
            float minDist = 999999.f;
            V2    closestBullet = e.pos;
            for (auto& b : G.bullets)
            {
                if (!b.active) continue;
                float d = (b.pos - e.pos).norm();
                if (d < minDist) { minDist = d; closestBullet = b.pos; }
            }
            if (minDist < 120.f && t - e.lastDodgeTime > 0.5f)
            {
                V2   toBullet = closestBullet - e.pos;
                toBullet.normalize();
                V2   perp = { -toBullet.y, toBullet.x };
                V2   optionA = e.pos + perp * 40.f;
                bool aValid = (optionA.x > 20 && optionA.x < G.WidthPix - 20);
                V2   dodgeDir = aValid ? perp : V2(-perp.x, -perp.y);
                e.pos = e.pos + dodgeDir * 25.f;
                e.lastDodgeTime = t;
            }
        }
    }

    // ===== DROPS =====
    for (auto& d : G.drops)
    {
        if (!d.active) continue;
        d.pos.y -= 1.5f;
        d.blinkTime += 0.05f;
        if (d.pos.y < 0) { d.active = false; continue; }

        if ((d.pos - G.player.pos).norm() < 20.f)
        {
            string txt = "";
            switch (d.type)
            {
            case DropType::SPEED:     G.player.speed = min(12.f, G.player.speed + 1.1f);  txt = "SPEED !";     break;
            case DropType::HEALTH:    G.player.hp++;                                                     txt = "HP +1 !";     break;
            case DropType::DAMAGE:    G.player.bulletSpeed = min(20.f, G.player.bulletSpeed + 2.f);    txt = "DAMAGE !";    break;
            case DropType::FIRERATE:  G.player.fireRate = max(0.08f, G.player.fireRate - 0.03f);  txt = "FIRERATE !";  break;
            case DropType::MULTISHOT: G.player.shotCount = min(3, G.player.shotCount + 1);      txt = "MULTISHOT !"; break;
            case DropType::SHIELD:    G.player.shield = min(2, G.player.shield + 1);      txt = "SHIELD !";    break;
            case DropType::BOMB:      G.player.bombs = min(3, G.player.bombs + 1);      txt = "BOMB !";      break;
            case DropType::PIERCE:    G.player.pierce = true;                                       txt = "PIERCE !";    break;
            }
            spawnFloatingText(G, G.player.pos, txt);
            d.active = false;
        }
    }
    G.drops.erase(
        remove_if(G.drops.begin(), G.drops.end(), [](const Drop& d) { return !d.active; }),
        G.drops.end());

    if (G.score > G.highScore) G.highScore = G.score;

    if (G.player.hp <= 0)
    {
        G.gameOver = true;
        G.state = GameState::GAME_OVER;
        G.enteringName = true;
        G.inputName = "";
    }
}

// ===================== RENDER =====================

void render(const GameData& G)
{
    float t = G2D::elapsedTimeFromStartSeconds() - G.timeOffset;
    G2D::clearScreen(Color::Black);

    // Etoiles toujours visibles
    for (auto& s : G.stars)
        G2D::drawRectWithTexture(G.sprites.IdSpriteStar, V2(s.pos.x - 8, s.pos.y - 8), V2(5, 5));

    // ===== MENU =====
    if (G.state == GameState::MENU)
    {
        G2D::drawStringFontMono(V2(120, 650), "SPACE  SHOOTER", 30, 3, Color::Cyan);
        G2D::drawStringFontMono(V2(150, 580), "[ ENTREE ] Jouer", 20, 2, Color::Green);
        G2D::drawStringFontMono(V2(180, 500), "-- TOP SCORES --", 18, 2, Color::Yellow);

        int y = 460;
        for (int i = 0; i < (int)G.leaderboard.size() && i < 8; i++)
        {
            const ScoreEntry& se = G.leaderboard[i];
            string line = to_string(i + 1) + ". " + se.name + "   "
                + to_string(se.score) + "  (vague " + to_string(se.wave) + ")";
            Color c = (i == 0) ? Color::Yellow : (i == 1 ? Color::White : Color::Cyan);
            G2D::drawStringFontMono(V2(80, float(y)), line, 14, 1, c);
            y -= 28;
        }

        G2D::drawStringFontMono(V2(20, 100), "Z/Q/S/D : Mouvement", 14, 1, Color::White);
        G2D::drawStringFontMono(V2(20, 76), "HAUT     : Tirer", 14, 1, Color::White);
        G2D::drawStringFontMono(V2(20, 52), "BAS      : Bombe", 14, 1, Color::White);
        G2D::drawStringFontMono(V2(20, 28), "GAUCHE   : Dash", 14, 1, Color::White);
        G2D::drawStringFontMono(V2(330, 100), "P        : Pause", 14, 1, Color::White);
        G2D::Show();
        return;
    }

    // ===== GAME OVER =====
    if (G.state == GameState::GAME_OVER)
    {
        G2D::drawStringFontMono(V2(100, 680), "VOUS ETES MORTS", 30, 3, Color::Red);
        G2D::drawStringFontMono(V2(150, 620), "Score:     " + to_string(G.score), 20, 2, Color::White);
        G2D::drawStringFontMono(V2(150, 590), "Highscore: " + to_string(G.highScore), 20, 2, Color::Yellow);
        G2D::drawStringFontMono(V2(150, 560), "Vague:     " + to_string(G.wave), 20, 2, Color::Cyan);

        if (G.enteringName)
        {
            G2D::drawStringFontMono(V2(100, 500), "Entrez vos initiales :", 18, 2, Color::White);
            G2D::drawStringFontMono(V2(260, 470), G.inputName + "_", 22, 2, Color::Yellow);
            G2D::drawStringFontMono(V2(130, 430), "(max 3 lettres, ENTREE valider)", 12, 1, Color::White);
        }
        else
        {
            G2D::drawStringFontMono(V2(180, 490), "-- TOP SCORES --", 16, 2, Color::Yellow);
            int y = 455;
            for (int i = 0; i < (int)G.leaderboard.size() && i < 8; i++)
            {
                const ScoreEntry& se = G.leaderboard[i];
                string line = to_string(i + 1) + ". " + se.name + "   "
                    + to_string(se.score) + "  (vague " + to_string(se.wave) + ")";
                Color c = (i == 0) ? Color::Yellow : (i == 1 ? Color::White : Color::Cyan);
                G2D::drawStringFontMono(V2(80, float(y)), line, 12, 1, c);
                y -= 24;
            }
            G2D::drawStringFontMono(V2(170, 100), "[ R ] Rejouer", 20, 2, Color::Green);
        }
        G2D::Show();
        return;
    }

    // ===== PAUSE =====
    if (G.inpause  )
    {
        G2D::drawStringFontMono(V2(200, 430), "PAUSE", 30, 3, Color::Yellow);
        G2D::drawStringFontMono(V2(160, 380), "[ P ] Reprendre", 20, 2, Color::White);
        G2D::Show();
        return;


        // Rectangle semi-transparent pour assombrir le jeu
        G2D::drawRectangle(V2(0, 0), V2(G.WidthPix, G.HeightPix), Color(0, 0, 0, 0.6f), true);

        // Textes du menu
        G2D::drawStringFontMono(V2(200, 430), "PAUSE", 30, 3, Color::Yellow);
        G2D::drawStringFontMono(V2(160, 380), "[ P ] Reprendre", 20, 2, Color::White);

 
    
    }

    // ===== METEORITES =====
    for (auto& m : G.meteors)
    {
        float d = m.radius * 2;
        G2D::drawRectWithTexture(G.sprites.IdSpriteAsteroid,
            V2(m.pos.x - m.radius, m.pos.y - m.radius), V2(d, d));
    }

    // ===== EXPLOSIONS =====
    for (auto& ex : G.explosions)
        G2D::drawCircle(ex.pos, ex.radius, Color::Red, false);

    // ===== AFTERIMAGES =====
    for (auto& ai : G.afterImages)
    {
        float alpha = ai.life / ai.maxLife;
        float drawR = ai.r * (0.4f + 0.6f * alpha);
        Color col = (ai.colorIndex == 0) ? Color::Cyan : Color::Magenta;
        G2D::drawCircle(ai.pos, drawR, col, true);
        G2D::drawCircle(ai.pos, drawR + 3.f, col, false);
    }

    // ===== MISSILES =====
    for (auto& m : G.missiles)
    {
        if (!m.active) continue;
        G2D::drawCircle(m.pos, 5.f, Color::Red, true);
        G2D::drawLine(m.pos, m.pos - m.velocity * 2.f, Color::Yellow);
    }

    // ===== ENEMIES =====
    for (auto& e : G.enemies)
    {
        if (!e.active || e.sandevistanActive) continue;

        float r = enemyRadius(e.type);
        int spriteId = -1;
        switch (e.type)
        {
        case EnemyType::BASIC:    spriteId = G.sprites.IdSpriteBasic;    break;
        case EnemyType::FAST:     spriteId = G.sprites.IdSpriteFast;     break;
        case EnemyType::TANK:     spriteId = G.sprites.IdSpriteTank;     break;
        case EnemyType::KAMIKAZE: spriteId = G.sprites.IdSpriteKamikaze; break;
        case EnemyType::BOSS:     spriteId = G.sprites.IdSpriteBoss;     break;
        case EnemyType::SNIPER:   spriteId = G.sprites.IdSpriteSniper;   break;
        case EnemyType::SPLITTER: spriteId = G.sprites.IdSpriteSplitter; break;
        }

        float d = r * 2;
        V2 drawPos((int)e.pos.x, (int)e.pos.y);
        G2D::drawRectWithTexture(spriteId, V2(drawPos.x - r, drawPos.y - r), V2(d, d));

        if (e.type == EnemyType::SNIPER && e.aiming)
        {
            V2 dir = G.player.pos - e.pos;
            dir.normalize();
            G2D::drawLine(e.pos, V2(e.pos.x + dir.x * 300.f, e.pos.y + dir.y * 300.f), Color::Red);
        }

        float pct = float(e.hp) / float(e.maxHp);
        G2D::drawLine(V2(e.pos.x - r, e.pos.y - r - 5),
            V2(e.pos.x + r, e.pos.y - r - 5), Color::Red);
        G2D::drawLine(V2(e.pos.x - r, e.pos.y - r - 5),
            V2(e.pos.x - r + 2 * r * pct, e.pos.y - r - 5), Color::Green);

        if (e.dodger)      G2D::drawCircle(e.pos, r + 10, Color::Yellow, false);
        if (e.shielded)    G2D::drawCircle(e.pos, r + 6, Color::Cyan, false);
        if (e.regenerates) G2D::drawCircle(e.pos, r + 4, Color::Green, false);
        if (e.teleporter)
        {
            float pulse = 0.5f + 0.5f * sinf(G2D::elapsedTimeFromStartSeconds() * 6.f);
            G2D::drawCircle(e.pos, r + 8.f + pulse * 4.f, Color::Magenta, false);
        }
    }

    // ===== PLAYER =====
    V2 playerSize(32 * 2, 27 * 2);
    G2D::drawRectWithTexture(G.sprites.IdSpritePlayer,
        V2(G.player.pos.x - playerSize.x / 2, G.player.pos.y - playerSize.y / 2),
        playerSize);

    if (G.player.shield > 0)
    {
        float shieldRadius = max(playerSize.x, playerSize.y) / 2.f + 10.f;
        G2D::drawCircle(G.player.pos, shieldRadius, Color::Cyan, false);
        if (G.player.shield > 1)
            G2D::drawCircle(G.player.pos, shieldRadius + 6.f, Color::Blue, false);
    }

    // ===== BULLETS =====
    for (auto& b : G.bullets)
        G2D::drawRectWithTexture(G.sprites.IdSpritePlayerBullet, V2(b.pos.x - 8, b.pos.y - 10), V2(16, 20));
    for (auto& b : G.enemyBullets)
        G2D::drawRectWithTexture(G.sprites.IdSpriteEnemyBullet, V2(b.pos.x - 7, b.pos.y - 7), V2(14, 14));

    // ===== DROPS =====
    for (auto& d : G.drops)
    {
        if (!d.active) continue;
        if ((int)(d.blinkTime * 3) % 2 == 0) continue;

        Color c = Color::White;
        switch (d.type)
        {
        case DropType::SPEED:     c = Color::Yellow;  break;
        case DropType::HEALTH:    c = Color::Green;   break;
        case DropType::DAMAGE:    c = Color::Cyan;    break;
        case DropType::FIRERATE:  c = Color::Magenta; break;
        case DropType::MULTISHOT: c = Color::Red;     break;
        case DropType::SHIELD:    c = Color::Blue;    break;
        case DropType::BOMB:      c = Color::Red;     break;
        case DropType::PIERCE:    c = Color::White;   break;
        }
        G2D::drawCircle(d.pos, 8, c, true);
        if (d.type == DropType::HEALTH) G2D::drawStringFontMono(V2(d.pos.x - 6, d.pos.y - 5), "HP", 8, 1, Color::White);
        else if (d.type == DropType::BOMB)   G2D::drawStringFontMono(V2(d.pos.x - 5, d.pos.y - 5), "B", 8, 1, Color::White);
        else if (d.type == DropType::SHIELD) G2D::drawStringFontMono(V2(d.pos.x - 5, d.pos.y - 5), "S", 8, 1, Color::Yellow);
    }

    // ===== FLOATING TEXTS =====
    for (auto& ft : G.floatingTexts)
    {
        if (!ft.active) continue;
        Color col = (ft.life / ft.maxLife > 0.5f) ? Color::Yellow : Color::White;
        G2D::drawStringFontMono(ft.pos, ft.text, 16, 2, col);
    }

    // ===== TRANSITION =====
    if (G.inTransition)
    {
        string msg = "VAGUE " + to_string(G.wave + 1) + " ...";
        int    countdown = (int)(3.f - (t - G.waveTransitionTime)) + 1;
        G2D::drawStringFontMono(V2(160, 420), msg, 25, 3, Color::Cyan);
        if (countdown > 0)
            G2D::drawStringFontMono(V2(285, 370), to_string(countdown), 30, 3, Color::White);
    }

    // ===== ANNONCE VAGUE =====
    if (t - G.waveAnnounceTime < 2.f)
    {
        string msg = (G.wave % 5 == 0) ? "!!! BOSS !!!" : "VAGUE " + to_string(G.wave);
        Color  col = (G.wave % 5 == 0) ? Color::Red : Color::Cyan;
        G2D::drawStringFontMono(V2(180, 400), msg, 25, 3, col);
    }

    if (G.enraged)
        G2D::drawStringFontMono(V2(200, 50), "!! ENRAGES !!", 18, 2, Color::Red);

    // ===== HUD =====
    G2D::drawStringFontMono(V2(250, 780), "Vague: " + to_string(G.wave), 18, 2, Color::Cyan);
    G2D::drawStringFontMono(V2(480, 780), "Score: " + to_string(G.score), 18, 2, Color::White);
    G2D::drawStringFontMono(V2(480, 758), "Best:  " + to_string(G.highScore), 18, 2, Color::Yellow);
    G2D::drawStringFontMono(V2(10, 780), "HP:    " + to_string(G.player.hp), 18, 2, Color::Red);
    G2D::drawStringFontMono(V2(10, 758), "Shield:" + to_string(G.player.shield), 18, 2, Color::Cyan);
    G2D::drawStringFontMono(V2(10, 736), "Bombs: " + to_string(G.player.bombs), 18, 2, Color::Red);
    G2D::drawStringFontMono(V2(10, 714), "Shots: " + to_string(G.player.shotCount), 18, 2, Color::Yellow);
    G2D::drawStringFontMono(V2(10, 692), "Speed: " + to_string((int)G.player.speed) + "/12", 18, 2, Color::White);

    if (G.combo > 1)
        G2D::drawStringFontMono(V2(250, 756), "COMBO x" + to_string(G.combo), 20, 2, Color::Red);

    G2D::Show();
}

// ===================== MAIN =====================

int main(int argc, char* argv[])
{
    GameData G;
    G2D::initWindow(V2(G.WidthPix, G.HeightPix), V2(200, 200), "Space Shooter 2D");
    G.sprites.InitTextures();
    G2D::Run(Logic, render, G, 60, true);
}
