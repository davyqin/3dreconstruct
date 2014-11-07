uniform sampler2D texture;
uniform vec2 texOffset[9];
uniform float horKernel[9];
uniform float verKernel[9];
uniform bool edgeDetection;

varying vec2 v_texcoord;

//! [0]
void main()
{
  vec4 fragColor = vec4(0.0);
  if (edgeDetection) {
    int i = 0;
    vec4 hSum = vec4(0.0);
    vec4 vSum = vec4(0.0);
    for (i = 0; i < 9; ++i)
    {
      vec4 tex = texture2D(texture, v_texcoord.st + texOffset[i]);
      hSum = hSum + tex * horKernel[i];
      vSum = vSum + tex * verKernel[i];
    }

    fragColor = sqrt(hSum*hSum + vSum*vSum);
  }
  else {
    fragColor = texture2D(texture, v_texcoord);
  }

  // Set fragment color from texture
  gl_FragColor = fragColor;
}
//! [0]
