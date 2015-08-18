#pragma once
#include "SparseMatrix.h"
#include "VectorX.h"

namespace GPP
{
    class LinearSparseLUSolverImpl;
    class GPP_EXPORT LinearSparseLUSolver
    {
    public:
        LinearSparseLUSolver();
        ~LinearSparseLUSolver();

        Int Factorize(const SparseMatrix& sparseMatrix);
        Int Solve(const VectorX& vecB, VectorX* result);

    private:
        LinearSparseLUSolverImpl* mpImpl;
    };
}
