#pragma warning( disable : 4996 ) 

#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include "G2D.h"
using namespace std;

int score = 0;
int *points = &score ;

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
        int CibleRadius = 10;

    }
};


struct Bumper
{
    V2 pos;
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

    //   : vector de Bumper 
    vector<Bumper> BumperPos{ {V2(200, 400)}, {V2(400, 400)}, {V2(300, 550)} };
    int BumperRadius = 40;

   

    //   : vector des cibles   on vas les mettr en format seg donc a b --> A-------B
    vector<Cible> CiblePos{
        // Rangée haute
        {V2(50,  150), V2(150, 150)},
        {V2(220, 150), V2(320, 150)},
        {V2(400, 150), V2(500, 150)},

        // Rangée milieu-haute (décalée)
        {V2(100, 280), V2(200, 280)},
        {V2(280, 280), V2(380, 280)},
        {V2(430, 280), V2(530, 280)},

        // Rangée milieu-basse
        {V2(50,  400), V2(180, 400)},
        {V2(250, 400), V2(350, 400)},
        {V2(420, 400), V2(550, 400)},

       
    
    };

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
    G2D::drawStringFontMono(V2(80, 60), to_string(score), 30, 3, Color::White);


    G2D::drawCircle(G.BallPos, G.BallRadius, Color::Red, true); //balle jouer 


    for (const Cible& c : G.CiblePos)
    {
        if (c.active)
        {
            G2D::drawLine(c.pointA, c.pointB, Color::Yellow);
            G2D::drawLine(c.pointA + V2(0, 1), c.pointB + V2(0, 1), Color::Yellow);
            G2D::drawLine(c.pointA + V2(0, 2), c.pointB + V2(0, 2), Color::Yellow);
        }

        else 
        {
        G2D::drawLine(c.pointA, c.pointB, Color::Red);
        G2D::drawLine(c.pointA + V2(0, 1), c.pointB + V2(0, 1), Color::Red);
        G2D::drawLine(c.pointA + V2(0, 2), c.pointB + V2(0, 2), Color::Red);
        }
    }



//bumper couleur
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
        
        else
        {
            G2D::drawCircle(b.pos, b.BumperRadius, Color::Blue, true);  // désactivé
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
        
       

        if (CollisionCirCir(G.BallPos, G.BallRadius, b.pos, G.BumperRadius))
        {
            *points += 100;

            V2 normal = G.BallPos - b.pos;
            G.BallMove = Rebond(G.BallMove, normal);

            //declare le moment ou on touche 
            b.T0 = G2D::elapsedTimeFromStartSeconds();  // timing demare


            break;
        }
    }


    // Collision avec les cibles
    for (Cible& c : G.CiblePos)
    {
        if (!c.active) continue;

        int collision = CollisionSegCir(c.pointA, c.pointB, G.BallRadius, G.BallPos);
        if (collision != 0)
        {
            *points += 500;      // incrémente le score
            c.active = false;   // disparaît

            V2 segmentAB = c.pointB - c.pointA;
            V2 normal = V2(-segmentAB.y, segmentAB.x);
            G.BallMove = Rebond(G.BallMove, normal);
            break;
        }
    }









    // Réactivation des CIBLES + bonus
    bool tousDesactivesCibles = true;
    for (Cible& c : G.CiblePos)
        if (c.active) { tousDesactivesCibles = false; break; }

    if (tousDesactivesCibles)
    {
        *points += 1111;              //  bonus toutes cibles touchées
        for (Cible& c : G.CiblePos)
            c.active = true;          //  réactivation
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