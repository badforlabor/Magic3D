#pragma once
#include "AppBase.h"
#include <vector>
#include "GPP.h"

namespace GPP
{
    class TriMesh;
    class DumpBase;
}

namespace MagicCore
{
    class ViewTool;
}

namespace MagicApp
{
    class ReliefAppUI;
    class ReliefApp : public AppBase
    {
    public:
        ReliefApp();
        ~ReliefApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);
        virtual void WindowFocusChanged(Ogre::RenderWindow* rw);

        bool ImportModel(void);
        void GenerateRelief(double compressRatio);
        void EnterMeshTool(void);
        void SetMesh(GPP::TriMesh* triMesh, GPP::Vector3 objCenterCoord, GPP::Real scaleValue);

#if DEBUGDUMPFILE
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
#endif

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);

        void InitViewTool(void);
        void UpdateModelRendering(void);
        GPP::TriMesh* GenerateTriMeshFromHeightField(const std::vector<GPP::Real>& heightField, int resolutionX, int resolutionY);

    private:
        ReliefAppUI* mpUI;
        GPP::TriMesh* mpTriMesh;
        MagicCore::ViewTool* mpViewTool;
#if DEBUGDUMPFILE
        GPP::DumpBase* mpDumpInfo;
#endif
        std::vector<GPP::Real> mHeightField;
        int mResolution;
    };
}
