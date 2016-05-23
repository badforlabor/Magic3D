#include "stdafx.h"
#include <process.h>
#include "TextureApp.h"
#include "TextureAppUI.h"
#include "AppManager.h"
#include "MeshShopApp.h"
#include "PointShopApp.h"
#include "../Common/LogSystem.h"
#include "../Common/ToolKit.h"
#include "../Common/ViewTool.h"
#include "../Common/PickTool.h"
#include "../Common/RenderSystem.h"
#include "GPP.h"

namespace MagicApp
{
    static unsigned __stdcall RunThread(void *arg)
    {
        TextureApp* app = (TextureApp*)arg;
        if (app == NULL)
        {
            return 0;
        }
        app->DoCommand(false);
        return 1;
    }

    TextureApp::TextureApp() :
        mpUI(NULL),
        mpTriMesh(NULL),
        mObjCenterCoord(),
        mScaleValue(0),
        mpUVMesh(NULL),
        mpImageFrameMesh(NULL),
        mDistortionImage(),
        mTextureImageSize(512),
        mpTriMeshTexture(NULL),
        mpViewTool(NULL),
        mDisplayMode(TRIMESH_SOLID),
        mCommandType(NONE),
        mIsCommandInProgress(false),
        mUpdateDisplay(false),
        mHideMarks(false),
        mpPickTool(NULL),
        mLastCutVertexId(-1),
        mCurPointsOnEdge(),
        mCurMarkCoords(),
        mCutLineList(),
        mInitChartCount(1),
        mTextureImageNames(),
        mCurrentTextureImageId(0),
        mPointCloudList(),
        mUpdatePointCloudRendering(false)
    {
    }

