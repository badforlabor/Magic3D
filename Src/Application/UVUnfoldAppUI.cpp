#include "UVUnfoldAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "UVUnfoldApp.h"

namespace MagicApp
{
    UVUnfoldAppUI::UVUnfoldAppUI() :
        mIsProgressbarVisible(false),
        mTextInfo(NULL)
    {
    }

    UVUnfoldAppUI::~UVUnfoldAppUI()
    {
    }

    void UVUnfoldAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/UVUnfoldApp", "FileSystem", "UVUnfoldApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("UVUnfoldApp.layout");
        mRoot.at(0)->findWidget("But_DisplayMode")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::SwitchDisplayMode);

        mRoot.at(0)->findWidget("But_ImportTriMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::ImportTriMesh);
        
        mRoot.at(0)->findWidget("But_Geodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::Geodesics);
        mRoot.at(0)->findWidget("But_SnapFrontGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::SnapFrontGeodesics);
        mRoot.at(0)->findWidget("But_SnapBackGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::SnapBackGeodesics);
        mRoot.at(0)->findWidget("But_CloseGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::CloseGeodesics);
        mRoot.at(0)->findWidget("But_ConfirmGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::ConfirmGeodesics);
        mRoot.at(0)->findWidget("But_DeleteGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::DeleteGeodesics);
        mRoot.at(0)->findWidget("But_SwitchMarkDisplay")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::SwitchMarkDisplay);
        mRoot.at(0)->findWidget("But_GenerateSplitLines")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::GenerateSplitLines);
        
        mRoot.at(0)->findWidget("But_UnFold2Disc")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::Unfold2Disc);

        mRoot.at(0)->findWidget("But_UnFoldTriMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::UnfoldTriMesh);
        
        mRoot.at(0)->findWidget("But_GenerateUVAtlas")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::GenerateUVAtlas);
        mRoot.at(0)->findWidget("But_DoGenerateUVAtlas")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::DoGenerateUVAtlas);

        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &UVUnfoldAppUI::BackToHomepage);

        mTextInfo = mRoot.at(0)->findWidget("Text_Info")->castType<MyGUI::TextBox>();
        mTextInfo->setTextColour(MyGUI::Colour(75.0 / 255.0, 131.0 / 255.0, 128.0 / 255.0));
    }

    void UVUnfoldAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("UVUnfoldApp");
    }

    void UVUnfoldAppUI::StartProgressbar(int range)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(true);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressRange(range);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(0);
        mIsProgressbarVisible = true;
    }

    void UVUnfoldAppUI::SetProgressbar(int value)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(value);
    }

    void UVUnfoldAppUI::StopProgressbar()
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(false);
        mIsProgressbarVisible = false;
    }

    bool UVUnfoldAppUI::IsProgressbarVisible()
    {
        return mIsProgressbarVisible;
    }

    void UVUnfoldAppUI::SetMeshInfo(int vertexCount, int triangleCount)
    {
        std::string textString = "";
        if (vertexCount > 0)
        {
            textString += "Vertex count = ";
            std::stringstream ss;
            ss << vertexCount;
            std::string numberString;
            ss >> numberString;
            textString += numberString;
            textString += "  Triangle count = ";
            ss.clear();
            ss << triangleCount;
            numberString.clear();
            ss >> numberString;
            textString += numberString;
            textString += "\n";
        }
        mTextInfo->setCaption(textString);
    }

    void UVUnfoldAppUI::SwitchDisplayMode(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->SwitchDisplayMode(UVUnfoldApp::AUTO);
        }
    }

    void UVUnfoldAppUI::ImportTriMesh(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->ImportTriMesh();
        }
    }

    void UVUnfoldAppUI::Geodesics(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_ConfirmGeodesics")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_SnapFrontGeodesics")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_SnapBackGeodesics")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_CloseGeodesics")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_ConfirmGeodesics")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_DeleteGeodesics")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_SwitchMarkDisplay")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_GenerateSplitLines")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_SplitChartCount")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::stringstream ss;
            std::string textString;
            ss << 25;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_SplitChartCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
        }
    }

    void UVUnfoldAppUI::SnapFrontGeodesics(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->SnapFrontGeodesics();
        }
    }

    void UVUnfoldAppUI::SnapBackGeodesics(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->SnapBackGeodesics();
        }
    }

    void UVUnfoldAppUI::CloseGeodesics(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->CloseGeodesics();
        }
    }

    void UVUnfoldAppUI::ConfirmGeodesics(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->ConfirmGeodesics();
        }
    }

    void UVUnfoldAppUI::DeleteGeodesics(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->DeleteGeodesics();
        }
    }

    void UVUnfoldAppUI::SwitchMarkDisplay(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->SwitchMarkDisplay();
        }
    }

    void UVUnfoldAppUI::GenerateSplitLines(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            std::string textString = mRoot.at(0)->findWidget("Edit_SplitChartCount")->castType<MyGUI::EditBox>()->getOnlyText();
            int splitChartCount = std::atoi(textString.c_str());
            if (splitChartCount > 0)
            {
                uvUnfoldApp->GenerateSplitLines(splitChartCount);
            }
            else
            {
                std::stringstream ss;
                std::string textString;
                ss << 25;
                ss >> textString;
                mRoot.at(0)->findWidget("Edit_SplitChartCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            }
        }
    }

    void UVUnfoldAppUI::Unfold2Disc(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->Unfold2Disc(true);
        }
    }

    void UVUnfoldAppUI::UnfoldTriMesh(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            uvUnfoldApp->UnfoldTriMesh(true);
        }
    }

    void UVUnfoldAppUI::GenerateUVAtlas(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DoGenerateUVAtlas")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DoGenerateUVAtlas")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_InitChartCount")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::stringstream ss;
            std::string textString;
            ss << 25;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_InitChartCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
        }
    }

    void UVUnfoldAppUI::DoGenerateUVAtlas(MyGUI::Widget* pSender)
    {
        UVUnfoldApp* uvUnfoldApp = dynamic_cast<UVUnfoldApp* >(AppManager::Get()->GetApp("UVUnfoldApp"));
        if (uvUnfoldApp != NULL)
        {
            std::string textString = mRoot.at(0)->findWidget("Edit_InitChartCount")->castType<MyGUI::EditBox>()->getOnlyText();
            int initChartCount = std::atoi(textString.c_str());
            if (initChartCount > 0)
            {
                uvUnfoldApp->GenerateUVAtlas(initChartCount);
            }
            else
            {
                std::stringstream ss;
                std::string textString;
                ss << 25;
                ss >> textString;
                mRoot.at(0)->findWidget("Edit_InitChartCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            }
        }
    }

    void UVUnfoldAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }
}
