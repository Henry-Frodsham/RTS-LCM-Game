#version 330 core

uniform float viewingPlayerID;
uniform float ownerID;

void main()
{
    if (abs(ownerID - viewingPlayerID) >= 0.5)
    {
        discard;
    }
    gl_FragColor = vec4(1.0, 0.85, 0.1, 1.0);
}
