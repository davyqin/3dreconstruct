uniform mat4 MVP;
attribute vec4 vPos;

void main()
{
  gl_Position = MVP * vPos;
}
