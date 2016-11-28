#include "stdafx.h"
#include <process.h>
#include "MeasureApp.h"
#include "MeasureAppUI.h"
#include "AppManager.h"
#include "ModelManager.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
#include "../Common/RenderSystem.h"
#if DEBUGDUMPFILE
#include "DumpMeasureMesh.h"
#endif
#include <numeric>

namespace MagicApp
{
    static unsigned __stdcall RunThread(void *arg)
    {
        MeasureApp* app = (MeasureApp*)arg;
        if (app == NULL)
        {
            return 0;
        }
        app->DoCommand(false);
        return 1;
    }

    MeasureApp::MeasureApp() :
        mpUI(NULL),
        mpViewTool(NULL),
        mpPickTool(NULL),
        mDisplayMode(0),
#if DEBUGDUMPFILE
        mpDumpInfo(NULL),
#endif
        mMarkIds(),
        mGeodesicsOnVertices(),
        mMarkPoints(),
        mCommandType(NONE),
        mIsCommandInProgress(false),
        mUpdateModelRendering(false),
        mUpdateMarkRendering(false),
        mGeodesicAccuracy(0.5),
        mIsFlatRenderingMode(true),
        mpRefTriMesh(NULL),
        mUpdateRefModelRendering(false),
        mMinCurvature(),
        mMaxCurvature(),
        mMinCurvatureDirs(),
        mMaxCurvatureDirs(),
        mDisplayPrincipalCurvature(0),
        mCurvatureWeight(0),
        mIsGeodesicsClose(false)
    {
    }

    MeasureApp::~MeasureApp()
    {
        GPPFREEPOINTER(mpUI);        
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
        GPPFREEPOINTER(mpRefTriMesh);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
    }

