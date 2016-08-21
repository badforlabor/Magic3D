#include "ModelManager.h"

namespace MagicApp
{
    ModelManager* ModelManager::mpModelManager = NULL;

    ModelManager::ModelManager() :
        mpPointCloud(NULL),
        mpTriMesh(NULL),
        mObjCenterCoord(),
        mScaleValue(0),
        mImageColorIds(),
        mTextureImageFiles(),
        mCloudIds()
    {
    }

    ModelManager* ModelManager::Get()
    {
        if (mpModelManager == NULL)
        {
            mpModelManager = new ModelManager;
        }
        return mpModelManager;
    }

    ModelManager::~ModelManager()
    {
        ClearPointCloud();
        ClearMesh();
    }

    bool ModelManager::ImportPointCloud(std::string fileName)
    {
        GPPFREEPOINTER(mpPointCloud);
        mpPointCloud = GPP::Parser::ImportPointCloud(fileName);
        if (mpPointCloud == NULL)
        {
            return false;
        }
        mpPointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
        return true;
    }

    void ModelManager::SetPointCloud(GPP::PointCloud* pointCloud)
    {
        GPPFREEPOINTER(mpPointCloud);
        mpPointCloud = pointCloud;
    }

    GPP::PointCloud* ModelManager::GetPointCloud()
    {
        return mpPointCloud;
    }

    void ModelManager::ClearPointCloud()
    {
        GPPFREEPOINTER(mpPointCloud);
    }

    void ModelManager::SetScaleValue(GPP::Real scaleValue)
    {
        mScaleValue = scaleValue;
    }

    GPP::Real ModelManager::GetScaleValue() const
    {
        return mScaleValue;
    }
    
    void ModelManager::SetObjCenterCoord(GPP::Vector3 objCenterCoord)
    {
        mObjCenterCoord = objCenterCoord;
    }

    GPP::Vector3 ModelManager::GetObjCenterCoord() const
    {
        return mObjCenterCoord;
    }

    void ModelManager::SetImageColorIds(const std::vector<GPP::ImageColorId>& imageColorIds)
    {
        mImageColorIds = imageColorIds;
    }

    std::vector<GPP::ImageColorId> ModelManager::GetImageColorIds()
    {
        return mImageColorIds;
    }

    void ModelManager::SetTextureImageFiles(const std::vector<std::string>& textureImageFiles)
    {
        mTextureImageFiles = textureImageFiles;
    }

    std::vector<std::string> ModelManager::GetTextureImageFiles(void) const
    {
        return mTextureImageFiles;
    }

    void ModelManager::SetCloudIds(const std::vector<int>& cloudIds)
    {
        mCloudIds = cloudIds;
    }

    std::vector<int> ModelManager::GetCloudIds(void) const
    {
        return mCloudIds;
    }

    void ModelManager::SetColorIds(const std::vector<int>& colorIds)
    {
        mColorIds = colorIds;
    }

    std::vector<int> ModelManager::GetColorIds(void) const
    {
        return mColorIds;
    }

    bool ModelManager::ImportMesh(std::string fileName)
    {
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = GPP::Parser::ImportTriMesh(fileName);
        if (mpTriMesh == NULL)
        {
            return false;
        }
        mpTriMesh->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
        mpTriMesh->UpdateNormal();
        return true;
    }

    void ModelManager::SetMesh(GPP::TriMesh* triMesh)
    {
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = triMesh;
    }

    GPP::TriMesh* ModelManager::GetMesh()
    {
        return mpTriMesh;
    }

    void ModelManager::ClearMesh()
    {
        GPPFREEPOINTER(mpTriMesh);
    }
}
