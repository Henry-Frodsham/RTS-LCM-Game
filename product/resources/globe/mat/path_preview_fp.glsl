#version 330 core

uniform float viewingPlayerID;
uniform float ownerID;

void main()
{
    if (abs(ownerID - viewingPlayerID) >= 0.5)
    {
        discard;
    }
    gl_FragColor = vec4(0.1, 0.1, 0.8, 1.0);
}
