#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

using namespace std;

// Draw Primitive(s)
void draw() 
{
    // Draw mode
    GLenum mode = GL_TRIANGLES;
    // First vert
    //GLint first = 0;
    // Number of Verts
    //GLsizei count = 6;
    GLsizei indices = 6;

    glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);
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
        
        //Triangle 1
        -1.0, 1.0, 0.0, // 0
        1.0, 0.0, 0.0,   // 0 color (red)
        -1.0, 0.0, 0.0,  // 1
        0.0, 1.0, 0.0,   // 1 color (green)
        -0.5, 0.0, 0.0,  // 2
        0.0, 0.0, 1.0,    // 2 color (blue)

        //Triangle 2
        0.0, 0.0, 0.0,  // 3
        0.0, 1.0, 0.0,   // 3 color (green)
        0.0, -1.0, 0.0,  // 4
        1.0, 0.0, 1.0    // 4 color (blue)
    };

    // Define element indices
    GLubyte indices[] = {
        // Triangle 1
        0, 1, 2,
        // Triangle 2
        2, 3, 4
    };

    GLuint VBO, EBO; // reference to VBO
    glGenBuffers(1, &VBO); // Create VBO
    glGenBuffers(1, &EBO); // Create EBo

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // specify VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // specify EBO

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attr
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indicies

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
        "layout(location = 0) in vec4 vPosition;"
        "layout(location = 1) in vec4 aColor;"
        "out vec4 oColor;"
        "void main()\n"
        "{\n"
        "gl_Position = vPosition;"
        "oColor = aColor;"
        "}\n";

    // Fragment shader source code
    string fragmentShaderSource =
        "#version 440 core\n"
        "in vec4 oColor;"
        "out vec4 FragColor;"
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