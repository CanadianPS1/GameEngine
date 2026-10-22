#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 worldPosition;
layout(location = 3) out vec3 worldNormal;
layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projectionViewMatrix;
    vec3 directionToLight;
    float defultLight;
} ubo;
layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;
void main(){
    worldPosition = vec3(push.modelMatrix * vec4(position, 1.0));
    worldNormal = normalize(mat3(push.normalMatrix) * normal);
    gl_Position = ubo.projectionViewMatrix * vec4(worldPosition, 1.0);
    float ambient = ubo.defultLight > -1.0 ? ubo.defultLight : 0.05;
    float lightIntensity = ambient + max(dot(worldNormal, ubo.directionToLight), 0.0);
    fragColor = lightIntensity * color;
    fragUV = uv;
}