#include "MeshShopAppUI.h"
#include "../Common/ResourceManager.h"
#include "../Common/ToolKit.h"
#include "../Common/RenderSystem.h"
#include "AppManager.h"
#include "MeshShopApp.h"

namespace MagicApp
{
    MeshShopAppUI::MeshShopAppUI()
    {
    }

    MeshShopAppUI::~MeshShopAppUI()
    {
    }

    void MeshShopAppUI::Setup()
    {
        MagicCore::ResourceManager::LoadResource("../../Media/MeshShopApp", "FileSystem", "MeshShopApp");
        mRoot = MyGUI::LayoutManager::getInstance().loadLayout("MeshShopApp.layout");
        mRoot.at(0)->findWidget("But_ImportMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::ImportMesh);
        mRoot.at(0)->findWidget("But_ExportMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::ExportMesh);
        mRoot.at(0)->findWidget("But_ConsolidateMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::ConsolidateMesh);
        mRoot.at(0)->findWidget("But_ConsolidateTopology")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::ConsolidateTopology);
        mRoot.at(0)->findWidget("But_ReverseDirection")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::ReverseDirection);
        mRoot.at(0)->findWidget("But_ConsolidateGeometry")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::ConsolidateGeometry);
        mRoot.at(0)->findWidget("But_SmoothMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::SmoothMesh);
        mRoot.at(0)->findWidget("But_EnhanceMeshDetail")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::EnhanceMeshDetail);
        mRoot.at(0)->findWidget("But_SubdivideMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::SubdivideMesh);
        mRoot.at(0)->findWidget("But_LoopSubdivide")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::LoopSubdivide);
        mRoot.at(0)->findWidget("But_CCSubdivide")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::CCSubdivide);
        mRoot.at(0)->findWidget("But_RefineMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::RefineMesh);
        mRoot.at(0)->findWidget("But_DoRefineMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::DoRefineMesh);
        mRoot.at(0)->findWidget("But_SimplifyMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::SimplifyMesh);
        mRoot.at(0)->findWidget("But_DoSimplifyMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::DoSimplifyMesh);
        mRoot.at(0)->findWidget("But_ParameterizeMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::ParameterizeMesh);
        mRoot.at(0)->findWidget("But_SampleMesh")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::SampleMesh);
        mRoot.at(0)->findWidget("But_BackToHomepage")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::BackToHomepage);
        mRoot.at(0)->findWidget("But_Contact")->castType<MyGUI::Button>()->eventMouseButtonClick += MyGUI::newDelegate(this, &MeshShopAppUI::Contact);
    }

    void MeshShopAppUI::Shutdown()
    {
        MyGUI::LayoutManager::getInstance().unloadLayout(mRoot);
        mRoot.clear();
        MagicCore::ResourceManager::UnloadResource("MeshShopApp");
    }

    void MeshShopAppUI::ImportMesh(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->ImportMesh();
        }
    }

    void MeshShopAppUI::ExportMesh(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->ExportMesh();
        }
    }

