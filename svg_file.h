#ifndef RAPIDSVG_SVG_FILE_H
#define RAPIDSVG_SVG_FILE_H

#include <string>
#include <vector>

#include "line.h"

// Represents a line in the SVG file.
class SVGFile
{
public:
	SVGFile();

	// Load a file from file.
	void load(const std::string& filename);
	void reload();
	void clear();

	double get_width() { return width; }
	double get_height() { return height; }

	// Lines in the SVG.
	std::vector<Line> lines;
private:
	std::string filename;
	double width, height;
};



#endif