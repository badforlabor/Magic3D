#pragma once
#include "AppBase.h"
#include "Gpp.h"
#include "opencv2/opencv.hpp"
#include "OgreTextureManager.h"
#if DEBUGDUMPFILE
#include "DumpBase.h"
#endif

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
    class UVUnfoldAppUI;
    class UVUnfoldApp : public AppBase
    {
    public:
        enum CommandType
        {
            NONE = 0,
            UNFOLD_INITIAL,
            GENERATE_UV_ATLAS,
            UNFOLD_DISC
        };

        enum DisplayMode
        {
            AUTO = 0,
            TRIMESH_SOLID,
            TRIMESH_WIREFRAME,
            UVMESH_WIREFRAME,
            TRIMESH_TEXTURE
        };

    public:

        UVUnfoldApp();
        ~UVUnfoldApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        void DoCommand(bool isSubThread);

        void SwitchDisplayMode(DisplayMode dm);

        void ImportTriMesh(void);

        void CloseGeodesics(void);
        void SnapFrontGeodesics(void);
        void SnapBackGeodesics(void);
        void ConfirmGeodesics(void);
        void DeleteGeodesics(void);
        void SwitchMarkDisplay(void);
        void SetCutLineType(bool isAccurate);
        void SmoothCutLine(void);
        void GenerateSplitLines(int splitChartCount);
        
        void UnfoldTriMesh(bool isSubThread = true);
        void GenerateUVAtlas(int initChartCount, bool isSubThread = true);
        void Unfold2Disc(bool isSubThread = true);

        int GetMeshVertexCount(void);

#if DEBUGDUMPFILE
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
#endif

    private:
        void InitViewTool(void);
        void UpdateDisplay(void);
        void UpdateMarkDisplay();
        void InitTriMeshTexture(void);
        void GenerateSplitMesh(void);
        void UnifyTextureCoords(std::vector<double>& texCoords, double scaleValue);

        void InsertHolesToSnapIds(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        bool IsCommandAvaliable(void);
        void ClearSplitData(void);

    private:
        UVUnfoldAppUI* mpUI;
        GPP::TriMesh* mpImageFrameMesh;
        cv::Mat mDistortionImage;
        Ogre::TexturePtr mpTriMeshTexture;
        MagicCore::ViewTool* mpViewTool;
        DisplayMode mDisplayMode;
        CommandType mCommandType;
        bool mIsCommandInProgress;
        bool mUpdateDisplay;
        bool mHideMarks;
        MagicCore::PickTool* mpPickTool;
        GPP::Int mLastCutVertexId;
        std::vector<GPP::Int> mCurPointsOnVertex;
        std::vector<GPP::PointOnEdge> mCurPointsOnEdge;
        std::vector<GPP::Vector3> mCurMarkCoords;
        std::vector<std::vector<GPP::Int> > mCutLineList;
        int mInitChartCount;
        std::vector<int> mSnapIds;
        int mTargetVertexCount;
        bool mIsCutLineAccurate;
    };
}
