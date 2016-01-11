#include "RegistrationAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "RegistrationApp.h"

namespace MagicApp
{
    RegistrationAppUI::RegistrationAppUI() : 
        mIsProgressbarVisible(false)
    {
    }

    RegistrationAppUI::~RegistrationAppUI()
    {
    }

    void RegistrationAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/RegistrationApp", "FileSystem", "RegistrationApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("RegistrationApp.layout");
        mRoot.at(0)->findWidget("But_SwitchDisplayMode")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::SwitchDisplayMode);

        mRoot.at(0)->findWidget("But_ImportPointCloudRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::ImportPointCloudRef);
        mRoot.at(0)->findWidget("But_RefNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::RefNormal);
        mRoot.at(0)->findWidget("But_CalRefNormalFront")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::CalculateRefNormalFront);
        mRoot.at(0)->findWidget("But_CalRefNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::CalculateRefNormal);
        mRoot.at(0)->findWidget("But_FlipRefNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FlipRefNormal);
        //mRoot.at(0)->findWidget("But_SmoothRefNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::SmoothRefNormal);
        
        mRoot.at(0)->findWidget("But_RefFeaturePoint")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::RefFeaturePoint);
        mRoot.at(0)->findWidget("But_RefDeleteMark")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::RefDeleteMark);
        mRoot.at(0)->findWidget("But_RefImportMark")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::RefImportMark);

        mRoot.at(0)->findWidget("But_ImportPointCloudFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::ImportPointCloudFrom);
        mRoot.at(0)->findWidget("But_FromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FromNormal);
        mRoot.at(0)->findWidget("But_CalFromNormalFront")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::CalculateFromNormalFront);
        mRoot.at(0)->findWidget("But_CalFromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::CalculateFromNormal);
        mRoot.at(0)->findWidget("But_FlipFromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FlipFromNormal);
        //mRoot.at(0)->findWidget("But_SmoothFromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::SmoothFromNormal);
        
        mRoot.at(0)->findWidget("But_FromFeaturePoint")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FromFeaturePoint);
        mRoot.at(0)->findWidget("But_FromDeleteMark")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FromDeleteMark);
        mRoot.at(0)->findWidget("But_FromImportMark")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FromImportMark);

        mRoot.at(0)->findWidget("But_AlignPointCloudFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignFrom);
        mRoot.at(0)->findWidget("But_AlignMark")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignMark);
        mRoot.at(0)->findWidget("But_AlignFree")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignFree);
        mRoot.at(0)->findWidget("But_AlignICP")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignICP);
        mRoot.at(0)->findWidget("But_FusePointCloudRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FuseRef);
        mRoot.at(0)->findWidget("But_EnterPointShop")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::EnterPointShop);
        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::BackToHomepage);
    }

    void RegistrationAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("RegistrationApp");
    }

    void RegistrationAppUI::StartProgressbar(int range)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(true);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressRange(range);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(0);
        mIsProgressbarVisible = true;
    }

    void RegistrationAppUI::SetProgressbar(int value)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(value);
    }

    void RegistrationAppUI::StopProgressbar()
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(false);
        mIsProgressbarVisible = false;
    }

    bool RegistrationAppUI::IsProgressbarVisible()
    {
        return mIsProgressbarVisible;
    }

    void RegistrationAppUI::SwitchDisplayMode(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->SwitchSeparateDisplay();
        }
    }

    void RegistrationAppUI::ImportPointCloudRef(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->ImportPointCloudRef();
        }
    }

    void RegistrationAppUI::RefNormal(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_CalRefNormal")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_CalRefNormalFront")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_CalRefNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_FlipRefNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);  
        //mRoot.at(0)->findWidget("But_SmoothRefNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);  
    }

    void RegistrationAppUI::CalculateRefNormalFront(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->CalculateRefNormal(true);
        }
    }

    void RegistrationAppUI::CalculateRefNormal(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->CalculateRefNormal(false);
        }
    }

    void RegistrationAppUI::FlipRefNormal(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->FlipRefNormal();
        }
    }

    /*void RegistrationAppUI::SmoothRefNormal(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->SmoothRefNormal();
        }
    }*/

    void RegistrationAppUI::RefFeaturePoint(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_RefDeleteMark")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_RefDeleteMark")->castType<MyGUI::Button>()->setVisible(!isVisible);  
        mRoot.at(0)->findWidget("But_RefImportMark")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void RegistrationAppUI::RefDeleteMark(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->DeleteRefMark();
        }
    }

    void RegistrationAppUI::RefImportMark(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->ImportRefMark();
        }
    }

    void RegistrationAppUI::FuseRef(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->FuseRef();
        }
    }

    void RegistrationAppUI::ImportPointCloudFrom(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->ImportPointCloudFrom();
        }
    }

    void RegistrationAppUI::FromNormal(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_CalFromNormal")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_CalFromNormalFront")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_CalFromNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_FlipFromNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
        //mRoot.at(0)->findWidget("But_SmoothFromNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void RegistrationAppUI::CalculateFromNormalFront(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->CalculateFromNormal(true);
        }
    }
    void RegistrationAppUI::CalculateFromNormal(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->CalculateFromNormal(false);
        }
    }

    void RegistrationAppUI::FlipFromNormal(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->FlipFromNormal();
        }
    }

    /*void RegistrationAppUI::SmoothFromNormal(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->SmoothFromNormal();
        }
    }*/

    void RegistrationAppUI::FromFeaturePoint(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_FromDeleteMark")->castType<MyGUI::Button>()->isVisible(); 
        mRoot.at(0)->findWidget("But_FromDeleteMark")->castType<MyGUI::Button>()->setVisible(!isVisible);  
        mRoot.at(0)->findWidget("But_FromImportMark")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void RegistrationAppUI::FromDeleteMark(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->DeleteFromMark();
        }
    }

    void RegistrationAppUI::FromImportMark(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->ImportFromMark();
        }
    }

    void RegistrationAppUI::AlignFrom(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_AlignMark")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_AlignMark")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_AlignFree")->castType<MyGUI::Button>()->setVisible(!isVisible);  
        mRoot.at(0)->findWidget("But_AlignICP")->castType<MyGUI::Button>()->setVisible(!isVisible);  
    }

    void RegistrationAppUI::AlignMark(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->AlignMark();
        }
    }

    void RegistrationAppUI::AlignFree(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->AlignFree();
        }
    }

    void RegistrationAppUI::AlignICP(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->AlignICP();
        }
    }

    void RegistrationAppUI::EnterPointShop(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->EnterPointShop();
        }
    }

    void RegistrationAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            if (registrationApp->IsCommandInProgress())
            {
                return;
            }
            AppManager::Get()->SwitchCurrentApp("Homepage");
        }
    }

}
