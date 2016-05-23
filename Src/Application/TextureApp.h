#pragma once
#include "AppBase.h"
#include "Gpp.h"
#include "opencv2/opencv.hpp"
#include "OgreTextureManager.h"

namespace GPP
{
    class TriMesh;
}

namespace MagicCore
{
    class ViewTool;
    class PickTool;
}

namespace MagicApp
{
    class TextureAppUI;
    class TextureApp : public AppBase
    {
        enum CommandType
        {
            NONE = 0,
            UNFOLD_INITIAL,
            OPTIMIZE_ISOMETRIC,
            GENERATE_UV_ATLAS
        };

        enum DisplayMode
        {
            TRIMESH_SOLID = 0,
            TRIMESH_WIREFRAME,
            UVMESH_WIREFRAME,
            TRIMESH_TEXTURE
        };

    public:

        TextureApp();
        ~TextureApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        void DoCommand(bool isSubThread);

        void SwitchDisplayMode(void);
        void SwitchTextureImage(void);

        void ImportTriMesh(void);
        void ExportTriMesh(void);

        void ConfirmGeodesics(void);
        void DeleteGeodesics(void);
        void SwitchMarkDisplay(void);
        
        void UnfoldTriMesh(bool isSubThread = true);
        void Optimize2Isometric(bool isSubThread = true);
        void GenerateUVAtlas(int initChartCount, bool isSubThread = true);

        void EnterMeshToolApp(void);

        void OptimizeColorConsistency(void);

        void EnterPointToolApp(void);

        void SetMesh(GPP::TriMesh* triMesh, GPP::Vector3 objCenterCoord, GPP::Real scaleValue);

    private:
        void InitViewTool(void);
        void UpdateDisplay(void);
        void UpdateMarkDisplay(bool display);
        void UpdateTriMeshTexture(void);
        void GenerateUVMesh(bool bWithTextureCoordinates = false);
        void UpdateTextureFromUVMesh(void);
        void UnifyTextureCoords(std::vector<double>& texCoords, double scaleValue);
        void UpdatePointCloudRendering(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        bool IsCommandAvaliable(void);
        void ClearMeshData(void);
        void ClearPointCloudData(void);

    private:
        TextureAppUI* mpUI;
        GPP::TriMesh* mpTriMesh;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
        GPP::TriMesh* mpUVMesh;
        GPP::TriMesh* mpImageFrameMesh;
        cv::Mat mDistortionImage;
        int mTextureImageSize;
        Ogre::TexturePtr mpTriMeshTexture;
        MagicCore::ViewTool* mpViewTool;
        DisplayMode mDisplayMode;
        CommandType mCommandType;
        bool mIsCommandInProgress;
        bool mUpdateDisplay;
        bool mHideMarks;
        MagicCore::PickTool* mpPickTool;
        GPP::Int mLastCutVertexId;
        std::vector<GPP::PointOnEdge> mCurPointsOnEdge;
        std::vector<GPP::Vector3> mCurMarkCoords;
        std::vector<std::vector<GPP::Int> > mCutLineList;
        int mInitChartCount;
        std::vector<std::string> mTextureImageNames;
        int mCurrentTextureImageId;
        std::vector<GPP::PointCloud*> mPointCloudList;
        bool mUpdatePointCloudRendering;
    };
}
