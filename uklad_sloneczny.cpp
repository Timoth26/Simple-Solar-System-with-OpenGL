#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <cmath>
#include <iostream>

typedef float point3[3];
static GLfloat viewer[] = { 0.0, 0.0, 0.0 };
GLfloat theta = 0.0;
GLfloat phi = 0.0;
GLfloat R = 100.0;
static GLfloat pix2angle;
static GLint status = 0;
static int x_pos_old = 0;
static int y_pos_old = 0;
static int z_pos_old = 0;
float location = 1.0;

#define PI_val 3.141592

using namespace std;

static int delta_x = 0;
static int delta_y = 0;
static int delta_z = 0;

int elipse = 1;
int planet_choose = 0;

//Ustawienie dlugosci osi elipsy dla kazdej z planet
float planet_position[9][2] = {
	//slonce
	{0,0},
	//merkury
	{7.0 + 3.0, 7.0 + 4.6},
	//wenus
	{7.0 + 7.2, 7.0 + 7.3},
	//ziemia
	{7.0 + 9.8, 7.0 + 10.1},
	//mars
	{7.0 + 13.8, 7.0 + 16.8},
	//jowisz
	{7.0 + 50.0, 7.0 + 54.0},
	//saturn
	{7.0 + 92.0, 7.0 + 99.5},
	//uran
	{7.0 + 186.0 , 7.0 + 197.0 },
	//neptun
	{7.0 + 298.0 , 7.0 + 302.0 }
};