    bool MeasureApp::Enter()
    {
        InfoLog << "Enter MeasureApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new MeasureAppUI;
        }
        mpUI->Setup();
        SetupScene();
        UpdateModelRendering();
        return true;
    }

    bool MeasureApp::Update(double timeElapsed)
    {
        if (mpUI && mpUI->IsProgressbarVisible())
        {
            int progressValue = int(GPP::GetApiProgress() * 100.0);
            mpUI->SetProgressbar(progressValue);
        }
        if (mUpdateMarkRendering)
        {
            UpdateMarkRendering();
            mUpdateMarkRendering = false;
        }
        if (mUpdateModelRendering)
        {
            UpdateModelRendering();
            mUpdateModelRendering = false;
        }
        if (mUpdateRefModelRendering)
        {
            UpdateRefModelRendering();
            mUpdateRefModelRendering = false;
        }
        return true;
    }

    bool MeasureApp::Exit()
    {
        InfoLog << "Exit MeasureApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool MeasureApp::MouseMoved( const OIS::MouseEvent &arg )
    {
        if (arg.state.buttonDown(OIS::MB_Middle) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_MIDDLE_DOWN);
        }
        else if (arg.state.buttonDown(OIS::MB_Left) && arg.state.buttonDown(OIS::MB_Right) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_RIGHT_DOWN);
        }
        else if (arg.state.buttonDown(OIS::MB_Left) && mpViewTool != NULL)
        {
            mpViewTool->MouseMoved(arg.state.X.abs, arg.state.Y.abs, MagicCore::ViewTool::MM_LEFT_DOWN);
        }     
        
        return true;
    }

    bool MeasureApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((arg.state.buttonDown(OIS::MB_Middle) || arg.state.buttonDown(OIS::MB_Left)) && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if (arg.state.buttonDown(OIS::MB_Right) && mIsCommandInProgress == false)
        {
            if (mpPickTool)
            {
                mpPickTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
            }
        }
        return true;
    }

    bool MeasureApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
        if (mIsCommandInProgress == false && id == OIS::MB_Right)
        {
            if (mpPickTool)
            {
                mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
                GPP::Int pickedId = -1;
                pickedId = mpPickTool->GetPickVertexId();
                mpPickTool->ClearPickedIds();
                if (pickedId != -1)
                {
                    mMarkIds.push_back(pickedId);
                    UpdateMarkRendering();
                }
            }
        }
        return  true;
    }

    static bool ChamferCurve(GPP::TriMesh* triMesh, const std::vector<int>& centerCurve, const std::vector<int>& topCurve,
        const std::vector<int>& downCurve, bool isCurveClose)
    {
        int originVertexCount = triMesh->GetVertexCount();
        std::vector<std::set<int> > vertexNeighbors(originVertexCount, std::set<int>());
        int vertexIds[3] = {-1};
        int originFaceCount = triMesh->GetTriangleCount();
        for (int fid = 0; fid < originFaceCount; fid++)
        {
            triMesh->GetTriangleVertexIds(fid, vertexIds);
            vertexNeighbors.at(vertexIds[0]).insert(vertexIds[1]);
            vertexNeighbors.at(vertexIds[0]).insert(vertexIds[2]);
            vertexNeighbors.at(vertexIds[1]).insert(vertexIds[2]);
            vertexNeighbors.at(vertexIds[1]).insert(vertexIds[0]);
            vertexNeighbors.at(vertexIds[2]).insert(vertexIds[0]);
            vertexNeighbors.at(vertexIds[2]).insert(vertexIds[1]);
        }
        std::vector<int> deleteTriangles;
        if (isCurveClose)
        {
            std::vector<bool> vertexMark(originVertexCount, 0);
            for (std::vector<int>::const_iterator itr = topCurve.begin(); itr != topCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = downCurve.begin(); itr != downCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = centerCurve.begin(); itr != centerCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            std::vector<int> vertexStack = centerCurve;
            while (vertexStack.size() > 0)
            {
                std::vector<int> vertexStackNext;
                for (std::vector<int>::iterator stackItr = vertexStack.begin(); stackItr != vertexStack.end(); ++stackItr)
                {
                    for (std::set<int>::iterator nitr = vertexNeighbors.at(*stackItr).begin(); nitr != vertexNeighbors.at(*stackItr).end(); ++nitr)
                    {
                        if (vertexMark.at(*nitr))
                        {
                            continue;
                        }
                        vertexMark.at(*nitr) = 1;
                        vertexStackNext.push_back(*nitr);
                    }
                }
                vertexStack.swap(vertexStackNext);
            }
            for (int fid = 0; fid < originFaceCount; fid++)
            {
                triMesh->GetTriangleVertexIds(fid, vertexIds);
                if (vertexMark.at(vertexIds[0]) && vertexMark.at(vertexIds[1]) && vertexMark.at(vertexIds[2]))
                {
                    deleteTriangles.push_back(fid);
                }
            }
            GPP::ErrorCode res = GPP::DeleteTriMeshTriangles(triMesh, deleteTriangles, false);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            int stringSize = topCurve.size();
            for (int vid = 0; vid < stringSize; vid++)
            {
                triMesh->InsertTriangle(topCurve.at(vid), topCurve.at((vid - 1 + stringSize) % stringSize), downCurve.at(vid));
                triMesh->InsertTriangle(downCurve.at(vid), topCurve.at((vid - 1 + stringSize) % stringSize), downCurve.at((vid - 1 + stringSize) % stringSize));
            }
            res = GPP::DeleteIsolatedVertices(triMesh);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            triMesh->UpdateNormal();
        }
        else
        {
            int stringSize = topCurve.size();
            std::vector<int> sectionVertexIds;
            sectionVertexIds.push_back(topCurve.at(0));
            sectionVertexIds.push_back(downCurve.at(0));
            std::vector<int> leftCurves;
            GPP::Real distance;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(triMesh, sectionVertexIds, false, leftCurves, distance);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            sectionVertexIds.clear();
            sectionVertexIds.push_back(topCurve.at(stringSize - 1));
            sectionVertexIds.push_back(downCurve.at(stringSize - 1));
            std::vector<int> rightCurves;
            res = GPP::MeasureMesh::ComputeApproximateGeodesics(triMesh, sectionVertexIds, false, rightCurves, distance);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }

            std::vector<bool> vertexMark(originVertexCount, 0);
            for (std::vector<int>::const_iterator itr = topCurve.begin(); itr != topCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = downCurve.begin(); itr != downCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = leftCurves.begin(); itr != leftCurves.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = rightCurves.begin(); itr != rightCurves.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            int seedVertex = centerCurve.at(centerCurve.size() / 2);
            std::vector<int> vertexStack;
            vertexStack.push_back(seedVertex);
            vertexMark.at(seedVertex) = 1;
            while (vertexStack.size() > 0)
            {
                std::vector<int> vertexStackNext;
                for (std::vector<int>::iterator stackItr = vertexStack.begin(); stackItr != vertexStack.end(); ++stackItr)
                {
                    for (std::set<int>::iterator nitr = vertexNeighbors.at(*stackItr).begin(); nitr != vertexNeighbors.at(*stackItr).end(); ++nitr)
                    {
                        if (vertexMark.at(*nitr))
                        {
                            continue;
                        }
                        vertexMark.at(*nitr) = 1;
                        vertexStackNext.push_back(*nitr);
                    }
                }
                vertexStack.swap(vertexStackNext);
            }
            for (int fid = 0; fid < originFaceCount; fid++)
            {
                triMesh->GetTriangleVertexIds(fid, vertexIds);
                if (vertexMark.at(vertexIds[0]) && vertexMark.at(vertexIds[1]) && vertexMark.at(vertexIds[2]))
                {
                    deleteTriangles.push_back(fid);
                }
            }
            res = GPP::DeleteTriMeshTriangles(triMesh, deleteTriangles, false);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            for (int vid = 1; vid < stringSize; vid++)
            {
                triMesh->InsertTriangle(topCurve.at(vid), topCurve.at((vid - 1 + stringSize) % stringSize), downCurve.at(vid));
                triMesh->InsertTriangle(downCurve.at(vid), topCurve.at((vid - 1 + stringSize) % stringSize), downCurve.at((vid - 1 + stringSize) % stringSize));
            }
            std::vector<int> boundarySeedIds;
            boundarySeedIds.push_back(topCurve.at(0));
            boundarySeedIds.push_back(topCurve.at(stringSize - 1));
            res = GPP::FillMeshHole::FillHoles(triMesh, &boundarySeedIds);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            res = GPP::DeleteIsolatedVertices(triMesh);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            triMesh->UpdateNormal();
        }
        return true;
    }

    static bool ComputeControlPoint(const GPP::Vector3& point, const GPP::Vector3& normal, const GPP::Vector3& opPoint,
        GPP::Vector3& controlPoint)
    {
        GPP::Vector3 opVec = opPoint - point;
        double opLength = fabs(opVec * normal);
        GPP::Vector3 pointVec = normal * opLength;
        if (pointVec * opVec > 0)
        {
            pointVec *= (-1.0);
        }
        GPP::Vector3 extendVec = pointVec + opVec;
        controlPoint = point + extendVec * 0.5;
        return true;
    }

    static bool BlendCurve(GPP::TriMesh* triMesh, const std::vector<int>& centerCurve, const std::vector<int>& topCurve,
        const std::vector<int>& downCurve, bool isCurveClose)
    {
        int originVertexCount = triMesh->GetVertexCount();
        std::vector<std::set<int> > vertexNeighbors(originVertexCount, std::set<int>());
        int vertexIds[3] = {-1};
        int originFaceCount = triMesh->GetTriangleCount();
        for (int fid = 0; fid < originFaceCount; fid++)
        {
            triMesh->GetTriangleVertexIds(fid, vertexIds);
            vertexNeighbors.at(vertexIds[0]).insert(vertexIds[1]);
            vertexNeighbors.at(vertexIds[0]).insert(vertexIds[2]);
            vertexNeighbors.at(vertexIds[1]).insert(vertexIds[2]);
            vertexNeighbors.at(vertexIds[1]).insert(vertexIds[0]);
            vertexNeighbors.at(vertexIds[2]).insert(vertexIds[0]);
            vertexNeighbors.at(vertexIds[2]).insert(vertexIds[1]);
        }
        std::vector<int> deleteTriangles;
        if (isCurveClose)
        {
            std::vector<bool> vertexMark(originVertexCount, 0);
            for (std::vector<int>::const_iterator itr = topCurve.begin(); itr != topCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = downCurve.begin(); itr != downCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = centerCurve.begin(); itr != centerCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            std::vector<int> vertexStack = centerCurve;
            while (vertexStack.size() > 0)
            {
                std::vector<int> vertexStackNext;
                for (std::vector<int>::iterator stackItr = vertexStack.begin(); stackItr != vertexStack.end(); ++stackItr)
                {
                    for (std::set<int>::iterator nitr = vertexNeighbors.at(*stackItr).begin(); nitr != vertexNeighbors.at(*stackItr).end(); ++nitr)
                    {
                        if (vertexMark.at(*nitr))
                        {
                            continue;
                        }
                        vertexMark.at(*nitr) = 1;
                        vertexStackNext.push_back(*nitr);
                    }
                }
                vertexStack.swap(vertexStackNext);
            }
            for (int fid = 0; fid < originFaceCount; fid++)
            {
                triMesh->GetTriangleVertexIds(fid, vertexIds);
                if (vertexMark.at(vertexIds[0]) && vertexMark.at(vertexIds[1]) && vertexMark.at(vertexIds[2]))
                {
                    deleteTriangles.push_back(fid);
                }
            }
            GPP::ErrorCode res = GPP::DeleteTriMeshTriangles(triMesh, deleteTriangles, false);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }

            int stringSize = topCurve.size();
            std::vector<std::vector<GPP::Vector3> > subdCoordList;
            for (int vid = 0; vid < stringSize; vid++)
            {
                GPP::Vector3 topControlCoord;
                if (!ComputeControlPoint(triMesh->GetVertexCoord(topCurve.at(vid)), triMesh->GetVertexNormal(topCurve.at(vid)), 
                    triMesh->GetVertexCoord(downCurve.at(vid)), topControlCoord))
                {
                    return false;
                }
                GPP::Vector3 downControlCoord;
                if (!ComputeControlPoint(triMesh->GetVertexCoord(downCurve.at(vid)), triMesh->GetVertexNormal(downCurve.at(vid)), 
                    triMesh->GetVertexCoord(topCurve.at(vid)), downControlCoord))
                {
                    return false;
                }
                std::vector<GPP::Vector3> subdCoords(4);
                subdCoords.at(0) = triMesh->GetVertexCoord(topCurve.at(vid));
                subdCoords.at(1) = topControlCoord;
                subdCoords.at(2) = downControlCoord;
                subdCoords.at(3) = triMesh->GetVertexCoord(downCurve.at(vid));
                if (GPP::OptimiseCurve::SubdividePolyline(subdCoords, 3, false) != GPP_NO_ERROR)
                {
                    return false;
                }
                subdCoordList.push_back(subdCoords);
            }
            int subdSize = subdCoordList.at(0).size();
            std::vector<std::vector<int> > subdIndexList(subdSize, std::vector<int>(stringSize)); 
            for (int vid = 0; vid < stringSize; vid++)
            {
                subdIndexList.at(0).at(vid) = topCurve.at(vid);
                subdIndexList.at(subdSize - 1).at(vid) = downCurve.at(vid);
                for (int sid = 1; sid < subdSize - 1; sid++)
                {
                    subdIndexList.at(sid).at(vid) = triMesh->InsertVertex(subdCoordList.at(vid).at(sid));
                }
            }
            for (int vid = 0; vid < stringSize; vid++)
            {
                for (int sid = 0; sid < subdSize - 1; sid++)
                {
                    triMesh->InsertTriangle(subdIndexList.at(sid).at((vid + 1) % stringSize), subdIndexList.at(sid).at(vid), 
                        subdIndexList.at(sid + 1).at((vid + 1) % stringSize));
                    triMesh->InsertTriangle(subdIndexList.at(sid).at(vid), subdIndexList.at(sid + 1).at(vid), 
                        subdIndexList.at(sid + 1).at((vid + 1) % stringSize));
                }
            }
            res = GPP::DeleteIsolatedVertices(triMesh);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            triMesh->UpdateNormal();
        }
        else
        {
            int stringSize = topCurve.size();
            std::vector<int> sectionVertexIds;
            sectionVertexIds.push_back(topCurve.at(0));
            sectionVertexIds.push_back(downCurve.at(0));
            std::vector<int> leftCurves;
            GPP::Real distance;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(triMesh, sectionVertexIds, false, leftCurves, distance);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            res = GPP::OptimiseCurve::SmoothCurveOnMesh(triMesh, leftCurves, false, 0.2, 10);
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "leftCurves Smooth Failed", "温馨提示", MB_OK);
            }
            sectionVertexIds.clear();
            sectionVertexIds.push_back(topCurve.at(stringSize - 1));
            sectionVertexIds.push_back(downCurve.at(stringSize - 1));
            std::vector<int> rightCurves;
            res = GPP::MeasureMesh::ComputeApproximateGeodesics(triMesh, sectionVertexIds, false, rightCurves, distance);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            res = GPP::OptimiseCurve::SmoothCurveOnMesh(triMesh, rightCurves, false, 0.2, 10);
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "rightCurves Smooth Failed", "温馨提示", MB_OK);
            }
            std::vector<bool> vertexMark(originVertexCount, 0);
            for (std::vector<int>::const_iterator itr = topCurve.begin(); itr != topCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = downCurve.begin(); itr != downCurve.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = leftCurves.begin(); itr != leftCurves.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            for (std::vector<int>::const_iterator itr = rightCurves.begin(); itr != rightCurves.end(); ++itr)
            {
                vertexMark.at(*itr) = 1;
            }
            int seedVertex = centerCurve.at(centerCurve.size() / 2);
            std::vector<int> vertexStack;
            vertexStack.push_back(seedVertex);
            vertexMark.at(seedVertex) = 1;
            while (vertexStack.size() > 0)
            {
                std::vector<int> vertexStackNext;
                for (std::vector<int>::iterator stackItr = vertexStack.begin(); stackItr != vertexStack.end(); ++stackItr)
                {
                    for (std::set<int>::iterator nitr = vertexNeighbors.at(*stackItr).begin(); nitr != vertexNeighbors.at(*stackItr).end(); ++nitr)
                    {
                        if (vertexMark.at(*nitr))
                        {
                            continue;
                        }
                        vertexMark.at(*nitr) = 1;
                        vertexStackNext.push_back(*nitr);
                    }
                }
                vertexStack.swap(vertexStackNext);
            }
            for (int fid = 0; fid < originFaceCount; fid++)
            {
                triMesh->GetTriangleVertexIds(fid, vertexIds);
                if (vertexMark.at(vertexIds[0]) && vertexMark.at(vertexIds[1]) && vertexMark.at(vertexIds[2]))
                {
                    deleteTriangles.push_back(fid);
                }
            }
            res = GPP::DeleteTriMeshTriangles(triMesh, deleteTriangles, false);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }

            std::vector<std::vector<GPP::Vector3> > subdCoordList;
            for (int vid = 0; vid < stringSize; vid++)
            {
                GPP::Vector3 topControlCoord;
                if (!ComputeControlPoint(triMesh->GetVertexCoord(topCurve.at(vid)), triMesh->GetVertexNormal(topCurve.at(vid)), 
                    triMesh->GetVertexCoord(downCurve.at(vid)), topControlCoord))
                {
                    return false;
                }
                GPP::Vector3 downControlCoord;
                if (!ComputeControlPoint(triMesh->GetVertexCoord(downCurve.at(vid)), triMesh->GetVertexNormal(downCurve.at(vid)), 
                    triMesh->GetVertexCoord(topCurve.at(vid)), downControlCoord))
                {
                    return false;
                }
                std::vector<GPP::Vector3> subdCoords(4);
                subdCoords.at(0) = triMesh->GetVertexCoord(topCurve.at(vid));
                subdCoords.at(1) = topControlCoord;
                subdCoords.at(2) = downControlCoord;
                subdCoords.at(3) = triMesh->GetVertexCoord(downCurve.at(vid));
                if (GPP::OptimiseCurve::SubdividePolyline(subdCoords, 3, false) != GPP_NO_ERROR)
                {
                    return false;
                }
                subdCoordList.push_back(subdCoords);
            }
            int subdSize = subdCoordList.at(0).size();
            std::vector<std::vector<int> > subdIndexList(subdSize, std::vector<int>(stringSize)); 
            for (int vid = 0; vid < stringSize; vid++)
            {
                subdIndexList.at(0).at(vid) = topCurve.at(vid);
                subdIndexList.at(subdSize - 1).at(vid) = downCurve.at(vid);
                for (int sid = 1; sid < subdSize - 1; sid++)
                {
                    subdIndexList.at(sid).at(vid) = triMesh->InsertVertex(subdCoordList.at(vid).at(sid));
                }
            }
            for (int vid = 0; vid < stringSize - 1; vid++)
            {
                for (int sid = 0; sid < subdSize - 1; sid++)
                {
                    triMesh->InsertTriangle(subdIndexList.at(sid).at(vid + 1), subdIndexList.at(sid).at(vid), 
                        subdIndexList.at(sid + 1).at(vid + 1));
                    triMesh->InsertTriangle(subdIndexList.at(sid).at(vid), subdIndexList.at(sid + 1).at(vid), 
                        subdIndexList.at(sid + 1).at(vid + 1));
                }
            }

            std::vector<int> boundarySeedIds;
            boundarySeedIds.push_back(topCurve.at(0));
            boundarySeedIds.push_back(topCurve.at(stringSize - 1));
            res = GPP::FillMeshHole::FillHoles(triMesh, &boundarySeedIds);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            res = GPP::DeleteIsolatedVertices(triMesh);
            if (res != GPP_NO_ERROR)
            {
                return false;
            }
            triMesh->UpdateNormal();
        }
        return true;
    }

    bool MeasureApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_G)
        {
            GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
            std::vector<GPP::Real> curvature;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeGaussCurvature(triMesh, curvature);
            if (res != GPP_NO_ERROR)
            {
                return true;
            }
            GPP::Int vertexCount = triMesh->GetVertexCount();
            triMesh->SetHasColor(true);
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                triMesh->SetVertexColor(vid, MagicCore::ToolKit::ColorCoding(0.6 + fabs(curvature.at(vid)) * 2.0));
            }
            UpdateModelRendering();
        }
        else if (arg.key == OIS::KC_L)
        {
            std::vector<GPP::Vector3> lineSegments;
            for (int mid = 0; mid < mMarkPoints.size() - 1; mid++)
            {
                lineSegments.push_back(mMarkPoints.at(mid));
                lineSegments.push_back(mMarkPoints.at(mid + 1));
            }
            GPP::Parser::ExportLineSegmentToPovray("edge.inc", lineSegments, 0.0025, GPP::Vector3(0.09, 0.48627, 0.69));
        }
        else if (arg.key == OIS::KC_C)
        {
            mIsGeodesicsClose = true;
        }
        else if (arg.key == OIS::KC_O)
        {
            mIsGeodesicsClose = false;
        }
        else if (arg.key == OIS::KC_A)
        {
            GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
            if (triMesh == NULL || mGeodesicsOnVertices.empty())
            {
                return true;
            }
            std::vector<GPP::Int> topCurve, downCurve, stringTriangles;
            GPP::ErrorCode res = GPP::OptimiseCurve::ApproximateOffsetCurveOnMesh(triMesh, mGeodesicsOnVertices, mIsGeodesicsClose, 0.1, 
                topCurve, downCurve, stringTriangles);
            if (res != GPP_NO_ERROR)
            {
                return true;
            }
            if (topCurve.size() > 0)
            {
                res = GPP::OptimiseCurve::SmoothCurveOnMesh(triMesh, topCurve, mIsGeodesicsClose, 0.2, 10);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "topString Smooth Failed", "温馨提示", MB_OK);
                }
            }
            if (downCurve.size() > 0)
            {
                res = GPP::OptimiseCurve::SmoothCurveOnMesh(triMesh, downCurve, mIsGeodesicsClose, 0.2, 10);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "downString Smooth Failed", "温馨提示", MB_OK);
                }
            }
            if (topCurve.size() > 0 && downCurve.size() > 0)
            {
                res = GPP::OptimiseCurve::MakeCurveBijective(triMesh, topCurve, downCurve, mIsGeodesicsClose);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "MakeCurveBijective Failed", "温馨提示", MB_OK);
                }
                res = GPP::OptimiseCurve::MakeCurvePairPerpendicular(triMesh, mGeodesicsOnVertices, topCurve, downCurve, 
                    mIsGeodesicsClose);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "MakeCurvePairPerpendicular Failed", "温馨提示", MB_OK);
                }
                if (!ChamferCurve(triMesh, mGeodesicsOnVertices, topCurve, downCurve, mIsGeodesicsClose))
                {
                    MessageBox(NULL, "ChamferCurve Failed", "温馨提示", MB_OK);
                }
            }
            mGeodesicsOnVertices.clear();
            mMarkPoints.clear();
            mMarkIds.clear();
            mUpdateModelRendering = true;
            mUpdateMarkRendering = true;
        }
        else if (arg.key == OIS::KC_D)
        {
            GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
            if (triMesh == NULL || mGeodesicsOnVertices.empty())
            {
                return true;
            }
            std::vector<GPP::Int> topCurve, downCurve, stringTriangles;
            GPP::ErrorCode res = GPP::OptimiseCurve::ApproximateOffsetCurveOnMesh(triMesh, mGeodesicsOnVertices, mIsGeodesicsClose, 0.1, 
                topCurve, downCurve, stringTriangles);
            if (res != GPP_NO_ERROR)
            {
                return true;
            }
            if (topCurve.size() > 0)
            {
                res = GPP::OptimiseCurve::SmoothCurveOnMesh(triMesh, topCurve, mIsGeodesicsClose, 0.2, 10);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "topString Smooth Failed", "温馨提示", MB_OK);
                }
            }
            if (downCurve.size() > 0)
            {
                res = GPP::OptimiseCurve::SmoothCurveOnMesh(triMesh, downCurve, mIsGeodesicsClose, 0.2, 10);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "downString Smooth Failed", "温馨提示", MB_OK);
                }
            }
            if (topCurve.size() > 0 && downCurve.size() > 0)
            {
                res = GPP::OptimiseCurve::MakeCurveBijective(triMesh, topCurve, downCurve, mIsGeodesicsClose);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "MakeCurveBijective Failed", "温馨提示", MB_OK);
                }
                res = GPP::OptimiseCurve::MakeCurvePairPerpendicular(triMesh, mGeodesicsOnVertices, topCurve, downCurve, 
                    mIsGeodesicsClose);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "MakeCurvePairPerpendicular Failed", "温馨提示", MB_OK);
                }
                if (!BlendCurve(triMesh, mGeodesicsOnVertices, topCurve, downCurve, mIsGeodesicsClose))
                {
                    MessageBox(NULL, "BlendCurve Failed", "温馨提示", MB_OK);
                }
            }
            mGeodesicsOnVertices.clear();
            mMarkPoints.clear();
            mMarkIds.clear();
            mUpdateModelRendering = true;
            mUpdateMarkRendering = true;
        }
        else if (arg.key == OIS::KC_S)
        {
            GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
            if (triMesh == NULL || mGeodesicsOnVertices.empty())
            {
                return true;
            }
            GPP::Real radius = 0.1 * 0.1;
            GPP::Int centerVertexCount = mGeodesicsOnVertices.size();
            GPP::Ann ann;
            GPP::Real* refData = new GPP::Real[centerVertexCount * 3];
            for (GPP::Int vid = 0; vid < centerVertexCount; vid++)
            {
                GPP::Vector3 coord = triMesh->GetVertexCoord(mGeodesicsOnVertices.at(vid));
                refData[vid * 3] = coord[0];
                refData[vid * 3 + 1] = coord[1];
                refData[vid * 3 + 2] = coord[2];
            }
            GPP::ErrorCode res = ann.Init(refData, centerVertexCount, 3);
            GPPFREEARRAY(refData);
            if (res != GPP_NO_ERROR)
            {
                return true;
            }
            GPP::Real searchData[3] = {-1};
            GPP::Int indexRes[1] = {-1};
            GPP::Real distanceRes[1] = {-1};
            GPP::Int vertexCount = triMesh->GetVertexCount();
            GPP::Int faceCount = triMesh->GetTriangleCount();
            std::vector<std::set<GPP::Int> > vertexNeighbors(vertexCount, std::set<GPP::Int>());
            GPP::Int vertexIds[3] = {-1};
            for (GPP::Int fid = 0; fid < faceCount; fid++)
            {
                triMesh->GetTriangleVertexIds(fid, vertexIds);
                vertexNeighbors.at(vertexIds[0]).insert(vertexIds[1]);
                vertexNeighbors.at(vertexIds[0]).insert(vertexIds[2]);
                vertexNeighbors.at(vertexIds[1]).insert(vertexIds[2]);
                vertexNeighbors.at(vertexIds[1]).insert(vertexIds[0]);
                vertexNeighbors.at(vertexIds[2]).insert(vertexIds[0]);
                vertexNeighbors.at(vertexIds[2]).insert(vertexIds[1]);
            }
            std::vector<bool> vertexVisitFlag(vertexCount, 0);
            std::vector<GPP::Int> vertexStack;
            std::vector<GPP::Int> vertexString;
            std::vector<GPP::Int> stringIds;
            int gvid = 0;
            for (std::vector<int>::iterator itr = mGeodesicsOnVertices.begin(); itr != mGeodesicsOnVertices.end(); ++itr)
            {
                vertexVisitFlag.at(*itr) = 1;
                vertexStack.push_back(*itr);
                vertexString.push_back(*itr);
                stringIds.push_back(gvid);
                gvid++;
            }
            while (vertexStack.size() > 0)
            {
                std::vector<GPP::Int> vertexStackNext;
                for (std::vector<GPP::Int>::iterator stackItr = vertexStack.begin(); stackItr != vertexStack.end(); ++stackItr)
                {
                    for (std::set<GPP::Int>::iterator nitr = vertexNeighbors.at(*stackItr).begin(); nitr != vertexNeighbors.at(*stackItr).end(); ++nitr)
                    {
                        if (vertexVisitFlag.at(*nitr))
                        {
                            continue;
                        }
                        vertexVisitFlag.at(*nitr) = 1;
                        GPP::Vector3 nCoord = triMesh->GetVertexCoord(*nitr);
                        searchData[0] = nCoord[0];
                        searchData[1] = nCoord[1];
                        searchData[2] = nCoord[2];
                        res = ann.FindNearestNeighbors(searchData, 1, 1, indexRes, distanceRes);
                        if (res != GPP_NO_ERROR)
                        {
                            return true;
                        }
                        if (distanceRes[0] > radius)
                        {
                            continue;
                        }
                        vertexStackNext.push_back(*nitr);
                        vertexString.push_back(*nitr);
                        stringIds.push_back(indexRes[0]);
                    }
                }
                vertexStack.swap(vertexStackNext);
            }
            triMesh->SetHasColor(true);
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                triMesh->SetVertexColor(vid, GPP::Vector3(0.86, 0.86, 0.86));
            }
            double colorDelta = 0.25;
            int maxColorId = 5;
            int stringVertexCount = vertexString.size();
            for (int svid = 0; svid < stringVertexCount; ++svid)
            {
                triMesh->SetVertexColor(vertexString.at(svid), MagicCore::ToolKit::ColorCoding(0.2 + colorDelta * (stringIds.at(svid) % maxColorId)));
            }
            mUpdateModelRendering = true;
        }
        return true;
    }

    void MeasureApp::WindowFocusChanged( Ogre::RenderWindow* rw)
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
    }

    void MeasureApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
        Ogre::Light* light = sceneManager->createLight("MeasureApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);
        InitViewTool();
        if (ModelManager::Get()->GetMesh())
        {
            // set up pick tool
            GPPFREEPOINTER(mpPickTool);
            mpPickTool = new MagicCore::PickTool;
            mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, NULL, ModelManager::Get()->GetMesh(), "ModelNode");
            mpUI->SetModelInfo(ModelManager::Get()->GetMesh()->GetVertexCount(), ModelManager::Get()->GetMesh()->GetTriangleCount());
        }
    }

    void MeasureApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("MeasureApp_SimpleLight");
        //MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        MagicCore::RenderSystem::Get()->HideRenderingObject("Mesh_Measure");       
        MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Measure");       
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPoints_MeasureApp");       
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLine_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLine_MeasureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PrincipalLine_MeasureApp");
        //MagicCore::RenderSystem::Get()->ResertAllSceneNode();
    }

    void MeasureApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
        GPPFREEPOINTER(mpRefTriMesh);
