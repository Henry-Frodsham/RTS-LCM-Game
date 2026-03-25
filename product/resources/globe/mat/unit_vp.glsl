#version 330 core
attribute vec4 vertex;
uniform mat4 worldViewProj;

void main()
{

    gl_Position = worldViewProj * vertex;
}