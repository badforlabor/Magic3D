#include "ToolKit.h"
#include <windows.h>
#include "LogSystem.h"

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

    bool ToolKit::MultiFileOpenDlg(std::vector<std::string>& selectFileNames, char* filterName)
    {
        char szOpenFileNames[MAX_PATH * 128] = "";
        char szPath[MAX_PATH];
        char* p;
        OPENFILENAME file = { 0 };
        file.lStructSize = sizeof(file);
        file.lpstrFile = szOpenFileNames;
        file.nMaxFile = sizeof(szOpenFileNames);
        file.lpstrFilter = filterName;
        file.lpstrFile[0] = '\0';
        file.Flags = OFN_EXPLORER|OFN_ALLOWMULTISELECT|OFN_NOCHANGEDIR;
        if(::GetOpenFileName(&file))
        {
            selectFileNames.clear();
            lstrcpyn(szPath, szOpenFileNames, file.nFileOffset);
            szPath[ file.nFileOffset ] = '\0';
            int nLen = lstrlen(szPath);
            if( szPath[nLen-1] != '\\' )   //如果选了多个文件,则必须加上'\\'
            {
                lstrcat(szPath, TEXT("\\"));
            }
            p = szOpenFileNames + file.nFileOffset; //把指针移到第一个文件
            while( *p )
            {
                selectFileNames.push_back(std::string(szPath) + std::string(p)); 
                p += lstrlen(p) +1;     //移至下一个文件
            }
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

    GPP::Vector3 ToolKit::ColorCoding(double f)
    {
        GPP::Vector3 colorV;
        if (f >= 0 && f < 0.2)
        {
            colorV[0] = f * 5.0;
            colorV[1] = 0;
            colorV[2] = 0;
        }
        else if (f >= 0.2 && f < 0.4)
        {
            colorV[0] = 1;
            colorV[1] = (f - 0.2) * 5;
            colorV[2] = 0;
        }
        else if (f >= 0.4 && f < 0.6)
        {
            colorV[0] = 1 - (f - 0.4) * 5;
            colorV[1] = 1;
            colorV[2] = 0;
        }
        else if (f >= 0.6 && f < 0.8)
        {
            colorV[0] = 0;
            colorV[1] = 1;
            colorV[2] = (f - 0.6) * 5;
        }
        else if (f >= 0.8 && f < 1)
        {
            colorV[0] = 0;
            colorV[1] = 1 - (f - 0.8) * 5;
            colorV[2] = 1;
        }
        else if (f >= 1 && f <= 1.2)
        {
            colorV[0] = (f - 1) * 5;
            colorV[1] = 0; 
            colorV[2] = 1;
        }
        else if (f > 1.2)
        {
            colorV[0] = 1;
            colorV[1] = 0;
            colorV[2] = 1;
        }
        else
        {
            colorV[0] = 0;
            colorV[1] = 0;
            colorV[2] = 0;
        }
        return colorV;
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
