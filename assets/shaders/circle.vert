attribute vec4 a_position;   // [x, y, z] from the shared quad VBO
attribute vec2 a_texCoord;   // 0..1 across the rectangle
uniform mat4 u_viewProj;     // shared by every object in a frame
uniform mat2 u_model;        // per-object 2D linear part
uniform vec2 u_translate;    // per-object world position
varying vec2 v_texCoord;

void main() {
    vec2 world = u_model * a_position.xy + u_translate;
    gl_Position = u_viewProj * vec4(world, a_position.z, 1.0);
    v_texCoord = a_texCoord;
}