#if DEBUGDUMPFILE
        GPPFREEPOINTER(mpDumpInfo);
#endif
        mMarkIds.clear();
        mGeodesicsOnVertices.clear();
        mMarkPoints.clear();
        mMinCurvature.clear();
        mMaxCurvature.clear();
        mMinCurvatureDirs.clear();
        mMaxCurvatureDirs.clear();
        mDisplayPrincipalCurvature = 0;
    }

    void MeasureApp::DoCommand(bool isSubThread)
    {
        if (isSubThread)
        {
            GPP::ResetApiProgress();
            mpUI->StartProgressbar(100);
            _beginthreadex(NULL, 0, RunThread, (void *)this, 0, NULL);
        }
        else
        {
            switch (mCommandType)
            {
            case MagicApp::MeasureApp::NONE:
                break;
            case MagicApp::MeasureApp::GEODESICS_APPROXIMATE:
                ComputeApproximateGeodesics(false);
                break;
            case MagicApp::MeasureApp::GEOMESICS_FAST_EXACT:
                FastComputeExactGeodesics(mGeodesicAccuracy, false);
                break;
            case MagicApp::MeasureApp::GEODESICS_EXACT:
                ComputeExactGeodesics(false);
                break;
            case MagicApp::MeasureApp::DISTANCE_POINTS_TO_MESH:
                ComputePointsToMeshDistance(false);
                break;
            case MagicApp::MeasureApp::GEODESICS_CURVATURE:
                ComputeCurvatureGeodesics(mCurvatureWeight, false);
                break;
            case MagicApp::MeasureApp::PRINCIPAL_CURVATURE:
                MeasurePrincipalCurvature(false);
                break;
            case MagicApp::MeasureApp::THICKNESS:
                MeasureThickness(false);
                break;
            default:
                break;
            }
            mpUI->StopProgressbar();
        }
    }

    bool MeasureApp::IsCommandAvaliable()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return false;
        }
        return true;
    }

