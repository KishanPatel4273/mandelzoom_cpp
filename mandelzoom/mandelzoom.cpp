
/*-----------------------------------------------------------------------*/
/*  mandelzoom.cpp   Draw a picture of the Mandelbrot set.               */
/*-----------------------------------------------------------------------*/

#include <cstdlib>
#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <list>
#include <cfloat>

using namespace std;

// Initial position of upper left corner of window.
#define INITIAL_WIN_X 150
#define INITIAL_WIN_Y 50
void restoreImage();
// Data structure to represent a region in the complex plane.
struct rectangle
{
	double xmin;
	double ymin;
	double xmax;
	double ymax;

	rectangle(double xmn, double ymn, double xmx, double ymx)
		: xmin(xmn), ymin(ymn), xmax(xmx), ymax(ymx)
	{}
};

// Data structure tosave history of regions viewed.
// Statically allocated and initialized at start up.
list<rectangle*>  rectList;
list<rectangle*>::iterator  rectListIter;
int rectListIndex;


// Variables represeting the view region requested by user.
double xmin, xmax, ymin, ymax;

// Data structure for RGB triples. Could have been a struct.
class rgbType
{
public:
	rgbType() : red(0.0), green(0.0), blue(0.0) {}
	rgbType(double r, double g, double b) : red(r), green(g), blue(b) {}
	double red, green, blue;
};

// Keeping track of the screen window dimensions.
int windowHeight, windowWidth;

// The variable "image" holds the current mandelzoom image.
// Will be dynamically allocated. 
GLfloat* image;

// The variable "table" holds the iteration count of
// each complex number in the selected region.
// Will be dynamically allocated. 
int** table;


// Allocate and initialize tables. 
void initTables(int newW, int newH)
{
	// The image is stored as a 1D dynamic array. 
	image = new GLfloat[3 * newW * newH];
	// The iteration count table is a 2D dynamic array. 
	table = new int* [newW];
	for (int i = 0; i < newW; i++) table[i] = new int[newH];
}

// Delete old tables.
void deleteTables()
{
	// Delete entire 1D array at once. 
	delete image;
	// First delete each column of table. 
	for (int i = 0; i < windowWidth; i++) delete table[i];
	// Then delete the table itself.
	delete table;
}

// Flag to control recomputation of pixmap.
bool recompute = true;

// Keeping track of the start and end points of the rubber band.
int xAnchor, yAnchor, xStretch, yStretch;

// Variables for use in rubberbanding.
bool rubberBanding = false, bandOn = false;


int iter() {
	return (int)1000;// *exp(-(xmax - xmin - 4) / 10);
}


//function that computes converge
// 0->converges
// n->value where it diverges
int convergence(double rc, double ic) {
	double rz = 0;
	double iz = 0;

	for (int i = 0; i < iter(); i++) {
		double rzz = rz * rz - iz * iz + rc;
		double izz = 2 * rz * iz + ic;
		rz = rzz;
		iz = izz;

		if (rz * rz + iz * iz > 4) {
			return i + 1;
		}
	}
	return 1001;
}

void fillTable() {
	
	double dx = (xmax - xmin) / (windowWidth - 1);
	double dy = (ymax - ymin) / (windowHeight - 1);
	for (int u = 0; u < windowWidth; u++) {
		for (int v = 0; v < windowHeight; v++) {
			table[u][v] = convergence(xmin + u * dx, ymin + v * dy);
		}
	}
}

void fillImage() {

	float r = 1;
	float g = 0;
	float b = 0;

	float r1 = 0;
	float g1 = 0;
	float b1 = 1;

	for (int u = 0; u < windowWidth; u++) {
		for (int v = 0; v < windowHeight; v++) {
			double t = table[u][v] / 1001;
			//cout << t << "\n";
			if (t == 0) {
				image[u * windowWidth + v + 0] = r + (r1 - r) * t;
				image[u * windowWidth + v + 1] = g + (g1 - g) * t;
				image[u * windowWidth + v + 2] = b + (b1 - b) * t;

			} else {
				image[u * windowWidth + v + 0] = 0;
				image[u * windowWidth + v + 1] = 0;
				image[u * windowWidth + v + 2] = 0;
			}

		}
	}
}


