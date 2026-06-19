#extension GL_OES_standard_derivatives : enable
precision mediump float;
varying vec2 v_texCoord;
uniform vec4 u_color;

void main()
{
    vec2 p = (v_texCoord - 0.5) * 2.0;
    float d = length(p);

    float aa = fwidth(d);
    float alpha = 1.0 - smoothstep(1.0 - aa, 1.0 + aa, d);

    gl_FragColor = vec4(u_color.rgb, u_color.a * alpha);
}