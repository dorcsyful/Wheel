precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform vec4 u_color;
uniform vec2 u_thicknessUV;    // outline thickness in UV units, per axis

float alphaAt(vec2 a_uv)
{
    vec2 inb = step(vec2(0.0), a_uv) * step(a_uv, vec2(1.0));
    return texture2D(s_texture, a_uv).a * inb.x * inb.y;
}

void main()
{
    // The real sprite is drawn on top in its own pass, so skip the interior.
    if (alphaAt(v_texCoord) > 0.5)
        discard;

    // Ring sample: this transparent pixel becomes outline-coloured if any opaque
    // pixel lies within `thickness`. Per-axis radius keeps the thickness even in
    // world space even when the texture is non-square.
    const int TAPS = 16;
    for (int i = 0; i < TAPS; i++)
    {
        float ang = 6.2831853 * float(i) / float(TAPS);
        vec2 offset = vec2(cos(ang), sin(ang)) * u_thicknessUV;
        if (alphaAt(v_texCoord + offset) > 0.5)
        {
            gl_FragColor = u_color;
            return;
        }
    }
    discard;
}