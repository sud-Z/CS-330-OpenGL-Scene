#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// GLM headers
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

using namespace std;


int width = 640, height = 480;
const double PI = 3.14159;
const float toRadians = PI / 180.0f;

// Input Callback Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// Decalre global view matrix
glm::mat4 viewMatrix;

// intialize global fov
GLfloat fov = 45.0f;

// Define Camera Attributes
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));

// Declare get target prototype
glm::vec3 getTarget();

// Camera transformation prototype (event listener)
void TransformCamera();

// Boolean array for keys and mouse buttons
bool keys[1024];
bool mouseButtons[3];

// Booleans to check camera transformation
bool isPanning = false;
bool isOrbiting = false;
bool isPerspective = true;

// Radius, Pitch, and Yaw
GLfloat radius = 6.0f;
GLfloat rawYaw = 0.0f;
GLfloat rawPtich = 0.0f;
GLfloat degYaw;
GLfloat degPitch;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat lastUse = 0.0f;

GLfloat lastX = width / 2;
GLfloat lastY = height / 2;
GLfloat xChange = 0.0f;
GLfloat yChange = 0.0f;

// Detect initial mouse movement
bool firstMouseMove = true;

void initCamera();


// Draw Primitive(s)
void draw(GLsizei indices)
{
	GLenum mode = GL_TRIANGLES;
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);

}

// Create and Compile Shaders
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	// Create Shader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// Attach source code to Shader object
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile Shader
	glCompileShader(shaderID);

	// Return ID of Compiled shader
	return shaderID;

}

// Create Program Object
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// Compile vertex shader
	GLuint vertexShaderComp = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fragmentShaderComp = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vertexShaderComp);
	glAttachShader(shaderProgram, fragmentShaderComp);

	// Link shaders to create executable
	glLinkProgram(shaderProgram);

	// Delete compiled vertex and fragment shaders
	glDeleteShader(vertexShaderComp);
	glDeleteShader(fragmentShaderComp);

	// Return Shader Program
	return shaderProgram;

}


