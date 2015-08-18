#pragma once
#include <string>

namespace MagicCore
{
    class ToolKit
    {
    private:
        static ToolKit*  mpToolKit;
        ToolKit(void);
    public:
        static ToolKit* Get(void);
        static double GetTime(void);
        static bool FileOpenDlg(std::string& selectFileName, char* filterName);
        static bool FileSaveDlg(std::string& selectFileName, char* filterName);
        static void OpenWebsite(std::string& address);

        bool IsAppRunning(void);
        void SetAppRunning(bool bRunning);
        void SetMousePressLocked(bool locked);
        bool IsMousePressLocked() const;
        
        virtual ~ToolKit(void);

    private:
        bool mAppRunning;
        bool mIsONIInitialized;
        bool mMousePressLocked; //this is just a hack
	};
}

