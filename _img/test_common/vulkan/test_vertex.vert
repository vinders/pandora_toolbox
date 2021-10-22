#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec2 outPosition;
layout(location = 1) out vec3 outColor;

void VSMain() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    outPosition = inPosition;
    outColor = inColor;
}
