#include "MagicPointCloud.h"

namespace MagicApp
{
    MagicPointCloud::MagicPointCloud(GPP::IPointCloud* pointCloud) :
        mPointCloud(pointCloud),
        mImageColorIds(NULL),
        mColorIds(NULL),
        mCloudIds(NULL)
    {
    }

    GPP::Int MagicPointCloud::GetPointCount() const
    {
        return mPointCloud->GetPointCount();
    }

    GPP::Vector3 MagicPointCloud::GetPointCoord(GPP::Int pid) const
    {
        return mPointCloud->GetPointCoord(pid);
    }

    void MagicPointCloud::SetPointCoord(GPP::Int pid, const GPP::Vector3& coord)
    {
        mPointCloud->SetPointCoord(pid, coord);
    }

    GPP::Vector3 MagicPointCloud::GetPointNormal(GPP::Int pid) const
    {
        return mPointCloud->GetPointNormal(pid);
    }

    void MagicPointCloud::SetPointNormal(GPP::Int pid, const GPP::Vector3& normal)
    {
        mPointCloud->SetPointNormal(pid, normal);
    }

    bool MagicPointCloud::HasNormal() const
    {
        return mPointCloud->HasNormal();
    }
    
    void MagicPointCloud::SetHasNormal(bool hasNormal)
    {
        mPointCloud->SetHasNormal(hasNormal);
    }

    GPP::Int MagicPointCloud::InsertPoint(const GPP::Vector3& coord)
    {
        return mPointCloud->InsertPoint(coord);
    }

    GPP::Int MagicPointCloud::InsertPoint(const GPP::Vector3& coord, const GPP::Vector3& normal)
    {
        return mPointCloud->InsertPoint(coord, normal);
    }

    void MagicPointCloud::SwapPoint(GPP::Int pointId0, GPP::Int pointId1)
    {
        mPointCloud->SwapPoint(pointId0, pointId1);
        if (mImageColorIds)
        {
            GPP::ImageColorId temp = mImageColorIds->at(pointId0);
            mImageColorIds->at(pointId0) = mImageColorIds->at(pointId1);
            mImageColorIds->at(pointId1) = temp;
        }
        if (mColorIds)
        {
            GPP::Int temp = mColorIds->at(pointId0);
            mColorIds->at(pointId0) = mColorIds->at(pointId1);
            mColorIds->at(pointId1) = temp;
        }
        if (mCloudIds)
        {
            GPP::Int temp = mCloudIds->at(pointId0);
            mCloudIds->at(pointId0) = mCloudIds->at(pointId1);
            mCloudIds->at(pointId1) = temp;
        }
    }

    void MagicPointCloud::PopbackPoints(GPP::Int popCount)
    {
        int pointCount = mPointCloud->GetPointCount();
        mPointCloud->PopbackPoints(popCount);
        if (mImageColorIds)
        {
            mImageColorIds->erase(mImageColorIds->begin() + pointCount - popCount, mImageColorIds->end());
        }
        if (mColorIds)
        {
            mColorIds->erase(mColorIds->begin() + pointCount - popCount, mColorIds->end());
        }
        if (mCloudIds)
        {
            mCloudIds->erase(mCloudIds->begin() + pointCount - popCount, mCloudIds->end());
        }
    }

    void MagicPointCloud::Clear()
    {
        mPointCloud->Clear();
        mImageColorIds = NULL;
        mColorIds = NULL;
        mCloudIds = NULL;
    }

    void MagicPointCloud::SetImageColorIds(std::vector<GPP::ImageColorId>* imageColorIds)
    {
        mImageColorIds = imageColorIds;
    }

    void MagicPointCloud::SetColorIds(std::vector<int>* colorIds)
    {
        mColorIds = colorIds;
    }

    void MagicPointCloud::SetCloudIds(std::vector<int>* cloudIds)
    {
        mCloudIds = cloudIds;
    }

    MagicPointCloud::~MagicPointCloud()
    {
    }
}
