attribute vec3 VertexPosition;
attribute vec2 VertexTexCoord;

uniform mat4 MVP;
varying vec2 v_texcoord;

void main()
{
    v_texcoord = VertexTexCoord;

    gl_Position = MVP * vec4(VertexPosition,1.0);
}
