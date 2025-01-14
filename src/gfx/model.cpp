#include "gfx/model.h"
#include "ext/stl_reader.h"

constexpr size_t VERTICES_PER_TRIANGLE = 3;
constexpr size_t DIMENSIONS_PER_VERTEX = 2;

Model Model::load(const char* filepath) {
	stl_reader::StlMesh<float,uint32_t> mesh{ filepath };

	Model model;
	
	gl::glGenVertexArrays(1, &model.vao);
	gl::glBindVertexArray(model.vao);

	gl::glGenBuffers(1, &model.vbo);
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, model.vbo);

	gl::glVertexAttribPointer(0, DIMENSIONS_PER_VERTEX, gl::GL_FLOAT, gl::GL_FALSE, DIMENSIONS_PER_VERTEX * sizeof(float), (void*)0);
	gl::glEnableVertexAttribArray(0);

	model.vc = (gl::GLsizei) mesh.num_tris() * VERTICES_PER_TRIANGLE * DIMENSIONS_PER_VERTEX;
	gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(float) * model.vc, nullptr, gl::GL_STATIC_DRAW);

	bool first = true;
	for (size_t ti = 0; ti < mesh.num_tris(); ti++) {
		for (size_t ci = 0; ci < VERTICES_PER_TRIANGLE; ci++) {
			auto c = mesh.tri_corner_coords(ti,ci);

			if (first) {
				first = false;
				model.x1 = model.x2 = c[0], model.y1 = model.y2 = c[1];
			}
			else {
				model.x1 = std::min(model.x1, c[0]), model.x2 = std::max(model.x2, c[0]);
				model.y1 = std::min(model.y1, c[1]), model.y2 = std::max(model.y2, c[1]);
			}

			gl::glBufferSubData(
				gl::GL_ARRAY_BUFFER, 
				DIMENSIONS_PER_VERTEX*sizeof(float)*(ti*VERTICES_PER_TRIANGLE + ci), 
				sizeof(float)*DIMENSIONS_PER_VERTEX,
				c
			);
		}
	}

	gl::glBindVertexArray(0);
	return model;
}
