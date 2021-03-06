#include "biconjugate-gradients-stabilized.hpp"

clsparseStatus
clsparseScsrbicgStab(cldenseVector* x, const clsparseCsrMatrix *A, const cldenseVector *b,
               clSParseSolverControl solverControl, clsparseControl control)
{
    using T = cl_float;

    if (!clsparseInitialized)
    {
        return clsparseNotInitialized;
    }

    if (control == nullptr)
    {
        return clsparseInvalidControlObject;
    }

    if (solverControl == nullptr)
    {
        return clsparseInvalidSolverControlObject;
    }

    cldenseVectorPrivate* pX = static_cast<cldenseVectorPrivate*> ( x );
    const clsparseCsrMatrixPrivate* pA = static_cast<const clsparseCsrMatrixPrivate*> ( A );
    const cldenseVectorPrivate* pB = static_cast<const cldenseVectorPrivate*> ( b );


    std::shared_ptr<PreconditionerHandler<T>> preconditioner;

    if (solverControl->preconditioner == DIAGONAL)
    {
        preconditioner = std::shared_ptr<PreconditionerHandler<T>>(new DiagonalHandler<T>());
        // call constructor of preconditioner class
        preconditioner->notify(pA, control);
    }
    else
    {
        preconditioner = std::shared_ptr<PreconditionerHandler<T>>(new VoidHandler<T>());
        preconditioner->notify(pA, control);
    }

    clsparseStatus status = bicgStab<T>(pX, pA, pB, *preconditioner, solverControl, control);

    solverControl->printSummary(status);

    return status;


}

clsparseStatus
clsparseDcsrbicgStab(cldenseVector* x, const clsparseCsrMatrix *A, const cldenseVector *b,
               clSParseSolverControl solverControl, clsparseControl control)
{
    using T = cl_double;

    if (!clsparseInitialized)
    {
        return clsparseNotInitialized;
    }

    if (control == nullptr)
    {
        return clsparseInvalidControlObject;
    }

    if (solverControl == nullptr)
    {
        return clsparseInvalidSolverControlObject;
    }

    cldenseVectorPrivate* pX = static_cast<cldenseVectorPrivate*> ( x );
    const clsparseCsrMatrixPrivate* pA = static_cast<const clsparseCsrMatrixPrivate*> ( A );
    const cldenseVectorPrivate* pB = static_cast<const cldenseVectorPrivate*> ( b );


    std::shared_ptr<PreconditionerHandler<T>> preconditioner;

    if (solverControl->preconditioner == DIAGONAL)
    {
        preconditioner = std::shared_ptr<PreconditionerHandler<T>>(new DiagonalHandler<T>());
        // call constructor of preconditioner class
        preconditioner->notify(pA, control);
    }
    else
    {
        preconditioner = std::shared_ptr<PreconditionerHandler<T>>(new VoidHandler<T>());
        preconditioner->notify(pA, control);
    }

    clsparseStatus status = bicgStab<T>(pX, pA, pB, *preconditioner, solverControl, control);

    solverControl->printSummary(status);

    return status;
}
