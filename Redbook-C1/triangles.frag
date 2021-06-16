#version 430 core // Match the version 4.30 core profile of OpenGL

out vec4 fColor; // Output fragment color as a 4-component vector

void main()
{
    // Each fragment is assigned this vector of four values in the RGB color space.
    // The fourth value is alpha which measures transparency.  
    fColor = vec4(0.0, 0.0, 1.0, 1.0);
}