//pusta tablica ktora bedzie zawierac aktualna wartosc cos dla osi x i sin dla osi z
float planet_move[9][2] = {
	//slonce
	{0.0, 0.0},
	//merkury
	{0.0, 0.0},
	//wenus
	{0.0, 0.0},
	//ziemia
	{0.0, 0.0},
	//mars
	{0.0, 0.0},
	//jowisz
	{0.0, 0.0},
	//saturn
	{0.0, 0.0},
	//uran
	{0.0, 0.0},
	//neptun
	{0.0, 0.0}
};
//tablica ktora bedzie zawierac aktualne katy uzywane w wyliczeniu planet_move
float speed_angle[9] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 };
//predkosc oborotu planet wokol slonca
float planet_speed[9] = { 0.0, 0.0047, 0.0035, 0.0029, 0.0024, 0.0013, 0.0010, 0.0007, 0.0005 };
//promien planet
float radius[9] = { 696.0 / 100, 2.5 / 100 * 2, 6.0 / 100 * 2, 6.3 / 100 * 2, 3.3 / 100 * 2, 70.0 / 100, 60.0 / 100 , 25.0 / 100 , 24.0 / 100 };
//przechylenie osi planet wzgledem osi y
float axis_angle[9] = { 0.00, 0.0, 2.0, 23.0, 25.0, 3.0, 26.0, 82.0, 28.0 };
//pusta tablica kotra bedzie zawierac aktualne polozenie planety w obrocie wokol wlasnej osi
float rotation_speed[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
//predkosc oborotu planety wokol wlasnej osi
float rotation_speed_changer[9] = { 6793.0 / 1000, 11.0 / 1000, 6.52 / 1000, 1670.0 / 1000, 868.0 / 1000, 45000.0 / 1000, 35000.0 / 1000, 9000.0 / 1000, 8000.0 / 1000 };

//ustawienie kata oborotu dla ksiezyca
float moon_speed_angle = 1.0;
//ustawienie wartosci poczatkowych dla osi x i z ksiezyca (polozenia)
float moon_move_x = 0.0;
float moon_move_z = 0.0;
//ustawienie poczatkowych opoznieni dla obrotow
float delay_speed = 1;
float delay_rotation = 1;
//ustawienie trybu kamery
int camera_type = 1;


/*************************************************************************************/
 // Funkcja wczytuje dane obrazu zapisanego w formacie TGA w pliku o nazwie
 // FileName, alokuje pamięć i zwraca wskaźnik (pBits) do bufora w którym
 // umieszczone są dane.
 // Ponadto udostępnia szerokość (ImWidth), wysokość (ImHeight) obrazu
 // tekstury oraz dane opisujące format obrazu według specyfikacji OpenGL
 // (ImComponents) i (ImFormat).
 // Jest to bardzo uproszczona wersja funkcji wczytującej dane z pliku TGA.
 // Działa tylko dla obrazów wykorzystujących 8, 24, or 32 bitowy kolor.
 // Nie obsługuje plików w formacie TGA kodowanych z kompresją RLE.
/*************************************************************************************/


GLbyte* LoadTGAImage(const char* FileName, GLint* ImWidth, GLint* ImHeight, GLint* ImComponents, GLenum* ImFormat)
{

	/*************************************************************************************/

	// Struktura dla nagłówka pliku  TGA


#pragma pack(1)           
	typedef struct
	{
		GLbyte    idlength;
		GLbyte    colormaptype;
		GLbyte    datatypecode;
		unsigned short    colormapstart;
		unsigned short    colormaplength;
		unsigned char     colormapdepth;
		unsigned short    x_orgin;
		unsigned short    y_orgin;
		unsigned short    width;
		unsigned short    height;
		GLbyte    bitsperpixel;
		GLbyte    descriptor;
	}TGAHEADER;
#pragma pack(8)

	FILE* pFile;
	TGAHEADER tgaHeader;
	unsigned long lImageSize;
	short sDepth;
	GLbyte* pbitsperpixel = NULL;


	/*************************************************************************************/

	// Wartości domyślne zwracane w przypadku błędu

	*ImWidth = 0;
	*ImHeight = 0;
	*ImFormat = GL_BGR_EXT;
	*ImComponents = GL_RGB8;

	fopen_s(&pFile, FileName, "rb");
	if (pFile == NULL)
		return NULL;

	/*************************************************************************************/
	// Przeczytanie nagłówka pliku 


	fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);


	/*************************************************************************************/

	// Odczytanie szerokości, wysokości i głębi obrazu

	*ImWidth = tgaHeader.width;
	*ImHeight = tgaHeader.height;
	sDepth = tgaHeader.bitsperpixel / 8;


	/*************************************************************************************/
	// Sprawdzenie, czy głębia spełnia założone warunki (8, 24, lub 32 bity)

	if (tgaHeader.bitsperpixel != 8 && tgaHeader.bitsperpixel != 24 && tgaHeader.bitsperpixel != 32)
		return NULL;

	/*************************************************************************************/

	// Obliczenie rozmiaru bufora w pamięci


	lImageSize = tgaHeader.width * tgaHeader.height * sDepth;


	/*************************************************************************************/

	// Alokacja pamięci dla danych obrazu


	pbitsperpixel = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));

	if (pbitsperpixel == NULL)
		return NULL;

	if (fread(pbitsperpixel, lImageSize, 1, pFile) != 1)
	{
		free(pbitsperpixel);
		return NULL;
	}


	/*************************************************************************************/

	// Ustawienie formatu OpenGL


	switch (sDepth)

	{

	case 3:

		*ImFormat = GL_BGR_EXT;

		*ImComponents = GL_RGB8;

		break;

	case 4:

		*ImFormat = GL_BGRA_EXT;

		*ImComponents = GL_RGBA8;

		break;

	case 1:

		*ImFormat = GL_LUMINANCE;

		*ImComponents = GL_LUMINANCE8;

		break;

	};



	fclose(pFile);



	return pbitsperpixel;

}

float delay_calc(float delay) {

	if (delay < 0.0) {
		delay *= -1;
		delay = fabs(delay) / 10;
		if (delay > 1.0) {
			delay = 0.9999;
		}
	}


	return delay;

}

