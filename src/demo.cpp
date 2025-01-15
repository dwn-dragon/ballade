#include "gfx/header.h"
#include "gfx/shaders.h"

#include "model.h"
#include "demo.h"

#include <iostream>
#include <chrono>
#include <thread>

constexpr glm::vec4 PLANE_COLOR = {
	000.0f / 255,
	000.0f / 255,
	000.0f / 255,
	1.0f,
};

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

//	1x1 rectangle centered in (0;0)
constexpr float RECT_VERTICES[] = {
	// first triangle
     0.5f,  0.5f,  // top right
     0.5f, -0.5f,  // bottom right
    -0.5f,  0.5f,  // top left 
    // second triangle
     0.5f, -0.5f,  // bottom right
    -0.5f, -0.5f,  // bottom left
    -0.5f,  0.5f,   // top left
};

constexpr float
	SCREEN_RES_W = 2560,
	SCREEN_RES_H = 1600;
constexpr float
	SCREEN_DIM_W = 192.000,
	SCREEN_DIM_H = 120.000;

constexpr size_t 
	WIN_WIDTH = 1024, 
	WIN_HEIGHT = 640;
constexpr float 
	SCREEN_WIN_RATIO_W = 0.9f,
	SCREEN_WIN_RATIO_H = 0.9f;

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
	auto viewMat = glm::mat4{ 1 };
	gl::glUniformMatrix4fv(uniViewLoc, 1, gl::GL_FALSE, glm::value_ptr(viewMat));

	//	models
	//	rect
	gl::GLuint rect_vao, rect_vbo;
	gl::glGenVertexArrays(1, &rect_vao);
	gl::glBindVertexArray(rect_vao);

	gl::glGenBuffers(1, &rect_vbo);
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, rect_vbo);
	gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(RECT_VERTICES), RECT_VERTICES, gl::GL_STATIC_DRAW);

	gl::glVertexAttribPointer(0, 2, gl::GL_FLOAT, gl::GL_FALSE, 2 * sizeof(float), (void*)0);
	gl::glEnableVertexAttribArray(0);

	//	stls
	auto cutout = Model::load(BOARD_CUTOUT);
	auto copper_f = Model::load(BOARD_COPPER_F);
	
	//	maps the screen dimension to mm using the scaled printer's plate
	float win_res_w = WIN_WIDTH, win_res_h = WIN_HEIGHT;
	float scr_res_w = SCREEN_WIN_RATIO_W * WIN_WIDTH, scr_res_h = SCREEN_WIN_RATIO_H * WIN_HEIGHT;
	float scr_dim_w = SCREEN_DIM_W, scr_dim_h = SCREEN_DIM_H;

	float win_dim_w = scr_dim_w*win_res_w/scr_res_w, win_dim_h = scr_dim_h*win_res_h/scr_res_h;
	float diff_w = (win_dim_w-scr_dim_w) / 2, diff_h = (win_dim_h-scr_dim_h) / 2;
	
	float wn = 0-diff_w, wp = scr_dim_w+diff_w;
	float hn = 0-diff_h, hp = scr_dim_h+diff_h;

	auto projectionMat = glm::ortho(wn,wp,hn,hp);
	gl::glUniformMatrix4fv(uniProjectionLoc, 1, gl::GL_FALSE, glm::value_ptr(projectionMat));

	//	main loop
	while (!glfwWindowShouldClose(window)) {
		std::this_thread::sleep_for( std::chrono::milliseconds(10) );
		glfwPollEvents();
		
		//	model mat
		auto modelMat = glm::mat4{ 1 };
		//	clears color buffer
		gl::glClear(gl::GL_COLOR_BUFFER_BIT);
		//	renders the base plane
		modelMat = glm::mat4{ 1 };
		modelMat = glm::scale(modelMat, { scr_dim_w, scr_dim_h, 0 });	//	scales the rectangle to the plate's dimensions in mm
		modelMat = glm::translate(modelMat, { 0.5, 0.5, 0 });			//	centers the rectangle to 0.5 0.5
		gl::glUniformMatrix4fv(uniModelLoc, 1, gl::GL_FALSE, glm::value_ptr(modelMat));

		gl::glUniform4fv(uniColorLoc, 1, glm::value_ptr(PLANE_COLOR));

		gl::glBindVertexArray(rect_vao);
		gl::glDrawArrays(gl::GL_TRIANGLES, 0, 6);

		//	board cutout
		modelMat = glm::mat4{ 1 };
		modelMat = glm::translate(modelMat, { -cutout.x1, -cutout.y1, 0 });			//	centers the rectangle to 0.5 0.5
		gl::glUniformMatrix4fv(uniModelLoc, 1, gl::GL_FALSE, glm::value_ptr(modelMat));
		gl::glUniform4fv(uniColorLoc, 1, glm::value_ptr(CUTOUT_COLOR));
		gl::glBindVertexArray(cutout.vao);
		gl::glDrawArrays(gl::GL_TRIANGLES, 0, cutout.vc);
		//	board copper front
		//	doesn't reset the model mat as the layers are aligned
		gl::glUniform4fv(uniColorLoc, 1, glm::value_ptr(COPPER_F_COLOR));
		gl::glBindVertexArray(copper_f.vao);
		gl::glDrawArrays(gl::GL_TRIANGLES, 0, copper_f.vc);

		//	shows the new rendered scene
		glfwSwapBuffers(window);
	}
	
	return 0;
}
