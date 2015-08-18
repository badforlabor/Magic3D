#pragma once

namespace MagicCore
{
    class MagicFramework
    {
    public:
        MagicFramework();
        ~MagicFramework();

        void Init(void);
        void Run(void);

    private:
        void Update(double timeElapsed);
        bool Running(void);

    private:
        double mTimeAccumulate;
        double mRenderDeltaTime;
    };
}
