#include "PointShopAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "PointShopApp.h"

namespace MagicApp
{
    PointShopAppUI::PointShopAppUI()
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
        mRoot.at(0)->findWidget("But_SmoothPointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::SmoothPointCloud);
        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::BackToHomepage);
        mRoot.at(0)->findWidget("But_Contact")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &PointShopAppUI::Contact);
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
            if (pointShop->ImportPointCloud())
            {
                mRoot.at(0)->findWidget("But_ExportPointCloud")->castType<MyGUI::Button>()->setEnabled(true);
                mRoot.at(0)->findWidget("But_SmoothPointCloud")->castType<MyGUI::Button>()->setEnabled(true);
            }
        }
    }

    void PointShopAppUI::ExportPointCloud(MyGUI::Widget* pSender)
    {
    }

    void PointShopAppUI::SmoothPointCloud(MyGUI::Widget* pSender)
    {
        PointShopApp* pointShop = dynamic_cast<PointShopApp* >(AppManager::Get()->GetApp("PointShopApp"));
        if (pointShop != NULL)
        {
            pointShop->SmoothPointCloud();
        }
    }

    void PointShopAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }

    void PointShopAppUI::Contact(MyGUI::Widget* pSender)
    {
        MagicCore::ToolKit::OpenWebsite(std::string("https://github.com/threepark"));
    }
}
