#version 420

layout(location = 0) in vec3 colorpos;

layout(location = 0) out vec4 outColor;

void main() {
 outColor = vec4(colorpos, 1.0f);
}
