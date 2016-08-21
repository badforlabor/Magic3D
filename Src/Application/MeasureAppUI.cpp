#include "MeasureAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "MeasureApp.h"

namespace MagicApp
{
    MeasureAppUI::MeasureAppUI() :
        mIsProgressbarVisible(false),
        mTextInfo(NULL),
        mVertexCount(0),
        mTriangleCount(0),
        mArea(0),
        mVolume(0),
        mGeodesicsDistance(0)
    {
    }

    MeasureAppUI::~MeasureAppUI()
    {
    }

    void MeasureAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/MeasureApp", "FileSystem", "MeasureApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("MeasureApp.layout");
        mRoot.at(0)->findWidget("But_DisplayMode")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::SwitchDisplayMode);
        
        mRoot.at(0)->findWidget("But_ImportModelRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ImportModel);

        mRoot.at(0)->findWidget("But_Geodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::Geodesics);
        mRoot.at(0)->findWidget("But_DeleteMeshRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::DeleteMeshMark);
        mRoot.at(0)->findWidget("But_ApproximateGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ComputeApproximateGeodesics);
        mRoot.at(0)->findWidget("But_QuickExactGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::FastComputeExactGeodesics);
        mRoot.at(0)->findWidget("But_ExactGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ComputeExactGeodesics);

        mRoot.at(0)->findWidget("But_MeasureRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::MeasureModel);
        mRoot.at(0)->findWidget("But_AreaRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::MeasureArea);
        mRoot.at(0)->findWidget("But_VolumeRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::MeasureVolume);
        mRoot.at(0)->findWidget("But_CurvatureRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::MeasureCurvature);

        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::BackToHomepage);

        mTextInfo = mRoot.at(0)->findWidget("Text_Info")->castType<MyGUI::TextBox>();
        mTextInfo->setTextColour(MyGUI::Colour(75.0 / 255.0, 131.0 / 255.0, 128.0 / 255.0));
    }

    void MeasureAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("MeasureApp");
    }

    void MeasureAppUI::StartProgressbar(int range)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(true);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressRange(range);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(0);
        mIsProgressbarVisible = true;
    }

    void MeasureAppUI::SetProgressbar(int value)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(value);
    }

    void MeasureAppUI::StopProgressbar()
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(false);
        mIsProgressbarVisible = false;
    }

    bool MeasureAppUI::IsProgressbarVisible()
    {
        return mIsProgressbarVisible;
    }

    void MeasureAppUI::SetModelInfo(int vertexCount, int triangleCount)
    {
        mVertexCount = vertexCount;
        mTriangleCount = triangleCount;
        UpdateTextInfo();
    }

    void MeasureAppUI::SetModelArea(double area)
    {
        mArea = area;
        UpdateTextInfo();
    }

    void MeasureAppUI::SetModelVolume(double volume)
    {
        mVolume = volume;
        UpdateTextInfo();
    }

    void MeasureAppUI::SetGeodesicsInfo(double distance)
    {
        mGeodesicsDistance = distance;
        UpdateTextInfo();
    }

    void MeasureAppUI::SwitchDisplayMode(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->SwitchDisplayMode();
        }
    }

    void MeasureAppUI::ImportModel(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->ImportModel();
        }
    }

    void MeasureAppUI::Geodesics(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DeleteMeshRef")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DeleteMeshRef")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_ApproximateGeodesics")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_QuickExactGeodesics")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_ExactGeodesics")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_GeodesicAccuracy")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::string textString = "0.5";
            mRoot.at(0)->findWidget("Edit_GeodesicAccuracy")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_GeodesicAccuracy")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }

    void MeasureAppUI::DeleteMeshMark(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->DeleteMeshMark();
        }
    }

    void MeasureAppUI::ComputeApproximateGeodesics(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->ComputeApproximateGeodesics();
        }
    }

    void MeasureAppUI::FastComputeExactGeodesics(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            std::string textString = mRoot.at(0)->findWidget("Edit_GeodesicAccuracy")->castType<MyGUI::EditBox>()->getOnlyText();
            double accuary = std::atof(textString.c_str());
            if (accuary <= 0 || accuary > 1)
            {
                std::string newStr = "0.5";
                mRoot.at(0)->findWidget("Edit_GeodesicAccuracy")->castType<MyGUI::EditBox>()->setOnlyText(newStr);
                return;
            }
            measureShop->FastComputeExactGeodesics(accuary);
        }
    }

    void MeasureAppUI::ComputeExactGeodesics(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->ComputeExactGeodesics();
        }
    }

    void MeasureAppUI::MeasureModel(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_AreaRef")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_AreaRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_VolumeRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_CurvatureRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void MeasureAppUI::MeasureArea(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->MeasureArea();
        }
    }

    void MeasureAppUI::MeasureVolume(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->MeasureVolume();
        }
    }

    void MeasureAppUI::MeasureCurvature(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->MeasureCurvature();
        }
    }

    void MeasureAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            if (measureShop->IsCommandInProgress())
            {
                return;
            }
            AppManager::Get()->SwitchCurrentApp("Homepage");
        }
    }

    void MeasureAppUI::UpdateTextInfo()
    {
        std::string textString = "";
        if (mVertexCount > 0)
        {
            textString += "Vertex count = ";
            std::stringstream ss;
            ss << mVertexCount;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += " ";
            if (mTriangleCount > 0)
            {
                textString += " Triangle count = ";
                std::stringstream ss;
                ss << mTriangleCount;
                std::string numberString;
                ss >> numberString;
                textString += numberString;
                textString += "\n";
            }
            else
            {
                textString += "\n";
            }
        }

        if (mGeodesicsDistance > 0)
        {
            textString += "Geodesics distance = ";
            std::stringstream ss;
            ss << mGeodesicsDistance;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += "\n";
        }

        if (mArea > 0)
        {
            textString += "Area = ";
            std::stringstream ss;
            ss << mArea;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += "\n";
        }

        if (mVolume > 0)
        {
            textString += "Volume = ";
            std::stringstream ss;
            ss << mVolume;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += "\n";
        }

        mTextInfo->setCaption(textString);
    }
}
