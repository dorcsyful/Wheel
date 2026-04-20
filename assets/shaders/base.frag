//precision mediump float;
//void main() {
//    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
//}

precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
void main()
{
    vec4 color = texture2D( s_texture, v_texCoord );
    //if(color.a < 0.1) discard;
  gl_FragColor = color;
}