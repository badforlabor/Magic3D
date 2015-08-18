#pragma once
#include "AppBase.h"

namespace MagicApp
{
    class HomepageUI;
    class Homepage : public AppBase
    {
    public: 
        Homepage();
        virtual ~Homepage();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);

    private:
        HomepageUI* mpUI;
    };
}
