#version 330 core

in vec4 vertex_color;
in vec2 texture_uv;

out vec4 frag_color;

uniform sampler2D in_texture;

void main() {
    frag_color = texture(in_texture, texture_uv);
}
