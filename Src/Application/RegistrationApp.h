#pragma once
#include "AppBase.h"
#include "Vector3.h"
#include <vector>

namespace GPP
{
    class PointCloud;
    class DumpBase;
    class FusePointCloud;
}

namespace MagicCore
{
    class ViewTool;
}

namespace MagicApp
{
    class RegistrationAppUI;
    class RegistrationApp : public AppBase
    {
    public:
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
        void SmoothRefNormal(void);
        void FuseRef(void);
        bool ImportPointCloudFrom(void);
        void CalculateFromNormal(void);
        void FlipFromNormal(void);
        void SmoothFromNormal(void);
        void AlignFast(void);
        void AlignPrecise(void);
        void AlignICP(void);

        void EnterPointShop(void);

        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);

    private:
        void InitViewTool(void);
        void UpdatePointCloudFromRendering(void);
        void UpdatePointCloudRefRendering(void);
        void SetPointCloudColor(GPP::PointCloud* pointCloud, const GPP::Vector3& color);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

    private:
        RegistrationAppUI* mpUI;
        MagicCore::ViewTool* mpViewTool;
        GPP::DumpBase* mpDumpInfo;
        GPP::PointCloud* mpPointCloudRef;
        GPP::PointCloud* mpPointCloudFrom;
        GPP::FusePointCloud* mpFusePointCloud;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
    };
}
