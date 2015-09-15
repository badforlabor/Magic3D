#pragma once
#include "GppDefines.h"

namespace GPP
{
    class ITriMesh;
    class TriMesh;
    class HalfMesh;

    //Share the same Vertex3D data
    extern GPP_EXPORT HalfMesh* ConvertTriMeshToHalfMesh(TriMesh* triMesh);

    //Share the same Vertex3D data
    extern GPP_EXPORT TriMesh*  ConvertHalfMeshToTriMesh(HalfMesh* halfMesh);

    //Seperate two meshes
    extern GPP_EXPORT HalfMesh* CreateHalfMeshFromITriMesh(const ITriMesh* triMesh);

    //triMesh should not be null
    extern GPP_EXPORT Int ConvertHalfMeshToITriMesh(HalfMesh* halfMesh, ITriMesh* triMesh);
}
