#pragma once

#include <cmath>

struct V2 
{


	
	float x, y;

	V2(float _x, float _y) { x = _x; y = _y; }
	

	float norm()          { return sqrt(x*x + y * y); }
	void normalize() { float n = norm(); if (n != 0) { x /= n; y /= n; } }
	
	V2 GetNormalized() { float n = norm(); return (n != 0) ? V2(x / n, y / n) : V2(0, 0); }
	
	V2() : x(0), y(0) {}
	


};

// comparaison sur des flottants... traitement spécial

bool operator == (const V2 & a, const V2 & b);

// redéfinition des opérateurs standards

V2 operator + (const V2 & a, const V2 & b);
V2 operator - (const V2 & a, const V2 & b);
V2 operator * (float      a, const V2 & b);
V2 operator * (const V2 & a, float      b);
V2 operator / (const V2 & a, float      b);
V2 operator - (const V2 & a);  // - unaire

// produit scalaire
float prodScal(const V2 & a, const V2 & b);
float prodVect(const V2 & a, const V2 & b);

// affichage 
std::ostream & operator << (std::ostream & os, V2 & t);

