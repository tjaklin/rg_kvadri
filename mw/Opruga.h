#include <iostream>
#include <vector>

#ifndef OPRUGA_H
#define OPRUGA_H

typedef struct Tocka Tocka;

struct Tocka{	// Tocka predstavlja jednu 3D tocku (s xyz koordinatama) -> ovo koristim
		// umjesto glm knjiznice
	// varijable
	float x;
	float y;
	float z;
	// funkcije
	Tocka();
	Tocka(float x, float y = 0.0f, float z = 0.0f){ this->x = x; this->y = y; this->z = z;};
};

typedef struct Cvor Cvor;

struct Cvor{
	// varijable
	Tocka pos;
	// funkcije
	Cvor();
};

//std::vector<SpringNode> neighbors; // Neighbors

class Opruga{
//	Opruga se sastoji od minimalno 2 povezana čvora. Svaki čvor ima svoju
//	poziciju (relativnu), pokazivač na susjedne čvorove te vrijednost
//	udaljenosti do tih susjednih čvorova.
//private:
public:
	std::vector<Cvor> popisCvorova;

//	glm::vec3 pos; // = popisCvorova[0].pos; -> IPAK NE OVAK
	Tocka pos; // = popisCvorova[0].pos; -> IPAK NE OVAK
	int koeficijent;
	float displacement;	// PREIMENOVATI U node_displacement -> tak neki dir
	float length_init; // = popisCvorova[ length()-1 ].pos - popisCvorova[0].pos;
	float length_current;
	float length_final = -1.0f;
	int broj_cvorova = 0;
//public:
//	funkcije za manipulaciju...
	Opruga( float pos, int numNodes, int koef = 100 );
	void Move( float pos, float offset = 0.0f );
};

#endif