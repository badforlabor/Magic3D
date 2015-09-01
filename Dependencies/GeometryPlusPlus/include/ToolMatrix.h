#pragma once
#include "SparseMatrix.h"
#include "GeneralMatrix.h"

namespace GPP
{
    class LinearSparseLUSolverImpl;
    class GPP_EXPORT LinearSparseLUSolver
    {
    public:
        LinearSparseLUSolver();
        ~LinearSparseLUSolver();

        Int Factorize(const SparseMatrix& sparseMatrix);
        Int Solve(const std::vector<Real>& vecB, std::vector<Real>* result);

    private:
        LinearSparseLUSolverImpl* mpImpl;
    };

    class SelfAdjointEigenSolverImpl;
    class GPP_EXPORT SelfAdjointEigenSolver
    {
    public:
        SelfAdjointEigenSolver();
        ~SelfAdjointEigenSolver();

        Int Compute(const GeneralMatrix& generalMatrix);
        
        std::vector<Real> GetEigenVector(Int index) const;

        //The eigenvalues are sorted in increasing order
        Real GetEigenValue(Int index) const;

    private:
        SelfAdjointEigenSolverImpl* mpImpl;
    };

}
