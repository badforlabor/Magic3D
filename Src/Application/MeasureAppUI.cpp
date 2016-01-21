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
        mRefVertexCount(0),
        mRefTriangleCount(0),
        mFromVertexCount(0),
        mFromTriangleCount(0),
        mRefArea(0),
        mRefVolume(0),
        mGeodesicsDistance(0),
        mMaxDeviationDistance(0)
    {
    }

    MeasureAppUI::~MeasureAppUI()
    {
    }

    void MeasureAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/MeasureApp", "FileSystem", "MeasureApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("MeasureApp.layout");
        mRoot.at(0)->findWidget("But_SwitchDisplayMode")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::SwitchDisplayMode);
        mRoot.at(0)->findWidget("But_ImportModelRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ImportModelRef);
        
        mRoot.at(0)->findWidget("But_Geodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::Geodesics);
        mRoot.at(0)->findWidget("But_DeleteMeshRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::DeleteMeshMarkRef);
        mRoot.at(0)->findWidget("But_ApproximateGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ComputeApproximateGeodesics);
        mRoot.at(0)->findWidget("But_ExactGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ComputeExactGeodesics);

        mRoot.at(0)->findWidget("But_MeasureRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::MeasureRefModel);
        mRoot.at(0)->findWidget("But_AreaRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::MeasureRefArea);
        mRoot.at(0)->findWidget("But_VolumeRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::MeasureRefVolume);

        mRoot.at(0)->findWidget("But_ImportModelFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ImportModelFrom);
        mRoot.at(0)->findWidget("But_Deviation")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::Deviation);
        mRoot.at(0)->findWidget("But_ComputeDeviation")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ComputeDeviation);

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

    void MeasureAppUI::SetRefModelInfo(int vertexCount, int triangleCount)
    {
        mRefVertexCount = vertexCount;
        mRefTriangleCount = triangleCount;
        UpdateTextInfo();
    }

    void MeasureAppUI::SetFromModelInfo(int vertexCount, int triangleCount)
    {
        mFromVertexCount = vertexCount;
        mFromTriangleCount = triangleCount;
        UpdateTextInfo();
    }

    void MeasureAppUI::SetRefModelArea(double area)
    {
        mRefArea = area;
        UpdateTextInfo();
    }

    void MeasureAppUI::SetRefModelVolume(double volume)
    {
        mRefVolume = volume;
        UpdateTextInfo();
    }

    void MeasureAppUI::SetGeodesicsInfo(double distance)
    {
        mGeodesicsDistance = distance;
        UpdateTextInfo();
    }

    void MeasureAppUI::SetDeviationInfo(double maxDistance)
    {
        mMaxDeviationDistance = maxDistance;
        UpdateTextInfo();
    }

    void MeasureAppUI::SwitchDisplayMode(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->SwitchSeparateDisplay();
        }
    }

    void MeasureAppUI::ImportModelRef(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->ImportModelRef();
        }
    }

    void MeasureAppUI::Geodesics(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DeleteMeshRef")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_DeleteMeshRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_ApproximateGeodesics")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_ExactGeodesics")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void MeasureAppUI::DeleteMeshMarkRef(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->DeleteMeshMarkRef();
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

    void MeasureAppUI::ComputeExactGeodesics(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->ComputeExactGeodesics();
        }
    }

    void MeasureAppUI::MeasureRefModel(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_AreaRef")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_AreaRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_VolumeRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void MeasureAppUI::MeasureRefArea(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->MeasureRefArea();
        }
    }

    void MeasureAppUI::MeasureRefVolume(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->MeasureRefVolume();
        }
    }

    void MeasureAppUI::ImportModelFrom(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->ImportModelFrom();
        }
    }

    void MeasureAppUI::Deviation(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_ComputeDeviation")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_ComputeDeviation")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void MeasureAppUI::ComputeDeviation(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->ComputeDeviation();
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
        if (mRefVertexCount > 0)
        {
            textString += "Fix model vertex count = ";
            std::stringstream ss;
            ss << mRefVertexCount;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += " ";
            if (mRefTriangleCount > 0)
            {
                textString += " triangle count = ";
                std::stringstream ss;
                ss << mRefTriangleCount;
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

        if (mRefArea > 0)
        {
            textString += "Area = ";
            std::stringstream ss;
            ss << mRefArea;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += "\n";
        }

        if (mRefVolume > 0)
        {
            textString += "Volume = ";
            std::stringstream ss;
            ss << mRefVolume;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += "\n";
        }

        if (mFromVertexCount > 0)
        {
            textString += "Float model vertex count = ";
            std::stringstream ss;
            ss << mFromVertexCount;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += " ";
            if (mFromTriangleCount > 0)
            {
                textString += " triangle count = ";
                std::stringstream ss;
                ss << mFromTriangleCount;
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

        if (mMaxDeviationDistance > 0)
        {
            textString += "Max deviation distance = ";
            std::stringstream ss;
            ss << mMaxDeviationDistance;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += "\n";
        }

        mTextInfo->setCaption(textString);
    }
}
