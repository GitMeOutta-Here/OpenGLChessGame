#version 460 core

out vec4 FragColor;

uniform vec3 squareColor;

void main()
{
    FragColor = vec4(squareColor, 1.0);
}
