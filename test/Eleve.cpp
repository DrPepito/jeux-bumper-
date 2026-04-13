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
    V2   pos;
    V2   velocity;
    bool active = true;
    bool pierce = false;
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

enum class EnemyType { BASIC, FAST, TANK, KAMIKAZE, BOSS, SNIPER, SPLITTER };

struct Enemy
{
    V2        pos;
    EnemyType type = EnemyType::BASIC;
    int       hp = 3;
    int       maxHp = 3;
    bool      active = true;
    float     lastShotTime = 0;
    bool      teleporter = false;
    bool      regenerates = false;
    bool      shielded = false;
    bool      dodger = false;
    float     lastRegenTime = 0;
    float     lastTeleportTime = 0;
    float     lastDodgeTime = 0;

    // sniper
    float     aimStartTime = 0;
    bool      aiming = false;
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
    int   WidthPix = 600;
    int   HeightPix = 800;
    bool  gameOver = false;
    bool  paused = false;
    
    float waveTransitionTime = -10.f;
    bool inTransition = false;


    Player player{ V2(300, 50) };

    vector<Bullet>       bullets;
    vector<Bullet>       enemyBullets;
    vector<Enemy>        enemies;
    vector<Drop>         drops;
    vector<Explosion>    explosions;
    vector<Star>         stars;
    vector<FloatingText> floatingTexts;
    vector<Meteor>       meteors;

    int   score = 0;
    int   highScore = 0;
    int   combo = 0;
    float lastKillTime = 0.f;
    int   wave = 1;
    bool  bossSpawned = false;

    // enragement
    float waveStartTime = 0.f;
    bool  enraged = false;

    // annonce de vague
    float waveAnnounceTime = -10.f;

    // météorites
    float lastMeteorTime = 0.f;
   

    //timing gen : 

    float timeOffset = 0.f;  

    float lastShotTime = 0.f;  // corrige la valeur négative empechant de tirer apres  un restart 


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
        bossSpawned = false;
        enraged = false;

        if (w % 5 == 0)
        {
            Enemy boss;
            boss.pos = V2(300.f, 700.f);
            boss.type = EnemyType::BOSS;
            boss.hp = 30 + w * 5;
            boss.maxHp = boss.hp;
            boss.regenerates = true;
            boss.dodger = true;
            enemies.push_back(boss);
            bossSpawned = true;
        }
        else
        {
            int count = min(2 + w, 14); // vague 1=3, v2=4, v3=5 ... plafond à 14
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

                // scaling difficulté
                if (w >= 2 && rand() % max(2, 4 - w / 3) == 0) e.shielded = true;
                if (w >= 2 && rand() % max(2, 4 - w / 3) == 0) e.teleporter = true;
                if (w >= 3 && rand() % max(2, 5 - w / 3) == 0) e.regenerates = true;
                if (w >= 3 && rand() % 5 == 0) e.dodger = true;

                enemies.push_back(e);
            }
        }
    }


};

// ===================== FUNCTIONS =====================

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

V2 EnemyShootPattern(const Enemy& e, const Player& player)
{
    V2 dir = player.pos - e.pos;
    dir.normalize();
    switch (e.type)
    {
    case EnemyType::BASIC:   return dir * 3;
    case EnemyType::FAST:    return dir * 8;
    case EnemyType::TANK:    return dir * 4;
    case EnemyType::BOSS:    return dir * 6;
    case EnemyType::SNIPER:  return dir * 20;
    default:                 return dir * 5;
    }
}

