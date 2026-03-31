#pragma warning( disable : 4996 ) 

#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include "G2D.h"
using namespace std;

struct Cible
{
    V2 pointA;
    V2 pointB;
    bool active = true;
};

struct Rangee 
{
    vector<Cible> cibles;

    bool toutesTouchees() const
    {
        for (const Cible& c : cibles)
            if (c.active) return false;
        return true;
    }

    void reactiver()
    {
        for (Cible& c : cibles)
            c.active = true;
    }
};

// ← NOUVEAU
struct Bumper
{
    V2 pos;
    bool active = true;
    float T0 = -10.0f; // datte de al derniere fois ou on touche  (ici si on a jamais touché)
    int  BumperRadius = 40;
    int  RBig = 70; // rayon max de l anim


};

///////////////////////////////////////////////////////////////////////////////

struct GameData
{
    int     idFrame = 0;
    int     HeightPix = 800;
    int     WidthPix = 600;
    V2      BallPos = V2(90, 100);
    V2      BallMove;
    int     BallRadius = 15;

    vector<V2> PreviousPos;

    vector<V2> LP{ V2(595, 550), V2(585, 596), V2(542, 638), V2(476, 671), V2(392, 692), V2(300, 700), V2(207, 692),
        V2(123, 671), V2(57, 638), V2(14, 596), V2(5, 550), V2(5,5), V2(595,5), V2(595,550) };

    // ← MODIFIÉ : vector de Bumper au lieu de V2
    vector<Bumper> BumperPos{ {V2(200, 400)}, {V2(400, 400)}, {V2(300, 550)} };
    int BumperRadius = 40;

    vector<Rangee> rangees;

    GameData()
    {
        PreviousPos.resize(50);
        BallMove = V2(10, 10);
        // r1 supprimé
    }
};

///////////////////////////////////////////////////////////////////////////////

int CollisionSegCir(V2 pointA, V2 pointB, float radius, V2 center)
{
    V2 segmentAB = pointB - pointA;
    V2 direction = segmentAB;
    direction.normalize();

    float projectionLength = prodScal(direction, center - pointA);

    if (projectionLength > 0 && projectionLength < segmentAB.norm())
    {
        V2 closestPoint = pointA + projectionLength * direction;
        V2 toCenter = center - closestPoint;
        if (toCenter.norm() < radius) return 2;
        else                          return 0;
    }
    if ((center - pointA).norm() < radius) return 1;
    if ((center - pointB).norm() < radius) return 3;
    return 0;
}

bool CollisionCirCir(V2 center1, float radius1, V2 center2, float radius2)
{
    V2 distanceVec = center1 - center2;
    return distanceVec.norm() < (radius1 + radius2);
}

///////////////////////////////////////////////////////////////////////////////

void render(const GameData& G)
{
    G2D::clearScreen(Color::Black);
    G2D::drawStringFontMono(V2(80, G.HeightPix - 70), string("Super Flipper"), 50, 5, Color::Blue);
    G2D::drawCircle(G.BallPos, G.BallRadius, Color::Red, true);

    // ← MODIFIÉ : couleur selon active
    for (const Bumper& b : G.BumperPos)
    {
        float t = G2D::elapsedTimeFromStartSeconds();
        float dt = t - b.T0; // temps écoulé depuis la collision

        if (dt >= 0.0f && dt <= 1.0f) // pendant 1 seconde : flash animé
        {
            // r(t) = RBig - (RBig - RBumper) * dt
            float r = b.RBig - (b.RBig - b.BumperRadius) * dt;
            G2D::drawCircle(b.pos, r, Color::Red, true); // cercle rouge grand
            G2D::drawCircle(b.pos, b.BumperRadius - 5, Color::Blue, true); // cœur bleu
        }
        else if (b.active)
        {
            G2D::drawCircle(b.pos, b.BumperRadius, Color::Blue, true); // normal
        }
        else
        {
            G2D::drawCircle(b.pos, b.BumperRadius, Color::Red, true);  // désactivé
        }
    }




    for (int i = 0; i < G.LP.size() - 1; i++)
        G2D::drawLine(G.LP[i], G.LP[i + 1], Color::Green);

    for (V2 previousPos : G.PreviousPos)
        G2D::setPixel(previousPos, Color::Green);

    if (G2D::isOnPause())
        G2D::drawStringFontMono(V2(100, G.HeightPix / 2), string("Pause..."), 50, 5, Color::Yellow);

    for (const Rangee& r : G.rangees)
    {
        for (const Cible& c : r.cibles)
        {
            if (c.active)
            {
                G2D::drawLine(c.pointA, c.pointB, Color::Green);
                G2D::drawLine(c.pointA + V2(1, 0), c.pointB + V2(1, 0), Color::Green);
                G2D::drawLine(c.pointA + V2(2, 0), c.pointB + V2(2, 0), Color::Green);
                G2D::drawLine(c.pointA + V2(3, 0), c.pointB + V2(3, 0), Color::Green);
            }
            else
            {
                G2D::drawLine(c.pointA, c.pointB, Color::Red);
            }
        }
    }

    G2D::Show();
}

