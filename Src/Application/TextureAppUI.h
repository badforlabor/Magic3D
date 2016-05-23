#pragma once
#include "MyGUI.h"

namespace MagicApp
{
    class TextureAppUI
    {
    public:
        TextureAppUI();
        ~TextureAppUI();

        void Setup();
        void Shutdown();

        void StartProgressbar(int range);
        void SetProgressbar(int value);
        void StopProgressbar(void);
        bool IsProgressbarVisible(void);

        void SetMeshInfo(int vertexCount, int triangleCount);

    private:
        void SwitchDisplayMode(MyGUI::Widget* pSender);
        void SwitchTextureImage(MyGUI::Widget* pSender);

        void ImportTriMesh(MyGUI::Widget* pSender);
        void ExportTriMesh(MyGUI::Widget* pSender);

        void Geodesics(MyGUI::Widget* pSender);
        void ConfirmGeodesics(MyGUI::Widget* pSender);
        void DeleteGeodesics(MyGUI::Widget* pSender);
        void SwitchMarkDisplay(MyGUI::Widget* pSender);

        void UnfoldTriMesh(MyGUI::Widget* pSender);
        void Optimize2Isometric(MyGUI::Widget* pSender);

        void GenerateUVAtlas(MyGUI::Widget* pSender);
        void DoGenerateUVAtlas(MyGUI::Widget* pSender);

        void EnterMeshToolApp(MyGUI::Widget* pSender);

        void OptimizeColorConsistency(MyGUI::Widget* pSender);

        void EnterPointToolApp(MyGUI::Widget* pSender);

        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
        bool mIsProgressbarVisible;
        MyGUI::TextBox* mTextInfo;
    };
}