void Calculate_Planet_Position() {

	for (int i = 0; i < 9; i++) {

		planet_move[i][0] = cos(speed_angle[i]);
		planet_move[i][1] = sin(speed_angle[i]);
		speed_angle[i] += planet_speed[i] * (cos(speed_angle[i])/1.5+1) * delay_calc(delay_speed);
		if (i == 2 || i == 0) {
			rotation_speed[i] += rotation_speed_changer[i] * delay_calc(delay_rotation);
		}
		else
		{
			rotation_speed[i] -= rotation_speed_changer[i] * delay_calc(delay_rotation);
		}
		if (rotation_speed[i] > 360.0) {
			rotation_speed[i] = 0.0;
		}
		if (rotation_speed[i] < 0.0) {
			rotation_speed[i] = 359.0;
		}
	}

	moon_move_x = planet_move[3][0] * (planet_position[3][0] + planet_position[3][1])
					- planet_position[3][1] + radius[3] * 3 * cos(moon_speed_angle) - radius[3];
	moon_move_z = planet_position[3][1] * planet_move[3][1] + radius[3]*3 * sin(moon_speed_angle)/1.5;
	moon_speed_angle += (0.1 * cos(moon_speed_angle)+1)/10 * delay_calc(delay_speed);

	if (moon_speed_angle > 360)
		moon_speed_angle = 0.1;

	glutPostRedisplay(); //odświeżenie zawartości aktualnego okna
}


void Planet(int id, const char* texturePath) {

	if (id == 0) {
		GLfloat sun_light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat sun_light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat sun_light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat sun_light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightfv(GL_LIGHT0, GL_POSITION, sun_light_position);
		glLightfv(GL_LIGHT0, GL_AMBIENT, sun_light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, sun_light_specular);


		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);

		GLfloat sun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLfloat sun_mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		GLfloat sun_mat_specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat sun_mat_emission[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat sun_mat_shininess = 0.0f;

		glMaterialfv(GL_FRONT, GL_AMBIENT, sun_mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, sun_mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, sun_mat_specular);
		glMaterialfv(GL_FRONT, GL_EMISSION, sun_mat_emission);
		glMaterialf(GL_FRONT, GL_SHININESS, sun_mat_shininess);
	}

	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;
	pBytes = LoadTGAImage(texturePath, &ImWidth, &ImHeight, &ImComponents, &ImFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);
	glEnable(GL_TEXTURE_2D);

	GLUquadricObj* pObj = gluNewQuadric();

	gluQuadricDrawStyle(pObj, GLU_TRUE);
	gluQuadricTexture(pObj, GL_TRUE);

	glPushMatrix();

	glTranslatef(planet_move[id][0] * (planet_position[id][0]+planet_position[id][1]) - planet_position[id][1], 0, planet_move[id][1] * planet_position[id][1]);

	glRotatef(90 + axis_angle[id], 1, 0, 0);
	glRotatef(rotation_speed[id], 0, 0, 1 - axis_angle[id]);

	gluSphere(pObj, radius[id], 100, 100);

	gluDeleteQuadric(pObj);
	glPopMatrix();

	if (elipse) {
		glBegin(GL_LINE_STRIP);
		for (float i = 0.1; i < 360.0; i += 0.1) {
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(cos(i) * (planet_position[id][0] + planet_position[id][1]) - planet_position[id][1], 0, planet_position[id][1] * sin(i));

		}
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);
}

void Moon(const char* texturePath) {
	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;
	pBytes = LoadTGAImage(texturePath, &ImWidth, &ImHeight, &ImComponents, &ImFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);
	glEnable(GL_TEXTURE_2D);

	GLUquadricObj* pObj = gluNewQuadric();

	gluQuadricDrawStyle(pObj, GLU_TRUE);
	gluQuadricTexture(pObj, GL_TRUE);

	glPushMatrix();

	glTranslatef(moon_move_x, 0, moon_move_z);
	glRotatef(90, 1, 0, 0);
	glRotatef(rotation_speed[3]/2, 0, 0, 1);

	gluSphere(pObj, 0.040, 100, 100);
	gluDeleteQuadric(pObj);
	glPopMatrix();

	if (elipse) {
		glBegin(GL_LINE_STRIP);
		for (float i = 0.1; i < 360.0; i += 0.1) {
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(planet_move[3][0] * (planet_position[3][0] + planet_position[3][1]) - planet_position[3][1] + radius[3] * 3 * cos(i) - radius[3], 0 ,planet_position[3][1] * planet_move[3][1] + radius[3]*3 * sin(i)/1.5 );

		}
		glEnd();
	}


	glDisable(GL_TEXTURE_2D);
}

