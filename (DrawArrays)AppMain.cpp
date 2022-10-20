#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

// Draw Primitive(s)
void draw() 
{
    GLenum mode = GL_TRIANGLES;
    GLint first = 0;
    GLsizei count = 3;

    glDrawArrays(mode, first, count);
}

// Create and compile shaders
static GLuint CompileShader(const string &source, GLuint shaderType) 
{
    // Create Shader object
    GLuint shaderID = glCreateShader(shaderType);
    const char* src = source.c_str();

    // Attach source code to shader object
    glShaderSource(shaderID, 1, &src, nullptr);

    // Compile the shader
    glCompileShader(shaderID);

    // Return ID of compiled shader
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

    // Attach compiled shaders
    glAttachShader(shaderProgram, vertexShaderComp);
    glAttachShader(shaderProgram, fragmentShaderComp);

    // Link shaders to create executable
    glLinkProgram(shaderProgram);

    glValidateProgram(shaderProgram);

    // Delete compiled shaders
    glDeleteShader(vertexShaderComp);
    glDeleteShader(fragmentShaderComp);

    // Return shader program
    return shaderProgram;

}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(750, 750, "Nicholas Ciesla", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) // initialize glew
        cout << "Error!" << endl;

    GLfloat vertices[] = { // position attributes
        
        -0.5, -0.5, 0.0, // vert 0
        1.0, 0.0, 0.0,   // vert 0 color (red)
        -0.5, 0.5, 0.0,  // vert 1
        0.0, 1.0, 0.0,   // vert 1 color (green)
        0.5, -0.5, 0.0,  // vert 2
        0.0, 0.0, 1.0    // vert 2 color (blue)

    };

    GLuint VBO; // reference to VBO
    glGenBuffers(1, &VBO); // Create VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // specify VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attr

    // Specify VBO attribute location and layout to GPU

    // describle layout of position attribute to the GPU
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // describe layout of color attribute to the GPU
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(1);


    // Vertex shader source code
    string vertexShaderSource =
        "#version 440 core\n"
        "layout(location = 0) in vec3 vPosition;"
        "layout(location = 1) in vec3 aColor;"
        "out vec3 oColor;"
        "void main()\n"
        "{\n"
        "gl_position = vPosition;"
        "oColor = aColor;"
        "}\n";

    // Fragment shader source code
    string fragmentShaderSource =
        "#version 440 core\n"
        "in vec3 oColor;"
        "out vec3 FragColor;"
        "void main()\n"
        "{\n"
        "FragColor = oColor;"
        "}\n";

    // Create Shader Program
    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Use Shader Program exe
    glUseProgram(shaderProgram);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw primitive
        draw();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}