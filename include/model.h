#pragma once

#ifndef	GFX_MODEL_INCLUDE
#define GFX_MODEL_INCLUDE

#include "gfx/header.h"

struct Model
{
	static Model load(const char* filepath);

	//	Number of vertices in the model
	gl::GLsizei vertices;
	//	Coordinates of the bounding box
	float x1, x2, y1, y2;

	//	OpenGL buffers
	gl::GLuint vao, vbo;
};

#endif	//	Include guard
