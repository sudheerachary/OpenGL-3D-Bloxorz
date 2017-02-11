#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projectionO, projectionP;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;
int proj_type;
glm::vec3 tri_pos, rect_pos;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
       std::string Line = "";
       while(getline(VertexShaderStream, Line))
          VertexShaderCode += "\n" + Line;
      VertexShaderStream.close();
  }

    // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
   std::string Line = "";
   while(getline(FragmentShaderStream, Line))
       FragmentShaderCode += "\n" + Line;
   FragmentShaderStream.close();
}

GLint Result = GL_FALSE;
int InfoLogLength;

    // Compile Vertex Shader
    //    printf("Compiling shader : %s\n", vertex_file_path);
char const * VertexSourcePointer = VertexShaderCode.c_str();
glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
glCompileShader(VertexShaderID);

    // Check Vertex Shader
glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
std::vector<char> VertexShaderErrorMessage(InfoLogLength);
glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //    printf("Compiling shader : %s\n", fragment_file_path);
char const * FragmentSourcePointer = FragmentShaderCode.c_str();
glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
glCompileShader(FragmentShaderID);

    // Check Fragment Shader
glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //    fprintf(stdout, "Linking program\n");
GLuint ProgramID = glCreateProgram();
glAttachShader(ProgramID, VertexShaderID);
glAttachShader(ProgramID, FragmentShaderID);
glLinkProgram(ProgramID);

    // Check the program
glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

glDeleteShader(VertexShaderID);
glDeleteShader(FragmentShaderID);

