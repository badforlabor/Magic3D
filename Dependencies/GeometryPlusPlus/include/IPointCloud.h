#pragma once
#include "Vector3.h"

namespace GPP
{
    class GPP_EXPORT IPointCloud
    {
    public:
        IPointCloud(){}

        virtual int GetPointCount() const = 0;
        virtual Vector3 GetPointCoord(int pid) const = 0;
        virtual void SetPointCoord(int pid, const Vector3& coord) = 0;
        virtual Vector3 GetPointNormal(int pid) const = 0;
        virtual void SetPointNormal(int pid, const Vector3& normal) = 0;

        virtual ~IPointCloud(){};
    };
}
