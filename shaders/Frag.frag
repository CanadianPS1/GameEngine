#version 450
layout(set = 1, binding = 0) uniform sampler2D baseColorTexture;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 0) out vec4 outColor;
layout(push_constant) uniform Push{
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;
void main(){
    outColor = texture(baseColorTexture, fragUV) * vec4(fragColor, 1.0);
}