    TextureApp::~TextureApp()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpUVMesh);
        GPPFREEPOINTER(mpImageFrameMesh);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
        for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
        {
            GPPFREEPOINTER(*itr);
        }
        mPointCloudList.clear();
    }

    void TextureApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpViewTool);
        ClearMeshData();
        ClearPointCloudData();
    }

    void TextureApp::ClearMeshData()
    {
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpUVMesh);
        GPPFREEPOINTER(mpImageFrameMesh);
        GPPFREEPOINTER(mpPickTool);
        mDisplayMode = TRIMESH_SOLID;
        mDistortionImage.release();
        mLastCutVertexId = -1;
        mCurPointsOnEdge.clear();
        mCurMarkCoords.clear();
        mCutLineList.clear();
    }
 
    void TextureApp::ClearPointCloudData()
    {
        for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
        {
            GPPFREEPOINTER(*itr);
        }
        mPointCloudList.clear();
    }

    bool TextureApp::IsCommandAvaliable()
    {
        if (mIsCommandInProgress)
        {
            MessageBox(NULL, "请等待当前命令执行完", "温馨提示", MB_OK);
            return false;
        }
        return true;
    }

    bool TextureApp::Enter()
    {
        InfoLog << "Enter TextureApp" << std::endl; 
        if (mpUI == NULL)
        {
            mpUI = new TextureAppUI;
        }
        mpUI->Setup();
        SetupScene();
        return true;
    }

    bool TextureApp::Update(double timeElapsed)
    {
        if (mpUI && mpUI->IsProgressbarVisible())
        {
            int progressValue = int(GPP::GetApiProgress() * 100.0);
            mpUI->SetProgressbar(progressValue);
        }
        if (mUpdateDisplay)
        {
            mUpdateDisplay = false;
            UpdateDisplay();
            InfoLog << "Update::UpdateDisplay" << std::endl;
        }
        if (mHideMarks)
        {
            mHideMarks = false;
            UpdateMarkDisplay(false);
        }
        if (mUpdatePointCloudRendering)
        {
            mUpdatePointCloudRendering = false;
            UpdatePointCloudRendering();
        }
        return true;
    }

    bool TextureApp::Exit()
    {
        InfoLog << "Exit TextureApp" << std::endl; 
        ShutdownScene();
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        ClearData();
        return true;
    }

    bool TextureApp::MouseMoved( const OIS::MouseEvent &arg )
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

    bool TextureApp::MousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if ((id == OIS::MB_Middle || id == OIS::MB_Left) && mpViewTool != NULL)
        {
            mpViewTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        else if (!arg.state.buttonDown(OIS::MB_Left) && arg.state.buttonDown(OIS::MB_Right) && mIsCommandInProgress == false && mpPickTool)
        {
            mpPickTool->MousePressed(arg.state.X.abs, arg.state.Y.abs);
        }
        return true;
    }

    bool TextureApp::MouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
    {
        if (mpViewTool)
        {
            mpViewTool->MouseReleased();
        }
        if (!arg.state.buttonDown(OIS::MB_Left) && mIsCommandInProgress == false && id == OIS::MB_Right && mpPickTool)
        {
            mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
            GPP::Int pickedId = mpPickTool->GetPickVertexId();;
            mpPickTool->ClearPickedIds();
            if (pickedId != -1)
            {
                if (mLastCutVertexId == -1)
                {
                    mLastCutVertexId = pickedId;
                }
                else
                {
                    std::vector<GPP::Int> sectionVertexIds;
                    sectionVertexIds.push_back(mLastCutVertexId);
                    sectionVertexIds.push_back(pickedId);
                    std::vector<GPP::Vector3> pathCoords;
                    std::vector<GPP::PointOnEdge> pathInfos;
                    GPP::Real distance = 0;
                    GPP::ErrorCode res = GPP::MeasureMesh::FastComputeExactGeodesics(mpTriMesh, sectionVertexIds, false, 
                        pathCoords, distance, &pathInfos, 0.5);
                    if (res != GPP_NO_ERROR)
                    {
                        MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                        return true;
                    }
                    int startId = 1;
                    if (mCurPointsOnEdge.empty())
                    {
                        startId = 0;
                    }
                    for (int pvid = startId; pvid < pathInfos.size(); pvid++)
                    {
                        mCurPointsOnEdge.push_back(pathInfos.at(pvid));
                    }
                    for (int pvid = startId; pvid < pathCoords.size(); pvid++)
                    {
                        mCurMarkCoords.push_back(pathCoords.at(pvid));
                    }
                    mLastCutVertexId = pickedId;
                }
                UpdateMarkDisplay(true);
            }
        }
        return true;
    }

    static void UnifyTextureCoords(std::vector<GPP::Real>& texCoords)
    {
        int vertexCount = texCoords.size() / 2;
        GPP::Real texCoordMax_X = texCoords.at(0);
        GPP::Real texCoordMax_Y = texCoords.at(1);
        GPP::Real texCoordMin_X = texCoords.at(0);
        GPP::Real texCoordMin_Y = texCoords.at(1);
        for (GPP::Int vid = 1; vid < vertexCount; vid++)
        {
            if (texCoords.at(vid * 2) > texCoordMax_X)
            {
                texCoordMax_X = texCoords.at(vid * 2);
            }
            else if (texCoords.at(vid * 2) < texCoordMin_X)
            {
                texCoordMin_X = texCoords.at(vid * 2);
            }
            if (texCoords.at(vid * 2 + 1) > texCoordMax_Y)
            {
                texCoordMax_Y = texCoords.at(vid * 2 + 1);
            }
            else if (texCoords.at(vid * 2 + 1) < texCoordMin_Y)
            {
                texCoordMin_Y = texCoords.at(vid * 2 + 1);
            }
        }

        GPP::Real range_X = texCoordMax_X - texCoordMin_X;
        GPP::Real range_Y = texCoordMax_Y - texCoordMin_Y;
        GPP::Real range_max = range_X > range_Y ? range_X : range_Y;
        if (range_max > GPP::REAL_TOL)
        {
            GPP::Real scaleV = 1.0 / range_max;
            GPP::Real center_X = (texCoordMax_X + texCoordMin_X) / 2.0;
            GPP::Real center_Y = (texCoordMax_Y + texCoordMin_Y) / 2.0;
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                texCoords.at(vid * 2) = (texCoords.at(vid * 2) - texCoordMin_X) * scaleV;
                texCoords.at(vid * 2 + 1) = (texCoords.at(vid * 2 + 1) - texCoordMin_Y) * scaleV;
            }
        }
    }

    static void GenerateTextureImage(const GPP::TriMesh* mesh)
    {
        int imageWidth = 1024;
        int imageHeight = 1024;
        std::vector<GPP::Vector3> imageData;
        std::vector<GPP::Real> textureCoords;
        std::vector<GPP::Vector3> colors;
        std::vector<GPP::Int> textureIds;
        if (mesh)
        {
            GPP::Int faceCount = mesh->GetTriangleCount();
            colors.resize(faceCount * 3);
            textureCoords.resize(faceCount * 3 * 2);
            textureIds.resize(faceCount * 3);
            for (GPP::Int fid = 0; fid < faceCount; ++fid)
            {
                GPP::Int vertexIds[3];
                mesh->GetTriangleVertexIds(fid, vertexIds);
                for (int ii = 0; ii < 3; ++ii)
                {
                    GPP::Int id = fid * 3 + ii;
                    colors.at(id) = mesh->GetVertexColor(vertexIds[ii]);
                    //mesh->SetTriangleTexcoord(fid, ii, mesh->GetVertexTexcoord(vertexIds[ii]));
                    textureCoords.at(id * 2) = mesh->GetVertexTexcoord(vertexIds[ii])[0];
                    textureCoords.at(id * 2 + 1) = mesh->GetVertexTexcoord(vertexIds[ii])[1];
                    textureIds.at(id) = id;
                }
            }
            UnifyTextureCoords(textureCoords);
        }
        GPP::ErrorCode res = GPP::TextureImage::CreateTextureImage(textureCoords, colors, textureIds, 1024, 1024, imageData);
        if (res != GPP_NO_ERROR)
        {
            return;
        }
        cv::Mat image(imageHeight, imageWidth, CV_8UC4);
        for (int y = 0; y < imageHeight; ++y)
        {
            for (int x = 0; x < imageWidth; ++x)
            {
                cv::Vec4b& col = image.at<cv::Vec4b>(imageHeight - 1 - y, x);
                GPP::Vector3 vCol = imageData.at(x + y * imageWidth);
                col[0] = vCol[2] * 255;
                col[1] = vCol[1] * 255;
                col[2] = vCol[0] * 255;
                col[3] = 255;
            }
        }
        cv::imshow("TextureImage", image);
    }

    bool TextureApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_S)
        {
            GPP::TriMeshPointList pointList(mpTriMesh);
            GPP::Int vertexCount = pointList.GetPointCount();
            GPP::Int sampleCount = 128;
            GPP::Int* sampleIndex = new GPP::Int[sampleCount];
            GPP::ErrorCode res = GPP::SamplePointCloud::_UniformSamplePointList(&pointList, sampleCount, sampleIndex, 100, GPP::SAMPLE_QUALITY_HIGH);
            if (res != GPP_NO_ERROR)
            {
                GPPFREEARRAY(sampleIndex);
                MessageBox(NULL, "网格顶点采样失败", "温馨提示", MB_OK);
                return true;
            }
            std::vector<GPP::Real> meanCurvature;
            //GPP::MeasureMesh::ComputeMeanCurvature(mpTriMesh, meanCurvature);
            GPP::MeasureMesh::ComputeGaussCurvature(mpTriMesh, meanCurvature);
            std::multimap<GPP::Real, int> curvatureMap;
            for (int markid = 0; markid < sampleCount; markid++)
            {
                curvatureMap.insert(std::pair<GPP::Real, int>(1.0 / (1.0 + fabs(meanCurvature.at(sampleIndex[markid]))), sampleIndex[markid]));
            }
            GPPFREEARRAY(sampleIndex);
            mCurMarkCoords.clear();
            int markCount = 16;
            std::multimap<GPP::Real, int>::iterator mapItr = curvatureMap.begin();
            for (int markid = 1; markid < markCount; markid++)
            {
                mCurMarkCoords.push_back(mpTriMesh->GetVertexCoord(mapItr->second));
                mapItr++;
            }
            
            UpdateMarkDisplay(true);
        }
        else if (arg.key == OIS::KC_Y)
        {
            GenerateTextureImage(mpUVMesh);
        }
        return true;
    }

    void TextureApp::SetupScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue(0.1, 0.1, 0.1));
        Ogre::Light* light = sceneManager->createLight("TextureApp_SimpleLight");
        light->setPosition(0, 0, 20);
        light->setDiffuseColour(0.8, 0.8, 0.8);
        light->setSpecularColour(0.5, 0.5, 0.5);

        if (mpImageFrameMesh == NULL)
        {
            mpImageFrameMesh = GPP::Parser::ImportTriMesh("../../Media/TextureApp/ImageMesh.obj");
            if (mpImageFrameMesh == NULL)
            {
                MessageBox(NULL, "纹理网格导入失败", "温馨提示", MB_OK);
                return;
            }
            mpImageFrameMesh->UpdateNormal();
        }

        mTextureImageNames.clear();
        mTextureImageNames.push_back("../../Media/TextureApp/grid.png");
        mTextureImageNames.push_back("../../Media/TextureApp/pattern0.jpg");
        mTextureImageNames.push_back("../../Media/TextureApp/pattern1.jpg");
        mTextureImageNames.push_back("../../Media/TextureApp/pattern2.jpg");
        mTextureImageNames.push_back("../../Media/TextureApp/pattern3.jpg");
        mTextureImageNames.push_back("../../Media/TextureApp/pattern4.jpg");
        mTextureImageNames.push_back("../../Media/TextureApp/pattern5.jpg");
        SwitchTextureImage();

        InitViewTool();
    }

    void TextureApp::ShutdownScene()
    {
        Ogre::SceneManager* sceneManager = MagicCore::RenderSystem::Get()->GetSceneManager();
        sceneManager->setAmbientLight(Ogre::ColourValue::Black);
        sceneManager->destroyLight("TextureApp_SimpleLight");
        MagicCore::RenderSystem::Get()->SetupCameraDefaultParameter();
        if (MagicCore::RenderSystem::Get()->GetSceneManager()->hasSceneNode("ModelNode"))
        {
            MagicCore::RenderSystem::Get()->GetSceneManager()->getSceneNode("ModelNode")->resetToInitialState();
        }
        MagicCore::RenderSystem::Get()->HideRenderingObject("TriMesh_TextureApp");
        MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudList_TextureApp");
        UpdateMarkDisplay(false);
    }

    void TextureApp::ImportTriMesh()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0STL Files(*.stl)\0*.stl\0OFF Files(*.off)\0*.off\0PLY Files(*.ply)\0*.ply\0";
        if (MagicCore::ToolKit::FileOpenDlg(fileName, filterName))
        {
            GPP::TriMesh* triMesh = GPP::Parser::ImportTriMesh(fileName);
            if (triMesh != NULL)
            { 
                if (triMesh->GetMeshType() == GPP::MeshType::MT_TRIANGLE_SOUP)
                {
                    triMesh->FuseVertex();
                }
                triMesh->UnifyCoords(1.8, &mScaleValue, &mObjCenterCoord);
                triMesh->UpdateNormal();
                GPPFREEPOINTER(mpTriMesh);
                mpTriMesh = triMesh;
                InfoLog << "Import Mesh,  vertex: " << mpTriMesh->GetVertexCount() << " triangles: " << triMesh->GetTriangleCount() << std::endl;
                mDisplayMode = TRIMESH_SOLID;
                UpdateDisplay();
                // set up pick tool
                GPPFREEPOINTER(mpPickTool);
                mpPickTool = new MagicCore::PickTool;
                mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, NULL, mpTriMesh, "ModelNode");
                // Clear data
                GPPFREEPOINTER(mpUVMesh);
                mLastCutVertexId = -1;
                mCurPointsOnEdge.clear();
                mCurMarkCoords.clear();
                mCutLineList.clear();
                UpdateMarkDisplay(false);
                mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
                ClearPointCloudData();
                mUpdatePointCloudRendering = true;
            }
            else
            {
                //mpUI->SetMeshInfo(0, 0);
                MessageBox(NULL, "网格导入失败", "温馨提示", MB_OK);
            }
        }
    }

    void TextureApp::ExportTriMesh()
    {
        MessageBox(NULL, "暂时不支持导出", "温馨提示", MB_OK);
        return;
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpTriMesh == NULL)
        {
            MessageBox(NULL, "请导入需要UV展开的网格", "温馨提示", MB_OK);
            return;
        }
        if (mpUVMesh == NULL)
        {
            MessageBox(NULL, "请先对网格进行UV展开", "温馨提示", MB_OK);
            return;
        }
        std::string fileName;
        char filterName[] = "OBJ Files(*.obj)\0*.obj\0";
        if (MagicCore::ToolKit::FileSaveDlg(fileName, filterName))
        {
            UpdateTextureFromUVMesh();
            mpTriMesh->UnifyCoords(1.0 / mScaleValue, mObjCenterCoord * (-mScaleValue));
            // Export obj by texture coordinates
            std::string objName = fileName + ".obj";
            size_t dotPos = fileName.rfind('\\');
            if (dotPos == std::string::npos)
            {
                dotPos = fileName.rfind('/');
                if (dotPos == std::string::npos)
                {
                    MessageBox(NULL, "文件名字无效", "温馨提示", MB_OK);
                    return;
                }
            }
            std::string fileNameNoPath = fileName.substr(dotPos + 1);
            std::ofstream objOut(objName.c_str());
            objOut << "mtllib " << fileNameNoPath << ".mtl" << "\n";
            objOut << "usemtl " << fileNameNoPath << "\n";
            GPP::Int vertexCount = mpTriMesh->GetVertexCount();
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Vector3 coord = mpTriMesh->GetVertexCoord(vid);
                objOut << "v " << coord[0] << " " << coord[1] << " " << coord[2] << "\n";
            }
            GPP::Int faceCount = mpTriMesh->GetTriangleCount();
            for (GPP::Int fid = 0; fid < faceCount; fid++)
            {
                for (int localId = 0; localId < 3; localId++)
                {
                    GPP::Vector3 texCoord = mpTriMesh->GetTriangleTexcoord(fid, localId);
                    objOut << "vt " << texCoord[0] << " " << texCoord[1] << "\n";
                }
            }
            GPP::Int vertexIds[3];
            for (GPP::Int fid = 0; fid < faceCount; fid++)
            {
                mpTriMesh->GetTriangleVertexIds(fid, vertexIds);
                objOut << "f " << vertexIds[0] + 1 << "/" << fid * 3 + 1 << " " 
                    << vertexIds[1] + 1 << "/" << fid * 3 + 2 << " " << vertexIds[2] + 1 << "/" << fid * 3 + 3 << "\n"; 
            }
            objOut.close();
            
            // export mtl file
            std::string mtlName = fileName + ".mtl";
            std::ofstream mtlOut(mtlName.c_str());
            mtlOut << "newmtl " << fileNameNoPath << std::endl;
            mtlOut << "Kd " << 0.75 << " " << 0.75 << " " << 0.75 << std::endl;
            mtlOut << "map_Kd " << fileNameNoPath << ".png" << std::endl;
            mtlOut.close();
            
            // export grid image
            std::string imageName = fileName + ".png";
            cv::imwrite(imageName, mDistortionImage);
            //
            mpTriMesh->UnifyCoords(mScaleValue, mObjCenterCoord);
        }
    }

    void TextureApp::ConfirmGeodesics()
    {
        if (mCurPointsOnEdge.size() > 0)
        {
            mCurMarkCoords.clear();
            mLastCutVertexId = -1;
            std::vector<GPP::Int> newSplitLineIds;
            GPP::ErrorCode res = GPP::UnfoldMesh::InsertSplitLineOnTriMesh(mpTriMesh, mCurPointsOnEdge, &newSplitLineIds);
            mCurPointsOnEdge.clear();
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "割线不能自交", "温馨提示", MB_OK);
                return;
            }
            mpTriMesh->UpdateNormal();
            mCutLineList.push_back(newSplitLineIds);
            UpdateMarkDisplay(true);
            UpdateDisplay();
            mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
        }
    }

    void TextureApp::DeleteGeodesics()
    {
        mLastCutVertexId = -1;
        if (!mCurMarkCoords.empty())
        {
            mCurMarkCoords.clear();
        }
        if (!mCurPointsOnEdge.empty())
        {
            mCurPointsOnEdge.clear();
        }
        UpdateMarkDisplay(true);
    }

    void TextureApp::SwitchMarkDisplay()
    {
        static bool display = true;
        UpdateMarkDisplay(display);
        display = !display;
    }

    void TextureApp::DoCommand(bool isSubThread)
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
            case MagicApp::TextureApp::NONE:
                break;
            case MagicApp::TextureApp::UNFOLD_INITIAL:
                UnfoldTriMesh(false);
                break;
            case MagicApp::TextureApp::OPTIMIZE_ISOMETRIC:
                Optimize2Isometric(false);
                break;
            case MagicApp::TextureApp::GENERATE_UV_ATLAS:
                GenerateUVAtlas(mInitChartCount, false);
                break;
            default:
                break;
            }
            mpUI->StopProgressbar();
        }
    }

    void TextureApp::UnfoldTriMesh(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpTriMesh == NULL)
        {
            MessageBox(NULL, "请导入需要UV展开的网格", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = UNFOLD_INITIAL;
            DoCommand(true);
        }
        else
        {
            if (GPP::ConsolidateMesh::_IsTriMeshManifold(mpTriMesh) == false)
            {
                MessageBox(NULL, "网格展开失败：网格有非流形结构，请先拓扑修复", "温馨提示", MB_OK);
                return;
            }
            if (GPP::ConsolidateMesh::_IsGeometryDegenerate(mpTriMesh) == false)
            {
                if (MessageBox(NULL, "警告：网格有退化几何，继续计算可能得到无效结果，可以先几何修复，是否继续？", "温馨提示", MB_OKCANCEL) != IDOK)
                {
                    return;
                }
            }
            GenerateUVMesh();
            // Generate fixed vertex
            std::vector<std::vector<GPP::Int> > holeIds;
            GPP::ErrorCode res = GPP::FillMeshHole::FindHoles(mpUVMesh, &holeIds);
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "找网格边界失败", "温馨提示", MB_OK);
                return;
            }
            if (holeIds.empty())
            {
                MessageBox(NULL, "网格没有边界，不能展开", "温馨提示", MB_OK);
                return;
            }
            // Is single connected region
            std::vector<GPP::Real> isolation;
            GPP::Int uvVertexCount = mpUVMesh->GetVertexCount();
            res = GPP::ConsolidateMesh::CalculateIsolation(mpUVMesh, &isolation);
            for (GPP::Int vid = 0; vid < uvVertexCount; vid++)
            {
                if (isolation.at(vid) < 1)
                {
                    MessageBox(NULL, "网格展开失败：网格需要是单连通区域", "温馨提示", MB_OK);
                    return;
                }
            }
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格展开失败：网格连同区域检测失败", "温馨提示", MB_OK);
                return;
            }
            std::vector<GPP::Int> fixedVertexIndices(2);
            std::vector<GPP::Real> fixedVertexCoords(4);
            fixedVertexIndices.at(0) = holeIds.at(0).at(0);
            fixedVertexIndices.at(1) = holeIds.at(0).at(holeIds.at(0).size() / 2);
            fixedVertexCoords.at(0) = -1.0;
            fixedVertexCoords.at(1) = -1.0;
            fixedVertexCoords.at(2) = 1.0;
            fixedVertexCoords.at(3) = 1.0;
            std::vector<GPP::Real> texCoords;
            mIsCommandInProgress = true;
            res = GPP::UnfoldMesh::ConformalMap(mpUVMesh, &fixedVertexIndices, &fixedVertexCoords, &texCoords);
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            mIsCommandInProgress = false;
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格展开失败", "温馨提示", MB_OK);
                return;
            }
            UnifyTextureCoords(texCoords, 2.4);
            GPP::Int vertexCount = mpUVMesh->GetVertexCount();
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                mpUVMesh->SetVertexTexcoord(vid, GPP::Vector3(texCoords.at(vid * 2), texCoords.at(vid * 2 + 1), 0));
            }
            mDisplayMode = UVMESH_WIREFRAME;
            mUpdateDisplay = true;
            mHideMarks = true;
        }
    }

    void TextureApp::Optimize2Isometric(bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpTriMesh == NULL)
        {
            MessageBox(NULL, "请导入需要UV展开的网格", "温馨提示", MB_OK);
            return;
        }
        if (mpUVMesh == NULL)
        {
            MessageBox(NULL, "请先计算初始UV", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mCommandType = OPTIMIZE_ISOMETRIC;
            DoCommand(true);
        }
        else
        {
            if (GPP::ConsolidateMesh::_IsTriMeshManifold(mpUVMesh) == false)
            {
                MessageBox(NULL, "网格展开失败：网格有非流形结构，请先拓扑修复", "温馨提示", MB_OK);
                return;
            }
            if (GPP::ConsolidateMesh::_IsGeometryDegenerate(mpUVMesh) == false)
            {
                if (MessageBox(NULL, "警告：网格有退化几何，继续计算可能得到无效结果，可以先几何修复，是否继续？", "温馨提示", MB_OKCANCEL) != IDOK)
                {
                    return;
                }
            }
            // Is single connected region
            std::vector<GPP::Real> isolation;
            GPP::Int uvVertexCount = mpUVMesh->GetVertexCount();
            GPP::ErrorCode res = GPP::ConsolidateMesh::CalculateIsolation(mpUVMesh, &isolation);
            for (GPP::Int vid = 0; vid < uvVertexCount; vid++)
            {
                if (isolation.at(vid) < 1)
                {
                    MessageBox(NULL, "网格展开失败：网格需要是单连通区域", "温馨提示", MB_OK);
                    return;
                }
            }
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格展开失败：网格连同区域检测失败", "温馨提示", MB_OK);
                return;
            }
            GPP::Int vertexCount = mpUVMesh->GetVertexCount();
            std::vector<GPP::Real> texCoords(vertexCount * 2);
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Vector3 texCoord = mpUVMesh->GetVertexTexcoord(vid);
                texCoords.at(vid * 2) = texCoord[0];
                texCoords.at(vid * 2 + 1) = texCoord[1];
            }
            mIsCommandInProgress = true;
            res = GPP::UnfoldMesh::OptimizeIsometric(mpUVMesh, &texCoords, 10, NULL);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "网格UV优化失败", "温馨提示", MB_OK);
                return;
            }
            UnifyTextureCoords(texCoords, 2.4);
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                mpUVMesh->SetVertexTexcoord(vid, GPP::Vector3(texCoords.at(vid * 2), texCoords.at(vid * 2 + 1), 0));
                //mpTriMesh->SetVertexCoord(vid, GPP::Vector3(texCoords.at(vid * 2), texCoords.at(vid * 2 + 1), 0));
            }
            mpTriMesh->UpdateNormal();
            mUpdateDisplay = true;
        }
    }

    void TextureApp::GenerateUVAtlas(int initChartCount, bool isSubThread)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpTriMesh == NULL)
        {
            MessageBox(NULL, "请导入需要UV展开的网格", "温馨提示", MB_OK);
            return;
        }
        if (isSubThread)
        {
            mInitChartCount = initChartCount;
            mCommandType = GENERATE_UV_ATLAS;
            DoCommand(true);
        }
        else
        {
            if (GPP::ConsolidateMesh::_IsTriMeshManifold(mpTriMesh) == false)
            {
                MessageBox(NULL, "网格展开失败：网格有非流形结构，请先拓扑修复", "温馨提示", MB_OK);
                return;
            }
            if (GPP::ConsolidateMesh::_IsGeometryDegenerate(mpTriMesh) == false)
            {
                if (MessageBox(NULL, "警告：网格有退化几何，继续计算可能得到无效结果，可以先几何修复，是否继续？", "温馨提示", MB_OKCANCEL) != IDOK)
                {
                    return;
                }
            }
            GenerateUVMesh(true);
            std::vector<GPP::Real> texCoords;
            std::vector<GPP::Int> faceTexIds;
            bool needInitSplit = mCutLineList.empty();
            bool needSplitFoldOver = needInitSplit;
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::UnfoldMesh::GenerateUVAtlas(mpUVMesh, initChartCount, needInitSplit, needSplitFoldOver, &texCoords, &faceTexIds);
            mIsCommandInProgress = false;
            if (res == GPP_API_IS_NOT_AVAILABLE)
            {
                MessageBox(NULL, "软件试用时限到了，欢迎购买激活码", "温馨提示", MB_OK);
                MagicCore::ToolKit::Get()->SetAppRunning(false);
            }
            if (res != GPP_NO_ERROR)
            {
                MessageBox(NULL, "UV Atlas 生成失败", "温馨提示", MB_OK);
                return;
            }
            GPPFREEPOINTER(mpUVMesh);
            UnifyTextureCoords(texCoords, 2.4);
            mpUVMesh = new GPP::TriMesh;
            int uvVertexCount = texCoords.size() / 2;
            std::vector<GPP::Vector3> uvVertexCoords(uvVertexCount);
            std::vector<GPP::Vector3> uvVertexColors(uvVertexCount);
            int uvFaceCount = faceTexIds.size() / 3;
            int vertexIds[3] = {-1};
            for (int fid = 0; fid < uvFaceCount; fid++)
            {
                mpTriMesh->GetTriangleVertexIds(fid, vertexIds);
                for (int localId = 0; localId < 3; localId++)
                {
                    GPP::Int tid = faceTexIds.at(fid * 3 + localId);
                    uvVertexCoords.at(faceTexIds.at(fid * 3 + localId)) = mpTriMesh->GetVertexCoord(vertexIds[localId]);
                    uvVertexColors.at(faceTexIds.at(fid * 3 + localId)) = mpTriMesh->GetVertexColor(vertexIds[localId]);
                    mpTriMesh->SetTriangleTexcoord(fid, localId, GPP::Vector3(texCoords.at(tid * 2), texCoords.at(tid * 2 + 1), 0));
                }
            }
            for (int vid = 0; vid < uvVertexCount; vid++)
            {
                mpUVMesh->InsertVertex(uvVertexCoords.at(vid));
                mpUVMesh->SetVertexTexcoord(vid, GPP::Vector3(texCoords.at(vid * 2), texCoords.at(vid * 2 + 1), 0));
                mpUVMesh->SetVertexColor(vid, uvVertexColors.at(vid));
            }
            for (int fid = 0; fid < uvFaceCount; fid++)
            {
                mpUVMesh->InsertTriangle(faceTexIds.at(fid * 3), faceTexIds.at(fid * 3 + 1), faceTexIds.at(fid * 3 + 2));
            }
            mpUVMesh->UpdateNormal();
            
            mDisplayMode = UVMESH_WIREFRAME;
            mUpdateDisplay = true;
            mHideMarks = true;
        }
    }

    void TextureApp::EnterMeshToolApp()
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mpTriMesh == NULL)
        {
            AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
            return;
        }
        GPP::TriMesh* copyMesh = GPP::CopyTriMesh(mpTriMesh);
        AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
        MeshShopApp* meshshopApp = dynamic_cast<MeshShopApp*>(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshshopApp)
        {
            meshshopApp->SetMesh(copyMesh, mObjCenterCoord, mScaleValue);
            copyMesh = NULL;
        }
        else
        {
            GPPFREEPOINTER(copyMesh);
        }
    }

    void TextureApp::OptimizeColorConsistency()
    {
        std::vector<std::string> fileNames;
        char filterName[] = "Geometry++ Point Cloud(*.gpc)\0*.gpc\0";
        if (MagicCore::ToolKit::MultiFileOpenDlg(fileNames, filterName))
        {
            if (fileNames.size() > 0)
            {
                for (std::vector<GPP::PointCloud*>::iterator pitr = mPointCloudList.begin(); pitr != mPointCloudList.end(); ++pitr)
                {
                    GPPFREEPOINTER(*pitr);
                }
                mPointCloudList.clear();
                for (int fileId = 0; fileId < fileNames.size(); fileId++)
                {
                    GPP::PointCloud* pointCloud = GPP::Parser::ImportPointCloud(fileNames.at(fileId));
                    if (pointCloud != NULL)
                    {
                        if (mPointCloudList.empty())
                        {
                            pointCloud->UnifyCoords(2.0, &mScaleValue, &mObjCenterCoord);
                        }
                        else
                        {
                            pointCloud->UnifyCoords(mScaleValue, mObjCenterCoord);
                        }
                        mPointCloudList.push_back(pointCloud);
                    }
                }
                //
                std::vector<GPP::IPointCloud*> pointCloudList;
                GPP::Int pointCountTotal = 0;
                for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
                {
                    pointCountTotal += (*itr)->GetPointCount();
                    pointCloudList.push_back(*itr);
                }
                std::vector<GPP::Real> colorList;
                colorList.reserve(pointCountTotal * 3);
                for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
                {
                    GPP::Int pointCountLocal = (*itr)->GetPointCount();
                    for (GPP::Int pid = 0; pid < pointCountLocal; pid++)
                    {
                        GPP::Vector3 color = (*itr)->GetPointColor(pid);
                        colorList.push_back(color[0]);
                        colorList.push_back(color[1]);
                        colorList.push_back(color[2]);
                    }
                }
                std::vector<GPP::Real> albedoList, shadingList;
                GPP::ErrorCode res = GPP::IntrinsicColor::DecomposePointCloudColor(pointCloudList, colorList, albedoList, shadingList);
                if (res != GPP_NO_ERROR)
                {
                    MessageBox(NULL, "颜色分解失败", "温馨提示", MB_OK);
                    return;
                }
                GPP::Int pointId = 0;
                GPP::Real r, g, b;
                for (std::vector<GPP::PointCloud*>::iterator itr = mPointCloudList.begin(); itr != mPointCloudList.end(); ++itr)
                {
                    GPP::Int pointCountLocal = (*itr)->GetPointCount();
                    for (GPP::Int pid = 0; pid < pointCountLocal; pid++)
                    {
                        r = albedoList.at(pointId * 3);
                        r = r > 1 ? 1 : (r < 0 ? 0 : r);
                        g = albedoList.at(pointId * 3 + 1);
                        g = g > 1 ? 1 : (g < 0 ? 0 : g);
                        b = albedoList.at(pointId * 3 + 2);
                        b = b > 1 ? 1 : (b < 0 ? 0 : b);
                        (*itr)->SetPointColor(pid, GPP::Vector3(r, g, b));
                        pointId++;
                    }
                }
                //
                mUpdatePointCloudRendering = true;
                ClearMeshData();
                mUpdateDisplay = true;
                mHideMarks = true;
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }

    }

    void TextureApp::EnterPointToolApp(void)
    {
        if (IsCommandAvaliable() == false)
        {
            return;
        }
        if (mPointCloudList.empty())
        {
            AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
            return;
        }
        GPP::PointCloud* pointCloud = new GPP::PointCloud;
        for (std::vector<GPP::PointCloud*>::iterator pitr = mPointCloudList.begin(); pitr != mPointCloudList.end(); ++pitr)
        {
            GPP::Int pointCount = (*pitr)->GetPointCount();
            for (GPP::Int pid = 0; pid < pointCount; pid++)
            {
                pointCloud->InsertPoint((*pitr)->GetPointCoord(pid), (*pitr)->GetPointNormal(pid));
                pointCloud->SetPointColor(pid, (*pitr)->GetPointColor(pid));
            }
        }
        pointCloud->SetHasNormal(true);
        pointCloud->SetHasColor(true);
        AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
        PointShopApp* pointShop = dynamic_cast<PointShopApp*>(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop)
        {
            pointShop->SetPointCloud(pointCloud, mObjCenterCoord, mScaleValue);
        }
        else
        {
            GPPFREEPOINTER(pointCloud);
        }
    }

    void TextureApp::SetMesh(GPP::TriMesh* triMesh, GPP::Vector3 objCenterCoord, GPP::Real scaleValue)
    {
        if (triMesh == NULL)
        {
            return;
        }
        GPPFREEPOINTER(mpTriMesh);
        mpTriMesh = triMesh;
        mObjCenterCoord = objCenterCoord;
        mScaleValue = scaleValue;
        mDisplayMode = TRIMESH_SOLID;
        UpdateDisplay();
        // set up pick tool
        GPPFREEPOINTER(mpPickTool);
        mpPickTool = new MagicCore::PickTool;
        mpPickTool->SetPickParameter(MagicCore::PM_POINT, true, NULL, mpTriMesh, "ModelNode");
        // Clear data
        GPPFREEPOINTER(mpUVMesh);
        mLastCutVertexId = -1;
        mCurPointsOnEdge.clear();
        mCurMarkCoords.clear();
        mCutLineList.clear();
        UpdateMarkDisplay(false);
        mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
    }

    void TextureApp::SwitchDisplayMode()
    {
        if (mDisplayMode == TRIMESH_SOLID)
        {
            mDisplayMode = TRIMESH_WIREFRAME;
        }
        else if (mDisplayMode == TRIMESH_WIREFRAME)
        {
            if (mpUVMesh)
            {
                mDisplayMode = UVMESH_WIREFRAME;
            }
            else
            {
                mDisplayMode = TRIMESH_SOLID;
            }
        }
        else if (mDisplayMode == UVMESH_WIREFRAME)
        {
            mDisplayMode = TRIMESH_TEXTURE;
        }
        else if (mDisplayMode == TRIMESH_TEXTURE)
        {
            mDisplayMode = TRIMESH_SOLID;
        }
        
        UpdateDisplay();
    }

    void TextureApp::SwitchTextureImage()
    {
        mDistortionImage.release();
        mDistortionImage = cv::imread(mTextureImageNames.at(mCurrentTextureImageId));
        mCurrentTextureImageId = (mCurrentTextureImageId + 1) % mTextureImageNames.size();
        if (mDistortionImage.data != NULL)
        {
            mTextureImageSize = mDistortionImage.rows;
        }
        else
        {
            MessageBox(NULL, "图片打开失败", "温馨提示", MB_OK);
        }
        UpdateDisplay();
    }

    void TextureApp::InitViewTool()
    {
        if (mpViewTool == NULL)
        {
            mpViewTool = new MagicCore::ViewTool;
            mpViewTool->SetRightNodeFixed(true);
        }
    }

    void TextureApp::UpdateDisplay()
    {
        if (mDisplayMode == TRIMESH_SOLID)
        {
            if (mpTriMesh == NULL)
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("TriMesh_TextureApp");
                return;
            }
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
            MagicCore::RenderSystem::Get()->RenderMesh("TriMesh_TextureApp", "CookTorrance", mpTriMesh, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else if (mDisplayMode == TRIMESH_WIREFRAME)
        {
            if (mpTriMesh == NULL)
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("TriMesh_TextureApp");
                return;
            }
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
            MagicCore::RenderSystem::Get()->RenderMesh("TriMesh_TextureApp", "CookTorrance", mpTriMesh, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else if (mDisplayMode == TRIMESH_TEXTURE)
        {
            if (mpTriMesh == NULL)
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("TriMesh_TextureApp");
                return;
            }
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
            UpdateTriMeshTexture();
            MagicCore::RenderSystem::Get()->RenderTextureMesh("TriMesh_TextureApp", "TextureMeshMaterial", mpUVMesh, MagicCore::RenderSystem::MODEL_NODE_LEFT);
        }
        else if (mDisplayMode == UVMESH_WIREFRAME)
        {
            if (mpUVMesh == NULL)
            {
                MagicCore::RenderSystem::Get()->HideRenderingObject("TriMesh_TextureApp");
                return;
            }
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
            MagicCore::RenderSystem::Get()->RenderUVMesh("TriMesh_TextureApp", "CookTorrance", mpUVMesh, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
    }

    void TextureApp::UpdateMarkDisplay(bool display)
    {
        if (display)
        {
            std::vector<GPP::Vector3> cutLines;
            for (std::vector<std::vector<GPP::Int> >::iterator lineItr = mCutLineList.begin(); lineItr != mCutLineList.end(); ++lineItr)
            {
                for (std::vector<GPP::Int>::iterator vertexItr = lineItr->begin(); vertexItr != lineItr->end(); ++vertexItr)
                {
                    cutLines.push_back(mpTriMesh->GetVertexCoord(*vertexItr));
                }
            }
            MagicCore::RenderSystem::Get()->RenderPointList("CutLine_TextureApp", "SimplePoint_Large", GPP::Vector3(1, 0, 0), cutLines, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        
            std::vector<GPP::Vector3> cutVertices;
            for (std::vector<GPP::Vector3>::iterator coordItr = mCurMarkCoords.begin(); coordItr != mCurMarkCoords.end(); ++coordItr)
            {
                cutVertices.push_back(*coordItr);
            }
            if (mLastCutVertexId != -1)
            {
                cutVertices.push_back(mpTriMesh->GetVertexCoord(mLastCutVertexId));
            }
            MagicCore::RenderSystem::Get()->RenderPointList("CutVertex_TextureApp", "SimplePoint_Large", GPP::Vector3(1, 1, 0), cutVertices, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("CutLine_TextureApp");
            MagicCore::RenderSystem::Get()->HideRenderingObject("CutVertex_TextureApp");
        }
    }

    void TextureApp::UpdatePointCloudRendering()
    {
        if (mPointCloudList.empty())
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudList_TextureApp");
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("PointCloudList_TextureApp");
            MagicCore::RenderSystem::Get()->RenderPointCloudList("PointCloudList_TextureApp", "SimplePoint_Large", 
                mPointCloudList, false, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
    }

    void TextureApp::UpdateTriMeshTexture()
    {
        if (mpTriMeshTexture.isNull())
        {
            mpTriMeshTexture = Ogre::TextureManager::getSingleton().createManual(  
                "TriMeshTexture",      // name  
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,  
                Ogre::TEX_TYPE_2D,   // type  
                mTextureImageSize,  // width  
                mTextureImageSize, // height  
                0,                   // number of mipmaps  
                Ogre::PF_B8G8R8A8,   // pixel format  
                Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE // usage, for textures updated very often  
                );
        }

        if (mDistortionImage.data != NULL)
        {
            // Get the pixel buffer  
            Ogre::HardwarePixelBufferSharedPtr pixelBuffer = mpTriMeshTexture->getBuffer();  
            cv::Mat textureImage = mDistortionImage;;
            // Lock the pixel buffer and get a pixel box  
            unsigned char* buffer = static_cast<unsigned char*>(  
                pixelBuffer->lock(0, mTextureImageSize * mTextureImageSize * 4, Ogre::HardwareBuffer::HBL_DISCARD) ); 
            int imgW = textureImage.cols;
            int imgH = textureImage.rows;
            for(int y = 0; y < mTextureImageSize; ++y)  
            {  
                for(int x = 0; x < mTextureImageSize; ++x)  
                {
                    int baseIndex = y * mTextureImageSize + x;
                    if (x < imgW && y < imgH)
                    {
                        const unsigned char* pixel = textureImage.ptr(imgH - y - 1, x);
                        *buffer++ = pixel[0];
                        *buffer++ = pixel[1];
                        *buffer++ = pixel[2];
                        *buffer++ = 255;
                    }
                    else
                    {
                        *buffer++ = 255;
                        *buffer++ = 255;
                        *buffer++ = 255;
                        *buffer++ = 255;
                    } 
                }  
            }  
            // Unlock the pixel buffer  
            pixelBuffer->unlock();
        }
          
        if (Ogre::MaterialManager::getSingleton().getByName("TextureMeshMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).isNull())
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create("TextureMeshMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME); 
            material->getTechnique(0)->getPass(0)->createTextureUnitState("TriMeshTexture");
            material->getTechnique(0)->getPass(0)->setDiffuse(0.7, 0.7, 0.7, 1.0);
            material->getTechnique(0)->getPass(0)->setSpecular(0.55, 0.55, 0.55, 1.0);
            material->getTechnique(0)->getPass(0)->setShininess(100);
        }
    }

    void TextureApp::GenerateUVMesh(bool bWithTextureCoords)
    {
        GPPFREEPOINTER(mpUVMesh);
        mpUVMesh = GPP::CopyTriMesh(mpTriMesh);
        if (bWithTextureCoords)
        {
            GPP::CopyTriMeshTextureCoordinates(mpTriMesh, mpUVMesh);
        }
        if (!mCutLineList.empty())
        {
            GPP::UnfoldMesh::SplitTriMesh(mpUVMesh, mCutLineList);
            mpUVMesh->UpdateNormal();
        }
    }

    void TextureApp::UpdateTextureFromUVMesh()
    {
        if (mpUVMesh == NULL)
        {
            return;
        }
        GPP::Int faceCount = mpTriMesh->GetTriangleCount();
        if (mpUVMesh->GetTriangleCount() != faceCount)
        {
            MessageBox(NULL, "UV网格面片数量有误", "温馨提示", MB_OK);
            return;
        }
        GPP::Int vertexIds[3];
        for (GPP::Int fid = 0; fid < faceCount; fid++)
        {
            mpUVMesh->GetTriangleVertexIds(fid, vertexIds);
            for (int localId = 0; localId < 3; localId++)
            {
                mpTriMesh->SetTriangleTexcoord(fid, localId, mpUVMesh->GetVertexTexcoord(vertexIds[localId]));
            }
        }
    }

    void TextureApp::UnifyTextureCoords(std::vector<double>& texCoords, double scaleValue)
    {
        int vertexCount = texCoords.size() / 2;
        GPP::Real texCoordMax_X = texCoords.at(0);
        GPP::Real texCoordMax_Y = texCoords.at(1);
        GPP::Real texCoordMin_X = texCoords.at(0);
        GPP::Real texCoordMin_Y = texCoords.at(1);
        for (GPP::Int vid = 1; vid < vertexCount; vid++)
        {
            if (texCoords.at(vid * 2) > texCoordMax_X)
            {
                texCoordMax_X = texCoords.at(vid * 2);
            }
            else if (texCoords.at(vid * 2) < texCoordMin_X)
            {
                texCoordMin_X = texCoords.at(vid * 2);
            }
            if (texCoords.at(vid * 2 + 1) > texCoordMax_Y)
            {
                texCoordMax_Y = texCoords.at(vid * 2 + 1);
            }
            else if (texCoords.at(vid * 2 + 1) < texCoordMin_Y)
            {
                texCoordMin_Y = texCoords.at(vid * 2 + 1);
            }
        }

        GPP::Real range_X = texCoordMax_X - texCoordMin_X;
        GPP::Real range_Y = texCoordMax_Y - texCoordMin_Y;
        GPP::Real range_max = range_X > range_Y ? range_X : range_Y;
        if (range_max > GPP::REAL_TOL)
        {
            GPP::Real scaleV = scaleValue / range_max;
            GPP::Real center_X = (texCoordMax_X + texCoordMin_X) / 2.0;
            GPP::Real center_Y = (texCoordMax_Y + texCoordMin_Y) / 2.0;
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                texCoords.at(vid * 2) = (texCoords.at(vid * 2) - center_X) * scaleV;
                texCoords.at(vid * 2 + 1) = (texCoords.at(vid * 2 + 1) - center_Y) * scaleV;
            }
        }
    }
}