    void MeshShopAppUI::ConsolidateMesh(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_ConsolidateTopology")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_ConsolidateTopology")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_ReverseDirection")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_ConsolidateGeometry")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_SmoothMesh")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_EnhanceMeshDetail")->castType<MyGUI::Button>()->setVisible(!isVisible);
    }

    void MeshShopAppUI::ConsolidateTopology(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->ConsolidateTopology();
        }
    }

    void MeshShopAppUI::ReverseDirection(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->ReverseDirection();
        }
    }

    void MeshShopAppUI::ConsolidateGeometry(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->ConsolidateGeometry();
        }
    }

    void MeshShopAppUI::SmoothMesh(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->SmoothMesh();
        }
    }

    void MeshShopAppUI::EnhanceMeshDetail(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->EnhanceMeshDetail();
        }
    }

    void MeshShopAppUI::SubdivideMesh(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_LoopSubdivide")->castType<MyGUI::Button>()->isVisible();
        mRoot.at(0)->findWidget("But_LoopSubdivide")->castType<MyGUI::Button>()->setVisible(!isVisible);
        mRoot.at(0)->findWidget("But_CCSubdivide")->castType<MyGUI::Button>()->setVisible(!isVisible);        
    }

    void MeshShopAppUI::LoopSubdivide(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->LoopSubdivide();
        }
    }

    void MeshShopAppUI::CCSubdivide(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->CCSubdivide();
        }
    }

    void MeshShopAppUI::RefineMesh(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DoRefineMesh")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DoRefineMesh")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_RefineTargetVertexCount")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            int meshVertexCount = 0;
            MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
            if (meshShop != NULL)
            {
                meshVertexCount = meshShop->GetMeshVertexCount();
            }
            std::stringstream ss;
            std::string textString;
            ss << meshVertexCount;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_RefineTargetVertexCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
        }
    }

    void MeshShopAppUI::DoRefineMesh(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            int meshVertexCount = meshShop->GetMeshVertexCount();
            std::string textString = mRoot.at(0)->findWidget("Edit_RefineTargetVertexCount")->castType<MyGUI::EditBox>()->getOnlyText();
            int targetVertexCount = std::atoi(textString.c_str());
            if (targetVertexCount < meshVertexCount)
            {
                std::stringstream ss;
                std::string textString;
                ss << meshVertexCount;
                ss >> textString;
                mRoot.at(0)->findWidget("Edit_RefineTargetVertexCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            }
            else
            {
                meshShop->RefineMesh(targetVertexCount);
            }            
        }
    }

    void MeshShopAppUI::SimplifyMesh(MyGUI::Widget* pSender)
    {
        bool isVisible = mRoot.at(0)->findWidget("But_DoSimplifyMesh")->castType<MyGUI::Button>()->isVisible();
        isVisible = !isVisible;
        mRoot.at(0)->findWidget("But_DoSimplifyMesh")->castType<MyGUI::Button>()->setVisible(isVisible);
        mRoot.at(0)->findWidget("Edit_SimplifyTargetVertexCount")->castType<MyGUI::EditBox>()->setVisible(isVisible);
        if (isVisible)
        {
            int meshVertexCount = 0;
            MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
            if (meshShop != NULL)
            {
                meshVertexCount = meshShop->GetMeshVertexCount();
            }
            std::stringstream ss;
            std::string textString;
            ss << meshVertexCount;
            ss >> textString;
            mRoot.at(0)->findWidget("Edit_SimplifyTargetVertexCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
        }
    }

    void MeshShopAppUI::DoSimplifyMesh(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            int meshVertexCount = meshShop->GetMeshVertexCount();
            std::string textString = mRoot.at(0)->findWidget("Edit_SimplifyTargetVertexCount")->castType<MyGUI::EditBox>()->getOnlyText();
            int targetVertexCount = std::atoi(textString.c_str());
            if (targetVertexCount > meshVertexCount || targetVertexCount < 3)
            {
                std::stringstream ss;
                std::string textString;
                ss << meshVertexCount;
                ss >> textString;
                mRoot.at(0)->findWidget("Edit_SimplifyTargetVertexCount")->castType<MyGUI::EditBox>()->setOnlyText(textString);
            }
            else
            {
                meshShop->SimplifyMesh(targetVertexCount);
            }            
        }
    }

    void MeshShopAppUI::ParameterizeMesh(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->ParameterizeMesh();
        }
    }

    void MeshShopAppUI::SampleMesh(MyGUI::Widget* pSender)
    {
        MeshShopApp* meshShop = dynamic_cast<MeshShopApp* >(AppManager::Get()->GetApp("MeshShopApp"));
        if (meshShop != NULL)
        {
            meshShop->SampleMesh();
        }
    }

    void MeshShopAppUI::BackToHomepage(MyGUI::Widget* pSender)
    {
        AppManager::Get()->SwitchCurrentApp("Homepage");
    }

    void MeshShopAppUI::Contact(MyGUI::Widget* pSender)
    {
        MagicCore::ToolKit::OpenWebsite(std::string("http://threepark.net/geometryplusplus/about"));
    }
}
