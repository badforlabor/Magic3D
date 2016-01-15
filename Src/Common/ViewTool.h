#pragma once

namespace MagicCore
{
    class ViewTool
    {
    public:
        enum MouseMode
        {
            MM_NONE,
            MM_LEFT_DOWN,
            MM_MIDDLE_DOWN,
            MM_RIGHT_DOWN
        };

        ViewTool();
        ViewTool(double scale);
        void MousePressed(int mouseCoordX, int mouseCoordY);
        void MouseMoved(int mouseCoordX, int mouseCoordY, MouseMode mm);
        void MouseReleased();
        void SetScale(double scale);
        ~ViewTool();

    private:
        int mMouseCoordX;
        int mMouseCoordY;
        double mScale;
        bool mIsMousePressed;
    };
}
