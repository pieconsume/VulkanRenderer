#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 incolorpos;

layout(location = 0) out vec3 outcolorpos;

void main() {
 outcolorpos = incolorpos;
 gl_Position = vec4(position, 1.0);
}
