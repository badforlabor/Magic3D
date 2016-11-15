#include "MagicMesh.h"

namespace MagicApp
{
    MagicMesh::MagicMesh(GPP::ITriMesh* triMesh) :
        mTriMesh(triMesh),
        mImageColorIds(NULL),
        mColorIds(NULL),
        mImageColorIdFlags(NULL)
    {
    }

    GPP::Int MagicMesh::GetVertexCount() const
    {
        return mTriMesh->GetVertexCount();
    }

    GPP::Vector3 MagicMesh::GetVertexCoord(GPP::Int vid) const
    {
        return mTriMesh->GetVertexCoord(vid);
    }

    void MagicMesh::SetVertexCoord(GPP::Int vid, const GPP::Vector3& coord)
    {
        mTriMesh->SetVertexCoord(vid, coord);
    }

    GPP::Vector3 MagicMesh::GetVertexNormal(GPP::Int vid) const
    {
        return mTriMesh->GetVertexNormal(vid);
    }

    void MagicMesh::SetVertexNormal(GPP::Int vid, const GPP::Vector3& normal)
    {
        mTriMesh->SetVertexNormal(vid, normal);
    }

    GPP::Int MagicMesh::GetTriangleCount() const
    {
        return mTriMesh->GetTriangleCount();
    }

    void MagicMesh::GetTriangleVertexIds(GPP::Int fid, GPP::Int vertexIds[3]) const
    {
        mTriMesh->GetTriangleVertexIds(fid, vertexIds);
    }

    void MagicMesh::SetTriangleVertexIds(GPP::Int fid, GPP::Int vertexId0, GPP::Int vertexId1, GPP::Int vertexId2)
    {
        mTriMesh->SetTriangleVertexIds(fid, vertexId0, vertexId1, vertexId2);
    }

    GPP::Vector3 MagicMesh::GetTriangleNormal(GPP::Int fid) const
    {
        return mTriMesh->GetTriangleNormal(fid);
    }

    void MagicMesh::SetTriangleNormal(GPP::Int fid, const GPP::Vector3& normal)
    {
        mTriMesh->SetTriangleNormal(fid, normal);
    }

    GPP::Int MagicMesh::InsertVertex(const GPP::Vector3& coord)
    {
        return mTriMesh->InsertVertex(coord);
    }

    void MagicMesh::Clear()
    {
        mTriMesh->Clear();
    }

    GPP::Int MagicMesh::InsertTriangle(GPP::Int vertexId0, GPP::Int vertexId1, GPP::Int vertexId2)
    {
        return mTriMesh->InsertTriangle(vertexId0, vertexId1, vertexId2);
    }

    void MagicMesh::SwapVertex(GPP::Int vertexId0, GPP::Int vertexId1)
    {
        mTriMesh->SwapVertex(vertexId0, vertexId1);
        if (mImageColorIds)
        {
            GPP::ImageColorId temp = mImageColorIds->at(vertexId0);
            mImageColorIds->at(vertexId0) = mImageColorIds->at(vertexId1);
            mImageColorIds->at(vertexId1) = temp;
        }
        if (mColorIds)
        {
            GPP::Int temp = mColorIds->at(vertexId0);
            mColorIds->at(vertexId0) = mColorIds->at(vertexId1);
            mColorIds->at(vertexId1) = temp;
        }
        if (mImageColorIdFlags)
        {
            bool temp = mImageColorIdFlags->at(vertexId0);
            mImageColorIdFlags->at(vertexId0) = mImageColorIdFlags->at(vertexId1);
            mImageColorIdFlags->at(vertexId1) = temp;
        }
    }

    void MagicMesh::PopbackVertices(GPP::Int popCount)
    {
        GPP::Int vertexCount = mTriMesh->GetVertexCount();
        mTriMesh->PopbackVertices(popCount);
        if (mImageColorIds)
        {
            mImageColorIds->erase(mImageColorIds->begin() + vertexCount - popCount, mImageColorIds->end());
        }
        if (mColorIds)
        {
            mColorIds->erase(mColorIds->begin() + vertexCount - popCount, mColorIds->end());
        }
        if (mImageColorIdFlags)
        {
            mImageColorIdFlags->erase(mImageColorIdFlags->begin() + vertexCount - popCount, mImageColorIdFlags->end());
        }
    }

    void MagicMesh::SwapTriangles(GPP::Int fid0, GPP::Int fid1)
    {
        mTriMesh->SwapTriangles(fid0, fid1);
    }

    void MagicMesh::PopbackTriangles(GPP::Int popCount)
    {
        mTriMesh->PopbackTriangles(popCount);
    }

    void MagicMesh::UpdateNormal()
    {
        mTriMesh->UpdateNormal();
    }

    MagicMesh::~MagicMesh()
    {
        mTriMesh = NULL;
        mImageColorIds = NULL;
        mColorIds = NULL;
        mImageColorIdFlags = NULL;
    }

    void MagicMesh::SetImageColorIds(std::vector<GPP::ImageColorId>* imageColorIds)
    {
        mImageColorIds = imageColorIds;
    }

    void MagicMesh::SetColorIds(std::vector<int>* colorIds)
    {
        mColorIds = colorIds;
    }

    void MagicMesh::SetImageColorIdFlags(std::vector<int>* flags)
    {
        mImageColorIdFlags = flags;
    }
}
