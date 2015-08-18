#pragma once
#include "IPointCloud.h"
#include <vector>

namespace GPP
{
    class GPP_EXPORT Point3D
    {
    public:
        Point3D();
        Point3D(const Vector3& coord);
        Point3D(const Vector3& coord, const Vector3& normal);

        void SetCoord(const Vector3& coord);
        Vector3 GetCoord() const;
        void SetNormal(const Vector3& normal);
        Vector3 GetNormal() const;
        void SetColor(const Vector3& color);
        Vector3 GetColor() const;
        void SetId(int id);
        int GetId() const;

        ~Point3D();

    private:
        Vector3 mCoord;
        Vector3 mNormal;
        Vector3 mColor;
        int mId;
    };

    class GPP_EXPORT PointCloud : public IPointCloud
    {
    public:
        PointCloud();

        virtual int GetPointCount() const;
        virtual Vector3 GetPointCoord(int pid) const;
        virtual void SetPointCoord(int pid, const Vector3& coord);
        virtual Vector3 GetPointNormal(int pid) const;
        virtual void SetPointNormal(int pid, const Vector3& normal);

        Vector3 GetPointColor(int pid) const;
        void SetPointColor(int pid, const Vector3& color);
        int InsertPoint(const Vector3& coord);
        int InsertPoint(const Vector3& coord, const Vector3& normal);
        void UpdatePointId(void);
        void UnifyCoords(Real bboxSize);
        bool HasNormal() const;
        void SetHasNormal(bool has);

        virtual ~PointCloud();

    private:
        std::vector<Point3D* > mPointList;
        bool mHasNormal;
    };
}
