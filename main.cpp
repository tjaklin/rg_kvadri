#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>	// iz ovoga koristim pow() i sqrt() funkcije
// moji includeovi
#include "mw/Kvadar.h"


// GLOBALNE VARIJABLE
Kvadar* a;	// Radi se pointer koji ce pokazivat na objekt klase Kvadar- a objekt ce se napravit u funkciji init(); Kvadar a ce biti onaj koji se giba i nema oprugu.
Kvadar* b;	// Isto tako, samo sto ce ovaj biti kvadar s oprugom koji miruje.
std::vector< float > brzineNakonSudara;

const float INIT_V = 0.07f;	// Ovo je pocetna brzina prvog kvadra.
bool isPaused = false;		// Ova bool varijabla odreduje je li simulaciju potrebno pauzirati. Vrijednost joj mijenjamo u funkciji keyboard();

// FUNKCIJE
// racunajBezier*
Tocka racunajBez( Tocka p0, Tocka p1, Tocka p2, float t, char word ){		// U ovoj funkciji se racunaju tocke koje cine zaobljeni rub kvadra.
									// U videu je objasnjeno kako radi Bezier... ugl potrebne su tri tocke koje cine prva tri parametra.
									// t je offset nase nove tocke, a parametar char word oznacava u kojoj 2D ravnini ce biti nasa nova
									// tocka. Ovisno o tome koje lice kvadra crtamo zelimo da nam ova funkcija vrati tocke u odgovarajucoj
									// ravnini. Prilozit cu sliku koja pokusava objasnit.
	Tocka res = Tocka(0.0f);

	if( word == 'x' ){
		res.x = p0.x;
		res.y = pow( 1-t, 2) * p0.y + (1-t)*2*t * p1.y + t*t*p2.y;
		res.z = pow( 1-t, 2) * p0.z + (1-t)*2*t * p1.z + t*t*p2.z;
	}
	else if( word == 'y'){
		res.x = pow( 1-t, 2) * p0.x + (1-t)*2*t * p1.x + t*t*p2.x;
		res.y = p0.y;
		res.z = pow( 1-t, 2) * p0.z + (1-t)*2*t * p1.z + t*t*p2.z;
	}
	else if( word == 'z'){
		res.x = pow( 1-t, 2) * p0.x + (1-t)*2*t * p1.x + t*t*p2.x;
		res.y = pow( 1-t, 2) * p0.y + (1-t)*2*t * p1.y + t*t*p2.y;
		res.z = p0.z;
	}
return res;
}

GLboolean CheckCollision(Kvadar& a, Kvadar& b) // AABB - AABB collision
{
	bool areColliding = false;

	areColliding = a.pos.x + a.width >= b.pos.x &&
			b.pos.x + b.width + b.opruga->length_init >= a.pos.x;

return areColliding;
}

std::vector<float> maksimalnaKompresija(Kvadar& a, Kvadar& b)		// U ovoj funkciji se racuna kolika ce biti duzina opruge pri maksimalnoj kompresiji, ali i kolika ce biti
{								// brzina kvadra nakon sudara ..Ime funkcije je zato dost lose.
	std::vector<float> noveBrzine;					// Tu ce se spremiti nove brzine dva kvadra.

		float v_a, v_b, v_a_sqrd, v_b_sqrd;
		v_a = (a.m - b.m)*a.v / (a.m + b.m);		// Nove brzine.
		v_b = (2 * a.m * a.v) / (a.m + b.m);

		v_a_sqrd = v_a * v_a;
		v_b_sqrd = v_b * v_b;

	float finalLength = 0.0f;
		finalLength = sqrt(
				 (a.m * ( (a.v*a.v) - v_a_sqrd ) - b.m * v_b_sqrd)
					 / b.opruga->koeficijent );
	b.opruga->length_final = finalLength;		// Sprema se finalna duzina opruge.
	noveBrzine.push_back( v_a );			// Spremaju se nove brzine.
	noveBrzine.push_back( v_b );

return noveBrzine;						// Brzine se vracaju da se mogu koristiti u display() funkciji.
}

