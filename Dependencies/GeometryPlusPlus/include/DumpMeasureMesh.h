#pragma once
#include "DumpInfo.h"
#include "Vector3.h"
#include <vector>

namespace GPP
{
    extern GPP_EXPORT DumpBase* CreateDumpMeshMeasureSectionApproximate(void);

    class GPP_EXPORT DumpMeshMeasureSectionApproximate : public DumpBase
    {
    public:
        DumpMeshMeasureSectionApproximate();
        ~DumpMeshMeasureSectionApproximate();

        virtual ApiName GetApiName(void);
        virtual void LoadDumpFile(const std::string& fileName);
        virtual ErrorCode Run(void);
        virtual TriMesh* GetTriMesh(Int id = 0);

        void DumpApiInfo(const ITriMesh* triMesh, const std::vector<Int>& sectionVertexIds, bool isSectionClose);

    private:
        TriMesh* mpTriMesh;
        std::vector<Int> mSectionVertexIds;
        bool mIsSectionClose;
    }; 

    extern GPP_EXPORT DumpBase* CreateDumpMeshMeasureSectionExact(void);

    class GPP_EXPORT DumpMeshMeasureSectionExact : public DumpBase
    {
    public:
        DumpMeshMeasureSectionExact();
        ~DumpMeshMeasureSectionExact();

        virtual ApiName GetApiName(void);
        virtual void LoadDumpFile(const std::string& fileName);
        virtual ErrorCode Run(void);
        virtual TriMesh* GetTriMesh(Int id = 0);

        void DumpApiInfo(const ITriMesh* triMesh, const std::vector<Int>& sectionVertexIds, bool isSectionClose);
        std::vector<Vector3> GetSectionPathPoints()const;

    private:
        TriMesh* mpTriMesh;
        std::vector<Int> mSectionVertexIds;
        bool mIsSectionClose;
        std::vector<Vector3> mSectionPathPoints;
    };
}
