#include "Homepage.h"
#include "HomepageUI.h"
#include "../Common/LogSystem.h"

namespace MagicApp
{
    Homepage::Homepage() :
        mpUI(NULL)
    {
    }

    Homepage::~Homepage()
    {
        if (mpUI != NULL)
        {
            delete mpUI;
            mpUI = NULL;
        }
    }

    bool Homepage::Enter()
    {
        InfoLog << "Enter Homepage" << std::endl;
        if (mpUI == NULL)
        {
            mpUI = new HomepageUI;
        }
        mpUI->Setup();
        return true;
    }

    bool Homepage::Update(double timeElapsed)
    {
        return true;
    }

    bool Homepage::Exit()
    {
        InfoLog << "Exit Homepage" << std::endl;
        if (mpUI != NULL)
        {
            mpUI->Shutdown();
        }
        return true;
    }
}
