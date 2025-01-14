#pragma once

#ifndef	GFX_MODEL_INCLUDE
#define GFX_MODEL_INCLUDE

#include "gfx/header.h"

struct Model
{
	static Model load(const char* filepath);

	gl::GLuint vao, vbo;
	gl::GLsizei vc;

	float x1, x2, y1, y2;
};

#endif	//	Include guard
