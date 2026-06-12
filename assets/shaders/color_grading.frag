// Color grading applied to the final virtual-screen blit.
// Order: tint and brightness in linear-ish space, then saturation, then
// contrast around mid-gray.

uniform sampler2D texture;
uniform vec3 tint;        // per-channel multiplier
uniform float brightness; // linear multiplier, 1.0 = unchanged
uniform float saturation; // 0.0 = grayscale, 1.0 = unchanged
uniform float contrast;   // pivot at mid-gray, 1.0 = unchanged

void main()
{
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    vec3 color = pixel.rgb * tint * brightness;

    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, saturation);

    color = (color - 0.5) * contrast + 0.5;

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), pixel.a);
}
