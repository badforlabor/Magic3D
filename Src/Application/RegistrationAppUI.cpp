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
        mRoot.at(0)->findWidget("But_SelectRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::SelectRef);
        mRoot.at(0)->findWidget("But_PushRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::PushRef);
        mRoot.at(0)->findWidget("But_PopRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::PopRef);
        mRoot.at(0)->findWidget("But_RefView")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::ViewRef);
        //mRoot.at(0)->findWidget("But_FusePointCloudRef")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FuseRef);
        mRoot.at(0)->findWidget("But_ImportPointCloudFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::ImportPointCloudFrom);
        mRoot.at(0)->findWidget("But_FromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FromNormal);
        mRoot.at(0)->findWidget("But_CalFromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::CalculateFromNormal);
        mRoot.at(0)->findWidget("But_FlipFromNormal")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::FlipFromNormal);
        mRoot.at(0)->findWidget("But_SelectFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::SelectFrom);
        mRoot.at(0)->findWidget("But_PushFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::PushFrom);
        mRoot.at(0)->findWidget("But_PopFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::PopFrom);
        mRoot.at(0)->findWidget("But_FromView")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::ViewFrom);
        mRoot.at(0)->findWidget("But_AlignPointCloudFrom")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::AlignFrom);
        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::BackToHomepage);
        mRoot.at(0)->findWidget("But_Contact")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &RegistrationAppUI::Contact);
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

    void RegistrationAppUI::SelectRef(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_PushRef")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_PushRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_PopRef")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_RefView")->castType<MyGUI::Button>()->setVisible(!isVisible); 
    }

    void RegistrationAppUI::PushRef(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->PushRef();
        }
    }

    void RegistrationAppUI::PopRef(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->PopRef();
        }
    }

    void RegistrationAppUI::ViewRef(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->ModelView();
        }
    }

    /*void RegistrationAppUI::FuseRef(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->FuseRef();
        }
    }*/

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

    void RegistrationAppUI::SelectFrom(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_PushFrom")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_PushFrom")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_PopFrom")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_FromView")->castType<MyGUI::Button>()->setVisible(!isVisible); 
    }

    void RegistrationAppUI::PushFrom(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->PushFrom();
        }
    }

    void RegistrationAppUI::PopFrom(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->PopFrom();
        }
    }

    void RegistrationAppUI::ViewFrom(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->ModelView();
        }
    }

    void RegistrationAppUI::AlignFrom(MyGUI::Widget* pSender)
    {
        RegistrationApp* registrationApp = dynamic_cast<RegistrationApp* >(AppManager::Get()->GetApp("RegistrationApp"));
        if (registrationApp != NULL)
        {
            registrationApp->AlignFrom();
        }
    }

    void RegistrationAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }

    void RegistrationAppUI::Contact(MyGUI::Widget* pSender)
    {
        MagicCore::ToolKit::OpenWebsite(std::string("http://threepark.net/magic3d"));
    }
}
