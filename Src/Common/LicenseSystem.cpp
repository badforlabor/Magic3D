#include "LicenseSystem.h"
#include "Gpp.h"
#include "LogSystem.h"
#include <fstream>

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
        bool res = false;
        std::ifstream fin("key.txt");
        char pLine[512];
        if (fin.getline(pLine, 512))
        {
            std::string keyStr(pLine);  
            res = GPP::SetActivationKey(keyStr);
        }
        fin.close();
        if (res)
        {
            DebugLog << "license passed" << std::endl;
        }
        else
        {
            DebugLog << "license failed" << std::endl;
        }
        return res;
    }
}
