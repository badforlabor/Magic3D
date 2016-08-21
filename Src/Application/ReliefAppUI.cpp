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
        mRoot.at(0)->findWidget("But_DisplayMode")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::SwitchDisplayMode);
        mRoot.at(0)->findWidget("But_ImportModel")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::ImportModel);
        mRoot.at(0)->findWidget("But_Relief")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::Relief);
        mRoot.at(0)->findWidget("But_GenerateRelief")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::GenerateRelief);        
        mRoot.at(0)->findWidget("But_Scan")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::Scan);
        mRoot.at(0)->findWidget("But_GenerateScanColor")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::GenerateScanColor);        
        mRoot.at(0)->findWidget("But_GenerateScanShade")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::GenerateScanShade);        
        mRoot.at(0)->findWidget("But_SavePointCloud")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::SavePointCloud);
        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &ReliefAppUI::BackToHomepage);
    }

    void ReliefAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("ReliefApp");
    }

    void ReliefAppUI::SwitchDisplayMode(MyGUI::Widget* pSender)
    {
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->SwitchDisplayMode();
        }
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
        mRoot.at(0)->findWidget("Edit_Resolution")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::string textString = "0.5";
            mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
            textString = "650";
            mRoot.at(0)->findWidget("Edit_Resolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_Resolution")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }

    void ReliefAppUI::GenerateRelief(MyGUI::Widget* pSender)
    {
        bool isInputValid = true;
        std::string textString = mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->getOnlyText();
        double compressRatio = std::atof(textString.c_str());
        if (compressRatio <= 0 || compressRatio > 1)
        {
            std::string textString = "0.5";
            mRoot.at(0)->findWidget("Edit_CompressRatio")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            isInputValid = false;
        }
        textString = mRoot.at(0)->findWidget("Edit_Resolution")->castType<MyGUI::EditBox>()->getOnlyText();
        int resolution = std::atof(textString.c_str());
        if (resolution < 16 || resolution > 2048)
        {
            std::string textString = "650";
            mRoot.at(0)->findWidget("Edit_Resolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            isInputValid = false;
        }
        if (!isInputValid)
        {
            return;
        }
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->GenerateRelief(compressRatio, resolution);
        }
    }

    void ReliefAppUI::Scan(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_GenerateScanColor")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_GenerateScanColor")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("But_GenerateScanShade")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_ScanResolution")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_ImageResolution")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            std::string textString = "800";
            mRoot.at(0)->findWidget("Edit_ScanResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_ScanResolution")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);

            textString = "1024";
            mRoot.at(0)->findWidget("Edit_ImageResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            mRoot.at(0)->findWidget("Edit_ImageResolution")->castType<MyGUI::EditBox>()->setTextSelectionColour(MyGUI::Colour::Black);
        }
    }

    void ReliefAppUI::GenerateScanColor(MyGUI::Widget* pSender)
    {
        bool isInputValid = true;
        std::string textString = mRoot.at(0)->findWidget("Edit_ScanResolution")->castType<MyGUI::EditBox>()->getOnlyText();
        int scanResolution = std::atof(textString.c_str());
        textString = mRoot.at(0)->findWidget("Edit_ImageResolution")->castType<MyGUI::EditBox>()->getOnlyText();
        int imageResolution = std::atof(textString.c_str());
        if (scanResolution < 16 || scanResolution > 2048 || imageResolution < 16 || imageResolution > 2048)
        {
            std::string textString = "800";
            mRoot.at(0)->findWidget("Edit_ScanResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            textString = "1024";
            mRoot.at(0)->findWidget("Edit_ImageResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            isInputValid = false;
        }
        if (!isInputValid)
        {
            return;
        }
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->CaptureDepthPointCloud(scanResolution, imageResolution, "CookTorranceColor");
        }
    }

    void ReliefAppUI::GenerateScanShade(MyGUI::Widget* pSender)
    {
        bool isInputValid = true;
        std::string textString = mRoot.at(0)->findWidget("Edit_ScanResolution")->castType<MyGUI::EditBox>()->getOnlyText();
        int scanResolution = std::atof(textString.c_str());
        textString = mRoot.at(0)->findWidget("Edit_ImageResolution")->castType<MyGUI::EditBox>()->getOnlyText();
        int imageResolution = std::atof(textString.c_str());
        if (scanResolution < 16 || scanResolution > 2048 || imageResolution < 16 || imageResolution > 2048)
        {
            std::string textString = "800";
            mRoot.at(0)->findWidget("Edit_ScanResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            textString = "1024";
            mRoot.at(0)->findWidget("Edit_ImageResolution")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            isInputValid = false;
        }
        if (!isInputValid)
        {
            return;
        }
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->CaptureDepthPointCloud(scanResolution, imageResolution, "CookTorranceShade");
        }
    }
    
    void ReliefAppUI::SavePointCloud(MyGUI::Widget* pSender)
    {
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->SavePointCloud();
        }
    }

    void ReliefAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        ReliefApp* reliefApp = dynamic_cast<ReliefApp* >(AppManager::Get()->GetApp("ReliefApp"));
        if (reliefApp != NULL)
        {
            reliefApp->ExportReliefMesh();
        }
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }

}
