# Magic3D
Magic 3D Applications.

Online document: http://threepark.net/magic3d

Run Software:

1. run setbin.bat

2. run bin/release/Magic3D.exe

3. configure the rendering from the ogre dialog, then it will save the configuration in ogre.cfg. You can change the configuration by modifying ogre.cfg or just delete it and then configure from ogre dialog

Build Code:
1. Configuration: VS2012 Release x64.

Commands:

PointShop:

Import: OBJ

Export: OBJ, PLY

Smooth

Sampling: Uniform sampling

Calculate point cloud normal

Reverse point cloud normal

Mesh reconstructiong(needs point cloud normal)


MeshShop:

Import: OBJ, STL, OFF

Export: OBJ, STL

Consolidate topology(Topo): Remove singular vertex and non-manifold structure

Reverse direction

Consolidate geometry(Geo): Optimize degenerate and flipped triangles

Smooth

Enhance mesh detail

Loop Subdivision(Loop)

C-C Subdivision(C-C)

Refine Mesh(Refine): Add vertices on triangles

Simplify Mesh(Sim)

Parameterization(Flat): Angle preserving parameterization

Sampling: Sample vertex to point cloud format

