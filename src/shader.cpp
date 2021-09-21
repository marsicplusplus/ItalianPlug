#include "shader.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include "spdlog/spdlog.h"

Shader::Shader(){
	programID = glCreateProgram();
}

int Shader::loadShader(const char* filePath, int type){
	std::string code;
	std::ifstream shaderFile;
	std::stringstream shaderStream;
	
	shaderFile.open(filePath);
	if(!shaderStream){
		spdlog::error("OPENSDL::SHADER::Shader::Cannot load shader file: {}", filePath);
		exit(1);
	}
	shaderStream << shaderFile.rdbuf();
	shaderFile.close();

	code = shaderStream.str();

	const char* codeChar = code.c_str();
	unsigned int shaderID = glCreateShader(type);
	glShaderSource(shaderID, 1, &codeChar, NULL);
	glCompileShader(shaderID);
	std::string typeS;
	switch (type) {
		case GL_VERTEX_SHADER:
			typeS = "VERTEX";
			break;
		case GL_FRAGMENT_SHADER:
			typeS = "FRAGMENT";
			break;
		case GL_TESS_CONTROL_SHADER:
			typeS = "CONTROL";
			break;
		case GL_TESS_EVALUATION_SHADER:
			typeS = "EVALUATION";
			break;
	}
	checkCompileErrors(shaderID, typeS);

	glAttachShader(programID, shaderID);
	
	shadersID.push_back(shaderID);

	return 0;
}

Shader::~Shader(){
	glDeleteProgram(programID);
}

void Shader::compileShaders(){
	glLinkProgram(programID);
	checkCompileErrors(programID, "PROGRAM");

	for(unsigned int shaderID : shadersID){
		glDeleteShader(shaderID);
	}
}

void Shader::use(){
	glUseProgram(programID);
}

int Shader::checkCompileErrors(unsigned int shader, std::string type) {
	int success;
	char infoLog[1024];
	if(type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			spdlog::error("ERROR::SHADER_COMPILATION_ERROR of type: {} \n {} \n  -- --------------------------------------------------- -- \n", type, infoLog);
			return -1;
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if(!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			spdlog::error("ERROR::SHADER_COMPILATION_ERROR of type: {} \n {} \n  -- --------------------------------------------------- -- \n", type, infoLog);
			return -1;
		}
	}
	return 0;
}

void Shader::setUniform(const std::string &name, const glm::mat4 &v) const{
	glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &v[0][0]);
}
void Shader::setUniform(const std::string &name, const glm::vec3 &v) const{
	setUniform(name, v.x, v.y, v.z);
}
void Shader::setUniform(const std::string &name, const int v) const{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), v);
}
void Shader::setUniform(const std::string &name, const float v) const{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), v);
}
void Shader::setUniform(const std::string &name, const float a, const float b, const float c) const {
	glUniform3f(glGetUniformLocation(programID, name.c_str()), a, b, c);
}