// Call back function that draws the image. 
void drawFractal() {
	
	if (recompute) {
		fillTable();
		recompute = false;
		cout << "Recompute\n";
	}
	

	//fillImage();
	//restoreImage();
	glBegin(GL_POINTS);

	for (int u = 0; u < windowWidth; u++) {
		for (int v = 0; v < windowHeight; v++) {
			double t = table[u][v] / 1000;
			//cout << t << "\n";
			if (t == 0) {
				glColor3f(255, 0, 0);
				glVertex2d(u, v);
			}
			else {
				glColor3f(0, 0, 0);
				glVertex2d(u, v);
			}

		}
	}

	glEnd();




	cout << "Redraw\n";
	glFlush();
	return;
}

void reshape(int w, int h)
// Callback for processing reshape events.
{
	if (w > 0 && h > 0 && (w != windowWidth || h != windowHeight))
	{


		int dw = w - windowWidth;
		double dx = (double)dw * (xmax - xmin) / windowWidth;

		int dh = h - windowHeight;
		double dy = (double)dh * (ymax - ymin) / windowHeight;

		deleteTables();
		initTables(w, h);
		windowWidth = w;
		windowHeight = h;
	
		xmin = xmin - dx / 2;
		xmax = xmax + dx / 2;

		ymin = ymin - dy / 2;
		ymax = ymax + dy / 2;

		recompute = true;
	}
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
}


// Get the current image from OpenGL and save it in our image variable. 
void saveImage()
{
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_FLOAT, image);
}

// Get the current image from our image variable and send it to OpenGL.
void restoreImage()
{
	glRasterPos2i(0, 0);
	glDrawPixels(windowWidth, windowHeight, GL_RGB, GL_FLOAT, image);
}


// Draw a rectangle defined by four integer parameters.
void drawRectangle(int xA, int yA, int xS, int yS)
{
	glBegin(GL_LINE_LOOP);
	glVertex2i(xA, yA);
	glVertex2i(xS, yA);
	glVertex2i(xS, yS);
	glVertex2i(xA, yS);
	glEnd();
}

void drawRubberBand(int xA, int yA, int xS, int yS)
// Draw the rubber band in XOR mode.
{
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_XOR);
	drawRectangle(xA, yA, xS, yS);
	glDisable(GL_COLOR_LOGIC_OP);
	glFlush();
}

int sign(int xo) {
	if (xo < 0) {
		return -1;
	}
	return 1;
}

void rubberBand(int x, int y)
// Callback for processing mouse motion.
{
	if (rubberBanding)
	{
		y = windowHeight - y;
		glColor3f(1.0, 1.0, 1.0);
		// If the band is on the screen, remove it. 
		if (bandOn) drawRubberBand(xAnchor, yAnchor, xStretch, yStretch);
		xStretch = x;
		yStretch = y;
		// Draw the rubber band at its new position.                                            
		drawRubberBand(xAnchor, yAnchor, xStretch, yStretch);
		//drawRubberBand(xAnchor, yAnchor, xStretch, yStretch);
		bandOn = true;
	}
}

void escExit(GLubyte key, int, int)
// Callback for processing keyboard events.
{
	if (key == 27 /* ESC */) std::exit(0);
}

