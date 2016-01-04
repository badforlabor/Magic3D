#include "MeasureAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "MeasureApp.h"

namespace MagicApp
{
    MeasureAppUI::MeasureAppUI() :
        mIsProgressbarVisible(false)
    {
    }

    MeasureAppUI::~MeasureAppUI()
    {
    }

    void MeasureAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/MeasureApp", "FileSystem", "MeasureApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("MeasureApp.layout");
        mRoot.at(0)->findWidget("But_ImportModelRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ImportModelRef);
        mRoot.at(0)->findWidget("But_Geodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::Geodesics);
        mRoot.at(0)->findWidget("But_MarkMeshRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::SelectMeshMarkRef);
        mRoot.at(0)->findWidget("But_DeleteMeshRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::DeleteMeshMarkRef);
        mRoot.at(0)->findWidget("But_ApproximateGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ComputeApproximateGeodesics);
        mRoot.at(0)->findWidget("But_ExactGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::ComputeExactGeodesics);
        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeasureAppUI::BackToHomepage);
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
        bool isVisible = mRoot.at(0)->findWidget("But_MarkMeshRef")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_MarkMeshRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_DeleteMeshRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_ApproximateGeodesics")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_ExactGeodesics")->castType<MyGUI::Button>()->setVisible(!isVisible);
        if (isVisible)
        {
            MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
            if (measureShop != NULL)
            {
                measureShop->SwitchToViewMode();
            }
        }
    }

    void MeasureAppUI::SelectMeshMarkRef(MyGUI::Widget* pSender)
    {
        MeasureApp* measureShop = dynamic_cast<MeasureApp* >(AppManager::Get()->GetApp("MeasureApp"));
        if (measureShop != NULL)
        {
            measureShop->SwitchMeshRefControlState();
        }
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
}
