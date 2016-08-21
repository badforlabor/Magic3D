#include "DepthVideoAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "DepthVideoApp.h"

namespace MagicApp
{
    DepthVideoAppUI::DepthVideoAppUI() : 
        mIsProgressbarVisible(false),
        mScrollRange(0),
        mScrollPosition(0)
    {
    }

    DepthVideoAppUI::~DepthVideoAppUI()
    {
    }

    void DepthVideoAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/DepthVideoApp", "FileSystem", "DepthVideoApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("DepthVideoApp.layout");
        
        mRoot.at(0)->findWidget("Slider_CurrentFrame")->castType<MyGUI::ScrollBar>()->eventScrollChangePosition += MyGUI::newDelegate(this, &DepthVideoAppUI::ChangeFrameIndex);

        mRoot.at(0)->findWidget("But_ImportPointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &DepthVideoAppUI::ImportPointCloud);
        mRoot.at(0)->findWidget("But_AlignPointCloudList")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &DepthVideoAppUI::AlignPointCloudList);
        mRoot.at(0)->findWidget("But_DoAlignPointCloudList")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &DepthVideoAppUI::DoAlignPointCloudList);

        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &DepthVideoAppUI::BackToHomepage);
    }

    void DepthVideoAppUI::StartProgressbar(int range)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(true);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressRange(range);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(0);
        mIsProgressbarVisible = true;
    }

    void DepthVideoAppUI::SetProgressbar(int value)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(value);
    }

    void DepthVideoAppUI::StopProgressbar()
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(false);
        mIsProgressbarVisible = false;
    }

    bool DepthVideoAppUI::IsProgressbarVisible()
    {
        return mIsProgressbarVisible;
    }

    void DepthVideoAppUI::SetScrollRange(int range)
    {
        mScrollRange = range;
        if (mScrollRange < mScrollPosition)
        {
            mScrollPosition = mScrollRange;
        }
        mRoot.at(0)->findWidget("Slider_CurrentFrame")->castType<MyGUI::ScrollBar>()->setScrollRange(mScrollRange);
        mRoot.at(0)->findWidget("Slider_CurrentFrame")->castType<MyGUI::ScrollBar>()->setScrollPosition(mScrollPosition);
    }

    void DepthVideoAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("DepthVideoApp");
    }

    void DepthVideoAppUI::ChangeFrameIndex(MyGUI::ScrollBar* pSender, size_t pos)
    {
        if (pos >= mScrollRange)
        {
            return;
        }
        mScrollPosition = pos;
        DepthVideoApp* depthVideoApp = dynamic_cast<DepthVideoApp* >(AppManager::Get()->GetApp("DepthVideoApp"));
        if (depthVideoApp != NULL)
        {
            depthVideoApp->SetPointCloudIndex(mScrollPosition);
        }
    }

    void DepthVideoAppUI::ImportPointCloud(MyGUI::Widget* pSender)
    {
        DepthVideoApp* depthVideoApp = dynamic_cast<DepthVideoApp* >(AppManager::Get()->GetApp("DepthVideoApp"));
        if (depthVideoApp != NULL)
        {
            depthVideoApp->ImportPointCloud();
        }
    }

    void DepthVideoAppUI::AlignPointCloudList(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DoAlignPointCloudList")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DoAlignPointCloudList")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_GroupSize")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::stringstream ss;
            std::string textString;
            ss << 25;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_GroupSize")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_GroupSize")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }

    void DepthVideoAppUI::DoAlignPointCloudList(MyGUI::Widget* pSender)
    {
        DepthVideoApp* depthVideoApp = dynamic_cast<DepthVideoApp* >(AppManager::Get()->GetApp("DepthVideoApp"));
        if (depthVideoApp != NULL)
        {
            std::string textString = mRoot.at(0)->findWidget("Edit_GroupSize")->castType<MyGUI::EditBox>()->getOnlyText();
            int groupCount = std::atoi(textString.c_str());
            depthVideoApp->AlignPointCloudList(groupCount);
        }
    }

    void DepthVideoAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        DepthVideoApp* depthVideoApp = dynamic_cast<DepthVideoApp* >(AppManager::Get()->GetApp("DepthVideoApp"));
        if (depthVideoApp != NULL)
        {
            if (depthVideoApp->IsCommandInProgress())
            {
                return;
            }
            AppManager::Get()->SwitchCurrentApp("Homepage");
        }
    }
}
