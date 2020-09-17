glslc.exe ./UnlitColor.vert -o ./UnlitColorVert.spv
glslc.exe ./UnlitColor.frag -o ./UnlitColorFrag.spv

glslc.exe ./UnlitTintedTextured.vert -o ./UnlitTintedTexturedVert.spv
glslc.exe ./UnlitTintedTextured.frag -o ./UnlitTintedTexturedFrag.spv

glslc.exe ./WavySurface.vert -o ./WavySurfaceVert.spv
glslc.exe ./WavySurface.frag -o ./WavySurfaceFrag.spv

glslc.exe ./BumpySurface.vert -o ./BumpySurfaceVert.spv
glslc.exe ./BumpySurface.frag -o ./BumpySurfaceFrag.spv

glslc.exe ./MotherShip.vert -o ./MotherShipVert.spv
glslc.exe ./MotherShip.frag -o ./MotherShipFrag.spv

pause
