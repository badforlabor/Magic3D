#include "TextureAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "TextureApp.h"

namespace MagicApp
{
    TextureAppUI::TextureAppUI() :
        mIsProgressbarVisible(false),
        mTextInfo(NULL)
    {
    }

    TextureAppUI::~TextureAppUI()
    {
    }

    void TextureAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/TextureApp", "FileSystem", "TextureApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("TextureApp.layout");
        mRoot.at(0)->findWidget("But_DisplayMode")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::SwitchDisplayMode);
        mRoot.at(0)->findWidget("But_TextureImage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::SwitchTextureImage);

        mRoot.at(0)->findWidget("But_ImportTriMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::ImportTriMesh);
        mRoot.at(0)->findWidget("But_ExportTriMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::ExportTriMesh);

        mRoot.at(0)->findWidget("But_Geodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::Geodesics);
        mRoot.at(0)->findWidget("But_ConfirmGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::ConfirmGeodesics);
        mRoot.at(0)->findWidget("But_DeleteGeodesics")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::DeleteGeodesics);
        mRoot.at(0)->findWidget("But_SwitchMarkDisplay")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::SwitchMarkDisplay);
        
        mRoot.at(0)->findWidget("But_UnFoldTriMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::UnfoldTriMesh);
        mRoot.at(0)->findWidget("But_Optimize2Isometric")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::Optimize2Isometric);
        
        mRoot.at(0)->findWidget("But_GenerateUVAtlas")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::GenerateUVAtlas);
        mRoot.at(0)->findWidget("But_DoGenerateUVAtlas")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::DoGenerateUVAtlas);

        mRoot.at(0)->findWidget("But_EnterMeshToolApp")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::EnterMeshToolApp);

        mRoot.at(0)->findWidget("But_OptimizeColorConsistency")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::OptimizeColorConsistency);

        mRoot.at(0)->findWidget("But_EnterPointShop")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::EnterPointToolApp);

        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::BackToHomepage);

        mTextInfo = mRoot.at(0)->findWidget("Text_Info")->castType<MyGUI::TextBox>();
        mTextInfo->setTextColour(MyGUI::Colour(75.0 / 255.0, 131.0 / 255.0, 128.0 / 255.0));
    }

    void TextureAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("TextureApp");
    }

    void TextureAppUI::StartProgressbar(int range)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(true);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressRange(range);
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(0);
        mIsProgressbarVisible = true;
    }

    void TextureAppUI::SetProgressbar(int value)
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setProgressPosition(value);
    }

    void TextureAppUI::StopProgressbar()
    {
        mRoot.at(0)->findWidget("APIProgress")->castType<MyGUI::ProgressBar>()->setVisible(false);
        mIsProgressbarVisible = false;
    }

    bool TextureAppUI::IsProgressbarVisible()
    {
        return mIsProgressbarVisible;
    }

    void TextureAppUI::SetMeshInfo(int vertexCount, int triangleCount)
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

    void TextureAppUI::SwitchDisplayMode(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->SwitchDisplayMode();
        }
    }

    void TextureAppUI::SwitchTextureImage(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->SwitchTextureImage();
        }
    }

    void TextureAppUI::ImportTriMesh(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->ImportTriMesh();
        }
    }

    void TextureAppUI::ExportTriMesh(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->ExportTriMesh();
        }
    }

    void TextureAppUI::Geodesics(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_ConfirmGeodesics")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_ConfirmGeodesics")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_DeleteGeodesics")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_SwitchMarkDisplay")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void TextureAppUI::ConfirmGeodesics(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->ConfirmGeodesics();
        }
    }

    void TextureAppUI::DeleteGeodesics(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->DeleteGeodesics();
        }
    }

    void TextureAppUI::SwitchMarkDisplay(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->SwitchMarkDisplay();
        }
    }

    void TextureAppUI::UnfoldTriMesh(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->UnfoldTriMesh();
        }
    }

    void TextureAppUI::Optimize2Isometric(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->Optimize2Isometric();
        }
    }

    void TextureAppUI::GenerateUVAtlas(MyGUI::Widget* pSender)
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

    void TextureAppUI::DoGenerateUVAtlas(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            std::string textString = mRoot.at(0)->findWidget("Edit_InitChartCount")->castType<MyGUI::EditBox>()->getOnlyText();
            int initChartCount = std::atoi(textString.c_str());
            if (initChartCount > 0)
            {
                textureApp->GenerateUVAtlas(initChartCount);
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

    void TextureAppUI::EnterMeshToolApp(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->EnterMeshToolApp();
        }
    }

    void TextureAppUI::OptimizeColorConsistency(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->OptimizeColorConsistency();
        }
    }

    void TextureAppUI::EnterPointToolApp(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->EnterPointToolApp();
        }
    }

    void TextureAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }
}
