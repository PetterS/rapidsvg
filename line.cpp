#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "line.h"

int hex_to_dec(char d1)
{
	if ('0' <= d1 && d1 <= '9') {
		return d1 - '0';
	}
	switch (d1) {
	case 'a': case 'A': return 10;
	case 'b': case 'B': return 11;
	case 'c': case 'C': return 12;
	case 'd': case 'D': return 13;
	case 'e': case 'E': return 14;
	case 'f': case 'F': return 15;
	}
	throw std::runtime_error("Invalid hex digit.");
}


void Line::parse_style_entry(char* style)
{
	using namespace std;
	//std::cerr << "=" << style << '\n';

	char* name = style;
	char* value = style;
	while (style) {
		if (*style == ':') {
			*style = '\0';
			value = style + 1;
			break;
		}
		style++;
	}

	if (strcmp(name, "stroke-width") == 0) {
		this->width = float(atof(value));
	}
	else if (strcmp(name, "stroke") == 0) {
		if (strlen(value) != 7 || value[0] != '#') {
			throw runtime_error("Invalid color.");
		}
		this->r = float(15 * hex_to_dec(value[1]) + hex_to_dec(value[2])) / 255.0f;
		this->g = float(15 * hex_to_dec(value[3]) + hex_to_dec(value[4])) / 255.0f;
		this->b = float(15 * hex_to_dec(value[5]) + hex_to_dec(value[6])) / 255.0f;
		if (this->r < 0.0f || this->r > 1.0f ||
			this->g < 0.0f || this->g > 1.0f ||
			this->b < 0.0f || this->b > 1.0f) {
			throw runtime_error("Invalid color range.");
		}
	}
}

void Line::parse_style(char* style)
{
	char* start = style;
	while (true) {
		if (*style == '\0') {
			if (*start) {
				parse_style_entry(start);
			}
			return;
		}
		else if (*style == ';') {
			*style = '\0';
			parse_style_entry(start);
			start = style + 1;
		}
		style++;
	}
}
