
#include <fstream>
#include <iostream>
#include <queue>
#include <stdexcept>

#include <omp.h>

#include <rapidxml.hpp>

#include "svg_file.h"

// Reads a binary file into a vector of char.
void read_file_data(const std::string& file_name, std::vector<char>* data)
{
	std::fstream fin(file_name, std::ios::binary | std::ios::in );
	if (!fin) {
		throw std::runtime_error("Could not open file.");
	}
	fin.seekg(0, std::ios::end);
	size_t file_size = fin.tellg();
	fin.seekg(0, std::ios::beg);
	data->resize(file_size + 1);
	if (!fin.read(&data->at(0), file_size)) {
		throw std::runtime_error("Failed to read file.");
	}
}

SVGFile::SVGFile() :
	width(0),
	height(0)
{
}

void SVGFile::clear()
{
	this->lines.clear();
}

void SVGFile::reload()
{
	if (this->filename.length() > 0) {
		this->load(this->filename);
	}
	else {
		throw std::runtime_error("No file previously loaded.");
	}
}

void SVGFile::load(const std::string& input_filename)
{
	using namespace std;
	using namespace rapidxml;
	double start_time, end_time;

	this->filename = input_filename;
	this->clear();

	start_time = ::omp_get_wtime();
	std::vector<char> data;
	read_file_data(filename, &data);

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

	this->width = 1;
	this->height = 1;

	for (auto attr = svg->first_attribute(); attr;
	          attr = attr->next_attribute())
	{
		if (strcmp(attr->name(), "width") == 0) {
			this->width = float(atof(attr->value()));
		}
		else if (strcmp(attr->name(), "height") == 0) {
			this->height = float(atof(attr->value()));
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
	
	std::cerr << "SVG is " << this->width << " x " << this->height << "\n";
	std::cerr << "Found " << lines.size() << " lines.\n";
}

