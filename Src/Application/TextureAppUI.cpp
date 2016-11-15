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
        
        mRoot.at(0)->findWidget("But_ImageColorIds")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::ImageColorIds);
        mRoot.at(0)->findWidget("But_ComputePixelMap")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::ComputeImageColorIds);
        mRoot.at(0)->findWidget("But_LoadImageColorIds")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::LoadImageColorIds);
        mRoot.at(0)->findWidget("But_SaveImageColorIds")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::SaveImageColorIds);

        mRoot.at(0)->findWidget("But_FuseMeshColor")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::FuseMeshColor);

        mRoot.at(0)->findWidget("But_GenerateTextureImage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::GenerateTextureImage);
        mRoot.at(0)->findWidget("But_TextureImageByVertex")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::GenerateTextureImageByVertexColor);
        mRoot.at(0)->findWidget("But_TextureImageByImage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::GenerateTextureImageByImage);
        mRoot.at(0)->findWidget("But_TuneTextureImageByVertex")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &TextureAppUI::TuneTextureImageByVertexColor);

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

    void TextureAppUI::ImageColorIds(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_LoadImageColorIds")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_ComputePixelMap")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_LoadImageColorIds")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_SaveImageColorIds")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void TextureAppUI::ComputeImageColorIds(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->ComputeImageColorIds(true);
        }
    }

    void TextureAppUI::LoadImageColorIds(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->LoadImageColorInfo();
        }
    }

    void TextureAppUI::SaveImageColorIds(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->SaveImageColorInfo();
        }
    }

    void TextureAppUI::FuseMeshColor(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->FuseMeshColor(true);
        }
    }

    void TextureAppUI::GenerateTextureImage(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_TextureImageByVertex")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_TextureImageByVertex")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_TextureImageByImage")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_TuneTextureImageByVertex")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void TextureAppUI::GenerateTextureImageByVertexColor(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->GenerateTextureImage(true);
        }
    }

    void TextureAppUI::GenerateTextureImageByImage(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->GenerateTextureImage(false);
        }
    }

    void TextureAppUI::TuneTextureImageByVertexColor(MyGUI::Widget* pSender)
    {
        TextureApp* textureApp = dynamic_cast<TextureApp* >(AppManager::Get()->GetApp("TextureApp"));
        if (textureApp != NULL)
        {
            textureApp->TuneTextureImageByVertexColor();
        }
    }

    void TextureAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }
}
