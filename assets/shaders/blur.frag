// One directional pass of a separable 5-tap Gaussian blur (sigma ~1.5).
// Run twice per iteration: horizontally, then vertically. Repeated
// iterations widen the blur (sigma grows with the square root).

uniform sampler2D texture;
uniform vec2 direction; // one texel step: (1/width, 0) or (0, 1/height)

void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    vec4 sum = texture2D(texture, uv) * 0.2921;
    sum += (texture2D(texture, uv + direction)       + texture2D(texture, uv - direction))       * 0.2339;
    sum += (texture2D(texture, uv + direction * 2.0) + texture2D(texture, uv - direction * 2.0)) * 0.1201;

    gl_FragColor = sum;
}
