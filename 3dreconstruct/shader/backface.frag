#version 400
// for raycasting

in vec3 Color;
layout (location = 0) out vec4 FragColor;


void main()
{
    FragColor = vec4(Color, 1.0);
}
