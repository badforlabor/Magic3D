#include "stdafx.h"
#include "PickTool.h"
#include "RenderSystem.h"

namespace MagicCore
{
    PickTool::PickTool() :
        mPickMode(PM_POINT),
        mIgnoreBack(true),
        mMouseCoord(),
        mpPointCloud(NULL),
        mpTriMesh(NULL),
        mModelNodeName(),
        mPickPointIds(),
        mPickVertexIds(),
        mPickPressed(false)
    {
    }

    PickTool::~PickTool()
    {
    }

    void PickTool::SetPickParameter(PickMode pm, bool ignoreBack, GPP::PointCloud* pointCloud, GPP::TriMesh* triMesh, std::string modelNodeName)
    {
        mPickMode = pm;
        mIgnoreBack = ignoreBack;
        mpPointCloud = pointCloud;
        mpTriMesh = triMesh;
        mModelNodeName = modelNodeName;
    }

    void PickTool::SetModelNodeName(std::string modelNodeName)
    {
        mModelNodeName = modelNodeName;
    }
    
    void PickTool::Reset()
    {
        mPickPointIds.clear();
        mPickVertexIds.clear();
    }

    void PickTool::MousePressed(int mouseCoordX, int mouseCoordY)
    {
        mPickPressed = true;
    }

    void PickTool::MouseMoved(int mouseCoordX, int mouseCoordY)
    {
    }

    void PickTool::MouseReleased(int mouseCoordX, int mouseCoordY)
    {
        if (mPickPressed)
        {
            GPP::Vector2 curCoord(mouseCoordX * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getWidth() - 1.0, 
                    1.0 - mouseCoordY * 2.0 / MagicCore::RenderSystem::Get()->GetRenderWindow()->getHeight());
            if (mpPointCloud)
            {
                GPP::Int pickedPointId = PickPointByPoint(mpPointCloud, curCoord, mIgnoreBack);
                if (pickedPointId >= 0)
                {
                    mPickPointIds.clear();
                    mPickPointIds.push_back(pickedPointId);
                }
            }
            if (mpTriMesh)
            {
                GPP::Int pickedVertexId = PickVertexByPoint(mpTriMesh, curCoord, mIgnoreBack);
                if (pickedVertexId >= 0)
                {
                    mPickVertexIds.clear();
                    mPickVertexIds.push_back(pickedVertexId);
                }
            }
            mPickPressed = false;
        }
    }
        
    GPP::Int PickTool::GetPickPointId()
    {
        if (mPickPointIds.size() == 1)
        {
            return mPickPointIds.at(0);
        }
        else
        {
            return -1;
        }
    }

    GPP::Int PickTool::GetPickVertexId()
    {
        if (mPickVertexIds.size() == 1)
        {
            return mPickVertexIds.at(0);
        }
        else
        {
            return -1;
        }
    }

    void PickTool::ClearPickedIds()
    {
        mPickVertexIds.clear();
        mPickPointIds.clear();
    }