int main(void)
{

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Set Input Callback Functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error!" << endl;

	// Enable Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Wireframe mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// CREATE AND BIND LAPIS START *******************************************************
	GLfloat verticesLapis[] = {

		// Triangle 1
		-0.25, -0.5, 0.435, // index 0
		1.0, 0.0, 0.0, // red

		0.0, 0.25, 0.0, // index 1
		0.0, 1.0, 0.0, // green

		0.25, -0.5, 0.435,  // index 2	
		0.0, 0.0, 1.0, // blue

		// Triangle 2 and 3
		0.25, -3.0, 0.435,  // index 3
		1.0, 0.0, 1.0, // purple

		-0.25, -3.0, 0.435,  // index 4
		1.0, 0.0, 1.0, // purple

	};

	// Define element indices
	GLubyte indicesLapis[] = {
		0, 1, 2,
		2, 4, 3,
		2, 4, 0
	};

	// Plane roations for Y axis
	glm::float32 planeRotationsYLapis[] = {
		0.0f,
		60.0f,
		120.0f,
		180.0f,
		240.0f,
		300.0f
	};


	GLuint VBOLapis, EBOLapis, VAOLapis;

	glGenBuffers(1, &VBOLapis); // Create VBO
	glGenBuffers(1, &EBOLapis); // Create EBO

	glGenVertexArrays(1, &VAOLapis); // Create VOA
	glBindVertexArray(VAOLapis);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBOLapis); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOLapis); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesLapis), verticesLapis, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesLapis), indicesLapis, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

	// CREATE AND BIND LAPIS END *******************************************************

	// CREATE AND BIND PLANE START *******************************************************
	GLfloat verticesPlane[] = {

		// Triangle 1
		-5.0, -3.0, 5.0, // index 0
		0.57, 0.31, 0.14, // BROWN

		5.0, -3.0, -5.0, // index 1
		0.57, 0.31, 0.14, // BROWN

		5.0, -3.0, 5.0,  // index 2	
		0.57, 0.31, 0.14, // BROWN

		// Triangle 2
		-5.0, -3.0, -5.0,  // index 3
		0.57, 0.31, 0.14, // BROWN

	};

	// Define element indices
	GLubyte indicesPlane[] = {
		0, 1, 2,
		0, 1, 3
	};

	GLuint VBOPlane, EBOPlane, VAOPlane;

	glGenBuffers(1, &VBOPlane); // Create VBO
	glGenBuffers(1, &EBOPlane); // Create EBO

	glGenVertexArrays(1, &VAOPlane); // Create VOA
	glBindVertexArray(VAOPlane);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBOPlane); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOPlane); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPlane), verticesPlane, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesPlane), indicesPlane, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

	// CREATE AND BIND PLANE END *******************************************************

	// Vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec4 vPosition;"
		"layout(location = 1) in vec4 aColor;"
		"out vec4 oColor;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vPosition;" // multiply matrix info by original position of verticies
		"oColor = aColor;"
		"}\n";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec4 oColor;"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor = oColor;"
		"}\n";

	// Creating Shader Program
	GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

	// Use Shader Program exe once
	//glUseProgram(shaderProgram);

	cout << "Scroll to Zoom." << endl;
	cout << "[L-Alt + LMB] to Orbit." << endl;
	cout << "[L-Alt + MMB] to Pan." << endl;
	cout << "[F] to Reset camera." << endl;
	cout << "[P] to Switch projection." << endl;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{

		// set delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use Shader Program exe and select VAO before drawing 
		glUseProgram(shaderProgram); // Call Shader per-frame when updating attributes

		// Declare identity matrix
		//glm::mat4 modelMatrix;
		glm::mat4 projectionMatrix;

		
		// Initialize transforms
		viewMatrix = glm::lookAt(cameraPosition, getTarget(), worldUp);

		// specify projection
		// fov, width, height, nearplane, farplane
		

		if (isPerspective) {
			// specify projection
			//glViewport(0, 0, width, height);
			// fov, width, height, nearplane, farplane
			projectionMatrix = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
			//cout << "perspective" << endl;

		}
		else {
			// specify projection
			//glOrtho(0, width, height, 0, 0.1, 50.0);
			// left, right, bottom, top, near, far
			projectionMatrix = glm::ortho(-9.0f, 9.0f, -9.0f, 9.0f, 0.1f, 50.0f);
			//cout << "ortho" << endl;
		}

		// Select shader and uniform variable
		GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

		// Pass transform to shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindVertexArray(VAOLapis); // User-defined VAO must be called before draw. 


		for (GLuint i = 0; i < 6; i++) { // Loop to draw duplicate shapes with different transformations
			// Declare identity matrix
			glm::mat4 modelMatrix;

			// Initialize transforms

			// matrix, angle in rad, vector(numbners multiplied by angle)
			modelMatrix = glm::rotate(modelMatrix, planeRotationsYLapis[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

			// Pass transform to shader
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw primitive(s)
			draw(sizeof(indicesLapis));
		}
		
		// Redeclare model matrix to identity after previous translations
		glm::mat4 modelMatrix;
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		// Unbind VOA after drawing
		glBindVertexArray(0); //Incase different VAO wii be used after

		// Bind to Plane VOA
		glBindVertexArray(VAOPlane);

		// Draw Plane
		draw(sizeof(indicesPlane));

		// Unbind VAO
		glBindVertexArray(0);

		// Unbind Shader
		glUseProgram(0); // Incase different shader will be used after

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		// Poll camera transformations
		TransformCamera();
	}

	//Clear GPU resources
	glDeleteVertexArrays(1, &VAOLapis);
	glDeleteBuffers(1, &VBOLapis);
	glDeleteBuffers(1, &EBOLapis);


	glfwTerminate();
	return 0;
}

// Define Input Callback funtions

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Display ASCII Keycode
	//cout << "ASCII: " << key << endl;

	if (action == GLFW_PRESS) {
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE) {
		keys[key] = false;
	}

	// Switch between orthographic and perspective view

	if (action == GLFW_PRESS && key == GLFW_KEY_P) {
		isPerspective = !isPerspective;
	}


}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Display scroll offset value
	/*
	if (yoffset > 0) {
		cout << "Scroll up: ";
	}
	else if (yoffset < 0) {
		cout << "Scroll down: ";
	}

	cout << yoffset << endl;
	*/

	// Clamp FOV
	if (fov >= 1.5f && fov <= 45.0f) {
		fov -= yoffset * 0.01f;
	}

	//Default FOV
	if (fov < 1.5f) {
		fov = 1.5f;
	}
	if (fov > 45.0f) {
		fov = 45.0f;
	}

}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// Display Mouse Position
	//cout << "Mouse pos [" << xpos << ", " << ypos << "]" << endl;

	if (firstMouseMove) { // Getting initial mouse position
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	// Calculate cursor offset
	xChange = xpos - lastX;
	yChange = lastY - ypos; // inverted controls for y

	lastX = xpos;
	lastY = ypos;


	if (isPanning) {

		if (cameraPosition.z < 0.0f) {
			cameraFront.z = 6.0f;
		}
		else {
			cameraFront.z = -1.0f;
		}

		// Pan camera X
		GLfloat cameraSpeed = xChange * deltaTime;
		cameraPosition += cameraSpeed * cameraRight;

		// Pan camera Y
		cameraSpeed = yChange * deltaTime;
		cameraPosition += cameraSpeed * cameraUp;
	}

	// Orbit camera
	if (isOrbiting) {
		rawYaw += xChange;
		rawPtich += yChange;

		// Convert Yaw and Pitch to degrees
		degYaw = glm::radians(rawYaw);
		degPitch = glm::radians(rawPtich);

		// Azimuth Altitude formula
		cameraPosition.x = target.x + radius * cosf(degPitch) * sinf(degYaw);
		cameraPosition.y = target.y + radius * sinf(degPitch);
		cameraPosition.z = target.z + radius * cosf(degPitch) * cosf(degYaw);
	}

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	/*
	// Display mouse button clicks
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		cout << "LMB clicked" << endl;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
		cout << "MMB clicked" << endl;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		cout << "RMB clicked" << endl;
	}
	*/

	if (action == GLFW_PRESS) {
		mouseButtons[button] = true;
	}
	else if (action == GLFW_RELEASE) {
		mouseButtons[button] = false;
	}
}

// Define getTarget function
glm::vec3 getTarget() {

	if (isPanning) {
		target = cameraPosition + cameraFront;
	}

	return target;
}

// Define TransformCamera function
void TransformCamera() {

	// Pan Camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE]) {
		isPanning = true;
	}
	else {
		isPanning = false;
	}

	// Orbit Camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_LEFT]) {
		isOrbiting = true;
	}
	else {
		isOrbiting = false;
	}

	// Reset/init camera
	if (keys[GLFW_KEY_F]) {
		initCamera();
	}

}

void initCamera() {
	cameraPosition = glm::vec3(0.0f, 0.0f, 6.0f);
	target = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPosition - target);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
	firstMouseMove = true;
	fov = 45.0f;
}