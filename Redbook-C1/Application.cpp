#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define BUFFER_OFFSET(a) ((void*)(a))

/*Global variables*/
enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0 }; // Note how this matches the value in the vertex shader

GLuint VAOs[ NumVAOs ];
GLuint Buffers[ NumBuffers ];

const GLuint NumVertices = 6;

/*This struct is used in loading the shaders.*/
typedef struct {
	GLenum       type;
	const char* filename;
	GLuint       shader;
} ShaderInfo;

/*This is a helper method for loading shaders. This routine, along with LoadShaders, and the above struct, were pulled out of LoadShaders.cpp and LoadShaders.h
from the Redbook sample code.*/
static const GLchar*
ReadShader(const char* filename)
{
#ifdef WIN32
	FILE* infile;
	fopen_s(&infile, filename, "rb");
#else
	FILE* infile = fopen(filename, "rb");
#endif // WIN32

	if (!infile) {
#ifdef _DEBUG
		std::cerr << "Unable to open file '" << filename << "'" << std::endl;
#endif /* DEBUG */
		return NULL;
	}

	fseek(infile, 0, SEEK_END);
	int len = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	GLchar* source = new GLchar[len + 1];

	fread(source, 1, len, infile);
	fclose(infile);

	source[len] = 0;

	return const_cast<const GLchar*>(source);
}

GLuint
LoadShaders(ShaderInfo* shaders)
{
	if (shaders == NULL) { return 0; }

	GLuint program = glCreateProgram();

	ShaderInfo* entry = shaders;
	while (entry->type != GL_NONE) {
		GLuint shader = glCreateShader(entry->type);

		entry->shader = shader;

		const GLchar* source = ReadShader(entry->filename);
		if (source == NULL) {
			for (entry = shaders; entry->type != GL_NONE; ++entry) {
				glDeleteShader(entry->shader);
				entry->shader = 0;
			}

			return 0;
		}

		glShaderSource(shader, 1, &source, NULL);
		delete[] source;

		glCompileShader(shader);

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {

			return 0;
		}

		glAttachShader(program, shader);

		++entry;
	}

	glLinkProgram(program);

	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked) {
#ifdef _DEBUG
		GLsizei len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(program, len, &len, log);
		std::cerr << "Shader linking failed: " << log << std::endl;
		delete[] log;
#endif /* DEBUG */

		for (entry = shaders; entry->type != GL_NONE; ++entry) {
			glDeleteShader(entry->shader);
			entry->shader = 0;
		}

		return 0;
	}

	return program;
}

/*This routine initializes all of the relevant OpenGL data for use in rendering later.*/
void init(void) {
	/*Allocate one or more vertex-array objects. NumVAOs amount of array objects names
	are allocated. The names of the objects are returned to VAOs.*/
	glGenVertexArrays(NumVAOs, VAOs);

	/*Up to here, we have names to reference objects but not memory allocated to them and connected to the name.
	The allocation of memory in association with the name is binding.
	1) Using the value array returned from glGenVertextArrays and it is non-zero, create a new vertex-array object and assign that name.
	2) When binding to a previously created vertex-array object, that vertex-array object becomes active and the vertex-array
	state is affected.
	3) When using an array value of 0, OpenGL stops using application-allocated vertex-array objects and returns to the default state.
	The first VAO, labeled triangles from the VAO_IDs enum, is bound.
	*/
	glBindVertexArray(VAOs[Triangles]);

	/*Note that x and y values are bounded by -1 and 1. This range
	is known as normalized-device coordinates.*/
	GLfloat vertices[NumVertices][2] = {
		{-0.95, -0.90}, // Triangle 1
		{0.85, -0.90},
		{-0.90, 0.85},
		{0.90, -0.85}, // Triangle 2
		{0.90, 0.90},
		{-0.85, 0.90},
	};

	/*Allocate NumBuffers amount of buffer names into the array Buffers.
	A vertex-array object hold data related to a collection of vertices
	using buffer objects managed by the vertex-array object.
	Here is the first step to allocating buffer objects directly.*/
	glGenBuffers(NumBuffers, Buffers);

	/*Bring the buffers for which names were just allocated into existence.
	"ArrayBuffer" is an enumeration constant. GL_ARRAY_BUFFER specifies
	that vertex data will be stored.*/
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);

	/*Transfer the vertex data from our objects into the buffer object.
	The routin allocates storage and copies the data.
	Memory is statically allocated with vertices and we use GL_ARRAY_BUFFER
	for vertex-attribute data. The size of the data is straightforwardly
	given by the sizeof. The data will not change during the life of the program,
	so GL_STATIC_DRAW is used.*/
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
		vertices, GL_STATIC_DRAW);

	/*Array of shaders to be processed by LoadShaders*/
	ShaderInfo shaders[] = {
	{ GL_VERTEX_SHADER, "triangles.vert" },
	{ GL_FRAGMENT_SHADER, "triangles.frag" },
	{ GL_NONE, NULL }
	};

	/*LoadShaders is a helper routine from the Red Book github repository. */
	GLuint program = LoadShaders(shaders);
	/*At least a fragment and vertex shader are mandatory. Finish loading them.*/
	glUseProgram(program);

	/*Connect a conduit between the application and the first shader - the vertex shader.
	This connects the "in" variables of the vertex shader to a vertex-attribute array.
	Note that the index value of 0 matches the vertex shader.
	The size matches the number of values in each vertex in the vertices array.
	Use GL_FALSE because we do not want positional values constrained from -1 to 1, nor
	are they integer types.
	Our data are tightly packed in an array.
	The data start at the first byte. */
	glVertexAttribPointer(vPosition, 2, GL_FLOAT,
		GL_FALSE, 0, BUFFER_OFFSET(0));

	/*Enable our vertex-attribute array.*/
	glEnableVertexAttribArray(vPosition);

	/*Change the clear color state variable of OpenGL to white.*/
	// glClearColor(1, 1, 1, 1);
}

