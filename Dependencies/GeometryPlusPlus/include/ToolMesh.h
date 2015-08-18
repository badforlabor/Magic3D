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
    extern GPP_EXPORT HalfMesh* ConvertITriMeshToHalfMesh(const ITriMesh* triMesh);

    //triMesh needs to be initialized as a blank entiry
    extern GPP_EXPORT void ConvertHalfMeshToITriMesh(HalfMesh* halfMesh, ITriMesh** triMesh);
}
