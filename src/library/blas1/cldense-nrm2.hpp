#pragma once
#ifndef _CLSPARSE_NRM_2_HPP_
#define _CLSPARSE_NRM_2_HPP_

#include "reduce.hpp"

template<typename T>
clsparseStatus
Norm2(clsparseScalarPrivate* pS,
      const cldenseVectorPrivate* pX,
      const clsparseControl control)
{
    return reduce<T, RO_SQR, RO_SQRT>(pS, pX, control);
}

/*
 * clsparse::array
 */
template<typename T>
clsparseStatus
Norm2(clsparse::vector<T>& pS,
      const clsparse::vector<T>& pX,
      const clsparseControl control)
{
    return reduce<T, RO_SQR, RO_SQRT>(pS, pX, control);
}


#endif //_CLSPARSE_NRM_2_HPP_
