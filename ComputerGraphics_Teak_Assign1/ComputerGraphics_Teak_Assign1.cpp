//
// COMP 371 Labs Framework
//
// Created by Nicolas Bergeron on 20/06/2019.
//

#include <iostream>
#include <list>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>



#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // cross-platform interface for creating a graphical context,
						// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <ctime>
#include <Sphere.h>

#include <shader.h>

#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <shaderloader.h>



using namespace glm;
using namespace std;




// shader variable setters
void SetUniformMat4(GLuint shader_id, const char* uniform_name, mat4 uniform_value)
{
	glUseProgram(shader_id);
	glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE, &uniform_value[0][0]);
}

void SetUniformVec3(GLuint shader_id, const char* uniform_name, vec3 uniform_value)
{
	glUseProgram(shader_id);
	glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1, value_ptr(uniform_value));
}

template <class T>
void SetUniform1Value(GLuint shader_id, const char* uniform_name, T uniform_value)
{
	glUseProgram(shader_id);
	glUniform1i(glGetUniformLocation(shader_id, uniform_name), uniform_value);
	glUseProgram(0);
}






const char* getVertexShaderSource()
{
	// For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
	return
		"#version 330 core\n"
		"layout (location = 0) in vec3 position;"
		"layout (location = 1) in vec3 normals;"
		"layout(location = 2) in vec2 aUV;"
		""
		"uniform mat4 worldMatrix;"
		"uniform mat4 viewMatrix = mat4(1.0);"  // default value for view matrix (identity)
		"uniform mat4 projectionMatrix = mat4(1.0);"
		"uniform mat4 light_view_proj_matrix;"
		""
		"out vec3 fragment_normal;"
		"out vec3 fragment_position;"
		"out vec4 fragment_position_light_space;"
		"out vec2 vertexUV;"
		""
		"void main()"
	
		"   mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;"
		"   gl_Position = modelViewProjection * vec4(position, 1.0);"
		"vertexUV = aUV;"
		"}";
}


const char* getFragmentShaderSource()
{
	return
		"#version 330 core\n"
		"uniform vec3 objectColor;"
		"in vec3 vertexColor;"
		"out vec4 FragColor;"
		""
		"uniform vec3 lightColor;"

		"void main()"
		"{"
		" FragColor = vec4(objectColor.r,  objectColor.g,  objectColor.b, 1.0f);"
		"}";
}


int compileAndLinkShaders()
{
	// compile and link shader program
	// return shader program id
	// ------------------------------------

	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexShaderSource = getVertexShaderSource();
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragmentShaderSource = getFragmentShaderSource();
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// link shaders
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

bool loadOBJ2(
	const char * path,
	std::vector<int> & vertexIndices,
	std::vector<glm::vec3> & temp_vertices,
	std::vector<glm::vec3> & out_normals,
	std::vector<glm::vec2> & out_uvs) {

	std::vector<int> uvIndices, normalIndices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE * file;
	file = fopen(path, "r");
	if (!file) {
		printf("Impossible to open the file ! Are you in the right path ground ?\n");
		printf(path);
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			res = fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);

			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			res = fscanf(file, "%f %f\n", &uv.x, &uv.y);
			if (res != 2) {
				printf("Missing uv information!\n");
			}
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			res = fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			if (res != 3) {
				printf("Missing normal information!\n");
			}
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			char* getRes;
			int vertexIndex[3], uvIndex[3], normalIndex[3];
			bool uv = true;
			bool norm = true;
			char line[128];
			getRes = fgets(line, 128, file);
			if (getRes == 0) {
				printf("incomplete face\n");
			}

			//vertex, uv, norm
			int matches = sscanf(line, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				//vertex, norm
				matches = sscanf(line, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 6) {
					//vertex, uv
					matches = sscanf(line, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
					if (matches != 6) {
						//vertex
						matches = sscanf(line, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
						if (matches != 3) {
							printf("File can't be read by our simple parser. 'f' format expected: d/d/d d/d/d d/d/d || d/d d/d d/d || d//d d//d d//d\n");
							printf("Character at %ld", ftell(file));
							return false;
						}
						uv, norm = false;
					}
					else {
						norm = false;
					}
				}
				else {
					uv = false;
				}
			}
			vertexIndices.push_back(abs(vertexIndex[0]) - 1);
			vertexIndices.push_back(abs(vertexIndex[1]) - 1);
			vertexIndices.push_back(abs(vertexIndex[2]) - 1);
			if (norm) {
				normalIndices.push_back(abs(normalIndex[0]) - 1);
				normalIndices.push_back(abs(normalIndex[1]) - 1);
				normalIndices.push_back(abs(normalIndex[2]) - 1);
			}
			if (uv) {
				uvIndices.push_back(abs(uvIndex[0]) - 1);
				uvIndices.push_back(abs(uvIndex[1]) - 1);
				uvIndices.push_back(abs(uvIndex[2]) - 1);
			}
		}
		else {
			char clear[1000];
			char* getsRes = fgets(clear, 1000, file);
		}
	}
	if (normalIndices.size() != 0)
		out_normals.resize(temp_normals.size());
	if (uvIndices.size() != 0)
		out_uvs.resize(temp_normals.size());
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		int vi = vertexIndices[i];
		if (normalIndices.size() != 0) {
			int ni = normalIndices[i];
			out_normals[vi] = temp_normals[ni];
		}
		if (uvIndices.size() != 0 && i < uvIndices.size()) {
			int ui = uvIndices[i];
			out_uvs[vi] = temp_uvs[ui];
		}
	}

	return true;
}


