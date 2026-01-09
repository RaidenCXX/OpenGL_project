
#ifndef UTILITIES_H
#define UTILITIES_H

#include "glew/glew.h"
#include <string>

GLuint loadCubemap(const std::string &cubmapName);
GLuint createCubMapVAO();

#endif // !UTILITIES_H