void mouse(int button, int state, int x, int y)
// Routine for processing mouse events.
{
	if (button == GLUT_MIDDLE_BUTTON) return;
	y = windowHeight - y;
	switch (state)
	{
	case GLUT_DOWN:
	{
		if (!rubberBanding)
		{

			xAnchor = x;
			yAnchor = y;
			xStretch = x;
			yStretch = y;

			drawRubberBand(xAnchor, yAnchor, xStretch, yStretch);
			bandOn = true;
			rubberBanding = true;			
			break;
		}
	}
	case GLUT_UP:
	{
		if (rubberBanding)
		{

			// Remove the rubber band currently on the screen.

			drawRubberBand(xAnchor, yAnchor, xStretch, yStretch);
			bandOn = false;
			rubberBanding = false;
			
			cout << "zoom\n";

			//window prams
			double ddx = xStretch - xAnchor;
			double ddy = yStretch - yAnchor;
			double ratio = (double) windowHeight / windowWidth;
			//complex plane prams
			double cdx = xmax-xmin;
			double cdy = ymax-ymin;
	
			if (abs(ddy) / abs(ddx) < ratio) {//increase cy
				xmax = ((double)max(xAnchor, xStretch) / windowWidth) * cdx + xmin;
				xmin = ((double)min(xAnchor, xStretch) / windowWidth) * cdx + xmin;

				double centerCY = ((double)((yAnchor + yStretch)/2) / windowHeight) * cdy + ymin;
				double intCY = (double) (xmax - xmin) * ratio / 2;

				ymin = centerCY - intCY;
				ymax = centerCY + intCY;
			}
			else {
				ymax = ((double)max(yAnchor, yStretch) / windowHeight) * cdy + ymin;
				ymin = ((double)min(yAnchor, yStretch) / windowHeight) * cdy + ymin;

				double centerCX = ((double)(xAnchor + xStretch) / (2 * windowWidth)) * cdx + xmin;
				double intCX = (double) (ymax - ymin) / (2* ratio);

				xmin = centerCX - intCX;
				xmax = centerCX + intCX;
			}
			
			recompute = true; 

			if (rectListIndex + 1 != rectList.size()) {
				auto it = rectList.begin();
				std::advance(it, rectListIndex + 1);
				rectList.erase(it, rectList.end());
			}

			rectList.push_back(new rectangle(xmin, ymin, xmax, ymax));

			rectListIndex += 1;

			glutPostRedisplay();
			break;
		}
	}
	}
}


void mainMenu(int item)
// Callback for processing main menu.
{
	int i = 0;

	switch (item)
	{
	case 1: // Push
		if (rectListIndex + 1 < rectList.size()) {
			for (auto it = rectList.begin(); it != rectList.end(); it++) {
				if (i == rectListIndex + 1) {
					rectangle* rec = *it;
					xmax = (*rec).xmax;
					xmin = rec->xmin;
					ymax = rec->ymax;
					ymin = rec->ymin;
					recompute = true;
					glutPostRedisplay();
				}
				i += 1;
			}
			rectListIndex += 1;
		}
		break;
	case 2: // Pop (go back one in the list)
		if (rectListIndex != 0) {
			for (auto it = rectList.begin(); it != rectList.end() ; it++) {
				if (i == rectListIndex - 1) {
					rectangle* rec = *it;
					xmax = (*rec).xmax;
					xmin = rec->xmin;
					ymax = rec->ymax;
					ymin = rec->ymin;
					recompute = true;
					glutPostRedisplay();

				}
				i += 1;

			}
			rectListIndex -= 1;
		}
		break;
	case 3: std::exit(0);
	}
}

void setMenu()
// Routine for creating menus.
{
	glutCreateMenu(mainMenu);
	glutAddMenuEntry("Push", 1);
	glutAddMenuEntry("Pop", 2);
	glutAddMenuEntry("Exit", 3);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

int main(int argc, char* argv[])
{
	// Process command line parameters and convert
	// them from string to int or float.
	xmin = atof(argv[1]);
	xmax = atof(argv[2]);
	ymin = atof(argv[3]);
	ymax = atof(argv[4]);

	windowWidth = atoi(argv[5]);
	windowHeight = atoi(argv[6]);

	// Initialize the dynamically allocated tables after
	// main function has begun. 
	initTables(windowWidth, windowHeight);

	// Push initial view rectangle onto rectList. Set rectListIter
	// to reference the first and only rectangle on the list. 
	rectList.push_front(new rectangle(xmin, ymin, xmax, ymax));
	rectListIter = rectList.begin();
	rectListIndex = 0;

	// Mask floating point exceptions.
	_control87(MCW_EM, MCW_EM);

	// Initialize glut with command line parameters.
	glutInit(&argc, argv);

	// Choose RGB display mode for normal screen window.
	glutInitDisplayMode(GLUT_RGB);

	// Set initial window size, position, and title.
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(INITIAL_WIN_X, INITIAL_WIN_Y);
	glutCreateWindow("Mandelzoom");

	// You don't (yet) want to know what this does.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, (double)windowWidth, 0.0, (double)windowHeight),
		glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0.0);

	// Set the color for clearing the normal screen window.
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// Set the callbacks for the normal screen window.
	glutDisplayFunc(drawFractal);
	glutMouseFunc(mouse);
	glutMotionFunc(rubberBand);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(escExit);

	setMenu();

	glutMainLoop();

	return 0;

}