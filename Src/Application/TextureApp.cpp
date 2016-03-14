#include "stdafx.h"
#include <process.h>
#include "TextureApp.h"
#include "TextureAppUI.h"
#include "AppManager.h"
#include "MeshShopApp.h"
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
        mCurMarkIds(),
        mCutLineList()
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
    }

    void TextureApp::ClearData()
    {
        GPPFREEPOINTER(mpUI);
        GPPFREEPOINTER(mpTriMesh);
        GPPFREEPOINTER(mpUVMesh);
        GPPFREEPOINTER(mpImageFrameMesh);
        GPPFREEPOINTER(mpViewTool);
        GPPFREEPOINTER(mpPickTool);
        mDisplayMode = TRIMESH_SOLID;
        mDistortionImage.release();
        mCurMarkIds.clear();
        mCutLineList.clear();
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
        else if (arg.state.buttonDown(OIS::MB_Right) && mIsCommandInProgress == false && mpPickTool)
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
        if (mIsCommandInProgress == false && id == OIS::MB_Right && mpPickTool)
        {
            mpPickTool->MouseReleased(arg.state.X.abs, arg.state.Y.abs);
            GPP::Int pickedId = mpPickTool->GetPickVertexId();;
            mpPickTool->ClearPickedIds();
            if (pickedId != -1)
            {
                if (mCurMarkIds.empty())
                {
                    mCurMarkIds.push_back(pickedId);
                }
                else
                {
                    std::vector<GPP::Int> sectionVertexIds;
                    sectionVertexIds.push_back(mCurMarkIds.at(mCurMarkIds.size() - 1));
                    sectionVertexIds.push_back(pickedId);
                    std::vector<GPP::Int> pathVertexIds;
                    GPP::Real distance = 0;
                    GPP::ErrorCode res = GPP::MeasureMesh::ComputeApproximateGeodesics(mpTriMesh, sectionVertexIds, false, 
                        pathVertexIds, distance);
                    if (res != GPP_NO_ERROR)
                    {
                        MessageBox(NULL, "测量失败", "温馨提示", MB_OK);
                        return true;
                    }
                    for (int pvid = 1; pvid < pathVertexIds.size(); pvid++)
                    {
                        mCurMarkIds.push_back(pathVertexIds.at(pvid));
                    }
                }
                UpdateMarkDisplay(true);
            }
        }
        return true;
    }

    bool TextureApp::KeyPressed( const OIS::KeyEvent &arg )
    {
        if (arg.key == OIS::KC_S)
        {
            GPP::TriMeshPointList pointList(mpTriMesh);
            GPP::Int vertexCount = pointList.GetPointCount();
            GPP::Int sampleCount = 10;
            GPP::Int* sampleIndex = new GPP::Int[sampleCount];
            GPP::ErrorCode res = GPP::SamplePointCloud::UniformSamplePointList(&pointList, sampleCount, sampleIndex, 100, GPP::SAMPLE_QUALITY_HIGH);
            if (res != GPP_NO_ERROR)
            {
                GPPFREEARRAY(sampleIndex);
                MessageBox(NULL, "网格顶点采样失败", "温馨提示", MB_OK);
                return true;
            }
            if (mCurMarkIds.size() > 0)
            {
                sampleCount = mCurMarkIds.size();
                GPPFREEARRAY(sampleIndex);
                sampleIndex = new GPP::Int[sampleCount];
                for (int sid = 0; sid < sampleCount; sid++)
                {
                    sampleIndex[sid] = mCurMarkIds.at(sid);
                }
            }
            std::vector<GPP::Int> seedVertexIds;
            for (int sid = 0; sid < sampleCount; sid++)
            {
                seedVertexIds.push_back(sampleIndex[sid]);
            }
            std::vector<GPP::Int> segmentRes;
            res = GPP::SegmentMesh::RegionGrowingFromSeeds(mpTriMesh, &seedVertexIds, &segmentRes);
            if (res != GPP_NO_ERROR)
            {
                GPPFREEARRAY(sampleIndex);
                MessageBox(NULL, "网格分割失败", "温馨提示", MB_OK);
                return true;
            }
            double colorDelta = 1.0 / sampleCount;
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Vector3 vColor = MagicCore::ToolKit::ColorCoding(0.2 + segmentRes.at(vid) * colorDelta);
                mpTriMesh->SetVertexColor(vid, vColor);
            }
            for (int sid = 0; sid < sampleCount; sid++)
            {
                mpTriMesh->SetVertexColor(sampleIndex[sid], GPP::Vector3(0, 0, 0));
            }
            GPPFREEARRAY(sampleIndex);
            UpdateDisplay();
        }
        else if (arg.key == OIS::KC_C)
        {
            std::vector<GPP::Real> curvature;
            GPP::MeasureMesh::ComputeMeanCurvature(mpTriMesh, curvature);
            GPP::Int vertexCount = mpTriMesh->GetVertexCount();
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                mpTriMesh->SetVertexColor(vid, MagicCore::ToolKit::ColorCoding(0.6 + curvature.at(vid) / 10.0));
            }
            UpdateDisplay();
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

        mDistortionImage.release();
        mDistortionImage = cv::imread("../../Media/TextureApp/grid.png");
        if (mDistortionImage.data != NULL)
        {
            mTextureImageSize = mDistortionImage.rows;
        }
        else
        {
            MessageBox(NULL, "图片打开失败", "温馨提示", MB_OK);
        }
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
                mCurMarkIds.clear();
                mCutLineList.clear();
                UpdateMarkDisplay(false);
                mpUI->SetMeshInfo(mpTriMesh->GetVertexCount(), mpTriMesh->GetTriangleCount());
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
        if (mCurMarkIds.size() > 0)
        {
            mCutLineList.push_back(mCurMarkIds);
            mCurMarkIds.clear();
            UpdateMarkDisplay(true);
        }
    }

    void TextureApp::DeleteGeodesics()
    {
        if (mCurMarkIds.empty())
        {
            if (mCutLineList.size() > 0)
            {
                mCutLineList.pop_back();
                UpdateMarkDisplay(true);
            }
        }
        else
        {
            mCurMarkIds.clear();
            UpdateMarkDisplay(true);
        }
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
            GPP::Int vertexCount = mpUVMesh->GetVertexCount();
            std::vector<GPP::Real> texCoords(vertexCount * 2);
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                GPP::Vector3 texCoord = mpUVMesh->GetVertexTexcoord(vid);
                texCoords.at(vid * 2) = texCoord[0];
                texCoords.at(vid * 2 + 1) = texCoord[1];
            }
            std::vector<GPP::Int> fixedVertexIndex;
            GPP::Int vertexIds[3] = {-1};
            mpUVMesh->GetTriangleVertexIds(0, vertexIds);
            fixedVertexIndex.push_back(vertexIds[0]);
            fixedVertexIndex.push_back(vertexIds[1]);
            mIsCommandInProgress = true;
            GPP::ErrorCode res = GPP::UnfoldMesh::OptimizeIsometric(mpUVMesh, &texCoords, 10, &fixedVertexIndex);
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
            for (GPP::Int vid = 0; vid < vertexCount; vid++)
            {
                mpUVMesh->SetVertexTexcoord(vid, GPP::Vector3(texCoords.at(vid * 2), texCoords.at(vid * 2 + 1), 0));
            }
            mUpdateDisplay = true;
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
        mCurMarkIds.clear();
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
                return;
            }
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_SOLID);
            MagicCore::RenderSystem::Get()->RenderMesh("TriMesh_TextureApp", "CookTorrance", mpTriMesh, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else if (mDisplayMode == TRIMESH_WIREFRAME)
        {
            if (mpTriMesh == NULL)
            {
                return;
            }
            MagicCore::RenderSystem::Get()->GetMainCamera()->setPolygonMode(Ogre::PolygonMode::PM_WIREFRAME);
            MagicCore::RenderSystem::Get()->RenderMesh("TriMesh_TextureApp", "CookTorrance", mpTriMesh, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else if (mDisplayMode == TRIMESH_TEXTURE)
        {
            if (mpTriMesh == NULL)
            {
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
            for (std::vector<GPP::Int>::iterator vertexItr = mCurMarkIds.begin(); vertexItr != mCurMarkIds.end(); ++vertexItr)
            {
                cutVertices.push_back(mpTriMesh->GetVertexCoord(*vertexItr));
            }
            MagicCore::RenderSystem::Get()->RenderPointList("CutVertex_TextureApp", "SimplePoint_Large", GPP::Vector3(1, 1, 0), cutVertices, MagicCore::RenderSystem::MODEL_NODE_CENTER);
        }
        else
        {
            MagicCore::RenderSystem::Get()->HideRenderingObject("CutLine_TextureApp");
            MagicCore::RenderSystem::Get()->HideRenderingObject("CutVertex_TextureApp");
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

    void TextureApp::GenerateUVMesh()
    {
        GPPFREEPOINTER(mpUVMesh);
        if (mCutLineList.empty())
        {
            mpUVMesh = GPP::CopyTriMesh(mpTriMesh);
        }
        else
        {
            mpUVMesh = GPP::SplitTriMesh(mpTriMesh, mCutLineList);
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
}
