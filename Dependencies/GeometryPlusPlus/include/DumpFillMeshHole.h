#pragma once

#include "DumpInfo.h"
#include <vector>

namespace GPP
{
    extern GPP_EXPORT DumpBase* CreateDumpFillMeshHole(void);
    extern GPP_EXPORT DumpBase* CreateDumpFindMeshHole(void);

    class GPP_EXPORT DumpFillMeshHole : public DumpBase
    {
    public:
        DumpFillMeshHole();
        ~DumpFillMeshHole();

        virtual ApiName GetApiName();
        virtual void LoadDumpFile(const std::string& fileName);
        virtual ErrorCode Run(void);
        virtual TriMesh* GetTriMesh(Int id = 0);

        void DumpApiInfo(const ITriMesh* triMesh, const std::vector<Int>& brdSeedIds, Int fillHoleMethod);
        std::vector<Int> GetBoundarySeedIds()const;

    private:
        TriMesh*         mpTriMesh;
        std::vector<Int> mBoundarySeeds;
        Int              mFillHoleMethod;
    };

    class GPP_EXPORT DumpFindMeshHole : public DumpBase
    {
    public:
        DumpFindMeshHole();
        ~DumpFindMeshHole();

        virtual ApiName GetApiName();
        virtual void LoadDumpFile(const std::string& fileName);
        virtual ErrorCode Run(void);
        virtual TriMesh* GetTriMesh(Int id = 0);

        void DumpApiInfo(const ITriMesh* triMesh);
        std::vector<std::vector<GPP::Int> > GetBoundaryLoopVrtIds() const;

    private:
        TriMesh* mpTriMesh;
        std::vector<std::vector<GPP::Int> > mBoundaryLoopVrtIds;
    };

}