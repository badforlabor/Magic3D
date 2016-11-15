#pragma once
#include "Gpp.h"
#include "opencv2/opencv.hpp"

namespace MagicApp
{
    class MeshShopApp;
    class ReliefApp;
    class AppApi
    {
    public:
        static bool EnterApp(const char* appName);

        static GPP::TriMesh* GetMesh();
        static MagicApp::MeshShopApp* GetMeshShopApp();
        static MagicApp::ReliefApp*   GetReliefApp();

    };
}