#include "AnimationAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "AnimationApp.h"

namespace MagicApp
{
    AnimationAppUI::AnimationAppUI()
    {
    }

    AnimationAppUI::~AnimationAppUI()
    {
    }

    void AnimationAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/AnimationApp", "FileSystem", "AnimationApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("AnimationApp.layout");
        mRoot.at(0)->findWidget("But_ImportModel")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::ImportModel);
        
        mRoot.at(0)->findWidget("But_InitControlPoint")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::InitControlPoint);
        mRoot.at(0)->findWidget("But_DoInitControlPoint")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::DoInitControlPoint);

        mRoot.at(0)->findWidget("But_SelectControlPoint")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::SelectControlPoint);
        mRoot.at(0)->findWidget("But_SelectByRectangle")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::SelectByRectangle);
        mRoot.at(0)->findWidget("But_EraseSelections")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::ClearSelection);
        mRoot.at(0)->findWidget("But_MoveControlPoint")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::MoveControlPoint);

        mRoot.at(0)->findWidget("But_Deformation")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::DeformModel);
        mRoot.at(0)->findWidget("But_InitDeformation")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::InitDeformation);
        mRoot.at(0)->findWidget("But_DoDeformation")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::DoDeformation);
        mRoot.at(0)->findWidget("But_RealTimeDeform")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::RealTimeDeform);

        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &AnimationAppUI::BackToHomepage);
    }

    void AnimationAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("AnimationApp");
    }

    void AnimationAppUI::ImportModel(MyGUI::Widget* pSender)
    {
        AnimationApp* animationApp = dynamic_cast<AnimationApp* >(AppManager::Get()->GetApp("AnimationApp"));
        if (animationApp != NULL)
        {
            animationApp->ImportModel();
        }
    }

    void AnimationAppUI::InitControlPoint(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DoInitControlPoint")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DoInitControlPoint")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_ControlPointCount")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::stringstream ss;
            std::string textString;
            ss << 300;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_ControlPointCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_ControlPointCount")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }

    void AnimationAppUI::DoInitControlPoint(MyGUI::Widget* pSender)
    {
        AnimationApp* animationApp = dynamic_cast<AnimationApp* >(AppManager::Get()->GetApp("AnimationApp"));
        if (animationApp != NULL)
        {
            std::string textString = mRoot.at(0)->findWidget("Edit_ControlPointCount")->castType<MyGUI::EditBox>()->getOnlyText();
            int controlPointCount = std::atoi(textString.c_str());
            if (controlPointCount < 1)
            {
                std::stringstream ss;
                std::string textString;
                ss << 300;
                ss >> textString;
                mRoot.at(0)->findWidget("Edit_ControlPointCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            }
            else
            {
                animationApp->InitControlPoint(controlPointCount);
            }      
        }
    }

    void AnimationAppUI::SelectControlPoint(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_SelectByRectangle")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_SelectByRectangle")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_EraseSelections")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_MoveControlPoint")->castType<MyGUI::Button>()->setVisible(isVisible);
    }
    
    void AnimationAppUI::SelectByRectangle(MyGUI::Widget* pSender)
    {
        AnimationApp* animationApp = dynamic_cast<AnimationApp* >(AppManager::Get()->GetApp("AnimationApp"));
        if (animationApp != NULL)
        {
            animationApp->SelectFreeControlPoint();
        }
    }
    
    void AnimationAppUI::ClearSelection(MyGUI::Widget* pSender)
    {
        AnimationApp* animationApp = dynamic_cast<AnimationApp* >(AppManager::Get()->GetApp("AnimationApp"));
        if (animationApp != NULL)
        {
            animationApp->ClearFreeControlPoint();
        }
    }

    void AnimationAppUI::MoveControlPoint(MyGUI::Widget* pSender)
    {
        AnimationApp* animationApp = dynamic_cast<AnimationApp* >(AppManager::Get()->GetApp("AnimationApp"));
        if (animationApp != NULL)
        {
            animationApp->MoveControlPoint();
        }
    }

    void AnimationAppUI::DeformModel(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_InitDeformation")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_InitDeformation")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_DoDeformation")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_RealTimeDeform")->castType<MyGUI::Button>()->setVisible(isVisible);
    }

    void AnimationAppUI::InitDeformation(MyGUI::Widget* pSender)
    {
        AnimationApp* animationApp = dynamic_cast<AnimationApp* >(AppManager::Get()->GetApp("AnimationApp"));
        if (animationApp != NULL)
        {
            animationApp->InitDeformation();
        }
    }

    void AnimationAppUI::DoDeformation(MyGUI::Widget* pSender)
    {
        AnimationApp* animationApp = dynamic_cast<AnimationApp* >(AppManager::Get()->GetApp("AnimationApp"));
        if (animationApp != NULL)
        {
            animationApp->DoDeformation();
        }
    }

    void AnimationAppUI::RealTimeDeform(MyGUI::Widget* pSender)
    {
        AnimationApp* animationApp = dynamic_cast<AnimationApp* >(AppManager::Get()->GetApp("AnimationApp"));
        if (animationApp != NULL)
        {
            animationApp->RealTimeDeform();
        }
    }

    void AnimationAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }
}