return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void initGLEW(void){
    glewExperimental = GL_TRUE;
    if(glewInit()!=GLEW_OK){
       fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
   }
   if(!GLEW_VERSION_3_3)
       fprintf(stderr, "3.3 version not available\n");
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = M_PI/2;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projectionP = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projectionO = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *createCell(float l, float b, float h, float r, float g, float bl)
{
    GLfloat vertex_buffer_data[ ] = {
        0, 0, 0, b, 0, 0, b, h, 0, b, h, 0, 0, h, 0, 0, 0 , 0,        //1
        0, 0, 0, 0, h, 0, 0, h, l, 0, h, l, 0, 0, l, 0, 0, 0,    //2
        0, 0, 0, 0, 0, l, b, 0, l, b, 0, l, b, 0, 0, 0, 0, 0,        //3 

        0, 0, l, b, 0, l, b, h, l, b, h, l, 0, h, l, 0, 0, l,        //4
        b, 0, l, b, 0, 0, b, h, 0, b, h, 0, b, h, l, b, 0, l,            //5
        0, h, l, b, h, l, b, h, 0, b, h, 0, 0, h, 0, 0, h, l         //6
    };
    GLfloat color_buffer_data [ ] = {
        r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl,
        r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl,
        r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl,
        
        r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl,
        r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl,
        r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl, r, g, bl, 
    };
    return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

class GraphicalObject
{
public:
    VAO* object;
    float x_ordinate;
    float y_ordinate;
    float z_ordinate;
    float angle;
    char color;
    glm::mat4 translate_matrix;
    glm::mat4 rotate_matrix;
public:
    GraphicalObject(float X=0, float Y=0, float Z=0, float rotation=0, char colour='D')
    {
      x_ordinate = X;
      y_ordinate = Y;
      z_ordinate = Z;
      angle = rotation;
      color = colour;
  }
  void rotator(float rotation=0, glm::vec3 rotating_vector=glm::vec3(0,0,1))
  {
      rotate_matrix = glm::rotate((float)(rotation*M_PI/180.0f), rotating_vector);
  }
  void translator(float x = 0, float y = 0, float z = 0)
  {
        translate_matrix = glm::translate(glm::vec3(x, y, z));
  }
  void render( )
  {
      glm::mat4 VP = Matrices.projectionO * Matrices.view;
      glm::mat4 MVP;
      Matrices.model = glm::mat4(1.0f);
      Matrices.model = translate_matrix*rotate_matrix;
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(object);
  }
};

// CONSTANTS //
float z_ordinate = 0.0f, y_ordinate = 0.0f, x_ordinate = 0.0f;
int board[10][10], flag = 0;
VAO *axes, *cell, *triangle, *rectangle;
float camera_rotation_angle = 45.0f;
GraphicalObject Block, Board[10][10];

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
           default:
                break;
       }
   }
   else if (action == GLFW_PRESS) {
    switch (key) {
         case GLFW_KEY_UP:
            if(flag == 0){
                flag = 1;
                Block.x_ordinate -= 0.6f;
                Block.rotator(90.0f, glm::vec3(0, 0, 1));
            }
            else if(flag == 1){
                flag = 0;
                Block.x_ordinate -= 0.3f;
                Block.translator(Block.x_ordinate, 0, Block.z_ordinate);
                Block.rotator( );
            }
            else{
                flag = 2;
                Block.x_ordinate -= 0.3f;
                Block.rotator(-90.0f, glm::vec3(1, 0, 0));
                Block.translator(Block.x_ordinate, 0, Block.z_ordinate + 0.6f);
            }
            break;
         case GLFW_KEY_DOWN:
                if(flag == 0){
                    flag = 1;
                    Block.x_ordinate += 0.3f;
                    Block.translator(Block.x_ordinate + 0.6f, 0, Block.z_ordinate);
                    Block.rotator(90.0f, glm::vec3(0, 0, 1));
                }
                else if(flag == 1){
                    flag = 0;
                    Block.x_ordinate += 0.6f;
                    Block.translator(Block.x_ordinate, 0, Block.z_ordinate);
                    Block.rotator( );
                }
                else{
                    flag = 2;
                    Block.x_ordinate += 0.3f;
                    Block.translator(Block.x_ordinate, 0, Block.z_ordinate + 0.6f);
                    Block.rotator(-90.0f, glm::vec3(1, 0, 0));
                }
                break;
         case GLFW_KEY_LEFT:
                if(flag == 0){
                    flag = 2;
                    Block.z_ordinate += 0.3f;
                    Block.translator(Block.x_ordinate, 0, Block.z_ordinate + 0.6f);
                    Block.rotator(-90.0f, glm::vec3(1, 0, 0));
                }
                else if(flag == 1){
                    flag = 1;
                    Block.z_ordinate += 0.3f;
                    Block.translator(Block.x_ordinate + 0.6f, 0, Block.z_ordinate);
                    Block.rotator(90.0f, glm::vec3(0, 0, 1));
                }
                else{
                    flag = 0;
                    Block.z_ordinate += 0.6f;
                    Block.translator(Block.x_ordinate, 0, Block.z_ordinate);
                    Block.rotator( );
                }
                break;
         case GLFW_KEY_RIGHT:
                if(flag == 0){
                    flag = 2;
                    Block.z_ordinate -= 0.6f;
                    Block.translator(Block.x_ordinate, 0, Block.z_ordinate + 0.6f);
                    Block.rotator(-90.0f, glm::vec3(1, 0, 0));
                }
                else if(flag == 1){
                    flag = 1;
                    Block.z_ordinate -= 0.3f;
                    Block.translator(Block.x_ordinate + 0.6f, 0, Block.z_ordinate);
                    Block.rotator(90.0f, glm::vec3(0, 0, 1));
                }
                else{
                    flag = 0;
                    Block.z_ordinate -= 0.3f;
                    Block.translator(Block.x_ordinate, 0, Block.z_ordinate);
                    Block.rotator( );
                }
                break;
         case GLFW_KEY_ESCAPE:
            quit(window);
             break;
       default:
            break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            break;
        default:
            break;
    }
}

