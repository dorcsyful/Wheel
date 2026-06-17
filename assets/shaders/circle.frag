precision mediump float;
varying vec2 v_texCoord;
uniform vec2 u_size;
uniform vec4 u_color;

void main() {
    vec2 p = (v_texCoord - 0.5) * u_size;
    float radius = 0.5 * min(u_size.x, u_size.y);
    float alpha = 1.0 - smoothstep(radius - 1.0, radius + 1.0, length(p));
    gl_FragColor = vec4(u_color.rgb, u_color.a * alpha);
}