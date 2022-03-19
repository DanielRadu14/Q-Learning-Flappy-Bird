#version 330

uniform sampler2D texture_1;

layout(location = 0) out vec4 out_color;

in vec2 texcoord;

void main()
{
    out_color =  texture2D(texture_1, texcoord);
}