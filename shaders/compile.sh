glslc ./UnlitColor.vert -o ./UnlitColor.spv
glslc ./UnlitColor.frag -o ./UnlitColor.spv

glslc ./UnlitTintedTextured.vert -o ./UnlitTintedTexturedVert.spv
glslc ./UnlitTintedTextured.frag -o ./UnlitTintedTexturedFrag.spv

glslc ./WavySurface.vert -o ./WavySurfaceVert.spv
glslc ./WavySurface.frag -o ./WavySurfaceFrag.spv

glslc ./BumpySurface.vert -o ./BumpySurfaceVert.spv
glslc ./BumpySurface.frag -o ./BumpySurfaceFrag.spv