void drawAxes( )
{
    static const GLfloat vertex_buffer_data [ ] = {
        0,0,0,
        0,0,0,
        5,0,0,

        0,0,0,
        0,0,0,
        0,5,0,

        0,0,0,
        0,0,0,
        0,0,5
    };

    static const GLfloat color_buffer_data [ ] = {
        0,1,1,
        0,1,1,
        0,1,1,
        
        0,1,1,
        0,1,1,
        0,1,1,

        0,1,1,
        0,1,1,
        0,1,1
    };
    axes = create3DObject(GL_TRIANGLES, 9, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void getBoard( )
{
    for ( int i = 0; i < 10; i++ )
        for( int j = 0; j < 10; j++)
            board[ i ][ j ] = rand( )%2;
    }


    void drawBoard( )
    {
        for(int i = 0; i < 10; i++ ){
            for(int j = 0; j < 10; j++ ){
                Board[ i ][ j ].translator(Board[ i ][ j ].x_ordinate,
                                                       Board[ i ][ j ].y_ordinate,
                                                       Board[ i ][ j ].z_ordinate);   
                Board[ i ][ j ].render( );
            }
        }
    }

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window, float x, float y, float w, float h)
{
    int fbwidth, fbheight;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    glViewport((int)(x*fbwidth), (int)(y*fbheight), (int)(w*fbwidth), (int)(h*fbheight));


    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram(programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye(3*cos((float)camera_rotation_angle*M_PI/180.0f), 3,  3*sin((float)camera_rotation_angle*M_PI/180.0f));
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);

    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projectionO * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model
    /*
    Load identity to model matrix
    Matrices.model = glm::mat4(1.0f);

    Render your scene
    glm::mat4 translateTriangle = glm::translate (tri_pos); // glTranslatef
    glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
    glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
    Matrices.model *= triangleTransform; 
    MVP = VP * Matrices.model; // MVP = p * V * M

    Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(triangle);

    Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
    glPopMatrix ();
    */

    drawBoard( );

    Matrices.model = glm::mat4(1.0f);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(axes);

    Block.render( );
    
    // Increment angles
    float  increments = 1;

    // camera_rotation_angle++; // Simulating camera rotation
    //  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
    //  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
       exit(EXIT_FAILURE);
       glfwTerminate();
   }

   glfwMakeContextCurrent(window);
    //    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
   glfwSwapInterval( 1 );
   glfwSetFramebufferSizeCallback(window, reshapeWindow);
   glfwSetWindowSizeCallback(window, reshapeWindow);
   glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    drawAxes();
    getBoard();

    float x_ordinate = 0.0f, y_ordinate = 0.0f, z_ordinate = 0.0f;
    for(int i = 0; i < 10; i++ ){
         x_ordinate = 0.0f;
        for(int j = 0; j < 10;j++){
            if((i + j) % 2 == 0){
                VAO *cell = createCell(0.3, 0.3, -0.1, 1, 0, 0);
                GraphicalObject temp = GraphicalObject(x_ordinate, y_ordinate, z_ordinate, 0, 'r');
                temp.object = cell;
                Board[ i ][ j ] = temp;
            }
            else{
                VAO *cell = createCell(0.3, 0.3, -0.1, 0, 1, 0);
                GraphicalObject temp = GraphicalObject(x_ordinate, y_ordinate, z_ordinate, 0, 'g');
                temp.object = cell;
                Board[ i ][ j ] = temp;
            }
            x_ordinate += 0.3f;
        }
        z_ordinate += 0.3f;
    }
    
    GraphicalObject temp = GraphicalObject( );
    temp.object  = createCell(0.3, 0.3, 0.6, 1, 1, 0);
    Block = temp;

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    // cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    // cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    // cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    // cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    int width = 1000;
    int height = 1000;
    proj_type = 0;
    tri_pos = glm::vec3(0, 0, 0);
    rect_pos = glm::vec3(0, 0, 0);

    GLFWwindow* window = initGLFW(width, height);
    initGLEW();
    initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

	// clear the color and depth in the frame buffer
       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // OpenGL Draw commands
       draw(window, 0, 0, 1, 1);
	// proj_type ^= 1;
	// draw(window, 0.5, 0, 0.5, 1);
	// proj_type ^= 1;

        // Swap Frame Buffer in double buffering
       glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
       glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    //    exit(EXIT_SUCCESS);
}
