#pragma once
#include "ITriMesh.h"
#include <vector>
#include <map>

namespace GPP
{
    class Edge3D;
    class GPP_EXPORT Vertex3D
    {
    public:
        Vertex3D();
        Vertex3D(const Vector3& coord);
        Vertex3D(const Vector3& coord, const Vector3& normal);

        Vector3 GetCoord() const;
        void    SetCoord(const Vector3& coord);
        Vector3 GetNormal() const;
        void    SetNormal(const Vector3& normal);
        Vector3 GetTexcoord() const;
        void    SetTexcoord(const Vector3& texCoord);
        Vector3 GetColor() const;
        void    SetColor(const Vector3& color);
        Edge3D* GetEdge();
        const Edge3D* GetEdge() const;
        void    SetEdge(Edge3D* edge);
        int     GetId() const;
        void    SetId(int id);

        ~Vertex3D();

    private:
        Vector3 mCoord;
        Vector3 mNormal;
        Vector3 mTexCoord;
        Vector3 mColor;
        Edge3D* mpEdge;
        int     mId;
    };

    class Face3D;
    class GPP_EXPORT Edge3D
    {
    public:
        Edge3D();

        Vertex3D* GetVertex();
        const Vertex3D* GetVertex() const;
        void      SetVertex(Vertex3D* vertex);
        Edge3D*   GetPair();
        const Edge3D* GetPair() const;
        void      SetPair(Edge3D* edge);
        Edge3D*   GetNext();
        const Edge3D* GetNext() const;
        void      SetNext(Edge3D* edge);
        Edge3D*   GetPre();
        const Edge3D* GetPre() const;
        void      SetPre(Edge3D* edge);
        Face3D*   GetFace();
        const Face3D* GetFace() const;
        void      SetFace(Face3D* face);
        int       GetId() const;
        void      SetId(int id);

        ~Edge3D();

    private:
        Vertex3D* mpVertex;
        Edge3D*   mpPair;
        Edge3D*   mpNext;
        Edge3D*   mpPre;
        Face3D*   mpFace;
        int       mId;
    };

    class GPP_EXPORT Face3D
    {
    public:
        Face3D();

        Edge3D* GetEdge();
        const Edge3D* GetEdge() const;
        void    SetEdge(Edge3D* edge);
        Vector3 GetNormal() const;
        void    SetNormal(const Vector3& normal);
        int     GetId() const;
        void    SetId(int id);

        ~Face3D();

    private:
        Edge3D* mpEdge;
        Vector3 mNormal;
        int     mId;
    };

    struct GPP_EXPORT TriangleVertexIndex
    {
        int mIndex[3];
    };

    class GPP_EXPORT TriMesh : public ITriMesh
    {
    public:
        TriMesh();

        virtual int GetVertexCount() const;
        virtual Vector3 GetVertexCoord(int vid) const;
        virtual void SetVertexCoord(int vid, const Vector3& coord);
        virtual Vector3 GetVertexNormal(int vid) const;
        virtual void SetVertexNormal(int vid, const Vector3& normal);
        virtual int GetTriangleCount() const;
        virtual void GetTriangleVertexIds(int fid, int vertexIds[3]) const;
        virtual void UpdateNormal(void);
        virtual int InsertTriangle(int vertexId0, int vertexId1, int vertexId2);
        virtual int InsertVertex(const Vector3& coord);
        void InsertVertex(Vertex3D* vertex);     
        int InsertVertex(const Vector3& coord, const Vector3& normal);
        virtual void Clear(void);

        Vector3 GetVertexColor(int vid) const;
        void SetVertexColor(int vid, const Vector3& color);
        Vector3 GetVertexTexcoord(int vid) const;
        void SetVertexTexcoord(int vid, const Vector3& texcoord);
        Vertex3D* GetVertex(int vid);
        const Vertex3D* GetVertex(int vid) const;  
        
        void UnifyCoords(Real bboxSize);

        virtual ~TriMesh();

    private:
        std::vector<Vertex3D*> mVertexList;
        std::vector<TriangleVertexIndex> mTriangleList;
    };

    class GPP_EXPORT HalfMesh
    {
    public:
        HalfMesh();
        
        Vertex3D* GetVertex(int vid);
        const Vertex3D* GetVertex(int vid) const;
        Edge3D* GetEdge(int eid);
        const Edge3D* GetEdge(int eid) const;
        Face3D* GetFace(int fid);
        const Face3D* GetFace(int fid) const;
        int GetVertexCount() const;
        int GetEdgeCount() const;
        int GetFaceCount() const;

        void InsertVertex(Vertex3D* vertex);
        Vertex3D* InsertVertex(const Vector3& coord);
        Edge3D*   InsertEdge(Vertex3D* vertexStart, Vertex3D* vertexEnd);
        Face3D*   InsertFace(const std::vector<Vertex3D* >& vertexList);

        void UnifyCoords(Real bboxSize);
        void UpdateNormal();
        void ValidateTopology();
        void RemoveEdgeFromEdgeMap(Edge3D* edge);
        void UpdateVertexIndex();
        void UpdateEdgeIndex();
        void UpdateFaceIndex();

        ~HalfMesh();

    private:
        std::vector<Vertex3D* > mVertexList;
        std::vector<Edge3D* >   mEdgeList;
        std::vector<Face3D* >   mFaceList;
        std::map<std::pair<Vertex3D*, Vertex3D*>, Edge3D* > mEdgeMap; //Only used in construct mesh
    };
}