GLboolean CompressSpring(Opruga& opruga, float pritisak)	// Funkcija prima oprugu i pritisak= trenutni polozaj prvog kvadra iz cega se izracuna koliko treba
{								// pritisnuti oprugu.
	bool maksimalnoKompresirana = false;			// Ovo jee return vrijednost. Na kraju se postavlja u true ako je opruga maksimalno kompresirana

	float offset = (opruga.pos.x + opruga.length_init - (pritisak - 0.1f) ); // Offset racuna koliko ce opruga biti kraca od svog pocetnog stanja.
	float displacement_perNode = (opruga.length_init - offset) / (opruga.popisCvorova.size() - 1);	// (trenutna duzina opruge/ broj njenih cvorova) da znamo za koliko ->
												// cemo smanjit udaljenost izmedu cvorova (da izgleda kao da je opruga pritisnuta).
		opruga.displacement = displacement_perNode;	// Vrijednost udaljenosti cvorova se sprema u objekt. ta se vrijednost koristi u Opruga->Move();

	opruga.length_current = opruga.length_init - offset;	// Azurira se trenutna duzina opruge.

	if( opruga.length_current <= opruga.length_final )	// Ukoliko je opruga maksimalno pritisnuta vrati true.
		maksimalnoKompresirana = true;

return maksimalnoKompresirana;
}

void crtajKvadar( Kvadar& kvadar ){		// Ovo je najruzniji dio programa. Crta se kvadar koristeci Bezierove krivulje.

	Tocka strana_1[4] = Tocka(0.0f);
	Tocka strana_2[4] = Tocka(0.0f);
	Tocka controlPoints[4] = Tocka(0.0f);
	Tocka fooPoints[4] = Tocka(0.0f);
	Tocka temp[2] = Tocka(0.0f);
	float offset = 0.5f;

//1
	strana_1[0] = Tocka( kvadar.pos.x+kvadar.width-offset, 1.0f, 1.0f);
	strana_2[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f-offset, 1.0f);
	controlPoints[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, 1.0f );
	fooPoints[0] = Tocka( controlPoints[0].x-offset, controlPoints[0].y-offset, controlPoints[0].z);

	strana_1[1] = Tocka( kvadar.pos.x+offset, 1.0f, 1.0f);
	strana_2[1] = Tocka( kvadar.pos.x, 1.0f-offset, 1.0f);
	controlPoints[1] = Tocka( kvadar.pos.x, 1.0f, 1.0f );
	fooPoints[1] = Tocka( controlPoints[1].x+offset, controlPoints[1].y-offset, controlPoints[1].z);

	strana_1[2] = Tocka( kvadar.pos.x+offset, -1.0f, 1.0f);
	strana_2[2] = Tocka( kvadar.pos.x, -1.0f+offset, 1.0f);
	controlPoints[2] = Tocka( kvadar.pos.x,-1.0f, 1.0f );
	fooPoints[2] = Tocka( controlPoints[2].x+offset, controlPoints[2].y+offset, controlPoints[2].z);

	strana_1[3] = Tocka( kvadar.pos.x+kvadar.width-offset, -1.0f, 1.0f);
	strana_2[3] = Tocka( kvadar.pos.x+kvadar.width, -1.0f+offset, 1.0f);
	controlPoints[3] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, 1.0f );
	fooPoints[3] = Tocka( controlPoints[3].x-offset, controlPoints[3].y+offset, controlPoints[3].z);

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_1[i].x, strana_1[i].y, strana_1[i].z );
	glEnd();

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_2[i].x, strana_2[i].y, strana_2[i].z );
	glEnd();

		for( float i = 0.0f; i<1.0f; i+=0.1f)
			for( int j = 0; j<4; j++){
				temp[0] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i, 'z');
				temp[1] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i+0.1f, 'z');
			glBegin( GL_TRIANGLES );				
				glVertex3f( temp[0].x, temp[0].y, temp[0].z);
				glVertex3f( temp[1].x, temp[1].y, temp[1].z);
				glVertex3f( fooPoints[j].x, fooPoints[j].y, fooPoints[j].z);
			glEnd();
			};

