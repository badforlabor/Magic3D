#pragma once
#include "GppDefines.h"

namespace GPP
{
    class GeneralMatrixImpl;
    class GPP_EXPORT GeneralMatrix
    {
    public:
        GeneralMatrix();
        GeneralMatrix(Int nRow, Int nCol);
        GeneralMatrix(Int nRow, Int nCol, const Real* values);
        ~GeneralMatrix();

        Int GetRowCount() const;
        Int GetColCount() const;
        Real GetValue(Int row, int col) const;
        void SetValue(Int row, int col, Real value);

    private:
        GeneralMatrixImpl* mpImpl;
    };
}
