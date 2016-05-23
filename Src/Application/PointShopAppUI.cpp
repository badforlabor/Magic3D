#include "PointShopAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "PointShopApp.h"

namespace MagicApp
{
    PointShopAppUI::PointShopAppUI() : 
        mIsProgressbarVisible(false),
        mTextInfo(NULL),
        mIgnoreBack(true)
    {
    }

    PointShopAppUI::~PointShopAppUI()
    {
    }

    void PointShopAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/PointShopApp", "FileSystem", "PointShopApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("PointShopApp.layout");
        mRoot.at(0)->findWidget("But_ImportPointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::ImportPointCloud);
        mRoot.at(0)->findWidget("But_ExportPointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::ExportPointCloud);
        
        mRoot.at(0)->findWidget("But_SamplePointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::SamplePointCloud);
        mRoot.at(0)->findWidget("But_DoSamplePointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::DoSamplePointCloud);

        mRoot.at(0)->findWidget("But_SimplifyPointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::SimplifyPointCloud);
        mRoot.at(0)->findWidget("But_DoSimplifyPointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::DoSimplifyPointCloud);
        
        mRoot.at(0)->findWidget("But_Normal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::PointCloudNormal);
        mRoot.at(0)->findWidget("But_CalNormalFront")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::CalculatePointCloudNormalFront);
        mRoot.at(0)->findWidget("But_CalNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::CalculatePointCloudNormal);
        mRoot.at(0)->findWidget("But_FlipNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::FlipPointCloudNormal);
        mRoot.at(0)->findWidget("But_ReversePatchNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::ReversePatchNormal);
        mRoot.at(0)->findWidget("But_SmoothNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::SmoothPointCloudNormal);

        mRoot.at(0)->findWidget("But_ConsolidatePointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::ConsolidatePointCloud);
        mRoot.at(0)->findWidget("But_RemovePointCloudOutlier")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::RemovePointCloudOutlier);
        mRoot.at(0)->findWidget("But_RemoveIsolatePart")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::RemoveIsolatePart);
        mRoot.at(0)->findWidget("But_SmoothGeometryByNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::SmoothPointCloudByNormal);       

        mRoot.at(0)->findWidget("But_Selection")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::SelectPoint);
        mRoot.at(0)->findWidget("But_SelectByRectangle")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::SelectByRectangle);
        mRoot.at(0)->findWidget("But_EraseByRectangle")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::EraseByRectangle);
        mRoot.at(0)->findWidget("But_DeleteSelections")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::DeleteSelections);
        mRoot.at(0)->findWidget("But_IgnoreBack")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::IgnoreBack);
        mRoot.at(0)->findWidget("But_MoveModel")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::MoveModel);

        mRoot.at(0)->findWidget("But_Reconstruction")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::ReconstructMesh);
        mRoot.at(0)->findWidget("But_DoReconstruction")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::DoReconstructMesh);

        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::BackToHomepage);

        mTextInfo = mRoot.at(0)->findWidget("Text_Info")->castType<MyGUI::TextBox>();
        mTextInfo->setTextColour(MyGUI::Colour(75.0 / 255.0, 131.0 / 255.0, 128.0 / 255.0));
    }

