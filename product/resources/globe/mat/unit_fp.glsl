#version 330 core
uniform float viewingPlayerID;
uniform float ownerID;

void main()
{
    if (abs(ownerID - viewingPlayerID) < 0.5)
    {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
    else
    {
        gl_FragColor = vec4(0.2, 0.04, 0.04, 1.0);
    }
}