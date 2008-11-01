///////////////////////////////////////////////////////////////////////////////
//
// File: MatrixStoragePolicy.hpp
//
// For more information, please see: http://www.nektar.info
//
// The MIT License
//
// Copyright (c) 2006 Division of Applied Mathematics, Brown University (USA),
// Department of Aeronautics, Imperial College London (UK), and Scientific
// Computing and Imaging Institute, University of Utah (USA).
//
// License for the specific language governing rights and limitations under
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// Description: Interface classes for matrices
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_MATRIX_STORAGE_POLICY_HPP
#define NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_MATRIX_STORAGE_POLICY_HPP

#include <LibUtilities/LinearAlgebra/MatrixStorageType.h>
#include <LibUtilities/BasicUtils/SharedArray.hpp>
//#include <LibUtilities/LinearAlgebra/Lapack.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>

namespace Nektar
{

    class DefaultPolicySpecificDataHolder {};

    // Blas divides storage into the following 3 high level storage schemes:
    //
    // 1.  Full.  Each element in the matrix has an associated memory location allocated
    //     for it. 
    // 2.  Packed.  Allows storage of symmetric and triangular matrices more efficiently.
    // 3.  Band.  Allows storage of banded matrices more efficiently.

    // Nektar++ supports the following matrix types.
    // Full Storage - General 
    // Packed Storage - Symmetric, Upper Triangular, Lower Triangular
    // Band Storage - General, Symmetric, Upper Triangular, Lower Triangular.

    /// \brief The class which must be specialized for each unique storage type.
    ///
    /// Any specialization must include the following methods:
    /// static Array<OneD, DataType> Initialize();
    template<typename StorageType>
    class MatrixStoragePolicy;



}

#endif //NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_MATRIX_STORAGE_POLICY_HPP
