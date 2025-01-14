#include "gfx/shaders.h"

constexpr char* SHADER_VERTEX_SOURCE = 
	"#version 330 core\n"

    "layout (location = 0) in vec2 inPos;\n"

	"uniform mat4 uniModel;\n"
	"uniform mat4 uniView;\n"
	"uniform mat4 uniProjection;\n"
	
    "void main() {\n"
    "   gl_Position = uniProjection * uniView * uniModel * vec4(inPos.x, inPos.y, 0.0, 1.0);\n"
    "}\0";

constexpr char* SHADER_FRAGMENT_SOURCE =
	"#version 330 core\n"

    "out vec4 outColor;\n"

	"uniform vec4 uniColor;\n"

    "void main() {\n"
    "   outColor = uniColor;\n"
    "}\0";

gl::GLuint load_shader() {
	auto shaderVtx = gl::glCreateShader(gl::GL_VERTEX_SHADER);
	gl::glShaderSource(shaderVtx, 1, &SHADER_VERTEX_SOURCE, nullptr);
	gl::glCompileShader(shaderVtx);

	auto shaderFrg = gl::glCreateShader(gl::GL_FRAGMENT_SHADER);
	gl::glShaderSource(shaderFrg, 1, &SHADER_FRAGMENT_SOURCE, nullptr);
	gl::glCompileShader(shaderFrg);

	auto shader = gl::glCreateProgram();
	gl::glAttachShader(shader, shaderVtx);
	gl::glAttachShader(shader, shaderFrg);
	gl::glLinkProgram(shader);
	
	gl::glDeleteShader(shaderVtx);
	gl::glDeleteShader(shaderFrg);

	gl::glUseProgram(shader);

	return shader;
}