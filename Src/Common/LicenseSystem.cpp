#include "LicenseSystem.h"
#include "Gpp.h"
#include "LogSystem.h"
#include <fstream>
#include <windows.h>

namespace MagicCore
{
    LicenseSystem::LicenseSystem()
    {
    }

    LicenseSystem::~LicenseSystem()
    {
    }

    bool LicenseSystem::Init(void)
    {
        // Generate Registration Key
        std::ofstream fout("RegistrationKey.txt");
        fout << GPP::GetRegistrationKey() << std::endl;
        fout.close();
        // Verify Activation Key
        bool res = false;
        std::ifstream fin("ActivationKey.txt");
        char pLine[512];
        if (fin.getline(pLine, 512))
        {
            std::string keyStr(pLine);  
            res = GPP::SetActivationKey(keyStr);
        }
        fin.close();
        if (!res)
        {
            MessageBox(NULL, "软件激活失败, 试用软件", "温馨提示", MB_OK);
        }
        return res;
    }
}
