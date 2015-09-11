#pragma once
#include "Vector3.h"

namespace GPP
{
    class GPP_EXPORT ITriMesh
    {
    public:
        ITriMesh(){}

        virtual int GetVertexCount() const = 0;
        virtual Vector3 GetVertexCoord(int pid) const = 0;
        virtual void SetVertexCoord(int pid, const Vector3& coord) = 0;
        virtual Vector3 GetVertexNormal(int pid) const = 0;
        virtual void SetVertexNormal(int, const Vector3& normal) = 0;
        virtual int GetTriangleCount() const = 0;
        virtual void GetTriangleVertexIds(int fid, int vertexIds[3]) const = 0;
        virtual int InsertTriangle(int vertexId0, int vertexId1, int vertexId2) = 0;
        virtual int InsertVertex(const Vector3& coord) = 0;
        virtual void UpdateNormal(void) = 0;
        virtual void Clear(void) = 0;

        virtual ~ITriMesh(){};
    };
}
