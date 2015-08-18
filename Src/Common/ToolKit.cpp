#include "ToolKit.h"
#include <windows.h>

namespace MagicCore
{
    ToolKit* ToolKit::mpToolKit = NULL;

    ToolKit::ToolKit(void) : 
        mAppRunning(true), 
        mIsONIInitialized(false),
        mMousePressLocked(false)
    {
    }

    ToolKit* ToolKit::Get()
    {
        if (mpToolKit == NULL)
        {
            mpToolKit = new ToolKit;
        }
        return mpToolKit;
    }

    double ToolKit::GetTime()
    {
        static __int64 start = 0;
        static __int64 frequency = 0;

        if (start == 0)
        {
            QueryPerformanceCounter((LARGE_INTEGER*)&start);
            QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
            return 0.0f;
        }

        __int64 counter = 0;
        QueryPerformanceCounter((LARGE_INTEGER*)&counter);
        return (double) ((counter - start) / double(frequency));
    }

    bool ToolKit::FileOpenDlg(std::string& selectFileName, char* filterName)
    {
        char szFileName[MAX_PATH] = "";
        OPENFILENAME file = { 0 };
        file.lStructSize = sizeof(file);
        file.lpstrFile = szFileName;
        file.nMaxFile = MAX_PATH;
        file.lpstrFilter = filterName;
        //file.Flags = OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_ALLOWMULTISELECT|OFN_NOCHANGEDIR;
        file.Flags = OFN_NOCHANGEDIR;
        if(::GetOpenFileName(&file))
        {
            selectFileName = szFileName;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool ToolKit::FileSaveDlg(std::string& selectFileName, char* filterName)
    {
        char szFileName[MAX_PATH] = "";
        OPENFILENAME file = { 0 };
        file.lStructSize = sizeof(file);
        file.lpstrFile = szFileName;
        file.nMaxFile = MAX_PATH;
        file.lpstrFilter = filterName;
        file.nFilterIndex = 1;
        file.Flags = OFN_NOCHANGEDIR;
        if(::GetSaveFileName(&file))
        {
            selectFileName = szFileName;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool ToolKit::IsAppRunning()
    {
        return mAppRunning;
    }

    void ToolKit::SetAppRunning(bool bRunning)
    {
        mAppRunning = bRunning;
    }

    void ToolKit::OpenWebsite(std::string& address)
    {
        ShellExecute(NULL, "open", address.c_str(), NULL, NULL, SW_SHOW);
    }

    void ToolKit::SetMousePressLocked(bool locked)
    {
        mMousePressLocked = locked;
    }

    bool ToolKit::IsMousePressLocked() const
    {
        return mMousePressLocked;
    }

    ToolKit::~ToolKit(void)
    {
    }
}