//2
	strana_1[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, 1.0f-offset);
	strana_2[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f-offset, 1.0f);
	controlPoints[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, 1.0f );
	fooPoints[0] = Tocka( controlPoints[0].x, controlPoints[0].y-offset, controlPoints[0].z-offset);

	strana_1[1] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, 1.0f-offset);
	strana_2[1] = Tocka( kvadar.pos.x+kvadar.width, -1.0f+offset, 1.0f);
	controlPoints[1] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, 1.0f );
	fooPoints[1] = Tocka( controlPoints[1].x, controlPoints[1].y+offset, controlPoints[1].z-offset);

	strana_1[2] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, -1.0f+offset);
	strana_2[2] = Tocka( kvadar.pos.x+kvadar.width, -1.0f+offset, -1.0f);
	controlPoints[2] = Tocka( kvadar.pos.x,-1.0f, -1.0f );
	fooPoints[2] = Tocka( controlPoints[2].x, controlPoints[2].y+offset, controlPoints[2].z+offset);

	strana_1[3] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, -1.0f+offset);
	strana_2[3] = Tocka( kvadar.pos.x+kvadar.width, 1.0f-offset, -1.0f);
	controlPoints[3] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, -1.0f );
	fooPoints[3] = Tocka( controlPoints[3].x, controlPoints[3].y-offset, controlPoints[3].z+offset);

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_1[i].x, strana_1[i].y, strana_1[i].z );
	glEnd();

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_2[i].x, strana_2[i].y, strana_2[i].z );
	glEnd();

		for( float i = 0.0f; i<1.0f; i+=0.1f)
			for( int j = 0; j<4; j++){
				temp[0] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i, 'x');
				temp[1] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i+0.1f, 'x');
			glBegin( GL_TRIANGLES );
				glVertex3f( temp[0].x, temp[0].y, temp[0].z);
				glVertex3f( temp[1].x, temp[1].y, temp[1].z);
				glVertex3f( fooPoints[j].x, fooPoints[j].y, fooPoints[j].z);
			glEnd();
			};

//3
		strana_1[0] = Tocka( kvadar.pos.x+kvadar.width-offset, 1.0f, -1.0f);
		strana_2[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f-offset, -1.0f);
	controlPoints[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, -1.0f );
	fooPoints[0] = Tocka( controlPoints[0].x-offset, controlPoints[0].y-offset, controlPoints[0].z);

		strana_1[1] = Tocka( kvadar.pos.x+offset, 1.0f, -1.0f);
		strana_2[1] = Tocka( kvadar.pos.x, 1.0f-offset, -1.0f);
	controlPoints[1] = Tocka( kvadar.pos.x, 1.0f, -1.0f );
	fooPoints[1] = Tocka( controlPoints[1].x+offset, controlPoints[1].y-offset, controlPoints[1].z);

		strana_1[2] = Tocka( kvadar.pos.x+offset, -1.0f, -1.0f);
		strana_2[2] = Tocka( kvadar.pos.x, -1.0f+offset, -1.0f);
	controlPoints[2] = Tocka( kvadar.pos.x,-1.0f, -1.0f );
	fooPoints[2] = Tocka( controlPoints[2].x+offset, controlPoints[2].y+offset, controlPoints[2].z);

		strana_1[3] = Tocka( kvadar.pos.x+kvadar.width-offset, -1.0f, -1.0f);
		strana_2[3] = Tocka( kvadar.pos.x+kvadar.width, -1.0f+offset, -1.0f);
	controlPoints[3] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, -1.0f );
	fooPoints[3] = Tocka( controlPoints[3].x-offset, controlPoints[3].y+offset, controlPoints[3].z);

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_1[i].x, strana_1[i].y, strana_1[i].z );
	glEnd();

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_2[i].x, strana_2[i].y, strana_2[i].z );
	glEnd();

		for( float i = 0.0f; i<1.0f; i+=0.1f)
			for( int j = 0; j<4; j++){
				temp[0] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i, 'z');
				temp[1] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i+0.1f, 'z');
			glBegin( GL_TRIANGLES );
				glVertex3f( temp[0].x, temp[0].y, temp[0].z);
				glVertex3f( temp[1].x, temp[1].y, temp[1].z);
				glVertex3f( fooPoints[j].x, fooPoints[j].y, fooPoints[j].z);
			glEnd();
			};