GLuint setupModelEBO(string path, int& vertexCount)
{
	vector<int> vertexIndices;
	//The contiguous sets of three indices of vertices, normals and UVs, used to make a triangle
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> UVs;

	//read the vertices from the cube.obj file
	//We won't be needing the normals or UVs for this program
	loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO); //Becomes active VAO
							// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).

							//Vertex VBO setup
	GLuint vertices_VBO;
	glGenBuffers(1, &vertices_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Normals VBO setup
	GLuint normals_VBO;
	glGenBuffers(1, &normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	//UVs VBO setup
	GLuint uvs_VBO;
	glGenBuffers(1, &uvs_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	//EBO setup
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(int), &vertexIndices.front(), GL_STATIC_DRAW);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	// Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
	vertexCount = vertexIndices.size();
	return VAO;
}

int lightVAO()
{
	vec3 vertexArray[] = {  // position,                            color
		vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f), //left - red
		vec3(-0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),

		vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
		vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

		vec3(0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f), // far - blue
		vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
		vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),

		vec3(0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
		vec3(0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
		vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),

		vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f), // bottom - turquoise
		vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),
		vec3(0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),

		vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f),
		vec3(-0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f),
		vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),

		vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), // near - green
		vec3(-0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
		vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),

		vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
		vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
		vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),

		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f), // right - purple
		vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),
		vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),

		vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),
		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f),
		vec3(0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f),

		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f), // top - yellow
		vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),
		vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),

		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f),
		vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f)
	};
	// first, configure the cube's VAO (and VBO)
	GLuint VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
		3,                   // size
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		2 * sizeof(glm::vec3), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
		3,                   // size
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		2 * sizeof(glm::vec3), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);


	return lightVAO;
}



int createVertexBufferObject()
{
	// A vertex is a point on a polygon, it contains positions and other data (eg: colors)
	 // Cube model
	vec3 vertexArray[] = {  // position,                            color
		vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f), //left - red
		vec3(-0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),

		vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
		vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

		vec3(0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f), // far - blue
		vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
		vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),

		vec3(0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
		vec3(0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
		vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),

		vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f), // bottom - turquoise
		vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),
		vec3(0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),

		vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f),
		vec3(-0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f),
		vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),

		vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), // near - green
		vec3(-0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
		vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),

		vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
		vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
		vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),

		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f), // right - purple
		vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),
		vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),

		vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),
		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f),
		vec3(0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f),

		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f), // top - yellow
		vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),
		vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),

		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f),
		vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f)
	};


	// Create a vertex array
	GLuint vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);


	// Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

	glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
		3,                   // size
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		2 * sizeof(vec3), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
		3,
		GL_FLOAT,
		GL_FALSE,
		2 * sizeof(vec3),
		(void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
	);
	glEnableVertexAttribArray(1);

	return vertexBufferObject;
}