    GPP::Int PickTool::PickPointByPoint(const GPP::PointCloud* pointCloud, const GPP::Vector2& mouseCoord, bool ignoreBack)
    {
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode(mModelNodeName) == false || pointCloud == NULL)
        {
            return -1;
        }
        double pointSizeSquared = 0.01 * 0.01;
        Ogre::Matrix4 worldM = MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode(mModelNodeName)->_getFullTransform();
        Ogre::Matrix4 viewM  = MagicCore::RenderSystem::Get()->GetMainCamera()->getViewMatrix();
        Ogre::Matrix4 projM  = MagicCore::RenderSystem::Get()->GetMainCamera()->getProjectionMatrix();
        Ogre::Matrix4 wvpM   = projM * viewM * worldM;
        double minZ = 1.0e10;
        GPP::Int pickedId = -1;
        GPP::Int pointCound = pointCloud->GetPointCount();
        if (pointCloud->HasNormal() && ignoreBack)
        {
            for (GPP::Int pid = 0; pid < pointCound; pid++)
            {
                GPP::Vector3 coord = pointCloud->GetPointCoord(pid);
                GPP::Vector3 normal = pointCloud->GetPointNormal(pid);
                Ogre::Vector3 ogreCoord(coord[0], coord[1], coord[2]);  
                ogreCoord = wvpM * ogreCoord;
                GPP::Vector2 screenCoord(ogreCoord.x, ogreCoord.y);
                if ((screenCoord - mouseCoord).LengthSquared() < pointSizeSquared)
                {
                    Ogre::Vector4 ogreNormal(normal[0], normal[1], normal[2], 0);
                    ogreNormal = worldM * ogreNormal;
                    if (ogreNormal.z > 0)
                    {
                        if (ogreCoord.z < minZ)
                        {
                            minZ = ogreCoord.z;
                            pickedId = pid;
                        }
                    }
                }
            }
        }
        else
        {
            for (GPP::Int pid = 0; pid < pointCound; pid++)
            {
                GPP::Vector3 coord = pointCloud->GetPointCoord(pid);
                Ogre::Vector3 ogreCoord(coord[0], coord[1], coord[2]);
                ogreCoord = wvpM * ogreCoord;
                GPP::Vector2 screenCoord(ogreCoord.x, ogreCoord.y);
                if ((screenCoord - mouseCoord).LengthSquared() < pointSizeSquared)
                {
                    if (ogreCoord.z < minZ)
                    {
                        minZ = ogreCoord.z;
                        pickedId = pid;
                    }
                }
            }
        }

        return pickedId;
    }

    GPP::Int PickTool::PickVertexByPoint(const GPP::TriMesh* triMesh, const GPP::Vector2& mouseCoord, bool ignoreBack)
    {
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode(mModelNodeName) == false || triMesh == NULL)
        {
            return -1;
        }
        double pointSizeSquared = 0.01 * 0.01;
        Ogre::Matrix4 worldM = MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode(mModelNodeName)->_getFullTransform();
        Ogre::Matrix4 viewM  = MagicCore::RenderSystem::Get()->GetMainCamera()->getViewMatrix();
        Ogre::Matrix4 projM  = MagicCore::RenderSystem::Get()->GetMainCamera()->getProjectionMatrix();
        Ogre::Matrix4 wvpM   = projM * viewM * worldM;
        double minZ = 1.0e10;
        GPP::Int pickIndex = -1;
        GPP::Int vertexCount = triMesh->GetVertexCount();
        if (ignoreBack)
        {
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Vector3 normal = triMesh->GetVertexNormal(vid);
                Ogre::Vector4 ogreNormal(normal[0], normal[1], normal[2], 0);
                ogreNormal = worldM * ogreNormal;
                if (ogreNormal.z > 0)
                {
                    GPP::Vector3 coord = triMesh->GetVertexCoord(vid);
                    Ogre::Vector3 ogreCoord(coord[0], coord[1], coord[2]);
                    ogreCoord = wvpM * ogreCoord;
                    GPP::Vector2 screenCoord(ogreCoord.x, ogreCoord.y);
                    if ((screenCoord - mouseCoord).LengthSquared() < pointSizeSquared)
                    {
                        if (ogreCoord.z < minZ)
                        {
                            minZ = ogreCoord.z;
                            pickIndex = vid;
                        }
                    }
                }
            }
        }
        else
        {
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Vector3 coord = triMesh->GetVertexCoord(vid);
                Ogre::Vector3 ogreCoord(coord[0], coord[1], coord[2]);
                ogreCoord = wvpM * ogreCoord;
                GPP::Vector2 screenCoord(ogreCoord.x, ogreCoord.y);
                if ((screenCoord - mouseCoord).LengthSquared() < pointSizeSquared)
                {
                    if (ogreCoord.z < minZ)
                    {
                        minZ = ogreCoord.z;
                        pickIndex = vid;
                    }
                }
            }
        }
        return pickIndex;
    }
}
