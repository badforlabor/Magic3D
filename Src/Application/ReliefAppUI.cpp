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
        mRoot.at(0)->findWidget("But_Relief")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::Relief);
        mRoot.at(0)->findWidget("But_GenerateRelief")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::GenerateRelief);        
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

    void ReliefAppUI::Relief(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_GenerateRelief")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_GenerateRelief")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::string textString = "0.5";
            mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }

    void ReliefAppUI::GenerateRelief(MyGUI::Widget* pSender)
    {
        std::string textString = mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->getOnlyText();
        double compressRatio = std::atof(textString.c_str());
        if (compressRatio > 0 && compressRatio <=1)
        {
            ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
            if (reliefApp != NULL)
            {
                reliefApp->GenerateRelief(compressRatio);
            }
        }
        else
        {
            std::string textString = "0.5";
            mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->setOnlyText(textString);
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