#if DEBUGDUMPFILE
    void MeasureApp::SetDumpInfo(GPP::DumpBase* dumpInfo)
    {
        /*if (dumpInfo == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpDumpInfo);
        mpDumpInfo = dumpInfo;
        if (mpDumpInfo->GetTriMesh() == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpTriMeshRef);
        mpTriMeshRef = CopyTriMesh(mpDumpInfo->GetTriMesh());
        InitViewTool();
        UpdateModelRefRendering();*/
    }

    void MeasureApp::RunDumpInfo()
    {
        //if (mpDumpInfo == NULL)
        //{
        //    return;
        //}
        //GPP::ErrorCode res = mpDumpInfo->Run();
        //if (res != GPP_NO_ERROR)
        //{
        //    MagicCore::ToolKit::Get()->SetAppRunning(false);
        //    return;
        //}

        ////Copy result
        //GPPFREEPOINTER(mpTriMeshRef);
        //mpTriMeshRef = CopyTriMesh(mpDumpInfo->GetTriMesh());
        //mpTriMeshRef->UnifyCoords(2.0);
        //mpTriMeshRef->UpdateNormal();

        //if (mpDumpInfo->GetApiName() == GPP::MESH_MEASURE_SECTION_EXACT || mpDumpInfo->GetApiName() == GPP::MESH_MEASURE_SECTION_FAST_EXACT)
        //{
        //    GPP::DumpMeshMeasureSectionExact* dumpDetails = dynamic_cast<GPP::DumpMeshMeasureSectionExact*>(mpDumpInfo);
        //    if (dumpDetails)
        //    {
        //        mRefMarkPoints = dumpDetails->GetSectionPathPoints();
        //    }
        //}

        //UpdateModelRefRendering();
        //UpdateMarkRefRendering();
        //GPPFREEPOINTER(mpDumpInfo);
    }
