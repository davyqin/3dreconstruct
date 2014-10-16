uniform sampler2D texture;
uniform vec2 texOffset[9];
uniform float kernelValue[9];
uniform bool edgeDetection;

varying vec2 v_texcoord;

//! [0]
void main()
{
  vec4 fragColor = vec4(0.0);
  if (edgeDetection) {
    int i = 0;
    vec4 sum = vec4(0.0);
    for (i = 0; i < 9; ++i)
    {
      vec4 tmp = texture2D(texture, v_texcoord.st + texOffset[i]);
      sum += tmp * vec4(vec3(kernelValue[i]), 1.0);
    }
    
    fragColor = sum;
  }
  else {
    fragColor = texture2D(texture, v_texcoord);
  }

  // Set fragment color from texture
  gl_FragColor = fragColor;
}
//! [0]
