#version 450
layout(vertices = 3) out;
layout(location = 0) in vec3 inColor[];
layout(location = 1) in vec2 inUV[];
layout(location = 0) out vec3 outColor[];
layout(location = 1) out vec2 outUV[];
void main(){
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outColor[gl_InvocationID] = inColor[gl_InvocationID];
    outUV[gl_InvocationID] = inUV[gl_InvocationID];

    if(gl_InvocationID == 0){
        gl_TessLevelOuter[0] = 4.0;
        gl_TessLevelOuter[1] = 4.0;
        gl_TessLevelOuter[2] = 4.0;
        gl_TessLevelInner[0] = 4.0;
    }
}