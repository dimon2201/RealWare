@echo off

glslc --target-env=vulkan1.2 passthrough.vert -o passthrough.vert.spv
glslc --target-env=vulkan1.2 passthrough.frag -o passthrough.frag.spv

glslc --target-env=vulkan1.2 passthrough_final.vert -o passthrough_final.vert.spv
glslc --target-env=vulkan1.2 passthrough_final.frag -o passthrough_final.frag.spv