void useBomb(GameData& G)
{
    if (G.player.bombs <= 0) return;
    G.player.bombs--;
    for (auto& b : G.enemyBullets) b.active = false;
    for (auto& e : G.enemies)
    {
        if (!e.active) continue;
        V2 diff = e.pos - G.player.pos;
        if (diff.norm() < 200.f)
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

    // SPLITTER : spawn 2 petits ennemis
    if (e.type == EnemyType::SPLITTER)
    {
        // aliveCount - 1 car e est déjà marqué inactive mais encore dans le vecteur
        int aliveCount = 0;
        for (auto& en : G.enemies) if (en.active) aliveCount++;
        // e vient d'être mis inactive donc aliveCount ne le compte plus, c'est bon

        int maxEnemies = min(2 + G.wave, 12); // même plafond que spawnWave
        if (aliveCount > 0 && aliveCount < maxEnemies)
        {
            for (int s = 0; s < 2; s++)
            {
                Enemy small;
                small.pos = V2(e.pos.x + (s == 0 ? -20.f : 20.f), e.pos.y);
                small.type = EnemyType::FAST;
                small.hp = 1;
                small.maxHp = 1;
                G.enemies.push_back(small);
            }
        }
        // Si aliveCount == 0 : c'était le dernier, on ne spawne rien → transition déclenchée
    }

    // drop
    int dropChance = max(10, 35 - (int)G.wave * 2);
    if (rand() % 100 < dropChance)
    {
        Drop d;
        d.pos = e.pos;
        d.type = static_cast<DropType>(rand() % 8);
        G.drops.push_back(d);
    }
}

// ===================== LOGIC =====================

void Logic(GameData& G)
{

    float t = G2D::elapsedTimeFromStartSeconds() - G.timeOffset;



    //=============IU==============


    // ===== RETRY =====


    if (G.gameOver && G2D::isKeyPressed(Key::R))
    {
        int best = G.highScore;
        float currentTime = G2D::elapsedTimeFromStartSeconds(); // dcp le bug etait la , mieux vaut avoir le temps juste avant d appuyer sur R afin de le remetre a 0 
        G = GameData();
        G.highScore = best;
        G.timeOffset = currentTime; // leo on met un offset pour eviter le bug
        return;
    }

    if (G.gameOver) return;


    // ===== PAUSE =====
    static bool pausePressed = false;
    if (G2D::isKeyPressed(Key::P))
    {
        if (!pausePressed) { G.paused = !G.paused; pausePressed = true; }
    }
    else pausePressed = false;

    if (G.paused) return;






    // ===== FOND ETOILE =====
    for (auto& s : G.stars)
    {
        s.pos.y -= s.speed;
        if (s.pos.y < 0) { s.pos.y = float(G.HeightPix); s.pos.x = float(rand() % G.WidthPix); }
    }

    





    
    // ===== EXPLOSIONS =====
    for (auto& ex : G.explosions) { ex.radius += ex.speed; if (ex.radius >= ex.maxRadius) ex.active = false; }
    G.explosions.erase(remove_if(G.explosions.begin(), G.explosions.end(), [](const Explosion& e) { return !e.active; }), G.explosions.end());

    // ===== FLOATING TEXTS =====
    for (auto& ft : G.floatingTexts)
    {
        ft.pos.y += 1.f;
        ft.life -= 0.02f;
        if (ft.life <= 0) ft.active = false;
    }
    G.floatingTexts.erase(remove_if(G.floatingTexts.begin(), G.floatingTexts.end(), [](const FloatingText& f) { return !f.active; }), G.floatingTexts.end());


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
        if (m.pos.y < 0) m.active = false;

        // collision joueur
        V2 diff = m.pos - G.player.pos;
        bool invincCheck = G.player.dashing || (t - G.player.lastHitTime < G.player.invincDuration);
        if (diff.norm() < 20.f && !invincCheck)
        {
            G.player.hp -= 2;
            G.player.lastHitTime = t;
            m.active = false;
            spawnExplosion(G, m.pos, 50.f);
        }
    }
    G.meteors.erase(remove_if(G.meteors.begin(), G.meteors.end(), [](const Meteor& m) { return !m.active; }), G.meteors.end());

    // ===== VAGUE SUIVANTE =====
    int alive = 0;
    for (auto& e : G.enemies) if (e.active) alive++;
    if (alive == 0 && !G.inTransition)
    {
        G.inTransition = true;
        G.waveTransitionTime = t;
    }

    if (G.inTransition)
    {
        float elapsed = t - G.waveTransitionTime;
        if (elapsed >= 3.f)
        {
            G.inTransition = false;
            G.wave++;
            G.waveStartTime = t;
            G.waveAnnounceTime = t;
            G.enraged = false;
            G.spawnWave(G.wave);
        }
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

    // ===== DASH =====
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

    // ===== CLAMP =====
    float radius = 10.f;
    if (G.player.pos.x < radius)               G.player.pos.x = radius;
    if (G.player.pos.x > G.WidthPix - radius) G.player.pos.x = G.WidthPix - radius;
    if (G.player.pos.y < radius)               G.player.pos.y = radius;
    if (G.player.pos.y > G.HeightPix - radius) G.player.pos.y = G.HeightPix - radius;

    // ===== BOMBE =====
    static bool bombPressed = false;
    if (G2D::isKeyPressed(Key::DOWN)) { if (!bombPressed) { useBomb(G); bombPressed = true; } }
    else bombPressed = false;

    // ===== PLAYER SHOOT =====
    
    if (G2D::isKeyPressed(Key::UP) && (t -G.lastShotTime > G.player.fireRate))
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

    // ===== UPDATE PLAYER BULLETS =====
    for (auto& b : G.bullets)
    {
        b.pos = b.pos + b.velocity;
        if (b.pos.y > G.HeightPix) b.active = false;
    }

    // ===== UPDATE ENEMY BULLETS =====
    bool invincible = G.player.dashing || (t - G.player.lastHitTime < G.player.invincDuration);
    for (auto& b : G.enemyBullets)
    {
        b.pos = b.pos + b.velocity;
        if (b.pos.y < 0 || b.pos.y > G.HeightPix ||
            b.pos.x < 0 || b.pos.x > G.WidthPix) b.active = false;

        if (!invincible)
        {
            V2 diff = b.pos - G.player.pos;
            if (diff.norm() < 10 && b.active)
            {
                if (G.player.shield > 0) G.player.shield--;
                else                     G.player.hp--;
                G.player.lastHitTime = t;
                b.active = false;
                G.combo = 0;
            }
        }
    }

    // ===== COLLISION PLAYER BULLETS / ENEMIES =====
    for (int i = 0; i < (int)G.bullets.size(); i++)
    {
        if (!G.bullets[i].active) continue;
        for (int j = 0; j < (int)G.enemies.size(); j++)
        {
            Enemy& e = G.enemies[j];
            if (!e.active) continue;

            float hitRadius = (e.type == EnemyType::BOSS) ? 40.f : 20.f;
            V2 diff = G.bullets[i].pos - e.pos;
            if (diff.norm() < hitRadius)
            {
                if (e.shielded)       e.shielded = false;
                else                  e.hp--;
                if (!G.bullets[i].pierce) G.bullets[i].active = false;

                if (e.hp <= 0) killEnemy(G, e, t);
            }
        }
    }

    if (t - G.lastKillTime > 3.f && G.combo > 0) G.combo = 0;

    // ===== CLEAN BULLETS =====
    G.bullets.erase(remove_if(G.bullets.begin(), G.bullets.end(), [](const Bullet& b) { return !b.active; }), G.bullets.end());
    G.enemyBullets.erase(remove_if(G.enemyBullets.begin(), G.enemyBullets.end(), [](const Bullet& b) { return !b.active; }), G.enemyBullets.end());

    // ===== COLLISION BULLETS / METEORITES =====
    for (auto& b : G.bullets)
    {
        if (!b.active) continue;
        for (auto& m : G.meteors)
        {
            if (!m.active) continue;
            V2 diff = b.pos - m.pos;
            if (diff.norm() < m.radius + 2.f)
            {
                m.active = false;
                if (!b.pierce) b.active = false;
                spawnExplosion(G, m.pos, 30.f);
                // pas de score, pas de killEnemy -> ça compte pas comme ennemi
            }
        }
    }

    // ===== ENEMY AI =====
    float enrageMultiplier = G.enraged ? 2.f : 1.f;

    for (int i = 0; i < (int)G.enemies.size(); i++)
    {
        Enemy& e = G.enemies[i];
        if (!e.active) continue;

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
        case EnemyType::SNIPER:   speed = 0.1f; break; // sniper reste loin
        case EnemyType::SPLITTER: speed = (0.5f + G.wave * 0.04f) * enrageMultiplier; break;
        }

        // sniper garde ses distances
        if (e.type == EnemyType::SNIPER)
        {
            float dist = (G.player.pos - e.pos).norm();
            if (dist < 250.f) e.pos = e.pos + dir * (-speed);
            else              e.pos = e.pos + dir * speed;
        }
        else
        {
            e.pos = e.pos + dir * speed;
        }

        // désactiver si complètement hors écran (marge de 100px)
        if (e.pos.x < -100.f || e.pos.x > G.WidthPix + 100.f ||
            e.pos.y < -100.f || e.pos.y > G.HeightPix + 100.f)
        {
            e.active = false;
        }

        // kamikaze
        if (e.type == EnemyType::KAMIKAZE)
        {
            V2 diff = e.pos - G.player.pos;
            if (diff.norm() < 20 && !invincible)
            {
                G.player.hp--;
                G.player.lastHitTime = t;
                killEnemy(G, e, t);
                G.combo = 0;
            }
            continue;
        }

        // tir
        float shotDelay = max(2.4f, (e.type == EnemyType::BOSS ? 0.5f : 1.5f) - G.wave * 0.05f);
        shotDelay /= enrageMultiplier;

        if (t - e.lastShotTime > shotDelay)
        {
            if (e.type == EnemyType::SNIPER)
            {
                // vise pendant 1.5s avant de tirer
                if (!e.aiming)
                {
                    e.aiming = true;
                    e.aimStartTime = t;
                }
                else if (t - e.aimStartTime > 1.5f)
                {
                    Bullet b;
                    b.pos = e.pos;
                    b.velocity = EnemyShootPattern(e, G.player);
                    G.enemyBullets.push_back(b);
                    e.lastShotTime = t;
                    e.aiming = false;
                }
            }
            else if (e.type == EnemyType::TANK || e.type == EnemyType::BOSS)
            {
                int spread = (e.type == EnemyType::BOSS) ? 5 : 3;
                for (int k = -(spread / 2); k <= spread / 2; k++)
                {
                    Bullet b;
                    b.pos = e.pos;
                    V2 d = { (G.player.pos - e.pos).x + 30.f * k, (G.player.pos - e.pos).y };
                    d.normalize();
                    b.velocity = d * ((e.type == EnemyType::BOSS) ? 5.f : 4.f);
                    G.enemyBullets.push_back(b);
                }
                e.lastShotTime = t;
            }
            else
            {
                Bullet b;
                b.pos = e.pos;
                b.velocity = EnemyShootPattern(e, G.player);
                G.enemyBullets.push_back(b);
                e.lastShotTime = t;
            }
        }

        // téléportation
        if (e.teleporter && t - e.lastTeleportTime > 3.f)
        {
            e.pos = V2(float(rand() % G.WidthPix), float(400 + rand() % 300));
            e.lastTeleportTime = t;
        }

        // régénération
        if (e.regenerates && e.hp < e.maxHp && t - e.lastRegenTime > 2.f)
        {
            e.hp++;
            e.lastRegenTime = t;
        }

        // dodge
        if (e.dodger)
        {
            float minDist = 999999.f;
            V2    closestBullet = e.pos;
            for (auto& b : G.bullets)
            {
                if (!b.active) continue;
                V2    diff = b.pos - e.pos;
                float dist = diff.norm();
                if (dist < minDist) { minDist = dist; closestBullet = b.pos; }
            }
            if (minDist < 120.f && t - e.lastDodgeTime > 0.5f)
            {
                V2 toBullet = closestBullet - e.pos;
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

    // ===== DROPS : mouvement + clignotement =====
    for (auto& d : G.drops)
    {
        if (!d.active) continue;
        d.pos.y -= 1.5f;
        d.blinkTime += 0.05f;
        if (d.pos.y < 0) d.active = false;
    }

    // ===== DROPS : collecte =====
    for (auto& d : G.drops)
    {
        if (!d.active) continue;
        V2 diff = d.pos - G.player.pos;
        if (diff.norm() < 20)
        {
            string txt = "";
            switch (d.type)
            {
            case DropType::SPEED:     G.player.speed = min(12.f, G.player.speed + 1.1f);       txt = "SPEED !";     break;
            case DropType::HEALTH:    G.player.hp++;                                                   txt = "HP +1 !";     break;
            case DropType::DAMAGE:    G.player.bulletSpeed = min(20.f, G.player.bulletSpeed + 2.f);   txt = "DAMAGE !";    break;
            case DropType::FIRERATE:  G.player.fireRate = max(0.08f, G.player.fireRate - 0.03f);   txt = "FIRERATE !";  break;
            case DropType::MULTISHOT: G.player.shotCount = min(3, G.player.shotCount + 1);          txt = "MULTISHOT !"; break;
            case DropType::SHIELD:    G.player.shield = min(2, G.player.shield + 1);             txt = "SHIELD !";    break;
            case DropType::BOMB:      G.player.bombs = min(3, G.player.bombs + 1);              txt = "BOMB !";      break;
            case DropType::PIERCE:    G.player.pierce = true;                                     txt = "PIERCE !";    break;
            }
            spawnFloatingText(G, G.player.pos, txt);
            d.active = false;
        }
    }

    // ===== DROPS : nettoyage =====
    G.drops.erase(remove_if(G.drops.begin(), G.drops.end(), [](const Drop& d) { return !d.active; }), G.drops.end());

    // ===== HIGHSCORE =====
    if (G.score > G.highScore) G.highScore = G.score;

    // ===== MORT DU JOUEUR =====
    if (G.player.hp <= 0) { G.gameOver = true; }
}

// ===================== RENDER =====================

void render(const GameData& G)
{   

    float t = G2D::elapsedTimeFromStartSeconds() - G.timeOffset;

    G2D::clearScreen(Color::Black);

    // ===== GAME OVER =====
    if (G.gameOver)
    {
        G2D::drawStringFontMono(V2(100, 450), "VOUS ETES MORTS", 30, 3, Color::Red);
        G2D::drawStringFontMono(V2(150, 400), "Score:     " + to_string(G.score), 20, 2, Color::White);
        G2D::drawStringFontMono(V2(150, 370), "Highscore: " + to_string(G.highScore), 20, 2, Color::Yellow);
        G2D::drawStringFontMono(V2(150, 340), "Vague:     " + to_string(G.wave), 20, 2, Color::Cyan);
        G2D::drawStringFontMono(V2(170, 300), "[ R ] Rejouer", 20, 2, Color::Green);
        G2D::Show();
        return;
    }

    // ===== PAUSE =====
    if (G.paused)
    {
        // on affiche quand même le jeu en arrière plan
        for (auto& s : G.stars) G2D::drawCircle(s.pos, 1, Color::White, true);
        G2D::drawStringFontMono(V2(200, 430), "PAUSE", 30, 3, Color::Yellow);
        G2D::drawStringFontMono(V2(170, 380), "[ P ] Reprendre", 20, 2, Color::White);
        G2D::Show();
        return;
    }

    // ===== FOND ETOILE =====
    for (auto& s : G.stars)
        G2D::drawCircle(s.pos, 1, Color::White, true);

    // ===== METEORITES =====
    for (auto& m : G.meteors)
        G2D::drawCircle(m.pos, m.radius, Color::Red, true);

    // ===== EXPLOSIONS =====
    for (auto& ex : G.explosions)
        G2D::drawCircle(ex.pos, ex.radius, Color::Red, false);

    // ===== ENEMIES =====
    for (auto& e : G.enemies)
    {
        if (!e.active) continue;
        Color c = Color::White;
        float r = 15.f;
        switch (e.type)
        {
        case EnemyType::BASIC:    c = Color::Green;   r = 15.f; break;
        case EnemyType::FAST:     c = Color::Cyan;    r = 12.f; break;
        case EnemyType::TANK:     c = Color::Red;     r = 20.f; break;
        case EnemyType::KAMIKAZE: c = Color::Magenta; r = 10.f; break;
        case EnemyType::BOSS:     c = Color::Red;     r = 40.f; break;
        case EnemyType::SNIPER:   c = Color::Yellow;  r = 13.f; break;
        case EnemyType::SPLITTER: c = Color::Green;   r = 18.f; break;
        }

        // rouge si enragé
        if (G.enraged) c = Color::Red;

        G2D::drawCircle(e.pos, r, c, true);

        // laser de visée sniper
        if (e.type == EnemyType::SNIPER && e.aiming)
        {
            V2 dir = G.player.pos - e.pos;
            dir.normalize();
            G2D::drawLine(e.pos, V2(e.pos.x + dir.x * 300.f, e.pos.y + dir.y * 300.f), Color::Red);
        }

        // barre de vie
        float pct = float(e.hp) / float(e.maxHp);
        G2D::drawLine(V2(e.pos.x - r, e.pos.y - r - 5), V2(e.pos.x + r, e.pos.y - r - 5), Color::Red);
        G2D::drawLine(V2(e.pos.x - r, e.pos.y - r - 5), V2(e.pos.x - r + 2 * r * pct, e.pos.y - r - 5), Color::Green);

        if (e.dodger)      G2D::drawCircle(e.pos, r + 10, Color::Yellow, false);
        if (e.shielded)    G2D::drawCircle(e.pos, r + 6, Color::Cyan, false);
        if (e.regenerates) G2D::drawCircle(e.pos, r + 4, Color::Green, false);
        if (e.teleporter)  G2D::drawCircle(e.pos, r + 8, Color::Magenta, false);
    }

    // ===== PLAYER =====
    bool invincible = G.player.dashing ||
        (t - G.player.lastHitTime < G.player.invincDuration); // invincible apres etre toucher



    G2D::drawCircle(G.player.pos, 10, invincible ? Color::Yellow : Color::White, true);
    if (G.player.shield > 0)
        G2D::drawCircle(G.player.pos, 16, Color::Cyan, false);



    // ===== BULLETS =====
    for (auto& b : G.bullets)
        G2D::drawCircle(b.pos, b.pierce ? 4 : 2, b.pierce ? Color::Magenta : Color::Red, true);
    for (auto& b : G.enemyBullets)
        G2D::drawCircle(b.pos, 3, Color::Yellow, true);

    // ===== DROPS =====
    for (auto& d : G.drops)
    {
        if (!d.active) continue;
        // clignotement
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
        case DropType::BOMB:      c = Color::Red;  break;
        case DropType::PIERCE:    c = Color::White;   break;
        }
        G2D::drawCircle(d.pos, 8, c, true);
    }

    // ===== FLOATING TEXTS =====
    for (auto& ft : G.floatingTexts)
    {
        if (!ft.active) continue;
        float alpha = ft.life / ft.maxLife;
        Color col = (alpha > 0.5f) ? Color::Yellow : Color::White;
        G2D::drawStringFontMono(ft.pos, ft.text, 16, 2, col);
    }

    // ===== TRANSITION INTER-VAGUE =====
    if (G.inTransition)
    {
        float t2 = G2D::elapsedTimeFromStartSeconds() - G.timeOffset;
        string msg = "VAGUE " + to_string(G.wave + 1) + " ...";
        G2D::drawStringFontMono(V2(160, 420), msg, 25, 3, Color::Cyan);
        int countdown = (int)(3.f - (t2 - G.waveTransitionTime)) + 1;
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

    // ===== ENRAGEMENT =====
    if (G.enraged) 
        G2D::drawStringFontMono(V2(200, 50), "!! ENRAGES !!", 18, 2, Color::Red);

    // ===== HUD =====
    G2D::drawStringFontMono(V2(10, 780), "Score: " + to_string(G.score), 18, 2, Color::White);
    G2D::drawStringFontMono(V2(10, 758), "Best:  " + to_string(G.highScore), 18, 2, Color::Yellow);
    G2D::drawStringFontMono(V2(10, 736), "Vague: " + to_string(G.wave), 18, 2, Color::Cyan);
    G2D::drawStringFontMono(V2(10, 714), "HP:    " + to_string(G.player.hp), 18, 2, Color::Red);
    G2D::drawStringFontMono(V2(10, 692), "Shield:" + to_string(G.player.shield), 18, 2, Color::Cyan);
    G2D::drawStringFontMono(V2(10, 670), "Bombs: " + to_string(G.player.bombs), 18, 2, Color::Red);
    G2D::drawStringFontMono(V2(10, 648), "Shots: " + to_string(G.player.shotCount), 18, 2, Color::Yellow);
    G2D::drawStringFontMono(V2(10, 626), "Speed: " + to_string((int)G.player.speed) + "/12", 18, 2, Color::White);
    if (G.combo > 1)
        G2D::drawStringFontMono(V2(400, 780), "COMBO x" + to_string(G.combo), 20, 2, Color::Red);

    G2D::Show();
}

// ===================== MAIN =====================

int main(int argc, char* argv[])
{
    GameData G;
    G2D::initWindow(V2(G.WidthPix, G.HeightPix), V2(200, 200), "Ball Shooter 2D");
    G2D::Run(Logic, render, G, 60, true);
}