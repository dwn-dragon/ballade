#include "gfx/header.h"
#include "gfx/shaders.h"

#include "model.h"
#include "demo.h"


#include <iostream>
#include <chrono>
#include <thread>

constexpr glm::vec4 CUTOUT_COLOR = { 
	102.0f / 255, 
	079.0f / 255, 
	066.0f / 255, 
	1.0f
};
constexpr glm::vec4 COPPER_F_COLOR = { 
	122.0f / 255, 
	107.0f / 255, 
	000.0f / 255, 
	1.0f
};

constexpr size_t 
	WIN_WIDTH = 640, 
	WIN_HEIGHT = 480;

constexpr size_t VERTICES_PER_TRIANGLE = 3;
constexpr size_t DIMENSIONS_PER_VERTEX = 2;

constexpr char BOARD_CUTOUT[] = "E:\\3D\\STLs\\PCBs\\Imported\\Board.stl";
constexpr char BOARD_COPPER_F[] = "E:\\3D\\STLs\\PCBs\\Imported\\1-copper.stl";

int main (size_t argc, char* argv[]) {
	//	sets exit function
	std::atexit(glfwTerminate);

	//	inits glfw
	if (!glfwInit()) {
		std::cerr << "GLFW Init error" << std::endl;
		exit(1);
	}

	//	inits window
	auto window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Title", nullptr, nullptr);
	if (!window) {
		std::cerr << "Window creation error" << std::endl;
		exit(1);
	}

	//	inits OpenGL
	glfwMakeContextCurrent(window);
	glbinding::initialize(glfwGetProcAddress);

	//	logs data
	std::cout << "OpenGL Version: " << gl::glGetString(gl::GL_VERSION) << std::endl;
	std::cout << "GLFW Version: " << glfwGetVersionString() << std::endl;

	//	setup
	glfwSwapInterval(1);
	gl::glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	//	shaders
	auto shader = load_shader();
	//	uniforms locations
	auto uniModelLoc = gl::glGetUniformLocation(shader, "uniModel");
	auto uniViewLoc = gl::glGetUniformLocation(shader, "uniView");
	auto uniProjectionLoc = gl::glGetUniformLocation(shader, "uniProjection");
	auto uniColorLoc = gl::glGetUniformLocation(shader, "uniColor");
	//	pre sets unused uniforms
	auto modelMat = glm::mat4{ 1 };
	gl::glUniformMatrix4fv(uniModelLoc, 1, gl::GL_FALSE, glm::value_ptr(modelMat));
	auto projectionMat = glm::mat4{ 1 };
	gl::glUniformMatrix4fv(uniProjectionLoc, 1, gl::GL_FALSE, glm::value_ptr(projectionMat));

	//	model
	auto cutout = Model::load(BOARD_CUTOUT);
	auto copper_f = Model::load(BOARD_COPPER_F);
	
	float mx = 1.0f / (cutout.x2-cutout.x1), my = 1.0f / (cutout.y2-cutout.y1);
	float qx = 0.5f - mx*cutout.x2, qy = 0.5f - my*cutout.y2;

	auto viewMat = glm::mat4{ 1 };
	viewMat = glm::translate(viewMat, { qx, qy, 0 });
	viewMat = glm::scale(viewMat, { mx, my, 1 });
	gl::glUniformMatrix4fv(uniViewLoc, 1, gl::GL_FALSE, glm::value_ptr(viewMat));

	//	main loop
	while (!glfwWindowShouldClose(window)) {
		std::this_thread::sleep_for( std::chrono::milliseconds(10) );
		glfwPollEvents();
		
		gl::glClear(gl::GL_COLOR_BUFFER_BIT);

		//	render here
		gl::glBindVertexArray(cutout.vao);
		gl::glUniform4fv(uniColorLoc, 1, glm::value_ptr(CUTOUT_COLOR));
		gl::glDrawArrays(gl::GL_TRIANGLES, 0, cutout.vc);
		
		gl::glBindVertexArray(copper_f.vao);
		gl::glUniform4fv(uniColorLoc, 1, glm::value_ptr(COPPER_F_COLOR));
		gl::glDrawArrays(gl::GL_TRIANGLES, 0, copper_f.vc);

		glfwSwapBuffers(window);
	}
	
	return 0;
}
