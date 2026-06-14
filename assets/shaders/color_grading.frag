// Post effects applied to the final virtual-screen blit: optional heat
// distortion (UV displacement) or underwater effects, then color grading —
// tint and brightness, saturation, contrast around mid-gray.

uniform sampler2D texture;
uniform vec3 tint;          // per-channel multiplier
uniform float brightness;   // linear multiplier, 1.0 = unchanged
uniform float saturation;   // 0.0 = grayscale, 1.0 = unchanged
uniform float contrast;     // pivot at mid-gray, 1.0 = unchanged
uniform float heatStrength; // horizontal UV amplitude; 0.0 disables the haze
uniform float water;        // underwater effects strength; 0.0 disables them
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

    // Underwater: a slow, lazy current that gently displaces the whole image.
    if (water > 0.0)
    {
        float w = sin(uv.y * 14.0 + time * 0.7)
                + 0.6 * sin(uv.x * 11.0 - time * 0.5);
        uv.x += w * 0.0016 * water;
        uv.y += 0.5 * sin(uv.x * 9.0 + time * 0.6) * 0.0016 * water;
    }

    vec4 pixel;
    if (water > 0.0)
    {
        // Faint chromatic split, as if seen through a moving body of water.
        float ca = 0.0018 * water;
        pixel.r = texture2D(texture, uv + vec2(ca, 0.0)).r;
        pixel.g = texture2D(texture, uv).g;
        pixel.b = texture2D(texture, uv - vec2(ca, 0.0)).b;
        pixel.a = texture2D(texture, uv).a;
    }
    else
    {
        pixel = texture2D(texture, uv);
    }

    vec3 color = pixel.rgb * tint * brightness;

    // Underwater: drifting caustic light ripples added over the scene.
    if (water > 0.0)
    {
        vec2 p = uv * vec2(7.0, 5.0);
        float c = sin(p.x + time * 0.8) * sin(p.y - time * 0.6)
                + 0.5 * sin(p.x * 1.7 - time * 1.0) * sin(p.y * 1.4 + time * 0.4);
        c = max(c, 0.0);
        color += vec3(0.05, 0.08, 0.07) * c * water;
    }

    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, saturation);

    color = (color - 0.5) * contrast + 0.5;

    // Underwater: vignette plus a bit of extra murk toward the bottom (depth).
    if (water > 0.0)
    {
        vec2 d = uv - 0.5;
        float vignette = 1.0 - dot(d, d) * 0.9 * water;
        vignette *= mix(1.0, 0.8, uv.y * water);
        color *= vignette;
    }

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), pixel.a);
}
