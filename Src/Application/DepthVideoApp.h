#pragma once
#include "AppBase.h"
#include "Gpp.h"

namespace MagicCore
{
    class ViewTool;
}

namespace MagicApp
{
    class DepthVideoAppUI;
    class DepthVideoApp : public AppBase
    {
        enum CommandType
        {
            CT_NONE = 0,
            CT_IMPORT_POINTCLOUD,
            CT_ALIGN_POINTCLOUD
        };

    public:
        DepthVideoApp();
        ~DepthVideoApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);
        virtual void WindowFocusChanged(Ogre::RenderWindow* rw);

        void DoCommand(bool isSubThread);

        void ImportPointCloud(bool isSubThread = true);
        void AlignPointCloudList(int groupSize, bool isSubThread = true);

        void SetPointCloudIndex(int index);
        bool IsCommandInProgress(void);

    private:
        void InitViewTool(void);
        bool IsCommandAvaliable(void);
        void ClearPointCloudList(void);
        void UpdatePointCloudListRendering(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

    private:
        DepthVideoAppUI* mpUI;
        MagicCore::ViewTool* mpViewTool;
        CommandType mCommandType;
        std::vector<GPP::PointCloud*> mPointCloudList;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
        int mSelectCloudIndex;
        bool mIsCommandInProgress;
        bool mUpdatePointCloudListRendering;
        bool mUpdateUIScrollBar;
        int mProgressValue;
        int mGroupSize;
    };
}