#endif

    bool MeasureApp::IsCommandInProgress()
    {
        return mIsCommandInProgress;
    }

    void MeasureApp::SwitchDisplayMode()
    {
        mDisplayMode++;
        if (mDisplayMode > 3)
        {
            mDisplayMode = 0;
        }
        if (mDisplayMode == 0)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
            mIsFlatRenderingMode = true;
        }
        else if (mDisplayMode == 1)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
            mIsFlatRenderingMode = false;
        }
        else if (mDisplayMode == 2)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
            mIsFlatRenderingMode = false;
        }
        else if (mDisplayMode == 3)
        {
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_POINTS);
            mIsFlatRenderingMode = false;
        }
        Ogre::Material* material = dynamic_cast<Ogre::Material*>(Ogre::MaterialManager::getSingleton().getByName("CookTorrance").getPointer());
        if (material)
        {
            if (mDisplayMode == 0 || mDisplayMode == 1)
            {
                material->setCullingMode(Ogre::CullingMode::CULL_NONE);
            }
            else
            {
                material->setCullingMode(Ogre::CullingMode::CULL_CLOCKWISE);
            }
        }
        mUpdateModelRendering = true;
    }

    bool MeasureApp::ImportModel()
    {
        if (IsCommandAvaliable() == false)
        {
            return false;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0GPT Files(*.gpt)\0*.gpt\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            mpUI->SetGeodesicsInfo(0);
            ModelManager::Get()->ClearPointCloud();
            if (ModelManager::Get()->ImportMesh(fileName) == false)
            {
                MessageBox(NULL, "网格导入失败", "温馨提示", MB_OK);
                return false;
            }
            GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
            mpUI->SetModelInfo(triMesh->GetVertexCount(), triMesh->GetTriangleCount());
            UpdateModelRendering();
            mMarkIds.clear();
            mGeodesicsOnVertices.clear();
            mMarkPoints.clear();
            mMinCurvature.clear();
            mMaxCurvature.clear();
            mMinCurvatureDirs.clear();
            mMaxCurvatureDirs.clear();
            mDisplayPrincipalCurvature = 0;
            UpdateMarkRendering();
            // set up pick tool
            GPPFREEPOINTER(mpPickTool);
            mpPickTool = new MagicCore::PickTool;
            mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, NULL, triMesh, "ModelNode");
            GPPFREEPOINTER(mpRefTriMesh);
            UpdateRefModelRendering();
            return true;
        }
        return false;
    }

    bool MeasureApp::ImportRefModel()
    {
        if (IsCommandAvaliable() == false)
        {
            return false;
        }
        if (ModelManager::Get()->GetMesh() == NULL)
        {
            MessageBox(NULL, "请先导入需要测量的网格", "温馨提示", MB_OK);
            return false;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0GPT Files(*.gpt)\0*.gpt\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            mpUI->SetGeodesicsInfo(0);
            ModelManager::Get()->ClearPointCloud();
            GPPFREEPOINTER(mpRefTriMesh);
            mpRefTriMesh = GPP::Parser::ImportTriMesh(fileName);
            if (mpRefTriMesh == NULL)
            {
                MessageBox(NULL, "网格导入失败", "温馨提示", MB_OK);
                return false;
            }
            mpRefTriMesh->UnifyCoords(ModelManager::Get()->GetScaleValue(), ModelManager::Get()->GetObjCenterCoord());
            mpRefTriMesh->UpdateNormal();
            mpUI->SetDistanceInfo(mpRefTriMesh->GetTriangleCount(), false, 0.0, 0.0);
            UpdateRefModelRendering();
            return true;
        }

        return false;
    }

    void MeasureApp::DeleteMeshMark()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mMarkIds.size() > 0)
        {
            mMarkIds.pop_back();
        }
        mGeodesicsOnVertices.clear();
        mMarkPoints.clear();
        UpdateMarkRendering();
        mpUI->SetGeodesicsInfo(0);
    }

    void MeasureApp::ComputeApproximateGeodesics(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMarkIds.size() < 2)
        {
            MessageBox(NULL, "请在测量的网格上选择标记点", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = GEODESICS_APPROXIMATE;
            DoCommand(true);
        }
        else
        {
            mGeodesicsOnVertices.clear();
            GPP::Real distance = 0;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(triMesh, mMarkIds, mIsGeodesicsClose, mGeodesicsOnVertices, distance);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / ModelManager::Get()->GetScaleValue());
            mMarkPoints.clear();
            for (std::vector<GPP::Int>::iterator pathItr = mGeodesicsOnVertices.begin(); pathItr != mGeodesicsOnVertices.end(); ++pathItr)
            {
                mMarkPoints.push_back(triMesh->GetVertexCoord(*pathItr));
            }
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::ComputeCurvatureGeodesics(double curvatureWeight, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        int vertexCount = triMesh->GetVertexCount();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMarkIds.size() < 2)
        {
            MessageBox(NULL, "请在测量的网格上选择标记点", "温馨提示", MB_OK);
            return;
        }
        else if (mMinCurvature.size() != vertexCount)
        {
            MessageBox(NULL, "请先给网格计算主曲率", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = GEODESICS_CURVATURE;
            mCurvatureWeight = curvatureWeight;
            DoCommand(true);
        }
        else
        {
            mGeodesicsOnVertices.clear();
            GPP::Real distance = 0;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::TriangleList triangleList(triMesh);
            GPP::PrincipalCurvatureDistance meshDistance(&triangleList, &mMinCurvatureDirs, &mMaxCurvatureDirs, 
                &mMinCurvature, &mMaxCurvature, curvatureWeight);
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(triMesh, mMarkIds, mIsGeodesicsClose, mGeodesicsOnVertices, 
                distance, &meshDistance);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / ModelManager::Get()->GetScaleValue());
            mMarkPoints.clear();
            for (std::vector<GPP::Int>::iterator pathItr = mGeodesicsOnVertices.begin(); pathItr != mGeodesicsOnVertices.end(); ++pathItr)
            {
                mMarkPoints.push_back(triMesh->GetVertexCoord(*pathItr));
            }
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::SmoothGeodesicsOnVertex()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mGeodesicsOnVertices.empty())
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入网格", "温馨提示", MB_OK);
            return;
        }
