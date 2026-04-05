#version 430 core

in vec2 TexCoord;
out vec4 FragColor;

uniform float saturation;
uniform sampler2D uTexture;

float hue2rgb(float p, float q, float t) {
    if (t < 0.0) t += 1.0;
    if (t > 1.0) t -= 1.0;
    if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
    if (t < 1.0/2.0) return q;
    if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
    return p;
}

vec3 rgbToHsl(vec3 rgb)
{
    float minVal = min(min(rgb.r, rgb.g), rgb.b);
    float maxVal = max(max(rgb.r, rgb.g), rgb.b);
    float delta = maxVal - minVal;

    float l = (maxVal + minVal) / 2.0;

    float s = 0.0;
    if (delta > 0.0) {
        s = delta / (1.0 - abs(2.0 * l - 1.0));
    }

    float h = 0.0;
    if (delta > 0.0) {
        if (maxVal == rgb.r) {
            h = (rgb.g - rgb.b) / delta + (rgb.g < rgb.b ? 6.0 : 0.0);
        } else if (maxVal == rgb.g) {
            h = (rgb.b - rgb.r) / delta + 2.0;
        } else {
            h = (rgb.r - rgb.g) / delta + 4.0;
        }
        h /= 6.0;
    }

    return vec3(h, s, l);
}

vec3 hslToRgb(vec3 hsl)
{
    float h = hsl.x;
    float s = hsl.y;
    float l = hsl.z;

    h = clamp(h, 0.0, 1.0);
    s = clamp(s, 0.0, 1.0);
    l = clamp(l, 0.0, 1.0);

    float r, g, b;

    if (s == 0.0) {
        r = g = b = l;
    } else {
        float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
        float p = 2.0 * l - q;

        r = hue2rgb(p, q, h + 1.0/3.0);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0/3.0);
    }

    return vec3(r, g, b);
}

void main()
{
    vec4 color = texture(uTexture, TexCoord);
    vec3 hslColor = rgbToHsl(color.xyz);
    hslColor.y = hslColor.y * saturation;

    FragColor = vec4(hslToRgb(hslColor).xyz, color.w);
}
