#version 450
layout(triangles, equal_spacing, ccw) in;
layout(location = 0) in vec3 inColor[];
layout(location = 1) in vec2 inUV[];
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
void main(){
    vec3 bary = gl_TessCoord;
    gl_Position =
        bary.x * gl_in[0].gl_Position +
        bary.y * gl_in[1].gl_Position +
        bary.z * gl_in[2].gl_Position;
    fragColor = bary.x * inColor[0] + bary.y * inColor[1] + bary.z * inColor[2];
    fragUV = bary.x * inUV[0] + bary.y * inUV[1] + bary.z * inUV[2];
}