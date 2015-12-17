#include "RegistrationAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "RegistrationApp.h"

namespace MagicApp
{
    RegistrationAppUI::RegistrationAppUI()
    {
    }

    RegistrationAppUI::~RegistrationAppUI()
    {
    }

    void RegistrationAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/RegistrationApp", "FileSystem", "RegistrationApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("RegistrationApp.layout");
        mRoot.at(0)->findWidget("But_ImportPointCloudRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::ImportPointCloudRef);
        mRoot.at(0)->findWidget("But_RefNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::RefNormal);
        mRoot.at(0)->findWidget("But_CalRefNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::CalculateRefNormal);
        mRoot.at(0)->findWidget("But_FlipRefNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FlipRefNormal);
        mRoot.at(0)->findWidget("But_FusePointCloudRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FuseRef);
        mRoot.at(0)->findWidget("But_ImportPointCloudFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::ImportPointCloudFrom);
        mRoot.at(0)->findWidget("But_FromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FromNormal);
        mRoot.at(0)->findWidget("But_CalFromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::CalculateFromNormal);
        mRoot.at(0)->findWidget("But_FlipFromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FlipFromNormal);
        mRoot.at(0)->findWidget("But_AlignPointCloudFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignFrom);
        mRoot.at(0)->findWidget("But_AlignFast")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignFast);
        mRoot.at(0)->findWidget("But_AlignPrecise")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignPrecise);
        mRoot.at(0)->findWidget("But_AlignICP")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignICP);
        mRoot.at(0)->findWidget("But_EnterPointShop")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::EnterPointShop);
        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::BackToHomepage);
    }

    void RegistrationAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("RegistrationApp");
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
        mRoot.at(0)->findWidget("But_CalRefNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_FlipRefNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);     
    }

    void RegistrationAppUI::CalculateRefNormal(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->CalculateRefNormal();
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
        mRoot.at(0)->findWidget("But_CalFromNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_FlipFromNormal")->castType<MyGUI::Button>()->setVisible(!isVisible);     
    }

    void RegistrationAppUI::CalculateFromNormal(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->CalculateFromNormal();
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

    void RegistrationAppUI::AlignFrom(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_AlignFast")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_AlignFast")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_AlignPrecise")->castType<MyGUI::Button>()->setVisible(!isVisible);   
    }

    void RegistrationAppUI::AlignFast(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->AlignFast();
        }
    }

    void RegistrationAppUI::AlignPrecise(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->AlignPrecise();
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
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }

}
