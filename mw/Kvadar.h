#include "Opruga.h"

#ifndef KVADAR_H
#define KVADAR_H

class Kvadar{

public:
	Tocka pos;
	float width	= 3.0f;
	float height	= 3.0f;

	float v;
	float m;

	Opruga* opruga = NULL;

	Kvadar( Tocka pos, float width, bool imaOprugu = false, int brojCvorova = 0);
	void Move();
};

#endif