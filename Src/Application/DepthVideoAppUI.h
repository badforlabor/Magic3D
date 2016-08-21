#pragma once
#include "MyGUI.h"

namespace MagicApp
{
    class DepthVideoAppUI
    {
    public:
        DepthVideoAppUI();
        ~DepthVideoAppUI();

        void Setup();
        void Shutdown();

        void StartProgressbar(int range);
        void SetProgressbar(int value);
        void StopProgressbar(void);
        bool IsProgressbarVisible(void);

        void SetScrollRange(int range);

    private:
        void ChangeFrameIndex(MyGUI::ScrollBar* pSender, size_t pos);
        void ImportPointCloud(MyGUI::Widget* pSender);
        void AlignPointCloudList(MyGUI::Widget* pSender);
        void DoAlignPointCloudList(MyGUI::Widget* pSender);
        void BackToHomepage(MyGUI::Widget* pSender);

    private:
        MyGUI::VectorWidgetPtr mRoot;
        bool mIsProgressbarVisible;
        int mScrollRange;
        int mScrollPosition;
    };
}