void Sky() {
	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;
	pBytes = LoadTGAImage("textures/space3.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);
	glEnable(GL_TEXTURE_2D);

	GLUquadricObj* pObj = gluNewQuadric();

	gluQuadricDrawStyle(pObj, GLU_TRUE);
	gluQuadricTexture(pObj, GL_TRUE);

	glPushMatrix();
	glScalef(-1, -1, -1);
	gluSphere(pObj, 1000.0, 100, 100);
	gluDeleteQuadric(pObj);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

void Mouse(int btn, int state, int x, int y)
{
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)

	{
		x_pos_old = x;
		y_pos_old = y;

		status = 1;

	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		y_pos_old = y;
		status = 2;

	}
	else
		status = 0;
}

void Motion(GLsizei x, GLsizei y)
{
	delta_x = x - x_pos_old;
	delta_y = y - y_pos_old;
	x_pos_old = x;
	y_pos_old = y;
	glutPostRedisplay();
}


void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if (status == 1) {

		theta += delta_x * pix2angle / 30.0;
		phi += delta_y * pix2angle / 30;
		// zabezpieczenie przed Gimbal Lock
		if (phi > 2 * PI_val)
			phi = 0;

		if (phi < 0)
			phi = 2 * PI_val;

		if (phi > PI_val / 2)
			location = -1.0;
		else
			location = 1.0;

		if (phi > PI_val + (PI_val / 2))
			location = 1.0;
	}
	else if (status == 2)
	{
		R += delta_y * pix2angle / 6.0;
	}

	if (camera_type) {

		// edycja polozenia
		viewer[0] = R * cos(theta) * cos(phi);
		viewer[1] = R * sin(phi);
		viewer[2] = R * sin(theta) * cos(phi);

		gluLookAt(planet_move[planet_choose][0] * (planet_position[planet_choose][0] + planet_position[planet_choose][1]) - planet_position[planet_choose][1] + viewer[0], viewer[1], planet_position[planet_choose][1] * planet_move[planet_choose][1] + viewer[2],
			planet_move[planet_choose][0] * (planet_position[planet_choose][0] + planet_position[planet_choose][1]) - planet_position[planet_choose][1], 0.0, planet_position[planet_choose][1] * planet_move[planet_choose][1],
			0.0, location, 0.0);
	}
	else {

		// edycja polozenia
		viewer[0] = R * cos(theta) * cos(phi);
		viewer[1] = R * sin(phi);
		viewer[2] = R * sin(theta) * cos(phi);


		gluLookAt(planet_move[planet_choose][0] * (planet_position[planet_choose][0] + planet_position[planet_choose][1]) - planet_position[planet_choose][1],
			radius[planet_choose] * 2,
			planet_move[planet_choose][1] * planet_position[planet_choose][1],
			planet_move[planet_choose][0] * (planet_position[planet_choose][0] + planet_position[planet_choose][1]) - planet_position[planet_choose][1] + viewer[0],
			-viewer[1],
			planet_position[planet_choose][1] * planet_move[planet_choose][1] + viewer[2],
			0.0, location, 0.0);
	}

	Planet(0, "textures/slonce.tga");
	Planet(1, "textures/merkury.tga");
	Planet(2, "textures/wenus.tga");
	Planet(3, "textures/ziemia.tga");
	Planet(4, "textures/mars.tga");
	Planet(5, "textures/jowisz.tga");
	Planet(6, "textures/saturn.tga");
	Planet(7, "textures/uran.tga");
	Planet(8, "textures/neptun.tga");
	Moon("textures/ksiezyc.tga");
	Sky();
	glFlush();
	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
	if (key == 'q') elipse = 1;
	if (key == 'w') elipse = 0;
	if (key == 'e') delay_speed += 1;
	if (key == 'r') delay_speed -= 1;
	if (key == 't') delay_rotation += 1;
	if (key == 'y') delay_rotation -= 1;
	if (key == '0') planet_choose = 0;
	if (key == '1') planet_choose = 1;
	if (key == '2') planet_choose = 2;
	if (key == '3') planet_choose = 3;
	if (key == '4') planet_choose = 4;
	if (key == '5') planet_choose = 5;
	if (key == '6') planet_choose = 6;
	if (key == '7') planet_choose = 7;
	if (key == '8') planet_choose = 8;
	if (key == 'c') {
		if (camera_type) {
			camera_type = 0;
		}
		else {
			camera_type = 1;
		}
	}

	RenderScene();
}

