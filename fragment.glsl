#version 430
in vec2 uv;
uniform sampler2D sampler;
out vec3 color;

void main() {
    color = texture(sampler, uv).rgb;
}
