// This is a pass-through shader. It only copies input data to output data.

#version 430 core // Use the version GLSL compatible with OpenGL core profile version 4.30

// Every time the shader executes, it merely sees its input variables populated with data.
// It is our reponsibility to connect the shader plumbing.
// vPosition is an input variable describing a vertex's position
// vec4 is a vPositions type, and a is 4-component vector of floating point values specifically
// In the main program only x and y are specified, so z and w will be 0 and 1 by default.
// in obviously denotes data entering
// "layout(location=0)" is a layout qualifier which sets the location attribute of vPosition to 0.
layout( location = 0 ) in vec4 vPosition;

// Every shader must have a main routine. This one just copies the input vertex position to the
// special vertex-shader output gl_Position provided by OpenGL.
void
main()
{
    gl_Position = vPosition;
}