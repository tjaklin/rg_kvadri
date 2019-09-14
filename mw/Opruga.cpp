#include "Opruga.h"

Tocka::Tocka(){
}

Cvor::Cvor(){
}

Opruga::Opruga( float pos, int numNodes, int koef )
{
	this->pos.x	= pos;
	this->pos.y	= 0.0f;
	this->pos.z	= 0.0f;

	this->koeficijent = koef;
	this->length_init = 2.0f;
	this->length_current = this->length_init;

	if( numNodes >=2 ) this->broj_cvorova = numNodes;
		else return;

	displacement = 1.87f / (this->broj_cvorova - 1 );

	float j = -1.0f;
	for( int i = 0; i< this->broj_cvorova; ++i )
	{
		this->popisCvorova.push_back( Cvor() );
		this->popisCvorova[i].pos = Tocka( this->pos.x + i * displacement, 0.5f * j, this->pos.z );
		j *= -1.0f;
	}
}

void Opruga::Move( float pos, float offset )
{
	this->pos.x = pos;

	float j = -1.0f;
	for( int i = 0; i< this->popisCvorova.size(); ++i )
	{
		this->popisCvorova[i].pos = Tocka( this->pos.x + i * displacement, 0.5f * j, this->pos.z );
		j *= -1.0f;
	}

}
