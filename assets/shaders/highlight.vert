attribute vec4 a_position;
attribute vec2 a_texCoord;
uniform mat4 u_viewProj;
uniform mat2 u_model;        // enlarged by the margin
uniform vec2 u_translate;
uniform vec2 u_uvScale;      // (size + 2*margin)/size
varying vec2 v_texCoord;

void main()
{
    vec2 world = u_model * a_position.xy + u_translate;
    gl_Position = u_viewProj * vec4(world, a_position.z, 1.0);

    // The quad is drawn larger than the sprite by `margin` on every side. Remap
    // the UVs so the texture sits inset in the centre; the margin ring then maps
    // to UVs outside [0,1], which the fragment shader treats as transparent.
    v_texCoord = (a_texCoord - 0.5) * u_uvScale + 0.5;
}