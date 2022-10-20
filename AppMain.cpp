#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// GLM headers
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include<SOIL2/SOIL2.h>

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
glm::vec3 cameraPosition = glm::vec3(0.0f, 1.25f, 6.0f);
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

// Light source position
glm::vec3 lightPosition(0.0f, -2.0f, 5.0f);
glm::vec3 lightPosition1(-2.0f, 0.0f, -5.0f);

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
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// CREATE AND BIND LAMP START *******************************************************

	GLfloat lampVertices[] = {

		// Triangle 1
		-0.5, -0.5, 0.0, // index 0

		-0.5, 0.5, 0.0, // index 1

		0.5, -0.5, 0.0,  // index 2	

		// Triangle 2	
		0.5, 0.5, 0.0  // index 3	

	};

	// Define element indices
	GLubyte lampIndices[] = {
		0, 1, 2,
		1, 2, 3
	};

	// Plane Transforms
	glm::vec3 lampPlanePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.5f),
		glm::vec3(0.5f,  0.0f,  0.0f),
		glm::vec3(0.0f,  0.0f,  -0.5f),
		glm::vec3(-0.5f, 0.0f,  0.0f),
		glm::vec3(0.0f, 0.5f,  0.0f),
		glm::vec3(0.0f, -0.5f,  0.0f)
	};

	glm::float32 lampPlaneRotations[] = {
		0.0f, 90.0f, 180.0f, -90.0f, -90.f, 90.f
	};

	GLuint lampVBO, lampEBO, lampVAO;

	glGenBuffers(1, &lampVBO); // Create VBO
	glGenBuffers(1, &lampEBO); // Create EBO

	glGenVertexArrays(1, &lampVAO); // Create VOA

	glBindVertexArray(lampVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lampVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lampEBO); // Select EBO

	glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lampIndices), lampIndices, GL_STATIC_DRAW); // Load indices 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// CREATE AND BIND LAMP END *******************************************************

	// CREATE AND BIND LAPIS START *******************************************************
	GLfloat verticesLapis[] = {

		// Triangle 1
		-0.25, -0.5, 0.435, // index 0
		1.0, 0.0, 0.0, // red
		0.0, 3.0,    // Texture UV left side near top
		0.0f, 0.0f, 1.0f, // normal positive z

		0.0, 0.25, 0.0, // index 1
		0.0, 1.0, 0.0, // green
		0.5, 3.435,      // Texture UV top middle
		0.0f, 0.0f, 1.0f, // normal positive z

		0.25, -0.5, 0.435,  // index 2	
		0.0, 0.0, 1.0, // blue
		1.0, 3.0,    // Texture UV right side near top
		0.0f, 0.0f, 1.0f, // normal positive z

		// Triangle 2 and 3
		0.25, -3.0, 0.435,  // index 3
		1.0, 0.0, 1.0, // purple
		1.0, 0.0,      // Texture UV bottom right
		0.0f, 0.0f, 1.0f, // normal positive z

		-0.25, -3.0, 0.435,  // index 4
		1.0, 0.0, 1.0, // purple
		0.0, 0.0,       // Texture UV bottom left
		0.0f, 0.0f, 1.0f // normal positive z

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
	
	// location
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	// normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

	glm::vec3 LapisPosition(1.5f, 0.0f, 0.0f);

	// CREATE AND BIND LAPIS END *******************************************************

	// CREATE AND BIND PLANE START *******************************************************
	GLfloat verticesPlane[] = {

		// Triangle 1
		-5.0, -3.0, 5.0, // index 0
		0.57, 0.31, 0.14, // BROWN
		0.0, 0.0,         // Texture UV bottom left
		0.0f, 1.0f, 0.0f, // normal positive z

		5.0, -3.0, -5.0, // index 1
		0.57, 0.31, 0.14, // BROWN
		1.0, 1.0,         // Texture UV top right
		0.0f, 1.0f, 0.0f, // normal positive z

		5.0, -3.0, 5.0,  // index 2	
		0.57, 0.31, 0.14, // BROWN
		1.0, 0.0,        // Texture UV bottom right
		0.0f, 1.0f, 0.0f, // normal positive z

		// Triangle 2
		-5.0, -3.0, -5.0,  // index 3
		0.57, 0.31, 0.14, // BROWN
		0.0, 1.0,         // Texture UV top left
		0.0f, 1.0f, 0.0f // normal positive z

	};

	// Define element indices
	GLubyte PlaneIndices[] = {
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(PlaneIndices), PlaneIndices, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU
	// location
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	// normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

	// CREATE AND BIND PLANE END *******************************************************

	// CREATE AND BIND CHARGER/LASER POINTER START (CYLINDER) 

	GLfloat verticesCyl[] = {

		// Triangle 1
		-0.25, -0.5, 0.435, // index 0
		1.0, 0.0, 0.0, // red
		0.0, 3.0,    // Texture UV left side near top
		0.0f, 0.0f, 1.0f, // normal positive z

		0.0, -0.5, 0.0, // index 1
		0.0, 1.0, 0.0, // green
		0.5, 3.435,      // Texture UV top middle
		0.0f, 1.0f, 0.0f, // normal positive y

		0.25, -0.5, 0.435,  // index 2	
		0.0, 0.0, 1.0, // blue
		1.0, 3.0,    // Texture UV right side near top
		0.0f, 0.0f, 1.0f, // normal positive z

		// Triangle 2 and 3
		0.25, -3.0, 0.435,  // index 3
		1.0, 0.0, 1.0, // purple
		1.0, 0.0,      // Texture UV bottom right
		0.0f, 0.0f, 1.0f, // normal positive z

		-0.25, -3.0, 0.435,  // index 4
		1.0, 0.0, 1.0, // purple
		0.0, 0.0,       // Texture UV bottom left
		0.0f, 0.0f, 1.0f // normal positive z

	};

	// Define element indices
	GLubyte indicesCyl[] = {
		0, 1, 2,
		2, 4, 3,
		2, 4, 0
	};

	// Plane roations for Y axis
	glm::float32 planeRotationsYCyl[] = {
		0.0f,
		60.0f,
		120.0f,
		180.0f,
		240.0f,
		300.0f
	};


	GLuint VBOCyl, EBOCyl, VAOCyl;

	glGenBuffers(1, &VBOCyl); // Create VBO
	glGenBuffers(1, &EBOCyl); // Create EBO

	glGenVertexArrays(1, &VAOCyl); // Create VOA
	glBindVertexArray(VAOCyl);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBOCyl); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOCyl); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCyl), verticesCyl, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCyl), indicesCyl, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU

	// location
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	// normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

	glm::vec3 chargerPosition(-1.5f, 0.0f, 0.0f);

	// CREATE AND BIND CHARGER/LASER POINTER/LEGO NUB END (CYLINDER)

	// CREATE AND BIND RECTANGULAR PRISM START

	GLfloat verticesRec[] = {

		// Triangle 1
		-0.5, 0.0, 0.5, // index 0
		1.0, 0.0, 0.0, // red
		0.0, 0.0,    // Texture bottom left
		0.0f, 0.0f, 1.0f, // normal positive z

		-0.5, 0.5, 0.5, // index 1
		0.0, 1.0, 0.0, // green
		0.0, 1.0,      // Texture UV top left
		0.0f, 0.0f, 0.0f, // normal positive z

		0.5, 0.5, 0.5,  // index 2	
		0.0, 0.0, 1.0, // blue
		1.0, 1.0,    // Texture UV top right
		0.0f, 0.0f, 1.0f, // normal positive z

		// Triangle 2
		0.5, 0.0, 0.5,  // index 3
		1.0, 0.0, 1.0, // purple
		1.0, 0.0,      // Texture UV bottom right
		0.0f, 0.0f, 1.0f, // normal positive z

		// Triangle 3
		0.0, 0.5, 0.0,  // index 4
		1.0, 0.0, 1.0, // purple
		0.5, 1.0,      // Texture UV top middle
		0.0f, 1.0f, 0.0f, // normal positive y



	};

	// Define element indices
	GLubyte indicesRec[] = {
		0, 1, 2,
		0, 2, 3,
		1, 4, 2
	};

	glm::float32 recPlaneRotations[] = {
		0.0f, 90.0f, 180.0f, -90.0f
	};

	GLuint VBORec, EBORec, VAORec;

	glGenBuffers(1, &VBORec); // Create VBO
	glGenBuffers(1, &EBORec); // Create EBO

	glGenVertexArrays(1, &VAORec); // Create VOA
	glBindVertexArray(VAORec);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBORec); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBORec); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRec), verticesRec, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesRec), indicesRec, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU

	// location
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	// normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

	// CREATE AND BIND RECTANGULAR PRISM END

	// Load textures
	int lapisTexWidth;
	int lapisTexHeight;
	int woodTexWidth;
	int woodTexHeight;
	int blackTexWidth;
	int blackTexHeight;
	int tanTexWidth;
	int tanTexHeight;
	int goldTexWidth;
	int goldTexHeight;

	unsigned char* lapisImage = SOIL_load_image("lapis.jpg", &lapisTexWidth, &lapisTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* woodImage = SOIL_load_image("wood.png", &woodTexWidth, &woodTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* blackImage = SOIL_load_image("grey.png", &blackTexWidth, &blackTexHeight, 0, SOIL_LOAD_RGB); // **************
	unsigned char* tanImage = SOIL_load_image("tan.jpg", &tanTexWidth, &tanTexHeight, 0, SOIL_LOAD_RGB); // ******************
	unsigned char* goldImage = SOIL_load_image("gold.png", &goldTexWidth, &goldTexHeight, 0, SOIL_LOAD_RGB);

	// Generate Textures

	GLuint lapisTexture;
	glGenTextures(1, &lapisTexture);
	glBindTexture(GL_TEXTURE_2D, lapisTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lapisTexWidth, lapisTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, lapisImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(lapisImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint woodTexture;
	glGenTextures(1, &woodTexture);
	glBindTexture(GL_TEXTURE_2D, woodTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, woodTexWidth, woodTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, woodImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(woodImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint blackTexture;
	glGenTextures(1, &blackTexture);
	glBindTexture(GL_TEXTURE_2D, blackTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, blackTexWidth, blackTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, blackImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(blackImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint tanTexture;
	glGenTextures(1, &tanTexture);
	glBindTexture(GL_TEXTURE_2D, tanTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tanTexWidth, tanTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, tanImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(tanImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint goldTexture;
	glGenTextures(1, &goldTexture);
	glBindTexture(GL_TEXTURE_2D, goldTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, goldTexWidth, goldTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, goldImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(goldImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec3 aColor;"
		"layout(location = 2) in vec2 texCoord;"
		"layout(location = 3) in vec3 normal;"
		"out vec3 oColor;"
		"out vec2 oTexCoord;"
		"out vec3 oNormal;"
		"out vec3 fragPos;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"oColor = aColor;"
		"oTexCoord = texCoord;"
		"oNormal = mat3(transpose(inverse(model))) * normal;"
		"fragPos = vec3(model * vec4(vPosition, 1.0f));"
		"}\n";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec3 oColor;"
		"in vec2 oTexCoord;"
		"in vec3 oNormal;"
		"in vec3 fragPos;"
		"out vec4 fragColor;"
		"uniform sampler2D myTexture;"
		"uniform vec3 objectColor;"
		"uniform vec3 lightColor;"
		"uniform vec3 lightPos;"
		"uniform vec3 lightColor1;"
		"uniform vec3 lightPos1;"
		"uniform vec3 viewPos;"
		"void main()\n"
		"{\n"
		"// Ambient\n"
		"float ambientStrength = 0.4f;"
		"float ambientStrength1 = 0.2f;"
		"vec3 ambient = ambientStrength * lightColor;"
		"vec3 ambient1 = ambientStrength1 * lightColor1;"
		"// Diffuse\n"
		"vec3 norm = normalize(oNormal);"
		"vec3 lightDir = normalize(lightPos - fragPos);"
		"vec3 lightDir1 = normalize(lightPos1 - fragPos);"
		"float diff = max(dot(norm, lightDir), 0.0);"
		"float diff1 = max(dot(norm, lightDir1), 0.0);"
		"vec3 diffuse = diff * lightColor;"
		"vec3 diffuse1 = diff1 * lightColor1;"
		"// Specular\n"
		"float specularStrength = 1.5f;"
		"vec3 viewDir = normalize(viewPos - fragPos);"
		"vec3 reflectDir = reflect(-lightDir, norm);"
		"vec3 reflectDir1 = reflect(-lightDir1, norm);"
		"float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 128);"
		"float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0f), 128);"
		"vec3 specular = specularStrength * spec * lightColor;"
		"vec3 specular1 = specularStrength * spec1 * lightColor1;"
		"vec3 result = (ambient + ambient1 + diffuse + diffuse1 + specular + specular1) * objectColor;"
		"fragColor = texture(myTexture, oTexCoord) * vec4(result, 1.0f);"
		"}\n";

	// LAMP Vertex shader source code
	string lampVertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"}\n";

	// LAMP Fragment shader source code
	string lampFragmentShaderSource =
		"#version 330 core\n"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor = vec4(1.0f);" // Set lamp to white.
		"}\n";

	// Creating Shader Program
	GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	GLuint lampShaderProgram = CreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource);

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
		glm::mat4 modelMatrix;
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
			projectionMatrix = glm::ortho(-9.0f, 9.0f, -9.0f, 9.0f, 0.1f, 100.0f);
			//cout << "ortho" << endl;
		}

		// Select shader and uniform variable
		GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

		// Get light and object color, and light position location
		GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
		GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
		GLint lightColorLoc1 = glGetUniformLocation(shaderProgram, "lightColor1");
		GLint lightPosLoc1 = glGetUniformLocation(shaderProgram, "lightPos1");
		GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

		// LAPIS *****************************

		// Assign light and object Colors
		glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f); // object was very dark without using white
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightColorLoc1, 1.0f, 1.0f, 0.0f);
		// Set light position 
		glUniform3f(lightPosLoc, lightPosition.x, lightPosition.y, lightPosition.z);
		glUniform3f(lightPosLoc1, lightPosition1.x, lightPosition1.y, lightPosition1.z);
		// Specify view position
		glUniform3f(viewPosLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

		// Pass transform to shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindTexture(GL_TEXTURE_2D, lapisTexture);

		glBindVertexArray(VAOLapis); // User-defined VAO must be called before draw. 


		for (GLuint i = 0; i < 6; i++) { // Loop to draw duplicate shapes with different transformations
			// Declare identity matrix
			glm::mat4 modelMatrix;

			// Initialize transforms

			// matrix, angle in rad, vector(numbners multiplied by angle)
			modelMatrix = glm::translate(modelMatrix, LapisPosition);
			modelMatrix = glm::rotate(modelMatrix, planeRotationsYLapis[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

			// Pass transform to shader
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw primitive(s)
			draw(sizeof(indicesLapis));
		}
		
		// Redeclare model matrix to identity after previous translations
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		// Unbind VOA after drawing
		glBindVertexArray(0); //Incase different VAO wii be used after

		// CHARGER *************************

		glBindTexture(GL_TEXTURE_2D, blackTexture);

		glBindVertexArray(VAOCyl); // User-defined VAO must be called before draw. 

		for (GLuint i = 0; i < 6; i++) { // Loop to draw duplicate shapes with different transformations
			// Declare identity matrix
			glm::mat4 modelMatrix;

			// Initialize transforms

			modelMatrix = glm::translate(modelMatrix, chargerPosition);
			// matrix, angle in rad, vector(numbners multiplied by angle)
			modelMatrix = glm::rotate(modelMatrix, planeRotationsYCyl[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

			// Pass transform to shader
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw primitive(s)
			draw(sizeof(indicesCyl));
		}

		// Redeclare model matrix to identity after previous translations
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		// LASER POINTER **********************

		glBindTexture(GL_TEXTURE_2D, goldTexture);

		for (GLuint i = 0; i < 6; i++) { // Loop to draw duplicate shapes with different transformations
			// Declare identity matrix
			glm::mat4 modelMatrix;

			// Initialize transforms



			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -2.835f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, 90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, 20.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
			modelMatrix = glm::rotate(modelMatrix, planeRotationsYCyl[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(.35f, .7f, .35f));

			// Pass transform to shader
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw primitive(s)
			draw(sizeof(indicesCyl));
		}

		// Redeclare model matrix to identity after previous translations
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		// Unbind VOA after drawing
	

		// LEGO NUB  *******************************************************
		glBindTexture(GL_TEXTURE_2D, tanTexture);

		for (GLuint i = 0; i < 6; i++) { // Loop to draw duplicate shapes with different transformations
			// Declare identity matrix
			glm::mat4 modelMatrix;

			// Initialize transforms

			modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, -2.85f, 1.0f));
			modelMatrix = glm::rotate(modelMatrix, planeRotationsYCyl[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(.15f, .03f, .15f));

			// Pass transform to shader
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			// Draw primitive(s)
			draw(sizeof(indicesCyl));
		}

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glBindVertexArray(0); //Incase different VAO wii be used after

		// LEGO BODY (RECTANGULAR PRISM) ***********************************************************

		glBindVertexArray(VAORec);

		for (GLuint i = 0; i < 4; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, -3.0f, 1.0f));
			modelMatrix = glm::rotate(modelMatrix, recPlaneRotations[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(.2f, .2f, .2f));
				
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			draw(sizeof(indicesRec));
		}

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

		glBindVertexArray(0); //Incase different VAO wii be used after

		// PLANE ***************************

		// Assign light and object Colors
		glUniform3f(objectColorLoc, 0.46f, 0.36f, 0.25f);

		// Bind texture
		glBindTexture(GL_TEXTURE_2D, woodTexture);

		// Bind to Plane VOA
		glBindVertexArray(VAOPlane);

		// Draw Plane
		draw(sizeof(PlaneIndices));

		// Unbind VAO
		glBindVertexArray(0);

		// Unbind Shader
		glUseProgram(0); // Incase different shader will be used after


		// LAMP *************

		glUseProgram(lampShaderProgram);

		// Get matrix's uniform location and set matrix
		GLint lampModelLoc = glGetUniformLocation(lampShaderProgram, "model");
		GLint lampViewLoc = glGetUniformLocation(lampShaderProgram, "view");
		GLint lampProjLoc = glGetUniformLocation(lampShaderProgram, "projection");

		glUniformMatrix4fv(lampViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(lampProjLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindVertexArray(lampVAO);

		// Transform planes to form cube
		// lamp 0
		for (GLuint i = 0; i < 6; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, lampPlanePositions[i] / glm::vec3(8.0f, 8.0f, 8.0f) + lightPosition);
			modelMatrix = glm::rotate(modelMatrix, lampPlaneRotations[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(.125f, .125f, .125f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, lampPlaneRotations[i] * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			draw(sizeof(lampIndices));
		}

		// lamp 1
		for (GLuint i = 0; i < 6; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, lampPlanePositions[i] / glm::vec3(8.0f, 8.0f, 8.0f) + lightPosition1);
			modelMatrix = glm::rotate(modelMatrix, lampPlaneRotations[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(.125f, .125f, .125f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, lampPlaneRotations[i] * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(lampModelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			draw(sizeof(lampIndices));
		}

		glBindVertexArray(0); //Incase different VAO will be used after

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
	cameraPosition = glm::vec3(0.0f, 1.25f, 6.0f);
	target = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPosition - target);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
	firstMouseMove = true;
	fov = 45.0f;
}