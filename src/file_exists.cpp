#include <stdio.h>
#include <string>
#include <inttypes.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "file_exists.h"


bool file_exists(std::string name)
{
	FILE* file = fopen(name.c_str(), "r");

	if (file) {
		fclose(file);
		return true;
	}
	else
		return false;
}

