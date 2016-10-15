#pragma once
#include "MyGUI.h"

namespace MagicApp
{
    class UVUnfoldAppUI
    {
    public:
        UVUnfoldAppUI();
        ~UVUnfoldAppUI();

        void Setup();
        void Shutdown();

        void StartProgressbar(int range);
        void SetProgressbar(int value);
        void StopProgressbar(void);
        bool IsProgressbarVisible(void);

        void SetMeshInfo(int vertexCount, int triangleCount);

    private:
        void SwitchDisplayMode(MyGUI::Widget* pSender);

        void ImportTriMesh(MyGUI::Widget* pSender);

        void Geodesics(MyGUI::Widget* pSender);
        void SnapFrontGeodesics(MyGUI::Widget* pSender);
        void SnapBackGeodesics(MyGUI::Widget* pSender);
        void CloseGeodesics(MyGUI::Widget* pSender);
        void ConfirmGeodesics(MyGUI::Widget* pSender);
        void DeleteGeodesics(MyGUI::Widget* pSender);
        void SwitchMarkDisplay(MyGUI::Widget* pSender);
        void GenerateSplitLines(MyGUI::Widget* pSender);

        void Unfold2Disc(MyGUI::Widget* pSender);

        void UnfoldTriMesh(MyGUI::Widget* pSender);

        void GenerateUVAtlas(MyGUI::Widget* pSender);
        void DoGenerateUVAtlas(MyGUI::Widget* pSender);

        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
        bool mIsProgressbarVisible;
        MyGUI::TextBox* mTextInfo;
    };
}