//4
	strana_1[0] = Tocka( kvadar.pos.x, 1.0f, 1.0f-offset);
	strana_2[0] = Tocka( kvadar.pos.x, 1.0f-offset, 1.0f);
	controlPoints[0] = Tocka( kvadar.pos.x, 1.0f, 1.0f );
	fooPoints[0] = Tocka( controlPoints[0].x, controlPoints[0].y-offset, controlPoints[0].z-offset);

	strana_1[1] = Tocka( kvadar.pos.x, -1.0f, 1.0f-offset);
	strana_2[1] = Tocka( kvadar.pos.x, -1.0f+offset, 1.0f);
	controlPoints[1] = Tocka( kvadar.pos.x, -1.0f, 1.0f );
	fooPoints[1] = Tocka( controlPoints[1].x, controlPoints[1].y+offset, controlPoints[1].z-offset);

	strana_1[2] = Tocka( kvadar.pos.x, -1.0f, -1.0f+offset);
	strana_2[2] = Tocka( kvadar.pos.x, -1.0f+offset, -1.0f);
	controlPoints[2] = Tocka( kvadar.pos.x,-1.0f, -1.0f );
	fooPoints[2] = Tocka( controlPoints[2].x, controlPoints[2].y+offset, controlPoints[2].z+offset);

	strana_1[3] = Tocka( kvadar.pos.x, 1.0f, -1.0f+offset);
	strana_2[3] = Tocka( kvadar.pos.x, 1.0f-offset, -1.0f);
	controlPoints[3] = Tocka( kvadar.pos.x, 1.0f, -1.0f );
	fooPoints[3] = Tocka( controlPoints[3].x, controlPoints[3].y-offset, controlPoints[3].z+offset);

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_1[i].x, strana_1[i].y, strana_1[i].z );
	glEnd();

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_2[i].x, strana_2[i].y, strana_2[i].z );
	glEnd();

		for( float i = 0.0f; i<1.0f; i+=0.1f)
			for( int j = 0; j<4; j++){
				temp[0] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i, 'x');
				temp[1] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i+0.1f, 'x');
			glBegin( GL_TRIANGLES );
				glVertex3f( temp[0].x, temp[0].y, temp[0].z);
				glVertex3f( temp[1].x, temp[1].y, temp[1].z);
				glVertex3f( fooPoints[j].x, fooPoints[j].y, fooPoints[j].z);
			glEnd();
			};
//5
		strana_1[0] = Tocka( kvadar.pos.x+kvadar.width-offset, 1.0f, 1.0f);
		strana_2[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, 1.0f-offset);
	controlPoints[0] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, 1.0f );
	fooPoints[0] = Tocka( controlPoints[0].x-offset, controlPoints[0].y, controlPoints[0].z-offset);

		strana_1[1] = Tocka( kvadar.pos.x+offset, 1.0f, 1.0f);
		strana_2[1] = Tocka( kvadar.pos.x, 1.0f, 1.0f-offset);
	controlPoints[1] = Tocka( kvadar.pos.x, 1.0f, 1.0f );
	fooPoints[1] = Tocka( controlPoints[1].x+offset, controlPoints[1].y, controlPoints[1].z-offset);

		strana_1[2] = Tocka( kvadar.pos.x+offset, 1.0f, -1.0f);
		strana_2[2] = Tocka( kvadar.pos.x, 1.0f, -1.0f+offset);
	controlPoints[2] = Tocka( kvadar.pos.x, 1.0f, -1.0f );
	fooPoints[2] = Tocka( controlPoints[2].x+offset, controlPoints[2].y, controlPoints[2].z+offset);

		strana_1[3] = Tocka( kvadar.pos.x+kvadar.width-offset, 1.0f, -1.0f);
		strana_2[3] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, -1.0f+offset);
	controlPoints[3] = Tocka( kvadar.pos.x+kvadar.width, 1.0f, -1.0f );
	fooPoints[3] = Tocka( controlPoints[3].x-offset, controlPoints[3].y, controlPoints[3].z+offset);

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_1[i].x, strana_1[i].y, strana_1[i].z );
	glEnd();

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_2[i].x, strana_2[i].y, strana_2[i].z );
	glEnd();

		for( float i = 0.0f; i<1.0f; i+=0.1f)
			for( int j = 0; j<4; j++){
				temp[0] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i, 'y');
				temp[1] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i+0.1f, 'y');
			glBegin( GL_TRIANGLES );
				glVertex3f( temp[0].x, temp[0].y, temp[0].z);
				glVertex3f( temp[1].x, temp[1].y, temp[1].z);
				glVertex3f( fooPoints[j].x, fooPoints[j].y, fooPoints[j].z);
			glEnd();
			};
