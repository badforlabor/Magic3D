#pragma once
#include "GppDefines.h"
#include <vector>

namespace GPP
{
    class GPP_EXPORT Triplet
    {
    public:
        Triplet();
        Triplet(Int row, Int col, Real value);
        ~Triplet();

        Int  GetRow() const;
        void SetRow(Int row);
        Int  GetCol() const;
        void SetCol(Int col);
        Real GetValue() const;
        void SetValue(Real value);
    
    private:
        Int mRow, mCol;
        Real mValue;
    };

    class SparseMatrixImpl;
    class GPP_EXPORT SparseMatrix
    {
    public:
        SparseMatrix();
        SparseMatrix(Int nRow, Int nCol);
        SparseMatrix(Int nRow, Int nCol, const std::vector<Triplet>& tripletList);
        ~SparseMatrix();

        void Init(Int nRow, Int nCol);
        bool AddTriplet(Int row, Int col, Real value);
        bool BuildFromTriplets(void);

    private:
        SparseMatrixImpl* mpImpl;
    };
}
