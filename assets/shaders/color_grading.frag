// Post effects applied to the final virtual-screen blit: optional heat
// distortion (UV displacement), then color grading — tint and brightness,
// saturation, contrast around mid-gray.

uniform sampler2D texture;
uniform vec3 tint;          // per-channel multiplier
uniform float brightness;   // linear multiplier, 1.0 = unchanged
uniform float saturation;   // 0.0 = grayscale, 1.0 = unchanged
uniform float contrast;     // pivot at mid-gray, 1.0 = unchanged
uniform float heatStrength; // horizontal UV amplitude; 0.0 disables the haze
uniform float time;         // seconds, wrapped on the CPU side

void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    if (heatStrength > 0.0)
    {
        // Two horizontal waves at different frequencies plus a faint vertical
        // wobble: hot air rising. Time speeds must stay multiples of 0.1 so
        // the CPU-side time wrap (mod 200*pi) is seamless.
        float wave = sin(uv.y * 90.0 + time * 3.0)
                   + 0.5 * sin(uv.y * 47.0 - time * 5.1);

        uv.x += wave * heatStrength;
        uv.y += 0.4 * heatStrength * sin(uv.x * 70.0 + time * 2.3);
    }

    vec4 pixel = texture2D(texture, uv);

    vec3 color = pixel.rgb * tint * brightness;

    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, saturation);

    color = (color - 0.5) * contrast + 0.5;

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), pixel.a);
}
