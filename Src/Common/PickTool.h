#pragma once
#include "PointCloud.h"
#include "TriMesh.h"
#include "Vector2.h"

namespace MagicCore
{
    enum PickMode
    {
        PM_POINT = 0,
        PM_RECTANGLE,
        PM_CYCLE
    };

    class PickTool
    {
    public:
        PickTool();
        ~PickTool();

        void SetPickParameter(PickMode pm, bool ignoreBack, GPP::PointCloud* pointCloud, GPP::TriMesh* triMesh);
        void Reset(void);

        void MousePressed(int mouseCoordX, int mouseCoordY);
        void MouseMoved(int mouseCoordX, int mouseCoordY);
        void MouseReleased(int mouseCoordX, int mouseCoordY);
        
        GPP::Int GetPickPointId(void);
        GPP::Int GetPickVertexId(void);
        void ClearPickedIds(void);

    private:
        GPP::Int PickPointByPoint(const GPP::PointCloud* pointCloud, const GPP::Vector2& mouseCoord, bool ignoreBack);
        GPP::Int PickVertexByPoint(const GPP::TriMesh* triMesh, const GPP::Vector2& mouseCoord, bool ignoreBack);

    private:
        PickMode mPickMode;
        bool mIgnoreBack;
        GPP::Vector2 mMouseCoord;
        GPP::PointCloud* mpPointCloud;
        GPP::TriMesh* mpTriMesh;
        std::vector<GPP::Int> mPickPointIds;
        std::vector<GPP::Int> mPickVertexIds;
        bool mPickPressed;
    };
}