    void PointShopAppUI::StartProgressbar(int range)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(true);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressRange(range);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(0);
        mIsProgressbarVisible = true;
    }

    void PointShopAppUI::SetProgressbar(int value)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(value);
    }

    void PointShopAppUI::StopProgressbar()
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(false);
        mIsProgressbarVisible = false;
    }

    bool PointShopAppUI::IsProgressbarVisible()
    {
        return mIsProgressbarVisible;
    }

    void PointShopAppUI::SetPointCloudInfo(int pointCount)
    {
        std::string textString = "";
        if (pointCount > 0)
        {
            textString += "Point count = ";
            std::stringstream ss;
            ss << pointCount;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += "\n";
        }
        mTextInfo->setCaption(textString);
    }

    void PointShopAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("PointShopApp");
    }

    void PointShopAppUI::ImportPointCloud(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->ImportPointCloud();
        }
    }

    void PointShopAppUI::ExportPointCloud(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->ExportPointCloud();
        }
    }

    void PointShopAppUI::SamplePointCloud(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DoSamplePointCloud")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DoSamplePointCloud")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_SampleTargetPointCount")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            int pointCount = 0;
            PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
            if (pointShop != NULL)
            {
                pointCount = pointShop->GetPointCount();
            }
            std::stringstream ss;
            std::string textString;
            ss << pointCount;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_SampleTargetPointCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_SampleTargetPointCount")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }

    void PointShopAppUI::DoSamplePointCloud(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            int pointCount = pointShop->GetPointCount();
            std::string textString = mRoot.at(0)->findWidget("Edit_SampleTargetPointCount")->castType<MyGUI::EditBox>()->getOnlyText();
            int targetPointCount = std::atoi(textString.c_str());
            if (targetPointCount > pointCount || pointCount < 1)
            {
                std::stringstream ss;
                std::string textString;
                ss << pointCount;
                ss >> textString;
                mRoot.at(0)->findWidget("Edit_SampleTargetPointCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            }
            else
            {
                pointShop->SamplePointCloud(targetPointCount);
            }            
        }
    }

    void PointShopAppUI::SimplifyPointCloud(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DoSimplifyPointCloud")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DoSimplifyPointCloud")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_SimplifyResolution")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::stringstream ss;
            std::string textString;
            ss << 512;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_SimplifyResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_SimplifyResolution")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }
    
    void PointShopAppUI::DoSimplifyPointCloud(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            std::string textString = mRoot.at(0)->findWidget("Edit_SimplifyResolution")->castType<MyGUI::EditBox>()->getOnlyText();
            int resolution = std::atoi(textString.c_str());
            if (resolution < 1)
            {
                std::stringstream ss;
                std::string textString;
                ss << 512;
                ss >> textString;
                mRoot.at(0)->findWidget("Edit_SimplifyResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            }
            else
            {
                pointShop->SimplifyPointCloud(resolution);
            }            
        }
    }

    void PointShopAppUI::PointCloudNormal(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_CalNormal")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_CalNormalFront")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_CalNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_FlipNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_ReversePatchNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_SmoothNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void PointShopAppUI::CalculatePointCloudNormalFront(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->CalculatePointCloudNormal(true);
        }
    }

    void PointShopAppUI::CalculatePointCloudNormal(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->CalculatePointCloudNormal(false);
        }
    }

    void PointShopAppUI::FlipPointCloudNormal(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->FlipPointCloudNormal();
        }
    }

    void PointShopAppUI::ReversePatchNormal(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->ReversePatchNormal();
        }
    }

    void PointShopAppUI::SmoothPointCloudNormal(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->SmoothPointCloudNormal();
        }
    }

    void PointShopAppUI::ConsolidatePointCloud(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_RemovePointCloudOutlier")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_RemovePointCloudOutlier")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_RemoveIsolatePart")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_SmoothGeometryByNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void PointShopAppUI::RemovePointCloudOutlier(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->RemovePointCloudOutlier();
        }
    }

    void PointShopAppUI::RemoveIsolatePart(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->RemoveIsolatePart();
        }
    }

    void PointShopAppUI::SmoothPointCloudByNormal(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->SmoothPointCloudByNormal();
        }
    }

    void PointShopAppUI::SelectPoint(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_SelectByRectangle")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_SelectByRectangle")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_EraseByRectangle")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_DeleteSelections")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_IgnoreBack")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_MoveModel")->castType<MyGUI::Button>()->setVisible(isVisible);
    }

    void PointShopAppUI::SelectByRectangle(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->SelectByRectangle();
        }
    }

    void PointShopAppUI::EraseByRectangle(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->EraseByRectangle();
        }
    }

    void PointShopAppUI::DeleteSelections(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->DeleteSelections();
        }
    }

    void PointShopAppUI::IgnoreBack(MyGUI::Widget* pSender)
    {
        mIgnoreBack = !mIgnoreBack;
        if (mIgnoreBack)
        {
            mRoot.at(0)->findWidget("But_IgnoreBack")->castType<MyGUI::Button>()->changeWidgetSkin("But_Front");
        }
        else
        {
            mRoot.at(0)->findWidget("But_IgnoreBack")->castType<MyGUI::Button>()->changeWidgetSkin("But_Back");
        }
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->IgnoreBack(mIgnoreBack);
        }
    }

    void PointShopAppUI::MoveModel(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->MoveModel();
        }
    }

    void PointShopAppUI::ReconstructMesh(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DoReconstruction")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DoReconstruction")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_ReconstructionQuality")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::stringstream ss;
            std::string textString;
            ss << 4;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_ReconstructionQuality")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_ReconstructionQuality")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }

    void PointShopAppUI::DoReconstructMesh(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            std::string textString = mRoot.at(0)->findWidget("Edit_ReconstructionQuality")->castType<MyGUI::EditBox>()->getOnlyText();
            int quality = std::atoi(textString.c_str());
            if (quality < 0)
            {
                std::stringstream ss;
                std::string textString;
                ss << 4;
                ss >> textString;
                mRoot.at(0)->findWidget("Edit_SimplifyResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            }
            else
            {
                pointShop->ReconstructMesh(quality);
            }            
        }
    }

    void PointShopAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            if (pointShop->IsCommandInProgress())
            {
                return;
            }
            AppManager::Get()->SwitchCurrentApp("Homepage");
        }
    }
}
