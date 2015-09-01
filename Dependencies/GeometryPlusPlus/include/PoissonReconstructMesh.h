#pragma once
#include "Mesh.h"
#include "IPointCloud.h"

namespace GPP
{
    class PoissonReconstructMeshImpl;
    class GPP_EXPORT PoissonReconstructMesh
    {
    public:
        PoissonReconstructMesh();
        ~PoissonReconstructMesh();

        Int Init(IPointCloud* pointCloud);
        TriMesh* ReconstructCloseMesh();
        TriMesh* ReconstructOpenMesh();
        void Free(void);

    private:
        PoissonReconstructMeshImpl* mpImpl;
    };
}