void MyInit(void)
{
	/*************************************************************************************/
// Funkcja ustalająca stan renderowania
/*************************************************************************************/

		/*************************************************************************************/

		// Zmienne dla obrazu tekstury



	GLbyte* pBytes;
	GLint ImWidth, ImHeight, ImComponents;
	GLenum ImFormat;


	// ..................................       
	//       Pozostała część funkcji MyInit()

	// ..................................

/*************************************************************************************/

// Teksturowanie będzie prowadzone tyko po jednej stronie ściany

	glEnable(GL_CULL_FACE);


	/*************************************************************************************/

	//  Przeczytanie obrazu tekstury z pliku o nazwie tekstura.tga

	pBytes = LoadTGAImage("D1_t.tga", &ImWidth, &ImHeight, &ImComponents, &ImFormat);


	/*************************************************************************************/

   // Zdefiniowanie tekstury 2-D

	glTexImage2D(GL_TEXTURE_2D, 0, ImComponents, ImWidth, ImHeight, 0, ImFormat, GL_UNSIGNED_BYTE, pBytes);

	/*************************************************************************************/

	// Zwolnienie pamięci

	free(pBytes);


	/*************************************************************************************/

	// Włączenie mechanizmu teksturowania

	glEnable(GL_TEXTURE_2D);

	/*************************************************************************************/

	// Ustalenie trybu teksturowania

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/*************************************************************************************/

	// Określenie sposobu nakładania tekstur

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);





}

void ChangeSize(GLsizei horizontal, GLsizei vertical)
{
	pix2angle = 360.0 / (float)horizontal;
	glMatrixMode(GL_PROJECTION);
	// Przełączenie macierzy bieżącej na macierz projekcji

	glLoadIdentity();
	// Czyszcznie macierzy bieżącej

	gluPerspective(70, 1.0, 0.001, 3000.0);
	// Ustawienie parametrów dla rzutu perspektywicznego


	if (horizontal <= vertical)
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

	else
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
	// Ustawienie wielkości okna okna widoku (viewport) w zależności
	// relacji pomiędzy wysokością i szerokością okna

	glMatrixMode(GL_MODELVIEW);
	// Przełączenie macierzy bieżącej na macierz widoku modelu  

	glLoadIdentity();
	// Czyszczenie macierzy bieżącej

}

void menu() {
	cout << "q - wlaczenie widoku orbit\n";
	cout << "w - wylaczenie widoku orbit\n";
	cout << "e - przyspieszenie obrotow planet wokol Slonca\n";
	cout << "r - spowolnienie obrotow planet wokol Slonca\n";
	cout << "t - przyspieszenie obrotow planet wokol wlasnej osi\n";
	cout << "y - spowolnienie obrotow planet wokol wlasnej osi\n";
	cout << "0 - Slonce\n";
	cout << "1 - Merkury\n";
	cout << "2 - Wenus\n";
	cout << "3 - Ziemia\n";
	cout << "4 - Mars\n";
	cout << "5 - Jowisz\n";
	cout << "6 - Saturn\n";
	cout << "7 - Uran\n";
	cout << "8 - Neptun\n";
	cout << "c - zmiana trybu kamery\n";
	cout << "LPM - poruszanie kamera\n";
	cout << "PPM - zoom kamery\n";

}


void main(void)
{
	menu();
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("Solar System");
	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	MyInit();
	glutIdleFunc(Calculate_Planet_Position);
	glEnable(GL_DEPTH_TEST);
	glutKeyboardFunc(keys);
	glutMainLoop();
}