/*This routine is pretty standard for rendering.*/
void display(void) {
	
	/*Begin by clearing the color buffer with the default black unless the state variable was changed elsewhere..*/
	glClear(GL_COLOR_BUFFER_BIT);

	/*Select the vertex-array that we want to use as vertex data.*/
	glBindVertexArray(VAOs[Triangles]);

	/*Send the vertex data to the OpenGL pipeline. Here we request that
	individual triangles are rendered via the rendering mode with an offset
	based on the buffer offset of glVertexAttribPointer() and continuing for NumVertices.*/
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// Use lines as geometric primitives. There should be three lines.
	// glDrawArrays(GL_LINES, 0, NumVertices); 
	// 
	// Use points as geometric primitives. There should be six points. 
	// glDrawArrays(GL_POINTS, 0, NumVertices); 

	/*Request that any pending OpenGL calls are flushed to the OpenGL server and processed.
	This guarantees that commands complete in a finite time.*/
	// glFlush();

	// Use this method is like glFlush but waits until
	// all commands to the OpenGL server have returned, allowing you to measure performance.
	// Do not put it in production code.
	glFinish();
}

// Note that the Nuget package manager was used to install FreeGlut and GLEW with the commands "Install-Package freeglut" and "Install-Package glew"
int main(int argc, char** argv) {
	/*Initialize the GLUT library and process the command-line arguments
	given to the program. This needs to be the first GLUT function called
	to set up GLUT data structures.*/
	glutInit(&argc, argv);

	/*This sets up the type of window we want to use with our application.
	Here it is specified that the window shall use the RGBA color space.*/
	glutInitDisplayMode(GLUT_RGBA);

	/*Specify the size of the window.*/
	glutInitWindowSize(512, 512);

	/*The following two calls together specify the OpenGL context, which is OpenGL's
	internal data structure for keeping track of state settings and operations.
	Here we request an OpenGL 4.3 core profile for our context.*/
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	/*A window is created if it's possible to do so using the display mode you
	requested with glutInitDisplayMode. OpenGL functions can be used after the window
	is created by GLUT as it also creates an OpenGL context.*/
	glutCreateWindow(argv[0]);

	/*Initialize another OpenGL helper library called GLEW. It simplified dealing with accessing functions and other
	programming phenomena introduced by various operating systems with OpenGL.*/
	if (glewInit()) {
		std::cerr << "Unable to initialize GLEW...exiting" << std::endl;
		exit(EXIT_FAILURE);
	}

	init(); // This initializes all of our relevant OpenGL data for use in rendering later

	/*This sets up the display callback, which is the routine GLUT will call when
	the contents of the window need to be updated. Display is a pointer to a function.
	Callbacks are used for other things like user input and window resizing.*/
	glutDisplayFunc(display);

	/*This is an infinite loop that works with the window and operating systems to
	process user input and other operations like that. This determines that a window needs to be repainted.
	Commands after it are not executed.*/
	glutMainLoop();
}