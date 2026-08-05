#version 330 core

uniform float viewingPlayerID;
uniform float ownerID;

void main()
{
    if (abs(ownerID - viewingPlayerID) < 0.5)
    {
        // owner's viewport only: selection highlight
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else
    {
        // must stay byte-identical to unit_fp.glsl's non-owned colour
        gl_FragColor = vec4(0.2, 0.04, 0.04, 1.0);
    }
}