#ifndef RAPIDSVG_POLYGON_H
#define RAPIDSVG_POLYGON_H

#include <vector>

namespace rapidsvg {

// Represents a line in the SVG file.
class Polygon
{
public:
	Polygon() : r(0), g(0), b(0)
	{ }
	std::vector<std::pair<float, float> > points;
	float r, g, b;

	// Parses a style string and modifies the polygon.
	// Also modifies the string itself.
	void parse_style(char* style);

	// Parses a string of points and adds them
	// to the polygon.
	// Also modifies the string itself.
	void parse_points(char* style);

private:
	void parse_style_entry(char* style);
};

}

#endif