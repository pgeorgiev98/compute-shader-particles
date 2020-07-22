#version 430
out vec2 uv;
in vec2 pos;

void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    uv = 0.5 * pos + 0.5;
}
