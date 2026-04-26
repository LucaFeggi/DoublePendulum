#version 450

layout(location = 0) in vec2 in_pos;       // from vertex buffer
layout(location = 1) in vec3 in_color;     // from vertex buffer

layout(location = 0) out vec3 v_color;

layout(set = 0, binding = 2) uniform UBO {
    mat4 u_mvp;
} ubo;

void main() {
    vec4 pos = vec4(in_pos.xy, 0.0, 1.0);
    gl_Position = ubo.u_mvp * pos;
    v_color = in_color;
}