//6
		strana_1[0] = Tocka( kvadar.pos.x+kvadar.width-offset, -1.0f, 1.0f);
		strana_2[0] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, 1.0f-offset);
	controlPoints[0] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, 1.0f );
	fooPoints[0] = Tocka( controlPoints[0].x-offset, controlPoints[0].y, controlPoints[0].z-offset);

		strana_1[1] = Tocka( kvadar.pos.x+offset, -1.0f, 1.0f);
		strana_2[1] = Tocka( kvadar.pos.x, -1.0f, 1.0f-offset);
	controlPoints[1] = Tocka( kvadar.pos.x, -1.0f, 1.0f );
	fooPoints[1] = Tocka( controlPoints[1].x+offset, controlPoints[1].y, controlPoints[1].z-offset);

		strana_1[2] = Tocka( kvadar.pos.x+offset, -1.0f, -1.0f);
		strana_2[2] = Tocka( kvadar.pos.x, -1.0f, -1.0f+offset);
	controlPoints[2] = Tocka( kvadar.pos.x,-1.0f, -1.0f );
	fooPoints[2] = Tocka( controlPoints[2].x+offset, controlPoints[2].y, controlPoints[2].z+offset);

		strana_1[3] = Tocka( kvadar.pos.x+kvadar.width-offset, -1.0f, -1.0f);
		strana_2[3] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, -1.0f+offset);
	controlPoints[3] = Tocka( kvadar.pos.x+kvadar.width, -1.0f, -1.0f );
	fooPoints[3] = Tocka( controlPoints[3].x-offset, controlPoints[3].y, controlPoints[3].z+offset);

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_1[i].x, strana_1[i].y, strana_1[i].z );
	glEnd();

	glBegin( GL_QUADS );
		for( int i = 0; i<4; i++)
			glVertex3f( strana_2[i].x, strana_2[i].y, strana_2[i].z );
	glEnd();

		for( float i = 0.0f; i<1.0f; i+=0.1f)
			for( int j = 0; j<4; j++){
				temp[0] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i, 'y');
				temp[1] = racunajBez( strana_1[j], controlPoints[j], strana_2[j], i+0.1f, 'y');
			glBegin( GL_TRIANGLES );
				glVertex3f( temp[0].x, temp[0].y, temp[0].z);
				glVertex3f( temp[1].x, temp[1].y, temp[1].z);
				glVertex3f( fooPoints[j].x, fooPoints[j].y, fooPoints[j].z);
			glEnd();
			};

}

void crtajOprugu( Opruga& opruga){		// Crtanjee opruge. Opruga se sastoji od nekoliko cvorova (moguce zadati koliko) koji se povezuju duzinama, a
						// prilikom kompresiranja opruge se udaljenost izmedu svih cvorova smanji kako bi izgledala realnije.

	glLineWidth(4.0f);			// Debljina linija koje povezuju cvorove u opruzi

	for(int i=0; i<opruga.broj_cvorova-1; ++i)	// Opruga se prikazuje tako da se nacrta duzina izmedu svakog x i x+1 cvora.
	{
		glBegin(GL_LINES);			// Crtaj duzinu
			glVertex3f( opruga.popisCvorova[i].pos.x, opruga.popisCvorova[i].pos.y, 0.0f);		// crtaj prvu tocku iz para (x,x+1)
			glVertex3f( opruga.popisCvorova[i+1].pos.x, opruga.popisCvorova[i+1].pos.y, 0.0f);	// crtaj drugu tocku
		glEnd();
	};

	glLineWidth(1.0f);			// Vrati se debljina na defaultnu vrijednost... to nicem ne sluzi bas.
}

void zadajVrijednostiKvadra(){		// Ova funkcija postavlja pocetne polozaje i brzine kvadrova. Funkcija se zove u init() i pri resetiranju simulacije u keyboard().
	a->pos = Tocka( 3.0f, 0.0f, 0.0f );
	b->pos = Tocka(-3.0f, 0.0f, 0.0f );

	a->v = -INIT_V * 1.0f;	b->v = 0.0f;		// Varijabla v cuva brzinu, a m cuva masu.
	a->m = 10.0f;	b->m = 5.0f;
}

void init (void) {	// Ovo je init funkcija koja priprema program za pravilno izvodenje. Funkcija se ove samo na jednom, na pocetku programa - iz main()-a.
    glEnable (GL_DEPTH_TEST);

	a = new Kvadar( Tocka( 3.0f, 0.0f, 0.0f ), 3.0f );		// Tu se izraduju 2 objekta kvadra i pridodaju im se zeljene vrijednosti.
	b = new Kvadar( Tocka(-3.0f, 0.0f, 0.0f ), 3.0f, true, 21);	// True parametar znaci da kvadar ima oprugu, a 21 je broj cvorova koji cine oprugu.
									// Taj broj sluzi samo za promjenu izgleda opruge, slobodno mijenjaj.
	zadajVrijednostiKvadra();			// Zadaj polozaj, brzine i mase kvadra.
}

