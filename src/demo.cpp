#include "gfx/header.h"
#include "gfx/shaders.h"

#include "model.h"
#include "demo.h"

#include <CLI/CLI.hpp>

#include <iostream>
#include <chrono>
#include <thread>

constexpr glm::vec4 COLOR_PLANE = {
	000.0f / 255,
	000.0f / 255,
	000.0f / 255,
	1.0f,
};

constexpr glm::vec4 COLOR_CUTOUT = { 
	102.0f / 255, 
	079.0f / 255, 
	066.0f / 255, 
	1.0f
};
constexpr glm::vec4 COLOR_COPPER_F = { 
	122.0f / 255, 
	107.0f / 255, 
	000.0f / 255, 
	1.0f
};

constexpr glm::vec4 COLOR_OUTPUT_CLEAR = {
	000.0f / 255,
	000.0f / 255,
	000.0f / 255,
	1.0f,
};
constexpr glm::vec4 COLOR_OUTPUT_DRAW = {
	255.0f / 255,
	255.0f / 255,
	255.0f / 255,
	1.0f,
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

constexpr size_t
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

using dtime_t = std::chrono::duration<float, std::milli>;
constexpr dtime_t RENDER_DELAY{ 1.0f / 60.0f };

constexpr size_t VERTICES_PER_TRIANGLE = 3;
constexpr size_t DIMENSIONS_PER_VERTEX = 2;

constexpr char* BOARD_CUTOUT = "F:\\Apps\\KiCad\\Projects\\TestPCB\\STLs\\Board.stl";

constexpr char* BOARD_LAYERS[] = {
	"F:\\Apps\\KiCad\\Projects\\TestPCB\\STLs\\1-copper.stl"
};

constexpr char* CONFIG_FILE = "config.ini";

template< class Ty >
struct array_info_t
{
	static constexpr bool is_array = false;
};
template< class Ty, size_t Len >
struct array_info_t<typename Ty[Len]>
{
	static constexpr bool is_array = true;
	static constexpr size_t length = Len;
};

int main (int argc, char* argv[]) {
	//	generation flag
	bool gen = false;

	//	arguments parser
	CLI::App args;
	//	should generate the files or not
	args.add_flag("-g,--generate", gen, "Generates output pngs");
	//	parses
	CLI11_PARSE(args, argc, argv);

	//	sets exit function
	std::atexit(glfwTerminate);

	//	inits glfw
	if (!glfwInit()) {
		std::cerr << "GLFW Init error" << std::endl;
		exit(1);
	}

	//	inits window
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
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
	auto uniModelLoc 		= gl::glGetUniformLocation(shader, "uniModel");
	auto uniProjectionLoc	= gl::glGetUniformLocation(shader, "uniProjection");
	auto uniColorLoc 		= gl::glGetUniformLocation(shader, "uniColor");

	//	number of models
	size_t len = array_info_t<decltype(BOARD_LAYERS)>::length + 1, cutout = 0;
	auto layers = std::make_unique<Model[]>(len);

	//	loads the board cutout
	layers[cutout] = Model::load(BOARD_CUTOUT);
	//	loads the board layers
	for (size_t i = 1; i < len; i++)
		layers[i] = Model::load(BOARD_LAYERS[i - 1]);

	if (gen) {
		//	generates the pngs
		//	framebuffer to render to
		gl::GLuint fbo;
		gl::glGenFramebuffers(1, &fbo);
		gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, fbo);

		//	texture object to read the data from
		//	the output pngs are grayscales, no need to use more one than color channel
		gl::GLuint texOut;
		gl::glGenTextures(1, &texOut);
		gl::glBindTexture(gl::GL_TEXTURE_2D, texOut);
		gl::glTexImage2D(gl::GL_TEXTURE_2D, 0, gl::GL_RED, SCREEN_RES_W, SCREEN_RES_H, 0, gl::GL_RED, gl::GL_UNSIGNED_BYTE, nullptr);
		gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR );
		gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
		gl::glFramebufferTexture2D(gl::GL_FRAMEBUFFER, gl::GL_COLOR_ATTACHMENT0, gl::GL_TEXTURE_2D, texOut, 0);
		gl::glBindTexture(gl::GL_TEXTURE_2D, 0);

		if(gl::glCheckFramebufferStatus(gl::GL_FRAMEBUFFER) != gl::GL_FRAMEBUFFER_COMPLETE) {
			//	handle errors here
		}

		//	sets the correct viewport
		gl::glViewport(0, 0, SCREEN_RES_W, SCREEN_RES_H);

		//	clears framebuffer with given color
		gl::glClearColor(COLOR_OUTPUT_CLEAR[0], COLOR_OUTPUT_CLEAR[1], COLOR_OUTPUT_CLEAR[2], COLOR_OUTPUT_CLEAR[3]);

		//	generates and sets the projection matrix
		auto projectionMat = glm::ortho<float>(0, SCREEN_DIM_W, SCREEN_DIM_H, 0);
		gl::glUniformMatrix4fv(uniProjectionLoc, 1, gl::GL_FALSE, glm::value_ptr(projectionMat));

		//	generates and sets the model matrix
		auto modelMat = glm::mat4{ 1 };
		modelMat = glm::translate(modelMat, { -layers[cutout].x1, -layers[cutout].y1, 0 });
		gl::glUniformMatrix4fv(uniModelLoc, 1, gl::GL_FALSE, glm::value_ptr(modelMat));

		//	sets the correct color
		gl::glUniform4fv(uniColorLoc, 1, glm::value_ptr(COLOR_OUTPUT_DRAW));

		stbi_write_png_compression_level = 16;

		//	pre allocs pixel memory
		auto data = std::make_unique<uint8_t[]>(SCREEN_RES_W * SCREEN_RES_H * 1);
		//	draws each layer
		for (size_t i = 0; i < len; i++) {
			//	clears texture
			gl::glClear(gl::GL_COLOR_BUFFER_BIT);
			//	renders the i-th layer
			gl::glBindVertexArray(layers[i].vao);
			gl::glDrawArrays(gl::GL_TRIANGLES, 0, layers[i].vertices);
			//	name generation
			std::string name = "layer_";
			name += std::to_string(i);
			name += ".png";
			//	saves the rendered image
			gl::glGetTextureImage(texOut, 0, gl::GL_RED, gl::GL_UNSIGNED_BYTE, SCREEN_RES_W * SCREEN_RES_H * 1, data.get());
			stbi_write_png(name.c_str(), SCREEN_RES_W, SCREEN_RES_H, 1, data.get(), SCREEN_RES_W * 1);
		}


		std::cout << "Image has been generated\n";
		exit(0);
	}

	//	models
	//	rect
	Model rect;

	gl::glGenVertexArrays(1, &rect.vao);
	gl::glBindVertexArray(rect.vao);

	gl::glGenBuffers(1, &rect.vbo);
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, rect.vbo);

	rect.vertices = array_info_t<decltype(RECT_VERTICES)>::length;
	gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(RECT_VERTICES), RECT_VERTICES, gl::GL_STATIC_DRAW);

	gl::glVertexAttribPointer(0, 2, gl::GL_FLOAT, gl::GL_FALSE, 2 * sizeof(float), (void*)(0));
	gl::glEnableVertexAttribArray(0);
	
	return 0;
}
