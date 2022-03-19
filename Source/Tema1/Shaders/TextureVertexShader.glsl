#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_text_coord;
layout(location = 3) in vec3 v_color;

uniform sampler2D texture_1;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec2 texcoord;

void main()
{
    texcoord = v_text_coord;

    gl_Position = Projection * View * Model * vec4(v_position, 1.0);
}