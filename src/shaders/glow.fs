#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform vec3 spriteColor;
uniform sampler2D image;

uniform vec2 position;

void main()
{    
    vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    vec4 yellow = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    float dist = distance(position.x, TexCoords.x);
    color = mix(yellow, white, smoothstep(-1, 0, dist));
    color = mix(color, yellow, smoothstep(0, 1, dist));
}  