//
// COMP 371 Labs Framework
//
// Created by Nicolas Bergeron on 20/06/2019.
//

#include <iostream>
#include <list>

#define GLEW_STATIC 1   // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>    // Include GLEW - OpenGL Extension Wrangler

#include <GLFW/glfw3.h> // cross-platform interface for creating a graphical context,
						// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp> // include this to create transformation matrices
#include <glm/common.hpp>
#include <glm/gtc/type_ptr.hpp>



using namespace glm;
using namespace std;




const char* getVertexShaderSource()
{
	// For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
	return
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;"
		"layout (location = 1) in vec3 aColor;"
		""
		"uniform mat4 worldMatrix;"
		"uniform mat4 viewMatrix = mat4(1.0);"  // default value for view matrix (identity)
		"uniform mat4 projectionMatrix = mat4(1.0);"
		""
		"out vec3 vertexColor;"
		"void main()"
		"{"
		"   vertexColor = aColor;"
		"   mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;"
		"   gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
		"}";
}


const char* getFragmentShaderSource()
{
	return
		"#version 330 core\n"
		"uniform vec3 objectColor;"
		"in vec3 vertexColor;"
		"out vec4 FragColor;"
		"void main()"
		"{"
		"   FragColor = vec4(objectColor.r, objectColor.g, objectColor.b, 1.0f);"
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


int createVertexBufferObjectGrid() // for the grid
{
	// Cube model
	vec3 vertexArray[] = {  // position,                            color
		vec3( 5.0f,0.0f,0.0f), vec3(0.0f, 1.0f, 0.0f), //left - green
		vec3(-5.0f,0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f),


	

		
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
		vec3(0.0f,0.0f, 0.5f), vec3(1.0f, 0.5f, 0.0f), 



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


int main(int argc, char*argv[])
{
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
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	

	// Compile and link shaders here ...
	int shaderProgram = compileAndLinkShaders();

	// We can set the shader once, since we have only one
	glUseProgram(shaderProgram);

	//ahhh


	// Camera parameters for view transform
	vec3 cameraPosition(0.6f, 1.0f, 10.0f);
	vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
	vec3 cameraUp(0.0f, 1.0f, 0.0f);

	// Other camera parameters
	float cameraSpeed = 1.0f;
	float cameraFastSpeed = 2 * cameraSpeed;
	float cameraHorizontalAngle = 90.0f;
	float cameraVerticalAngle = 0.0f;
	bool  cameraFirstPerson = true; // press 1 or 2 to toggle this variable

	// Spinning cube at camera position
	float spinningCubeAngle = 0.0f;

	// Set projection matrix for shader, this won't change
	mat4 projectionMatrix = glm::perspective(70.0f,            // field of view in degrees
		1024.0f / 768.0f,  // aspect ratio
		0.01f, 100.0f);   // near and far (near > 0)

	GLuint projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	// Set initial view matrix
	mat4 viewMatrix = lookAt(cameraPosition,  // eye
		cameraPosition + cameraLookAt,  // center
		cameraUp); // up

	GLuint viewMatrixLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);



	// Define and upload geometry to the GPU here ...
	int vbo_cube = createVertexBufferObject();
	int vbo_grid = createVertexBufferObjectGrid();
	int vbo_axis = createVertexBufferObjectAxis();


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

	// Entering Main Loop
	while (!glfwWindowShouldClose(window))
	{
		

		// Each frame, reset color of each pixel to glClearColor

		// @TODO 1 - Clear Depth Buffer Bit as well
		// ...
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		
		

		glBindVertexArray(vbo_cube);
		
		// Frame time calculation
		float dt = glfwGetTime() - lastFrameTime;
		lastFrameTime += dt;

		

		angle = (angle + rotationSpeed * dt); // angles in degrees, but glm expects radians (conversion below)


		// the position of each piece is computed using hierarchical modeling

		// first, we apply the part matrix, which specifies where the triangle lands in world space on it's own, when no special transformation is applied by the group or parent. the part matrix is unique for each part.
		// in the context of the first assignment with the snowman, the part matrix is the one that has the role of making each cube into a specific part of the snowman (head, eye, torso, etc.), when the snowman rests at its default position (at the origin, with no scaling or rotation).
		// here, we're just applying a scaling to the triangle such that it faces the camera
		glm::mat4 partMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, 1.0f));

		// second, we take the result of that, and apply another transformation on top of it with the group matrix. the group matrix is identical for all the parts. you can think of it as treating all the parts as if they made up a single model that you manipulate as one entity.
		// in the context of the first assignment with the snowman, the group matrix is the one that allows you to interactively move around, scale and rotate yhe snowman.
		// here, we're making all the triangles spin around the y axis (which in terms of the camera space is the "up" axis).
		//glm::mat4 bodyMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 bodyMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

		// finally we actually compute the world matrix using this!
		// note that matrix composition notation is the reverse of the way we form sentences in english! "apply part matrix then group matrix" means "group matrix * part matrix"
		glm::mat4 worldMatrix = bodyMatrix * partMatrix;

		// in a full fledged hierarchical modeling implementation, instead of "group" and "part", you would have "parent" and "child" and a tree data structure that relates objects together.
		// the way the maths extend is pretty simple: we go from "worldMatrix = parentMatrix * childMatrix" to "worldMatrix = ... grandParentMatrix * parentMatrix * childMatrix * grandChildMatrix ..."


		
		
		GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");


		// drawing the feet left
		glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, -0.2f, 0.175f));
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.15f, 0.1f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0

		// drawing the feet right
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, -0.2f, 0.175f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.15f, 0.1f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0


		// drawing the body
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, -1.0f, 0.2f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.6f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0

		// drawing the body upper
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, -0.25f, 0.2f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.2f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0

		// drawing the head
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.25f, 0.2f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.4f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0

		// drawing the nose
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.4f, 0.1f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 0.0, 1.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0

		// drawing the hat
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, -0.25f, 0.1f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.6f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0

		// drawing the left arm
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -0.075f, 0.1f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.4f, 1.2f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0

		// drawing the right arm
		scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -0.075f, 0.1f));
		translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, 1.2f, 0.0f));

		partMatrix = translationMatrix * scalingMatrix;
		worldMatrix = bodyMatrix * partMatrix;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &worldMatrix[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 1.0, 1.0)));

		glDrawArrays(GL_TRIANGLES, 0, 36); // 3 vertices, starting at index 0

		

		

		
	

		//grid
		glBindVertexArray(vbo_grid);
		glLineWidth(1.0f);



		

		mat4 gridWorldMatrix = mat4(1.0f);
		for (int i = 0; i < 50; i++) {
			gridWorldMatrix = translate(mat4(1.0f), vec3(0.0f , 0.0f, 0.0f + i * 0.1f) );
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &gridWorldMatrix[0][0]);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 0.0)));
			glDrawArrays(GL_LINES, 0, 2);	

			gridWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -0.0f + i * -0.1f));
			glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &gridWorldMatrix[0][0]);
			glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 0.0)));
			glDrawArrays(GL_LINES, 0, 2);
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

		// axis 

		glBindVertexArray(vbo_axis);
		glLineWidth(3.0f);
		mat4 axis_mat = mat4(1.0f) ;
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &axis_mat[0][0]);
		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 0.5, 1.0)));
		glDrawArrays(GL_LINES, 0, 2);

		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(0.0, 1.0, 1.0)));
		glDrawArrays(GL_LINES, 2, 4);

		glUniform3fv(colorLocation, 1, glm::value_ptr(glm::vec3(1.0, 0.0, 1.0)));
		glDrawArrays(GL_LINES, 4, 6);



		

		

		



		// End Frame
		glfwSwapBuffers(window);
		

		// Detect inputs
		glfwPollEvents();

		//Handle inputs
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);





		// Projection Transform
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		{
			glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f),  // field of view in degrees
				800.0f / 600.0f,      // aspect ratio
				0.01f, 100.0f);       // near and far (near > 0)

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

		// View Transform

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // move camera to the left
		{
			cameraPosition.x -= currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // move camera to the right
		{
			cameraPosition.x += currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // move camera up
		{
			cameraPosition.y -= currentCameraSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // move camera down
		{
			cameraPosition.y += currentCameraSpeed * dt;
		}

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
