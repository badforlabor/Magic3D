#pragma once
#include "ITriMesh.h"
#include <vector>

namespace GPP
{
    class GPP_EXPORT SimplifyMesh
    {
    public:
        SimplifyMesh();
        ~SimplifyMesh();

        static ErrorCode QuadricSimplify(ITriMesh* triMesh, Int targetVertexCount, const std::vector<Real>* vertexFields = NULL, 
            std::vector<Real>* simplifiedVertexFields = NULL);
    };
}
