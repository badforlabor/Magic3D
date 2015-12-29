#include "HomepageUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "PointShopApp.h"
#include "MeshShopApp.h"
#include "RegistrationApp.h"
#include "MeasureApp.h"
#include "ReliefApp.h"

namespace MagicApp
{
    HomepageUI::HomepageUI()
    {
    }

    HomepageUI::~HomepageUI()
    {

    }

    void HomepageUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/Homepage", "FileSystem", "Homepage");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("HomeLayout.layout");
        int winWidth  = MagicCore::RenderSystem::Get()->GetRenderWindowWidth();
        int winHeight = MagicCore::RenderSystem::Get()->GetRenderWindowHeight();
        int root0Width = mRoot.at(0)->getWidth();
        int root0Height = mRoot.at(0)->getHeight();
        mRoot.at(0)->setPosition((winWidth - root0Width) / 2, (winHeight - root0Height) / 2);
        mRoot.at(0)->findWidget("Title")->castType<MyGUI::ImageBox>()->setSize(406, 110);
        mRoot.at(0)->findWidget("But_PointShop")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &HomepageUI::EnterPointShopApp);
        mRoot.at(0)->findWidget("But_MeshShop")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &HomepageUI::EnterMeshShopApp);
        mRoot.at(0)->findWidget("But_RegistrationApp")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &HomepageUI::EnterRegistrationApp);
        mRoot.at(0)->findWidget("But_MeasureApp")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &HomepageUI::EnterMeasureApp);
        mRoot.at(0)->findWidget("But_ReliefApp")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &HomepageUI::EnterReliefApp);
        mRoot.at(0)->findWidget("But_Contact")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &HomepageUI::Contact);
    }

    void HomepageUI::EnterPointShopApp(MyGUI::Widget* pSender)
    {
        AppManager::Get()->EnterApp(new PointShopApp, "PointShopApp");
    }

    void HomepageUI::EnterMeshShopApp(MyGUI::Widget* pSender)
    {
        AppManager::Get()->EnterApp(new MeshShopApp, "MeshShopApp");
    }

    void HomepageUI::EnterRegistrationApp(MyGUI::Widget* pSender)
    {
        AppManager::Get()->EnterApp(new RegistrationApp, "RegistrationApp");
    }

    void HomepageUI::EnterMeasureApp(MyGUI::Widget* pSender)
    {
        AppManager::Get()->EnterApp(new MeasureApp, "MeasureApp");
    }

    void HomepageUI::EnterReliefApp(MyGUI::Widget* pSender)
    {
        AppManager::Get()->EnterApp(new ReliefApp, "ReliefApp");
    }

    void HomepageUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("Homepage");
    }

    void HomepageUI::Contact(MyGUI::Widget* pSender)
    {
        MagicCore::ToolKit::OpenWebsite(std::string("http://threepark.net/magic3d"));
    }
}
