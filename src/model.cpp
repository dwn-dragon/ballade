#include "model.h"

#include <limits>
#include "stl_reader.h"

//	TODO: Mesh optimization
//	The Z-axis is removed so a lot of triangles end up overlapping

constexpr size_t VERTICES_PER_TRIANGLE = 3;
constexpr size_t DIMENSIONS_PER_VERTEX = 2;

constexpr gl::GLsizeiptr VERTEX_COORDINATES_SIZE = sizeof(float) * DIMENSIONS_PER_VERTEX;

Model Model::load(const char* filepath) {
	Model model;

	//	loads the data
	stl_reader::StlMesh<float,uint32_t> mesh{ filepath };
	
	//	generates VAO and binds
	gl::glGenVertexArrays(1, &model.vao);
	gl::glBindVertexArray(model.vao);
	//	generates VBO and binds
	gl::glGenBuffers(1, &model.vbo);
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, model.vbo);

	//	pre-sets the vertex attributes, indipendent from the model
	gl::glVertexAttribPointer(0, DIMENSIONS_PER_VERTEX, gl::GL_FLOAT, gl::GL_FALSE, DIMENSIONS_PER_VERTEX * sizeof(float), (void*)0);
	gl::glEnableVertexAttribArray(0);

	//	sets number of vertices and pre-allocs vbo memory
	model.vertices = (gl::GLsizei) mesh.num_tris() * VERTICES_PER_TRIANGLE * DIMENSIONS_PER_VERTEX;
	gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(float) * model.vertices, nullptr, gl::GL_STATIC_DRAW);

	//	sets values to smallest to avoid issues with std::max
	model.x2 = model.y2 = std::numeric_limits<float>::min();
	//	sets values to largest to avoid issues with std::min
	model.x1 = model.y1 = std::numeric_limits<float>::max();

	//	loops through every triangle of the mesh and saves it to VBO
	for (size_t triag_ind = 0; triag_ind < mesh.num_tris(); triag_ind++) {
		for (size_t vert_ind = 0; vert_ind < VERTICES_PER_TRIANGLE; vert_ind++) {
			//	vertices coordinates
			auto vert_coords = mesh.tri_corner_coords(triag_ind, vert_ind);
			//	saves to VBO
			gl::GLintptr data_offset = VERTEX_COORDINATES_SIZE * (vert_ind + triag_ind * VERTICES_PER_TRIANGLE);
			gl::glBufferSubData(gl::GL_ARRAY_BUFFER, data_offset, VERTEX_COORDINATES_SIZE, vert_coords);

			//	finds the bounding box for positioning
			model.x1 = std::min(model.x1, vert_coords[0]), model.x2 = std::max(model.x2, vert_coords[0]);
			model.y1 = std::min(model.y1, vert_coords[1]), model.y2 = std::max(model.y2, vert_coords[1]);
		}
	}

	gl::glBindVertexArray(0);
	return model;
}
