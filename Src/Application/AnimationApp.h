#pragma once
#include "AppBase.h"
#include "Gpp.h"

namespace GPP
{
    class TriMesh;
    class PointCloud;
}

namespace MagicCore
{
    class ViewTool;
    class PickTool;
}

namespace MagicApp
{
    class AnimationAppUI;
    class AnimationApp : public AppBase
    {
        enum CommandType
        {
            NONE = 0
        };

        enum RightMouseType
        {
            DEFORM = 0,
            SELECT,
            MOVE
        };

    public:

        AnimationApp();
        ~AnimationApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        bool ImportModel(void);
        void InitDeformation(int controlPointCount);
        void SelectFreeControlPoint(void);
        void ClearFreeControlPoint(void);
        void DeformControlPoint(void);
        void MoveControlPoint(void);
        void AppJump(void);

    private:
        void InitViewTool(void);
        
        void PickControlPoint(int mouseCoordX, int mouseCoordY);
        void DragControlPoint(int mouseCoordX, int mouseCoordY, bool mouseReleased);
        void UpdateDeformation(int mouseCoordX, int mouseCoordY);
        void SelectControlPointByRectangle(int startCoordX, int startCoordY, int endCoordX, int endCoordY);
        void UpdateRectangleRendering(int startCoordX, int startCoordY, int endCoordX, int endCoordY);
        void ClearRectangleRendering(void);

        void UpdateModelRendering(void);
        void UpdateControlRendering(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        void ClearPointCloudData(void);
        void ClearMeshData(void);

    private:
        AnimationAppUI* mpUI;
        GPP::TriMesh* mpTriMesh;
        GPP::PointCloud* mpPointCloud;
        GPP::Vector3 mObjCenterCoord;
        GPP::Real mScaleValue;
        MagicCore::ViewTool* mpViewTool;
        GPP::DeformPointList* mDeformation;
        std::vector<GPP::Int> mControlIds;
        bool mIsDeformationInitialised;
        int mPickControlId;
        GPP::Vector3 mPickTargetCoord;
        GPP::Vector2 mMousePressdCoord;
        std::vector<std::set<int> > mControlNeighbors;
        std::vector<bool> mControlFixFlags;
        RightMouseType mRightMouseType;
        bool mAddSelection;
        bool mFirstAlert;
        std::vector<GPP::Vector3> mTargetControlCoords;
        std::vector<int> mTargetControlIds;
    };
}
