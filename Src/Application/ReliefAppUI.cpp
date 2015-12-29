#include "ReliefAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "ReliefApp.h"

namespace MagicApp
{
    ReliefAppUI::ReliefAppUI()
    {
    }

    ReliefAppUI::~ReliefAppUI()
    {
    }

    void ReliefAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/ReliefApp", "FileSystem", "ReliefApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("ReliefApp.layout");
        mRoot.at(0)->findWidget("But_ImportModel")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::ImportModel);
        mRoot.at(0)->findWidget("But_Relief")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::GenerateRelief);
        mRoot.at(0)->findWidget("But_EnterMeshTool")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::EnterMeshTool);
        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::BackToHomepage);
        mRoot.at(0)->findWidget("But_Contact")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::Contact);
    }

    void ReliefAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("ReliefApp");
    }

    void ReliefAppUI::ImportModel(MyGUI::Widget* pSender)
    {
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->ImportModel();
        }
    }

    void ReliefAppUI::GenerateRelief(MyGUI::Widget* pSender)
    {
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->GenerateRelief();
        }
    }

    void ReliefAppUI::EnterMeshTool(MyGUI::Widget* pSender)
    {
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->EnterMeshTool();
        }
    }

    void ReliefAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }

    void ReliefAppUI::Contact(MyGUI::Widget* pSender)
    {
        MagicCore::ToolKit::OpenWebsite(std::string("http://threepark.net/magic3d"));
    }
}
