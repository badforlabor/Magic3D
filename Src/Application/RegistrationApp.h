#pragma once
#include "AppBase.h"
#include "Vector3.h"
#include <vector>

namespace GPP
{
    class PointCloud;
    class DumpBase;
}

namespace MagicCore
{
    class ViewTool;
    class PickTool;
}

namespace MagicApp
{
    class RegistrationAppUI;
    class RegistrationApp : public AppBase
    {
    public:
        enum MouseMode
        {
            MM_VIEW = 0,
            MM_PICK_POINT_REF,
            MM_PICK_POINT_FROM
        };

        RegistrationApp();
        ~RegistrationApp();

        virtual bool Enter(void);
        virtual bool Update(float timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        bool ImportPointCloudRef(void);
        void CalculateRefNormal(void);
        void FlipRefNormal(void);
        void PushRef(void);
        void PopRef(void);
        //void FuseRef(void);
        bool ImportPointCloudFrom(void);
        void CalculateFromNormal(void);
        void FlipFromNormal(void);
        void PushFrom(void);
        void PopFrom(void);
        void AlignFrom(void);
        void ModelView(void);

        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);

    private:
        void InitViewTool(void);
        void InitPickTool(void);
        void UpdatePointCloudFromRendering(void);
        void UpdatePointCloudRefRendering(void);
        void UpdatePointPairsRendering(void);
        void SetPointCloudColor(GPP::PointCloud* pointCloud, const GPP::Vector3& color);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

    private:
        RegistrationAppUI* mpUI;
        MouseMode mMouseMode;
        MagicCore::ViewTool* mpViewTool;
        MagicCore::PickTool* mpPickTool;
        GPP::DumpBase* mpDumpInfo;
        GPP::PointCloud* mpPointCloudRef;
        GPP::PointCloud* mpPointCloudFrom;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
        std::vector<GPP::Vector3> mPickedPointsRef;
        std::vector<GPP::Vector3> mPickedPointsFrom;
    };
}