int createVertexBufferObject_Sphere() {
	// create a sphere with default params; radius=1, sectors=36, stacks=18, smooth=true
	Sphere sphere;

	// copy interleaved vertex data (V/N/T) to VBO
	GLuint vboId;
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);           // for vertex data
	glBufferData(GL_ARRAY_BUFFER,                   // target
		sphere.getInterleavedVertexSize(), // data size, # of bytes
		sphere.getInterleavedVertices(),   // ptr to vertex data
		GL_STATIC_DRAW);                   // usage

// copy index data to VBO
	GLuint iboId;
	glGenBuffers(1, &iboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);   // for index data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,           // target
		sphere.getIndexSize(),             // data size, # of bytes
		sphere.getIndices(),               // ptr to index data
		GL_STATIC_DRAW);                   // usage
	


		// bind VBOs
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	// activate attrib arrays
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// set attrib arrays with stride and offset
	int stride = sphere.getInterleavedStride();     // should be 32 bytes
	glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, stride, (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (void*)(sizeof(float) * 6));

	return iboId; 

}

int createVertexBufferObjectGrid() // for the grid
{
	// Cube model
	vec3 vertexArray[] = {  // position,                            color
		vec3( 5.0f,0.0f,0.0f), vec3(0.0f, 1.0f, 0.0f), //left - green
		vec3(-5.0f,0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)


	

		
	};
	float vertices[] = {
		// positions          // colors           // texture coords (note that we changed them to 2.0f!)
		 5.0f,  0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // top right
		 -5.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   0.0f,1.0f // bottom right
	
	};

	// Create a vertex array
	GLuint vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);


	// Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

	glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
		3,                   // size
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		2 * sizeof(vec3), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
		3,
		GL_FLOAT,
		GL_FALSE,
		2 * sizeof(vec3),
		(void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
	);
	glEnableVertexAttribArray(1);


	return vertexBufferObject;
}


int createVertexBufferObjectAxis() // for the grid
{
	// Cube model
	vec3 vertexArray[] = {  // position,                            color
		vec3(0.0f,0.0f,0.0f), vec3(1.0f, 0.0f, 1.0f), //purple
		vec3(0.5f,0.0f, 0.0f), vec3(1.0f, 0.0f, 1.0f),

		vec3(0.0f,0.0f,0.0f), vec3(1.0f, 1.0f, 0.0f), //cyan
		vec3(0.0f,0.5f, 0.0f), vec3(1.0f, 1.0f, 0.0f),

		vec3(0.0f,0.0f,0.0f), vec3(1.0f, 0.5f, 0.0f), //oranges
		vec3(0.0f,0.0f, 0.5f), vec3(1.0f, 0.5f, 0.0f)



	};


	// Create a vertex array
	GLuint vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);


	// Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
	GLuint vertexBufferObject;
	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

	glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
		3,                   // size
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		2 * sizeof(vec3), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);


	glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
		3,
		GL_FLOAT,
		GL_FALSE,
		2 * sizeof(vec3),
		(void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
	);
	glEnableVertexAttribArray(1);


	return vertexBufferObject;
}


GLuint loadTexture(const char *filename);

