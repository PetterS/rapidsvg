#ifndef RAPIDSVG_LINE_H
#define RAPIDSVG_LINE_H

// Represents a line in the SVG file.
class Line
{
public:
	Line() : x1(0), y1(0), x2(0), y2(0),
	         width(1), r(0), g(0), b(0)
	{ }
	float x1, y1, x2, y2;
	float width;
	float r, g, b;

	// Parses a style string and modifies the line.
	// Also modifies the string itself.
	void parse_style(char* style);

private:
	void parse_style_entry(char* style);
};

#endif