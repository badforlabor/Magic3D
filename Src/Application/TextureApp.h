#pragma once
#include "AppBase.h"
#include "Gpp.h"
#include "opencv2/opencv.hpp"
#include "OgreTextureManager.h"
#if DEBUGDUMPFILE
#include "DumpBase.h"
#endif

namespace GPP
{
    class TriMesh;
}

namespace MagicCore
{
    class ViewTool;
    class PickTool;
}

namespace MagicApp
{
    class TextureAppUI;
    class TextureApp : public AppBase
    {
        enum CommandType
        {
            NONE = 0
        };

        enum DisplayMode
        {
            TRIMESH_SOLID = 0,
            TRIMESH_WIREFRAME,
            UVMESH_WIREFRAME,
            TRIMESH_TEXTURE
        };

        enum TextureType
        {
            TT_NONE = 0,
            TT_SINGLE,
            TT_MULTIPLE
        };

    public:

        TextureApp();
        ~TextureApp();

        virtual bool Enter(void);
        virtual bool Update(double timeElapsed);
        virtual bool Exit(void);
        virtual bool MouseMoved(const OIS::MouseEvent &arg);
        virtual bool MousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool MouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
        virtual bool KeyPressed(const OIS::KeyEvent &arg);

        void DoCommand(bool isSubThread);

        void SwitchDisplayMode(void);
        void SwitchTextureImage(void);
        void UpdatetextureImage(void);

        void ImportTriMesh(void);

        void SaveImageColorInfo(void);
        void LoadImageColorInfo(void);
        void PickMeshColorFromImages(void);

        void GenerateTextureImage(bool isByVertexColor);
        void TuneTextureImageByVertexColor(void);

        int GetMeshVertexCount(void);

#if DEBUGDUMPFILE
        void SetDumpInfo(GPP::DumpBase* dumpInfo);
        void RunDumpInfo(void);
#endif

    private:
        void InitViewTool(void);
        void UpdateDisplay(void);
        void UpdateTriMeshTexture(void);
        void UnifyTextureCoords(std::vector<double>& texCoords, double scaleValue);
        void ExportObjFile(void);

    private:
        void SetupScene(void);
        void ShutdownScene(void);
        void ClearData(void);
        bool IsCommandAvaliable(void);
        void ClearMeshData(void);

    private:
        TextureAppUI* mpUI;
        GPP::TriMesh* mpImageFrameMesh;
        cv::Mat mDistortionImage;
        int mTextureImageSize;
        Ogre::TexturePtr mpTriMeshTexture;
        MagicCore::ViewTool* mpViewTool;
        DisplayMode mDisplayMode;
        CommandType mCommandType;
        bool mIsCommandInProgress;
        bool mUpdateDisplay;
        std::vector<std::string> mTextureImageNames;
        int mCurrentTextureImageId;
        TextureType mTextureType;
        std::string mTextureImageName;
        std::vector<GPP::Int> mTextureImageMasks;
    };
}
