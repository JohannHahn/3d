#version 330 core

in vec2 uv;
uniform sampler2D screenTex;

out vec4 fragColor;

void main() {
    vec2 uv_inv = {uv.x, -uv.y};
    fragColor = texture(screenTex, uv_inv);
}