int main(int argc, char*argv[])
{

	int cubeVertices;
	GLuint cubeVAO;
	int sphereVertices;
	GLuint sphereVAO;
	int cylinderVertices;
	GLuint cylinderVAO;



	string cylinderPath = "../Assets/Models/cylinder.obj";
	string cubePath = "../Assets/Models/cube.obj";
	string spherePath = "../Assets/Models/sphere.obj";

	// Initialize GLFW and OpenGL version
	glfwInit();

#if defined(PLATFORM_OSX)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	// On windows, we set OpenGL version to 2.1, to support more hardware
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

	// Create Window and rendering context using GLFW, resolution is 800x600
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Olaafff", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// @TODO 3 - Disable mouse cursor
	// ...
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to create GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Black background
	glClearColor(0.3f, 0.1f, 0.6f, 1.0f);
	
	cubeVAO = setupModelEBO(cubePath, cubeVertices);
	sphereVAO = setupModelEBO(spherePath, sphereVertices);
	cylinderVAO = setupModelEBO(cylinderPath, cylinderVertices);

	// Compile and link shaders here ...
	//int shaderGrid = compileAndLinkShaders();

  
	// We can set the shader once, since we have only one
	//glUseProgram(shaderProgram);

	//ahhh
	
	GLuint grassTextureID = loadTexture("../Assets/Textures/grass.jpg");
	GLuint snowTextureID = loadTexture("../Assets/Textures/snow.jpg");
	GLuint carrotTextureID = loadTexture("../Assets/Textures/cement.jpg");

#if defined(PLATFORM_OSX)
	std::string shaderPathPrefix = "Shaders/";
#else
	std::string shaderPathPrefix = "../Assets/Shaders/";
#endif

	GLuint shaderProgram = loadSHADER(shaderPathPrefix + "scene_vertex.glsl", shaderPathPrefix + "scene_fragment.glsl");
	GLuint shaderShadow = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl", shaderPathPrefix + "shadow_fragment.glsl");
	GLuint shaderGrid = loadSHADER(shaderPathPrefix + "grid_vertex.glsl", shaderPathPrefix + "grid_fragment.glsl");

	glUseProgram(shaderProgram);
	
	/*
	// Setup texture and framebuffer for creating shadow map
	// Dimensions of the shadow texture, which should cover the viewport window size and shouldn't be oversized and waste resources
	const unsigned int DEPTH_MAP_TEXTURE_SIZE = 1024;

	// Variable storing index to texture used for shadow mapping
	GLuint depth_map_texture;
	// Get the texture
	glGenTextures(1, &depth_map_texture);
	// Bind the texture so the next glTex calls affect it
	glBindTexture(GL_TEXTURE_2D, depth_map_texture);
	// Create the texture and specify it's attributes, including widthn height, components (only depth is stored, no color information)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
		NULL);
	// Set texture sampler parameters.
	// The two calls below tell the texture sampler inside the shader how to upsample and downsample the texture. Here we choose the nearest filtering option, which means we just use the value of the closest pixel to the chosen image coordinate.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// The two calls below tell the texture sampler inside the shader how it should deal with texture coordinates outside of the [0, 1] range. Here we decide to just tile the image.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	// Variable storing index to framebuffer used for shadow mapping
	GLuint depth_map_fbo;  // fbo: framebuffer object
						   // Get the framebuffer
	glGenFramebuffers(1, &depth_map_fbo);
	// Bind the framebuffer so the next glFramebuffer calls affect it
	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	// Attach the depth map texture to the depth map framebuffer
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depth_map_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map_texture, 0);
	glDrawBuffer(GL_NONE); //disable rendering colors, only write depth values
	*/

	// Camera parameters for view transform
	vec3 cameraPosition(0.0f, 1.0f, 12.0f);
	vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
	vec3 cameraUp(0.0f, 1.0f, 0.0f);

	// Other camera parameters
	float cameraSpeed = 1.0f;
	float cameraFastSpeed = 2 * cameraSpeed;
	float cameraHorizontalAngle = 90.0f;
	float cameraVerticalAngle = 0.0f;
	bool  cameraFirstPerson = true; // press 1 or 2 to toggle this variable


	
	vec3 position = vec3(0.0f, 0.0f, 0.0f);
	float rotation_angle = 0.0f;

	// Spinning cube at camera position
	float spinningCubeAngle = 0.0f;

	// Set projection matrix for shader, this won't change
	mat4 projectionMatrix = glm::perspective(70.0f,            // field of view in degrees
		1024.0f / 768.0f,  // aspect ratio
		0.01f, 100.0f);   // near and far (near > 0)
	
	//GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	//glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	

	// Set initial view matrix
	mat4 viewMatrix = lookAt(cameraPosition,  // eye
		cameraPosition + cameraLookAt,  // center
		cameraUp); // up

	//GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	//glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

	
	// Set projection matrix on both shaders
	SetUniformMat4(shaderProgram, "projectionMatrix", projectionMatrix);

	// Set view matrix on both shaders
	SetUniformMat4(shaderProgram, "viewMatrix", viewMatrix);

	// Set light color on scene shader
	SetUniformVec3(shaderProgram, "light_color", vec3(1.0, 1.0, 1.0));

	// Set object color on scene shader
	SetUniformVec3(shaderProgram, "objectColor", vec3(1.0, 1.0, 1.0));
	

	// Define and upload geometry to the GPU here ...
	int vbo_cube = createVertexBufferObject();
	int vbo_grid = createVertexBufferObjectGrid();
	int vbo_axis = createVertexBufferObjectAxis();
	//int vbo_sphere = createVertexBufferObject_Sphere();
	//int vao_light = lightVAO();

	// For frame time
	float lastFrameTime = glfwGetTime();
	int lastMouseLeftState = GLFW_RELEASE;
	double lastMousePosX, lastMousePosY;
	glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);


	// Variables to be used later in tutorial
	float angle = 0;
	float rotationSpeed = 180.0f;  // 180 degrees per second
	

	// Other OpenGL states to set once
	// Enable Backface culling
	glEnable(GL_CULL_FACE);

	// @TODO 1 - Enable Depth Test
	// ...
	glEnable(GL_DEPTH_TEST); 


	
	

	GLuint colorLocation = glGetUniformLocation(shaderProgram, "objectColor");


	// the position of each piece is computed using hierarchical modeling

		// first, we apply the part matrix, which specifies where the triangle lands in world space on it's own, when no special transformation is applied by the group or parent. the part matrix is unique for each part.
		// in the context of the first assignment with the snowman, the part matrix is the one that has the role of making each cube into a specific part of the snowman (head, eye, torso, etc.), when the snowman rests at its default position (at the origin, with no scaling or rotation).
		// here, we're just applying a scaling to the triangle such that it faces the camera
	glm::mat4 partMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));

	// second, we take the result of that, and apply another transformation on top of it with the group matrix. the group matrix is identical for all the parts. you can think of it as treating all the parts as if they made up a single model that you manipulate as one entity.
	// in the context of the first assignment with the snowman, the group matrix is the one that allows you to interactively move around, scale and rotate yhe snowman.
	// here, we're making all the triangles spin around the y axis (which in terms of the camera space is the "up" axis).
	//glm::mat4 bodyMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 bodyMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 centerMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	srand(static_cast <unsigned> (time(0)));

	
	// Entering Main Loop
	while (!glfwWindowShouldClose(window))
	{
		

		// Each frame, reset color of each pixel to glClearColor

		// @TODO 1 - Clear Depth Buffer Bit as well
		// ...
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		
		
		

		
		
		// Frame time calculation
		float dt = glfwGetTime() - lastFrameTime;
		lastFrameTime += dt;

		
		/*************************** LIGHTING ********************************/
		
		// light parameters
		vec3 lightPosition = vec3(0.0f, 3.0f, 0.0f); // the location of the light in 3D space
		vec3 lightFocus(0.0f, -0.1f, 0.0f);      // the point in 3D space the light "looks" at
		vec3 lightDirection = normalize(lightFocus - lightPosition);

		float lightNearPlane = 3.0f;
		float lightFarPlane = 2000.0f;

		mat4 lightProjectionMatrix = perspective(50.0f, (float)1024 / (float)768, lightNearPlane, lightFarPlane);
		
		mat4 lightViewMatrix = lookAt(lightPosition, lightFocus, vec3(1.0f, 0.0f,1.0f));
		mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

		// Set light space matrix on both shaders
		SetUniformMat4(shaderShadow, "light_view_proj_matrix", lightSpaceMatrix);
		SetUniformMat4(shaderProgram, "light_view_proj_matrix", lightSpaceMatrix);
		// Set light position on scene shader
		SetUniformVec3(shaderProgram, "light_position", lightPosition);
		// Set light direction on scene shader
		SetUniformVec3(shaderProgram, "light_direction", lightDirection);

		/*
		// Render shadow in 2 passes: 1- Render depth map, 2- Render scene
		// 1- Render shadow map:
		// a- use program for shadows
		// b- resize window coordinates to fix depth map output size
		// c- bind depth map framebuffer to output the depth values
		{
			// Use proper shader
			glUseProgram(shaderShadow);
			// Use proper image output size
			glViewport(0, 0, 1024, 768);
			// Bind depth map texture as output framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
			// Clear depth data on the framebuffer
			glClear(GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(cubeVAO);

			mat4 partMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * rotate(mat4(1.0f), 0.0f, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
			mat4 groupMatrix = translate(mat4(1.0f), position) * rotate(mat4(1.0f), rotation_angle, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));
			mat4 worldMatrix = groupMatrix * partMatrix;

			SetUniformVec3(shaderProgram, "objectColor", vec3(0.0, 1.0, 0.0));
			SetUniformMat4(shaderProgram, "worldMatrix", worldMatrix);

			glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);
			glBindTexture(GL_TEXTURE_2D, depth_map_texture);
			//tree.Draw(shaderShadow);
			//tree.moveLocation(0.0f, 10.0f, 0.0f);

			
			// Unbind geometry
			glBindVertexArray(0);
		}
		*/

		

		angle = (angle + rotationSpeed * dt); // angles in degrees, but glm expects radians (conversion below)


	


		
		// light source 
		glBindVertexArray(cubeVAO);

	
		mat4 worldMatrixcube = mat4(1.0f);
		worldMatrixcube = glm::translate(worldMatrixcube, lightPosition);
		worldMatrixcube = glm::scale(worldMatrixcube, glm::vec3(0.2f));
		SetUniformVec3(shaderProgram, "objectColor", vec3(1.0, 1.0, 1.0));
		SetUniformMat4(shaderProgram, "worldMatrix", worldMatrixcube);

		glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);
		

		glBindVertexArray(cubeVAO);

		// Pressing the spacebar should re-position the Olaf at a random location on the grid. 
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			
			bodyMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f))), 0.0f, -5.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (10.0f)))));
		}

		//The user can incrementally size up the Olaf by pressing ‘U’ for scale-up and ‘I’ for scale-down. Each key press should result in a small size change.
		if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		{

			bodyMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.00f,1.01f, 1.00f)) * bodyMatrix;
		}

		//The user can incrementally size up the Olaf by pressing ‘U’ for scale-up and ‘I’ for scale-down. Each key press should result in a small size change.
		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		{

			bodyMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.00f, 0.99f, 1.00f)) * bodyMatrix;
		}

	


		// finally we actually compute the world matrix using this!
		// note that matrix composition notation is the reverse of the way we form sentences in english! "apply part matrix then group matrix" means "group matrix * part matrix"
		glm::mat4 worldMatrix = bodyMatrix * partMatrix;

		// in a full fledged hierarchical modeling implementation, instead of "group" and "part", you would have "parent" and "child" and a tree data structure that relates objects together.
		// the way the maths extend is pretty simple: we go from "worldMatrix = parentMatrix * childMatrix" to "worldMatrix = ... grandParentMatrix * parentMatrix * childMatrix * grandChildMatrix ..."

		GLenum mode = GL_TRIANGLES;
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		{

			mode = GL_LINES;

		}
		if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		{

			mode = GL_POINTS;

		}
		
		
		GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
		
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.15f, 0.1f, 0.0f));
		// drawing the feet left
		glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, -0.2f, 0.175f));
		glm::mat4 translationMatrix_lfeet = glm::translate(glm::mat4(1.0f), glm::vec3(-0.15f, 0.1f, 0.0f));
		
		partMatrix = translationMatrix_lfeet * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawElements(mode, cubeVertices, GL_UNSIGNED_INT, 0);
		

		// drawing the feet right
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, -0.2f, 0.175f));
		glm::mat4 translationMatrix_rfeet = glm::translate(glm::mat4(1.0f), glm::vec3(0.15f, 0.1f, 0.0f));

		partMatrix = translationMatrix_rfeet * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawElements(mode, cubeVertices, GL_UNSIGNED_INT, 0);



		// drawing the body
		glBindVertexArray(sphereVAO);

		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.02f, 0.02f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.6f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawElements(mode, sphereVertices, GL_UNSIGNED_INT, 0);


		glBindVertexArray(sphereVAO);
		// drawing the body upper
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.01f, 0.01f, 0.01f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawElements(mode, sphereVertices, GL_UNSIGNED_INT, 0);


		glBindVertexArray(sphereVAO);
		// drawing the head
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.4f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawElements(mode, sphereVertices, GL_UNSIGNED_INT, 0);


		glBindVertexArray(cubeVAO);
		// drawing the nose
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.5f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.4f, 0.1f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 0.0, 1.0)));
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		{

			glBindTexture(GL_TEXTURE_2D, carrotTextureID);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 0.64, 0.0)));

		}
		glDrawElements(mode, cubeVertices, GL_UNSIGNED_INT, 0);

		// drawing the hat
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, -0.25f, 0.1f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.6f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		{

			glBindTexture(GL_TEXTURE_2D, carrotTextureID);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.70, 0.71, 0.62)));

		}

		glDrawElements(mode, cubeVertices, GL_UNSIGNED_INT, 0);

		// drawing the left arm
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -0.075f, 0.1f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.4f, 1.2f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawElements(mode, cubeVertices, GL_UNSIGNED_INT, 0);

		// drawing the right arm
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -0.075f, 0.1f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, 1.2f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawElements(mode, cubeVertices, GL_UNSIGNED_INT, 0);

		
		//Ground
		
		
		glBindVertexArray(cubeVAO);


		mat4 ground = mat4(1.0f);

		
		ground = glm::translate(ground, vec3(0.0f,-0.02f,0.0f));
		ground = glm::scale(ground, glm::vec3(25.0f,0.02f,25.0f));
		
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 0.0)));
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		{

			glBindTexture(GL_TEXTURE_2D, snowTextureID);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		}
		
		
		
		
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &ground[0][0]);
		
		glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);




		
		
		//grid
		/*
		glBindVertexArray(cubeVAO);

		mat4 partMatrixg = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * rotate(mat4(1.0f), 0.0f, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
		mat4 groupMatrixg = translate(mat4(1.0f), position) * rotate(mat4(1.0f), rotation_angle, vec3(0.0f, 1.0f, 0.0f)) * scale(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f));
		mat4 gridMatrix = groupMatrixg * partMatrixg;

		glBindTexture(GL_TEXTURE_2D, grassTextureID);
		SetUniformVec3(shaderProgram, "objectColor", vec3(0.0, 1.0, 0.0));
		SetUniformMat4(shaderProgram, "worldMatrix", gridMatrix);

		glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);
		*/
		


		/*
		glBindVertexArray(cubeVAO);

		glLineWidth(1.0f);



		

		mat4 gridWorldMatrix = mat4(1.0f);
		for (int i = 0; i < 50; i++) {
			gridWorldMatrix = translate(mat4(1.0f), vec3(0.0f , 0.0f, 0.0f + i * 0.1f) );
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &gridWorldMatrix[0][0]);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 0.0)));
			glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);

			gridWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -0.0f + i * -0.1f));
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &gridWorldMatrix[0][0]);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 0.0)));
			glDrawElements(GL_TRIANGLES, cubeVertices, GL_UNSIGNED_INT, 0);
		}

		
		
		mat4 rotatemat = rotate(gridWorldMatrix, radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
		
		for (int i = 0; i < 50; i++) {
			
			gridWorldMatrix = translate(mat4(1.0f), vec3(0.0f + i * 0.1f, 0.0f,5.0f )) * rotatemat;
			
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &gridWorldMatrix[0][0]);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 0.0)));
			glDrawArrays(GL_LINES, 0, 2);

			
			gridWorldMatrix = translate(mat4(1.0f), vec3(-0.0f + i * -0.1f, 0.0f, 5.0f)) * rotatemat;

			
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &gridWorldMatrix[0][0]);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 0.0)));
			glDrawArrays(GL_LINES, 0, 2);
		}

		*/


		/*
		// axis 

		glBindVertexArray(vbo_axis);
		glLineWidth(5.0f);
		mat4 axis_mat = mat4(1.0f) ;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &axis_mat[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 0.5, 1.0)));
		glDrawArrays(GL_LINES, 0, 2);

		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &axis_mat[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 1.0)));
		glDrawArrays(GL_LINES, 2, 4);

		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &axis_mat[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 0.0, 1.0)));
		glDrawArrays(GL_LINES, 4, 6);


		*/
		

		
		



		// End Frame
		glfwSwapBuffers(window);
		

		// Detect inputs
		glfwPollEvents();

		//Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);


		//The user can control the Olaf position and orientation using keyboard input i.e. 
	//A → move left, D → move right, W → move up, S → move down, a → rotate left 5 degrees about Y axis, d → rotate right 5 degrees about Y axis. You may add other rotations about other axis, if you want. 
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{

			bodyMatrix = bodyMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(-0.05f, 0.0f, 0.0f));
			
			
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{

			bodyMatrix = bodyMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.05f, 0.0f, 0.0f));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{

			bodyMatrix = bodyMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.05f));
		}
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		{

			bodyMatrix = bodyMatrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.05f));
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		{

			bodyMatrix = bodyMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		{

			bodyMatrix = bodyMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		}


		// World Transform
	

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // move world Wx
		{
			projectionMatrix = projectionMatrix * glm::rotate(mat4(1.0f), glm::radians(0.1f), glm::vec3(0.001f, 0.0f, 0.0f));

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // move world W-x
		{
			projectionMatrix = projectionMatrix * glm::rotate(mat4(1.0f), glm::radians(0.1f), glm::vec3(-0.001f, 0.0f, 0.0f));

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // move world Wy
		{
			projectionMatrix = projectionMatrix * glm::rotate(mat4(1.0f), glm::radians(0.1f), glm::vec3(0.0f, 0.001f, 0.0f));

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // move world W-y
		{
			
			projectionMatrix = projectionMatrix * glm::rotate(mat4(1.0f), glm::radians(0.1f), glm::vec3(0.0f, -0.001f, 0.0f));

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}

		if (glfwGetKey(window, GLFW_KEY_V== GLFW_PRESS)) // reset cam
		{
			projectionMatrix = glm::scale(projectionMatrix, glm::vec3(1.0f, 1.0f, 1.01f));;

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}

		if (glfwGetKey(window, GLFW_KEY_B == GLFW_PRESS)) // reset cam
		{
			viewMatrix = viewMatrix * glm::translate(mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
			glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
		}
		
		// Projection Transform
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		{
			projectionMatrix = glm::perspective(70.0f,            // field of view in degrees
				1024.0f / 768.0f,  // aspect ratio
				0.01f, 100.0f);   // near and far (near > 0)

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}
		

		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		{
			glm::mat4 projectionMatrix = glm::ortho(-4.0f, 4.0f,    // left/right
				-3.0f, 3.0f,    // bottom/top
				-100.0f, 100.0f);  // near/far (near == 0 is ok for ortho)

			GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
			glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
		}

		bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
		float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;

		/*
		// View Transform

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // move camera to the left
		{
			cameraPosition.x -= currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // move camera to the right
		{
			cameraPosition.x += currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // move camera up
		{
			cameraPosition.y -= currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) // move camera down
		{
			cameraPosition.y += currentCameraSpeed * dt;
		}

		*/

		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) // move camera backward
		{
			cameraPosition.z -= currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) // move camera forward
		{
			cameraPosition.z += currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) // move camera left
		{
			cameraLookAt.x -= currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) // move camera right
		{
			cameraLookAt.x += currentCameraSpeed * dt;
		}

		mat4 viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);

		GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
		glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);


		
		

	}

	

	// Shutdown GLFW
	glfwTerminate();

	return 0;
}


GLuint loadTexture(const char *filename)
{
	// Step1 Create and bind textures
	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	assert(textureId != 0);
	

	glBindTexture(GL_TEXTURE_2D, textureId);

	// Step2 Set filter parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Step3 Load Textures with dimension data
	int width, height, nrChannels;
	unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (!data)
	{
		std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
		return 0;
	}

	// Step4 Upload the texture to the PU
	GLenum format = 0;
	if (nrChannels == 1)
		format = GL_RED;
	else if (nrChannels == 3)
		format = GL_RGB;
	else if (nrChannels == 4)
		format = GL_RGBA;
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
		0, format, GL_UNSIGNED_BYTE, data);

	// Step5 Free resources
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureId;
}