#if MAKEDUMPFILE
        GPP::DumpOnce();
#endif
        GPP::ErrorCode res = GPP::OptimiseCurve::SmoothCurveOnMesh(triMesh, mGeodesicsOnVertices, mIsGeodesicsClose, 0.2, 10);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            MessageBox(NULL, "顶点上的测地线优化失败", "温馨提示", MB_OK);
            return;
        }
        mMarkPoints.clear();
        for (std::vector<GPP::Int>::iterator pathItr = mGeodesicsOnVertices.begin(); pathItr != mGeodesicsOnVertices.end(); ++pathItr)
        {
            mMarkPoints.push_back(triMesh->GetVertexCoord(*pathItr));
        }
        mUpdateModelRendering = true;
        mUpdateMarkRendering = true;
    }
 
    void MeasureApp::FastComputeExactGeodesics(double accuracy, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMarkIds.size() < 2)
        {
            MessageBox(NULL, "请在测量的网格上选择标记点", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = GEOMESICS_FAST_EXACT;
            mGeodesicAccuracy = accuracy;
            DoCommand(true);
        }
        else
        {
            std::vector<GPP::Vector3> pathPoints;
            std::vector<GPP::PointOnEdge> pathInfos;
            GPP::Real distance = 0;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::MeasureMesh::FastComputeExactGeodesics(triMesh, mMarkIds, mIsGeodesicsClose, 
                pathPoints, distance, &pathInfos, accuracy);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / ModelManager::Get()->GetScaleValue());
            mMarkPoints.clear();
            mMarkPoints.swap(pathPoints);
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::ComputeExactGeodesics(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        else if (mMarkIds.size() < 2)
        {
            MessageBox(NULL, "请在测量的网格上选择标记点", "温馨提示", MB_OK);
            return;
        }
        else if (triMesh->GetVertexCount() > 200000 && isSubThread)
        {
            if (MessageBox(NULL, "测量网格顶点大于200k，测量时间会比较长，是否继续？", "温馨提示", MB_OKCANCEL) != IDOK)
            {
                return;
            }
        }
        if (isSubThread)
        {
            mCommandType = GEODESICS_EXACT;
            DoCommand(true);
        }
        else
        {
            std::vector<GPP::Vector3> pathPoints;
            std::vector<GPP::PointOnEdge> pathInfos;
            GPP::Real distance = 0;
            //GPP::DumpOnce();
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeExactGeodesics(triMesh, mMarkIds, mIsGeodesicsClose, pathPoints, distance, &pathInfos);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                return;
            }
            mpUI->SetGeodesicsInfo(distance / ModelManager::Get()->GetScaleValue());
            mMarkPoints.clear();
            mMarkPoints.swap(pathPoints);
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::MeasureArea()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        GPP::Real area = 0;
        GPP::ErrorCode res = GPP::MeasureMesh::ComputeArea(triMesh, area);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        mpUI->SetModelArea(area / ModelManager::Get()->GetScaleValue() / ModelManager::Get()->GetScaleValue());
    }

    void MeasureApp::MeasureVolume()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        GPP::Real volume = 0;
        GPP::ErrorCode res = GPP::MeasureMesh::ComputeVolume(triMesh, volume);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        mpUI->SetModelVolume(volume / ModelManager::Get()->GetScaleValue() / ModelManager::Get()->GetScaleValue() / ModelManager::Get()->GetScaleValue());
    }

    void MeasureApp::MeasureCurvature()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        std::vector<GPP::Real> curvature;
        GPP::ErrorCode res = GPP::MeasureMesh::ComputeMeanCurvature(triMesh, curvature);
        if (res == GPP_API_IS_NOT_AVAILABLE)
        {
            MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
            MagicCore::ToolKit::Get()->SetAppRunning(false);
        }
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        GPP::Int vertexCount = triMesh->GetVertexCount();
        triMesh->SetHasColor(true);
        for (GPP::Int vid = 0; vid < vertexCount; vid++)
        {
            triMesh->SetVertexColor(vid, MagicCore::ToolKit::ColorCoding(0.6 + curvature.at(vid) / 10.0));
        }
        mDisplayPrincipalCurvature = 0;
        UpdateMarkRendering();
        UpdateModelRendering();
    }

    void MeasureApp::MeasurePrincipalCurvature(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        if (triMesh == NULL)
        {
            MessageBox(NULL, "请导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = PRINCIPAL_CURVATURE;
            DoCommand(true);
        }
        else
        {
            if (mDisplayPrincipalCurvature != 0 && triMesh->GetVertexCount() == mMinCurvature.size())
            {
                mDisplayPrincipalCurvature *= -1;
            }
            else
            {
#if MAKEDUMPFILE
                GPP::DumpOnce();
#endif
                GPP::ErrorCode res = GPP::MeasureMesh::ComputePrincipalCurvature(triMesh, mMinCurvature, mMaxCurvature, 
                    mMinCurvatureDirs, mMaxCurvatureDirs);
                if (res == GPP_API_IS_NOT_AVAILABLE)
                {
                    MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                    MagicCore::ToolKit::Get()->SetAppRunning(false);
                }
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "主曲率计算失败", "温馨提示", MB_OK);
                    return;
                }
                mDisplayPrincipalCurvature = 1;
            }
            mUpdateMarkRendering = true;
        }
    }

    void MeasureApp::MeasureThickness(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (ModelManager::Get()->GetMesh() == NULL)
        {
            MessageBox(NULL, "请先导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }

        if (isSubThread)
        {
            mCommandType = THICKNESS;
            DoCommand(true);
        }
        else
        {
            GPP::TriMesh* measureMesh = ModelManager::Get()->GetMesh();
            std::vector<GPP::Real> thickness;
            GPP::ErrorCode res = GPP::MeasureMesh::ComputeThickness(measureMesh, thickness);
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "计算厚度失败", "温馨提示", MB_OK);
                return ;
            }

            measureMesh->SetHasColor(true);
            GPP::Real maxValue = *std::max_element(thickness.begin(), thickness.end());
            GPP::Real minValue = *std::min_element(thickness.begin(), thickness.end());
            GPP::Real unChangedMaxValue = maxValue;
            if (maxValue < GPP::REAL_TOL)
            {
                maxValue = 1.0;
            }
            for (GPP::Int vid = 0; vid < measureMesh->GetVertexCount(); ++vid)
            {
                GPP::Real dist = thickness.at(vid);
                GPP::Vector3 color = MagicCore::ToolKit::ColorCoding(dist + 0.2);
                measureMesh->SetVertexColor(vid, color);
            }
            GPP::Int halfVId = thickness.size() / 2;
            std::nth_element(thickness.begin(), thickness.begin() + halfVId, thickness.end());
            GPP::Real midValue = thickness.at(halfVId);
            InfoLog << "Median thickness is: " << midValue << std::endl;
            mpUI->SetThicknessInfo(true, midValue / ModelManager::Get()->GetScaleValue());

            mUpdateModelRendering = true;
        }
    }

    void MeasureApp::ComputePointsToMeshDistance(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (ModelManager::Get()->GetMesh() == NULL)
        {
            MessageBox(NULL, "请先导入需要测量的网格", "温馨提示", MB_OK);
            return;
        }
        if (mpRefTriMesh == NULL)
        {
            MessageBox(NULL, "请先导入参考网格", "温馨提示", MB_OK);
            return;
        }

        if (isSubThread)
        {
            mCommandType = DISTANCE_POINTS_TO_MESH;
            DoCommand(true);
        }
        else
        {
            GPP::TriMesh* measureMesh = ModelManager::Get()->GetMesh();
            std::vector<GPP::Vector3> points(measureMesh->GetVertexCount());
            for (GPP::Int pid = 0; pid < points.size(); ++pid)
            {
                points.at(pid) = measureMesh->GetVertexCoord(pid);
            }
            std::vector<GPP::Real> distances;
            GPP::MeshQueryTool queryTool;
            GPP::ErrorCode res = queryTool.Init(mpRefTriMesh);
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格距离工具初始化失败", "温馨提示", MB_OK);
                return;
            }
            res = queryTool.QueryNearestTriangles(points, NULL, &distances);
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "距离计算失败", "温馨提示", MB_OK);
                return;
            }
            measureMesh->SetHasColor(true);
            GPP::Real maxValue = *std::max_element(distances.begin(), distances.end());
            GPP::Real minValue = *std::min_element(distances.begin(), distances.end());
            GPP::Real unChangedMaxValue = maxValue;
            if (maxValue < GPP::REAL_TOL)
            {
                maxValue = 1.0;
            }
            for (GPP::Int pid = 0; pid < points.size(); ++pid)
            {
                GPP::Real dist = distances.at(pid);
                GPP::Vector3 color = MagicCore::ToolKit::ColorCoding(dist / maxValue + 0.2);
                measureMesh->SetVertexColor(pid, color);
            }
            mpUI->SetDistanceInfo(measureMesh->GetVertexCount(), true, 
                minValue / ModelManager::Get()->GetScaleValue(), unChangedMaxValue/ ModelManager::Get()->GetScaleValue());
            mUpdateRefModelRendering = true;
            mUpdateModelRendering = true;
        }
    }

    void MeasureApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
        }
    }

    void MeasureApp::UpdateModelRendering()
    {
        if (ModelManager::Get()->GetMesh() == NULL)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("Mesh_Measure");
            return;
        }
        MagicCore::RenderSystem::Get()->RenderMesh("Mesh_Measure", "CookTorrance", ModelManager::Get()->GetMesh(), 
            MagicCore::RenderSystem::MODEL_NODE_CENTER, NULL, NULL, mIsFlatRenderingMode);
    }

    void MeasureApp::UpdateRefModelRendering()
    {
        if (mpRefTriMesh == NULL)
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Measure");
            return;
        }
        MagicCore::RenderSystem::Get()->RenderMesh("MeshRef_Measure", "CookTorranceTransparent", mpRefTriMesh, 
            MagicCore::RenderSystem::MODEL_NODE_CENTER, NULL, NULL, true);
    }

    void MeasureApp::UpdateMarkRendering()
    {
        GPP::TriMesh* triMesh = ModelManager::Get()->GetMesh();
        std::vector<GPP::Vector3> markCoords = mMarkPoints;
        if (mMarkIds.size() > 0 && triMesh != NULL)
        {
            for (std::vector<GPP::Int>::iterator itr = mMarkIds.begin(); itr != mMarkIds.end(); ++itr)
            {
                markCoords.push_back(triMesh->GetVertexCoord(*itr));
            }
            MagicCore::RenderSystem::Get()->RenderPointList("MarkPoints_MeasureApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), markCoords, MagicCore::RenderSystem::MODEL_NODE_CENTER);
            MagicCore::RenderSystem::Get()->RenderPolyline("MarkPointLine_MeasureApp", "Simple_Line", GPP::Vector3(0, 1, 0), mMarkPoints, true, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else
        {     
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPoints_MeasureApp");       
            MagicCore::RenderSystem::Get()->HideRenderingObject("MarkPointLine_MeasureApp");
        }
        if (mDisplayPrincipalCurvature != 0 && triMesh != NULL)
        {
            int vertexCount = triMesh->GetVertexCount();
            if (vertexCount == mMinCurvature.size() && mDisplayPrincipalCurvature == -1)
            {
                std::vector<GPP::Vector3> startCoords(vertexCount);
                std::vector<GPP::Vector3> endCoords(vertexCount);
                double curvatureLen = 0.005;
                triMesh->SetHasColor(true);
                for (int vid = 0; vid < vertexCount; vid++)
                {
                    GPP::Real minCurvature = mMinCurvature.at(vid);
                    triMesh->SetVertexColor(vid, MagicCore::ToolKit::ColorCoding(0.8 + minCurvature));
                    if (fabs(minCurvature) > GPP::REAL_TOL)
                    {
                        GPP::Vector3 vertexCoord = triMesh->GetVertexCoord(vid);
                        GPP::Vector3 minDir = mMinCurvatureDirs.at(vid);
                        startCoords.at(vid) = vertexCoord - minDir * curvatureLen;
                        endCoords.at(vid) = vertexCoord + minDir * curvatureLen;
                    }
                }
                MagicCore::RenderSystem::Get()->RenderLineSegments("PrincipalLine_MeasureApp", "Simple_Line", startCoords, endCoords);
            }
            else if (vertexCount == mMaxCurvature.size() && mDisplayPrincipalCurvature == 1)
            {
                std::vector<GPP::Vector3> startCoords(vertexCount);
                std::vector<GPP::Vector3> endCoords(vertexCount);
                double curvatureLen = 0.005;
                triMesh->SetHasColor(true);
                for (int vid = 0; vid < vertexCount; vid++)
                {
                    GPP::Real maxCurvature = mMaxCurvature.at(vid);
                    triMesh->SetVertexColor(vid, MagicCore::ToolKit::ColorCoding(0.8 + maxCurvature));
                    if (fabs(maxCurvature) > GPP::REAL_TOL)
                    {
                        GPP::Vector3 vertexCoord = triMesh->GetVertexCoord(vid);
                        GPP::Vector3 maxDir = mMaxCurvatureDirs.at(vid);
                        startCoords.at(vid) = vertexCoord - maxDir * curvatureLen;
                        endCoords.at(vid) = vertexCoord + maxDir * curvatureLen;
                    }
                }
                MagicCore::RenderSystem::Get()->RenderLineSegments("PrincipalLine_MeasureApp", "Simple_Line", startCoords, endCoords);
            }
            else
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("PrincipalLine_MeasureApp");
            }
            MagicCore::RenderSystem::Get()->RenderMesh("Mesh_Measure", "CookTorrance", ModelManager::Get()->GetMesh(), 
                MagicCore::RenderSystem::MODEL_NODE_CENTER, NULL, NULL, mIsFlatRenderingMode);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PrincipalLine_MeasureApp");
        }
    }

    void MeasureApp::ShowReferenceMesh(bool isShow)
    {
        if (isShow)
        {
            UpdateRefModelRendering();
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("MeshRef_Measure");
        }
    }
}
