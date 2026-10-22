/usr/bin/glslangValidator -V shaders/ThreeDVert.vert -o shaders/ThreeDVert.vert.spv
/usr/bin/glslangValidator -V shaders/TwoDVert.vert -o shaders/TwoDVert.vert.spv
/usr/bin/glslangValidator -V shaders/Frag.frag -o shaders/Frag.frag.spv
/usr/bin/glslangValidator -V --target-env vulkan1.2 shaders/Task.task -o shaders/Task.task.spv
/usr/bin/glslangValidator -V --target-env vulkan1.2 shaders/Mesh.mesh -o shaders/Mesh.mesh.spv
/usr/bin/glslangValidator -V --target-env vulkan1.2 shaders/TessEval.tese -o shaders/TessEval.tese.spv
/usr/bin/glslangValidator -V --target-env vulkan1.2 shaders/TessControl.tesc -o shaders/TessControl.tesc.spv
/usr/bin/glslangValidator -V shaders/MeshFrag.frag -o shaders/MeshFrag.frag.spv