void display (void) {
    glClearColor (0.7,0.7,0.7,1.0);			//	Postavlja pozadinsku boju.
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//	Ispunjuje prozor odredenom pozadinskom bojom.
    glMatrixMode ( GL_MODELVIEW );
    glLoadIdentity();
    
    gluLookAt ( 5.0,10.0,10.0, // polozaj kamere
                0.0,0.0,-1.0, //  smjer kamere
                0.0f,1.0f,0.0f ); // postavlja +y os kao "tjeme" kamere

	glColor3f(1.0f, 0.7f, 0.3f);			// odreduje kojom bojom ce se crtati svi nadolazeci oblici
	glBegin(GL_QUADS);				// odreduje da cemo crtati kvadar (i ocekuje da mu damo 4 tocke)
		glVertex3f(-2.5f, -2.5f, 2.5f);		// damo mu 4 tocke
		glVertex3f(2.5f, -2.5f, 2.5f);
		glVertex3f(2.5f, -2.5f, -2.5f);
		glVertex3f(-2.5f, -2.5f, -2.5f);
	glEnd();

	if( !isPaused ){				// ako je isPaused true zadrzavamo kvadre na mjestu -- isPaused se mijenja u funkciji keyboard
		a->Move();		// Move() je metoda klase Kvadar koja odreduje novi polozaj kvadra, ovisno o brzini gibanja kvadra
		b->Move();
	}

	if( CheckCollision(*a, *b) )	// funkcija CheckCollision prima kao parametre 2 kvadra (sto nije nuzno jer su kvadri globalne varijable)
	{				// i racuna da li se kvadri dodiruju. Ako da, vratiti ce true
		if( b->opruga->length_final < 0.0f )	// Opruga pamti svoju "finalnu" duzinu; duzinu opruge pri maksimalnoj kompresiji da bi je program znao pritiskati
							// A ta duzina (length_final) se racuna prilikom prvog dodira opruge i kvadra koji na nju nalijece.
							// Prije nego se dodirnu, vrijednost te varijable je, po defaultu, -1.0f.
							// Ovaj izraz u if() provjerava da li se izračunala vrijednost length_final. Ako nije se izvrsi nadolazeci izraz.
			brzineNakonSudara = maksimalnaKompresija(*a, *b);	// Izracunaj length_final, koji se odmah spremi u oprugu, a usput izracunaj i koje ce biti brzine dva
								// kvadra nakon njihovog sudara te ih spremi u vektor temp. Temp cuva dvije float vrijednosti.

		if( CompressSpring( *b->opruga, a->pos.x ) )	// Ovaj izraz smanjuje oprugu sve dok se ne postigne njena "finalna" duzina. Kad se postigne ce vratiti true.
		{
			a->v = brzineNakonSudara[0]; b->v = brzineNakonSudara[1];		// Kad se opruga maksimalno smanji znaci da se njena energija prenijela na drugi kvadar pa ce se
		}						// izvrsiti promjena brzina. Temp.x je prva float vrijednost, a predstavlja brzinu prvog kvadra, a temp.y drugog.
	}

	glColor3f(0.0f, 0.4f, 0.7f);		// Odreduje boju kvadra
		crtajKvadar( *a );		// Crta kvadar koji mu se salje
		crtajKvadar( *b );		// i ovaj isto

	glColor3f(1.0f, 0.0f, 0.0f);		// Za oprugu se koristi crvena boja
		crtajOprugu( *b->opruga );	// Posebna funkcija za crtanje opruge

    glutSwapBuffers();
}

void reshape (int w, int h) {					// Ovu cijelu funkciju sam prekopirao iz skificevih labosa
    if ( h == 0 )
    {
        h = 1;
    }

    float ratio = 1.0* w / h;

    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity();
    glViewport ( 0, 0, w, h );
    gluPerspective ( 45,ratio,1,1000 );
}

void keyboard (unsigned char key, int x, int y) {
    if (key=='p')	// pauziraj
    {
	isPaused = !isPaused;	// mijenja stanje varijable u suprotno 
    }
    
    if (key=='r')	// resetiraj
    {
	zadajVrijednostiKvadra();	// postavlja pocetne vrijednosti kvadra
    }
}

int main (int argc, char **argv) {
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("v06");

    init ();

    glutDisplayFunc (display);
    glutIdleFunc (display);
    glutKeyboardFunc (keyboard);
    glutReshapeFunc (reshape);
    glutMainLoop ();
    return 0;
}
