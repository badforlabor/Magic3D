#pragma once
#include "GPP.h"

namespace MagicApp
{
    class MagicPointCloud : public GPP::IPointCloud
    {
    public:
        MagicPointCloud(GPP::IPointCloud* pointCloud);

        virtual GPP::Int GetPointCount() const;
        virtual GPP::Vector3 GetPointCoord(GPP::Int pid) const;
        virtual void SetPointCoord(GPP::Int pid, const GPP::Vector3& coord);
        virtual GPP::Vector3 GetPointNormal(GPP::Int pid) const;
        virtual void SetPointNormal(GPP::Int pid, const GPP::Vector3& normal);
        virtual bool HasNormal() const;
        virtual void SetHasNormal(bool has);
        
        // Return inserted triangle id
        virtual GPP::Int InsertPoint(const GPP::Vector3& coord);
        // Return inserted triangle id
        virtual GPP::Int InsertPoint(const GPP::Vector3& coord, const GPP::Vector3& normal);
        
        virtual void SwapPoint(GPP::Int pointId0, GPP::Int pointId1); 
        virtual void PopbackPoints(GPP::Int popCount);
        virtual void Clear(void);

        virtual ~MagicPointCloud();

        void SetImageColorIds(std::vector<GPP::ImageColorId>* imageColorIds);
        void SetColorIds(std::vector<int>* colorIds);
        void SetCloudIds(std::vector<int>* cloudIds);

    private:
        GPP::IPointCloud* mPointCloud;
        std::vector<GPP::ImageColorId>* mImageColorIds;
        std::vector<int>* mColorIds;
        std::vector<int>* mCloudIds;
    };
}
