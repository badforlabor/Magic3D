#pragma once
#include "GPP.h"

namespace MagicApp
{
    class MagicMesh : public GPP::ITriMesh
    {
    public:
        MagicMesh(GPP::ITriMesh* triMesh);

        virtual GPP::Int GetVertexCount(void) const;
        virtual GPP::Int GetTriangleCount(void) const;

        virtual GPP::Vector3 GetVertexCoord(GPP::Int vid) const;
        virtual void SetVertexCoord(GPP::Int vid, const GPP::Vector3& coord);
        virtual GPP::Vector3 GetVertexNormal(GPP::Int vid) const;
        virtual void SetVertexNormal(GPP::Int vid, const GPP::Vector3& normal);

        // vertexIds are in a consistent order in all connected triangles
        virtual void GetTriangleVertexIds(GPP::Int fid, GPP::Int vertexIds[3]) const;
        // make sure vertexIdx are in a consistent order in its connected triangles
        virtual void SetTriangleVertexIds(GPP::Int fid, GPP::Int vertexId0, GPP::Int vertexId1, GPP::Int vertexId2);
        virtual GPP::Vector3 GetTriangleNormal(GPP::Int fid) const;
        virtual void SetTriangleNormal(GPP::Int fid, const GPP::Vector3& normal);
        
        // Return inserted triangle id
        virtual GPP::Int InsertTriangle(GPP::Int vertexId0, GPP::Int vertexId1, GPP::Int vertexId2);
        // Return inserted vertex id
        virtual GPP::Int InsertVertex(const GPP::Vector3& coord);
        
        // Be careful: if you swap vertex and popback them, then vertex index after the deleted vertices will be changed.
        // If you want to delete some vertices, please use api DeleteTriMeshVertex in ToolMesh.h which is still developping.
        virtual void SwapVertex(GPP::Int vertexId0, GPP::Int vertexId1); 
        virtual void PopbackVertices(GPP::Int popCount);
        virtual void SwapTriangles(GPP::Int fid0, GPP::Int fid1);
        virtual void PopbackTriangles(GPP::Int popCount);

        virtual void UpdateNormal(void);
        // Clear all geometry information to initial state
        virtual void Clear(void);

        virtual ~MagicMesh();

        void SetImageColorIds(std::vector<GPP::ImageColorId>* imageColorIds);
        void SetColorIds(std::vector<int>* colorIds);

    private:
        GPP::ITriMesh* mTriMesh;
        std::vector<GPP::ImageColorId>* mImageColorIds;
        std::vector<int>* mColorIds;
    };
}
