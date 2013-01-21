// Petter Strandmark 2013.

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
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

#include <rapidxml.hpp>

#include "line.h"

// Reads a binary file into a vector of char.
void read_file_data(const std::string& file_name, std::vector<char>* data)
{
	std::fstream fin(file_name, std::ios::binary | std::ios::in );
	fin.seekg(0, std::ios::end);
	size_t file_size = fin.tellg();
	fin.seekg(0, std::ios::beg);
	data->resize(file_size + 1);
	if (!fin.read(&data->at(0), file_size)) {
		throw std::runtime_error("Failed to read file.");
	}
}

// Part of the SVG currently being viewed.
float view_left   = 0.0f;
float view_right  = 1.0f;
float view_bottom = 0.0f;
float view_top    = 1.0f;

// Lines which should be drawn.
std::vector<Line> lines;

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
	std::cerr << "key=" << int(key) << " x=" << x << " y=" << y << '\n';
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

	for (auto& line : lines) {
		glColor3d(line.r, line.g, line.b);
		draw_line(line.x1, line.y1, line.x2, line.y2, line.width, line.width);
	}

	glDisable(GL_BLEND); //restore blending options

	end_time = ::omp_get_wtime();
	if (first_time) {
		std::cerr << "Rendered in " << end_time - start_time << " seconds.\n";
		first_time = false;
	}
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
	using namespace rapidxml;
	double start_time, end_time;

	start_time = ::omp_get_wtime();
	std::vector<char> data;
	if (argc <= 1) {
		read_file_data("example.svg", &data);
	}
	else {
		read_file_data(argv[1], &data);
	}
	end_time = ::omp_get_wtime();
	std::cerr << "Read file in " << end_time - start_time << " seconds.\n";

	start_time = ::omp_get_wtime();
	xml_document<> doc;
	data.push_back(0);
	doc.parse<0>(&data[0]);
	end_time = ::omp_get_wtime();
	std::cerr << "Parsed XML in " << end_time - start_time << " seconds.\n";

	start_time = ::omp_get_wtime();

	xml_node<>* svg = doc.first_node("svg");
	if (!svg) {
		throw std::runtime_error("No <svg> node.");
	}

	float svg_width = 1;
	float svg_height = 1;

	for (auto attr = svg->first_attribute(); attr;
	          attr = attr->next_attribute())
	{
		if (strcmp(attr->name(), "width") == 0) {
			svg_width = float(atof(attr->value()));
		}
		else if (strcmp(attr->name(), "height") == 0) {
			svg_height = float(atof(attr->value()));
		}
	}

	// Queue of nodes we need to explore.
	queue<xml_node<>*> nodes;
	nodes.push(svg);

	// Process the queue.
	while ( !nodes.empty()) {
		auto node = nodes.front();
		nodes.pop();

		// For each child of this node.
		for (auto child = node->first_node(); child;
		          child = child->next_sibling()) {
			if (strcmp(child->name(), "g") == 0) {
				// Found a group; add it to queue.
				nodes.push(child);
			}
			else if (strcmp(child->name(), "line") == 0) {
				// Add line to the collection of lines.
				lines.push_back(Line());
				Line& line = lines.back();

				// To through the line attributes.
				for (xml_attribute<> *attr = child->first_attribute();
						attr; attr = attr->next_attribute())
				{
					if (strcmp(attr->name(), "x1") == 0) {
						line.x1 = float(atof(attr->value()));
					}
					else if (strcmp(attr->name(), "x2") == 0) {
						line.x2 = float(atof(attr->value()));
					}
					else if (strcmp(attr->name(), "y1") == 0) {
						line.y1 = float(atof(attr->value()));
					}
					else if (strcmp(attr->name(), "y2") == 0) {
						line.y2 = float(atof(attr->value()));
					}
					else if (strcmp(attr->name(), "style") == 0) {
						// Process this style string.
						line.parse_style(attr->value());
					}
				}
			}
		}
	}
	end_time = ::omp_get_wtime();
	std::cerr << "Walked XML in " << end_time - start_time << " seconds.\n";

	std::cerr << "Found " << lines.size() << " lines.\n";

	// From the beginning, look at the entire SVG.
	view_left   = 0;
	view_right  = svg_width;
	view_bottom = svg_height;
	view_top    = 0;

	// Start OpenGL.
	glutInit(&argc,argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGBA );
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

int main(int argc, char** argv)
{
	try {
		main_function(argc, argv);
	}
	catch (std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}
}
