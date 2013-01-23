// Petter Strandmark 2013.

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <omp.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h> // glut.h includes gl.h.
#endif

#include "line.h"
#include "svg_file.h"


namespace rapidsvg {

// SVG file currently opened.
SVGFile svg_file;

// Part of the SVG currently being viewed.
float view_left   = 0.0f;
float view_right  = 1.0f;
float view_bottom = 0.0f;
float view_top    = 1.0f;

void center_display(int view_x, int view_y, float radius_x, float radius_y)
{
	float window_width  = float(glutGet(GLUT_WINDOW_WIDTH));
	float window_height = float(glutGet(GLUT_WINDOW_HEIGHT));

	float x = view_left   + float(view_x) / window_width;
	float y = view_bottom + float(view_x) / window_height;

	view_left   = x - radius_x;
	view_right  = x + radius_x;
	view_bottom = y - radius_y;
	view_top    = y + radius_y;

	glLoadIdentity ();
	glOrtho(view_left, view_right, view_bottom, view_top, 0.0, 1.0);
	glutPostRedisplay();
}

void center_display_zoom(float radius_x, float radius_y)
{
	float x = (view_right + view_left) / 2.0f;
	float y = (view_top + view_bottom) / 2.0f;

	view_left   = x - radius_x;
	view_right  = x + radius_x;
	view_bottom = y - radius_y;
	view_top    = y + radius_y;

	glLoadIdentity ();
	glOrtho(view_left, view_right, view_bottom, view_top, 0.0, 1.0);
	glutPostRedisplay();
}

void center_display_delta(int delta_view_x, int delta_view_y)
{
	float window_width  = float(glutGet(GLUT_WINDOW_WIDTH));
	float window_height = float(glutGet(GLUT_WINDOW_HEIGHT));

	float dx = (view_right - view_left) * float(delta_view_x) / window_width;
	float dy = (view_top - view_bottom) * float(delta_view_y) / window_height;

	view_left   += dx;
	view_right  += dx;
	view_bottom += dy;
	view_top    += dy;

	glLoadIdentity ();
	glOrtho(view_left, view_right, view_bottom, view_top, 0.0, 1.0);
	glutPostRedisplay();
}

bool left_button_down = false;
int left_drag_start_x = 0;
int left_drag_start_y = 0;

void mouse_move(int x, int y)
{
	if (left_button_down) {
		center_display_delta(left_drag_start_x - x, y - left_drag_start_y);
		left_drag_start_x = x;
		left_drag_start_y = y;
	}
}

void mouse(int button, int action, int x, int y)
{
	//std::cerr << "button=" << button << " action=" << action << " x=" << x << " y=" << y << '\n';

	if (button == 0 && action == 0) {
		left_button_down = true;
		left_drag_start_x = x;
		left_drag_start_y = y;
	}
	if (button == 0 && action == 1) {
		left_button_down = false;
	}

	// Mouse wheel up and down.
	if ((button == 3 || button == 4) && action == 0) {
		float radius_x = (view_right - view_left) / 2.0f;
		float radius_y = (view_top - view_bottom) / 2.0f;

		const float fac = 0.8f;
		if (button == 3) {
			radius_x *= fac;
			radius_y *= fac;
		}
		else if (button == 4) {
			radius_x /= fac;
			radius_y /= fac;
		}
		center_display_zoom(radius_x, radius_y);
	}
}

void keyboard (unsigned char key, int x, int y)
{
	//std::cerr << "key=" << int(key) << " x=" << x << " y=" << y << '\n';
	
	if (key == 'r') {
		svg_file.reload();
		glutPostRedisplay();
	}
}

// Draws a line between (x1,y1) - (x2,y2) with a start thickness of t1 and
// end thickness t2.
void draw_line(float x1, float y1, float x2, float y2, float t1, float t2)
{
    float angle = std::atan2(y2 - y1, x2 - x1);
    float t2sina1 = t1 / 2 * std::sin(angle);
    float t2cosa1 = t1 / 2 * std::cos(angle);
    float t2sina2 = t2 / 2 * std::sin(angle);
    float t2cosa2 = t2 / 2 * std::cos(angle);

    glBegin(GL_TRIANGLES);
    glVertex2f(x1 + t2sina1, y1 - t2cosa1);
    glVertex2f(x2 + t2sina2, y2 - t2cosa2);
    glVertex2f(x2 - t2sina2, y2 + t2cosa2);
    glVertex2f(x2 - t2sina2, y2 + t2cosa2);
    glVertex2f(x1 - t2sina1, y1 + t2cosa1);
    glVertex2f(x1 + t2sina1, y1 - t2cosa1);
    glEnd();
}

void display(void)
{
	using namespace std;

	static bool first_time = true;
	double start_time, end_time;
	start_time = ::omp_get_wtime();

	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_POLYGON_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	for (auto& polygon : svg_file.polygons) {
		glBegin(GL_POLYGON);
		glColor3d(polygon.r, polygon.g, polygon.b);
		for (auto& point : polygon.points) {
			glVertex2d(point.first, point.second);
		}
		glEnd();
	}

	for (auto& line : svg_file.lines) {
		glColor3d(line.r, line.g, line.b);
		draw_line(line.x1, line.y1, line.x2, line.y2, line.width, line.width);
	}

	glDisable(GL_BLEND); //restore blending options

	end_time = ::omp_get_wtime();
	if (first_time) {
		std::cerr << "Rendered in " << end_time - start_time << " seconds.\n";
		first_time = false;
	}
	
	glFlush();
	glutSwapBuffers();
}
void split(const std::string &s, char delim, std::vector<std::string>* elems) {
	elems->clear();
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems->push_back(item);
    }
}

void main_function(int argc, char** argv)
{
	using namespace std;

	if (argc <= 1) {
		svg_file.load("example.svg");
	}
	else {
		svg_file.load(argv[1]);
	}

	// From the beginning, look at the entire SVG.
	view_left   = 0;
	view_right  = svg_file.get_width();
	view_bottom = svg_file.get_height();
	view_top    = 0;

	// Start OpenGL.
	glutInit(&argc,argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA );
	glutInitWindowSize(500,500);
	glutCreateWindow("RapidSVG");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_move);
	glLoadIdentity ();
	glOrtho(view_left, view_right, view_bottom, view_top, 0.0, 1.0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glutMainLoop();
}

}

int main(int argc, char** argv)
{
	if (argc == 1) {
		std::cerr << "Usage: " << argv[0] << " <filename>\n";
		return 0;
	}
	try {
		rapidsvg::main_function(argc, argv);
	}
	catch (std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		std::cerr << "Press enter.";
		std::cin.get();
		return 1;
	}
}
