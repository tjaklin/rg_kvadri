#include "Kvadar.h"

Kvadar::Kvadar( Tocka pos, float width, bool imaOprugu, int brojCvorova)
{
	this->pos = pos;
	this->width = width;
	if( imaOprugu ) this->opruga = new Opruga( this->pos.x + this->width, brojCvorova);
}

void Kvadar::Move()
{
	this->pos.x += this->v;
	if( this->opruga != NULL )
	{
		this->opruga->Move( this->pos.x + this->width );
//this->opruga->pos.x = this->pos.x + this->width;
	//(this->width - this->opruga->length_init);
	}
}
