#pragma once
#include "ITriMesh.h"

namespace GPP
{
    class GPP_EXPORT ConsolidateMesh
    {
    public:
        ConsolidateMesh();
        ~ConsolidateMesh();

        static Int LaplaceSmooth(ITriMesh* triMesh, Real percentage, Int times, bool keepBoundary);
    };
}