///////////////////////////////////////////////////////////////////////////////

float dot(V2 vecA, V2 vecB) {
    return vecA.x * vecB.x + vecA.y * vecB.y;
}

V2 Rebond(V2& velocity, V2 normal) {
    normal.normalize();
    V2 tangent = { normal.y, -normal.x };
    float speedAlongTangent = dot(velocity, tangent);
    float speedAlongNormal = dot(velocity, normal);
    return tangent * speedAlongTangent - normal * speedAlongNormal;
}

void Logic(GameData& G)
{
    G.idFrame += 1;

    V2 newPos = G.BallPos + G.BallMove;

    if (newPos.x + G.BallRadius >= G.WidthPix)  G.BallMove.x = -G.BallMove.x;
    if (newPos.x - G.BallRadius <= 0)            G.BallMove.x = -G.BallMove.x;
    if (newPos.y + G.BallRadius >= G.HeightPix)  G.BallMove.y = -G.BallMove.y;
    if (newPos.y - G.BallRadius <= 0)            G.BallMove.y = -G.BallMove.y;

    G.BallPos = G.BallPos + G.BallMove;

    // Collision avec les segments des bords
    for (int i = 0; i < G.LP.size() - 1; i++)
    {
        int collision = CollisionSegCir(G.LP[i], G.LP[i + 1], G.BallRadius, G.BallPos);
        if (collision != 0)
        {
            V2 segmentAB = G.LP[i + 1] - G.LP[i];
            V2 normal = V2(-segmentAB.y , segmentAB.x *0.998);
            G.BallMove = Rebond(G.BallMove, normal);
            break;
        }
    }

    //       Collision avec les bumprs
    for (Bumper& b : G.BumperPos)
    {
        if (!b.active) continue;

        if (CollisionCirCir(G.BallPos, G.BallRadius, b.pos, G.BumperRadius))
        {
            V2 normal = G.BallPos - b.pos;
            G.BallMove = Rebond(G.BallMove, normal);
            b.active = false; // désactive le bumper

            //declare le moment ou on touche 
            b.T0 = G2D::elapsedTimeFromStartSeconds();  // timing demare


            break;
        }
    }

    // Si tous les bumpers sont désactivés → on réactive tout
    bool tousDesactives = true;
    for (Bumper& b : G.BumperPos)
        if (b.active) { tousDesactives = false; break; }

    if (tousDesactives)
        for (Bumper& b : G.BumperPos)
            b.active = true;

    // Collision avec les cibles
    for (Rangee& r : G.rangees)
    {
        for (Cible& c : r.cibles)
        {
            if (!c.active) continue;

            int collision = CollisionSegCir(c.pointA, c.pointB, G.BallRadius, G.BallPos);
            if (collision != 0)
            {
                c.active = false;
                V2 segmentAB = c.pointB - c.pointA;
                V2 normal = V2(-segmentAB.y, segmentAB.x);
                G.BallMove = Rebond(G.BallMove, normal);
            }
        }

        if (r.toutesTouchees())
            r.reactiver();
    }

    // Historique des positions
    G.PreviousPos.push_back(G.BallPos);
    G.PreviousPos.erase(G.PreviousPos.begin());
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    GameData G;
    G2D::initWindow(V2(G.WidthPix, G.HeightPix), V2(200, 200), string("Super Flipper 600 !!"));
    
    
    //HORLOGE

    int callToLogicPerSec = 50;
    G2D::Run(Logic, render, G, callToLogicPerSec, true);
}