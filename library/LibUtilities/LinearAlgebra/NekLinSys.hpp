///////////////////////////////////////////////////////////////////////////////
//
// File: NekLinSys.hpp
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
// Description: 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_NEK_LINSYS_HPP
#define NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_NEK_LINSYS_HPP

#include <LibUtilities/BasicConst/NektarUnivTypeDefs.hpp>
#include <LibUtilities/LinearAlgebra/Lapack.hpp>
#include <LibUtilities/LinearAlgebra/NekMatrix.hpp>
#include <LibUtilities/LinearAlgebra/NekVector.hpp>
#include <LibUtilities/Memory/DeleteNothing.hpp>
#include <LibUtilities/LinearAlgebra/MatrixType.h>
#include <LibUtilities/BasicUtils/ConsistentObjectAccess.hpp>
#include <LibUtilities/BasicUtils/RawType.hpp>
#include <LibUtilities/LinearAlgebra/NekLinSys.hpp>
#include <iostream>

#include <boost/shared_ptr.hpp> 
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Nektar
{
    template<typename DataType>
    struct IsSharedPointer : public boost::false_type {};

    template<typename DataType>
    struct IsSharedPointer<boost::shared_ptr<DataType> > : public boost::true_type {};

    // The solving of the linear system is located in this class instead of in the LinearSystem 
    // class because XCode gcc 4.2 didn't compile it correctly when it was moved to the 
    // LinearSystem class.
    struct LinearSystemSolver
    {
        template<typename BVectorType, typename XVectorType>
        static void Solve(const BVectorType& b, XVectorType& x, MatrixStorage m_matrixType,
                   const Array<OneD, const int>& m_ipivot, unsigned int n,
                   const Array<OneD, const double>& A,
                   char m_transposeFlag, unsigned int m_numberOfSubDiagonals,
                   unsigned int m_numberOfSuperDiagonals)
        {
            switch(m_matrixType)
            {
                case eFULL:
                    {
                        x = b;
                        int info = 0;
                        Lapack::Dgetrs('N',n,1,A.get(),n,(int *)m_ipivot.get(),x.GetRawPtr(),n,info);
                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dgetrs";
                            ASSERTL0(false, message.c_str());
                        }

                    }
                    break;
                case eDIAGONAL:
                    for(unsigned int i = 0; i < A.num_elements(); ++i)
                    {
                        x[i] = b[i]*A[i];
                    }
                    break;
                case eUPPER_TRIANGULAR:
                    {
                        x = b;
                        int info = 0;
                        Lapack::Dtptrs('U', m_transposeFlag, 'N', n, 1, A.get(), x.GetRawPtr(), n, info);

                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dtrtrs";
                            ASSERTL0(false, message.c_str());
                        }
                        else if( info > 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th diagonal element of A is 0 for dtrtrs";
                            ASSERTL0(false, message.c_str());
                        }
                    }
                    break;
                case eLOWER_TRIANGULAR:
                    {
                        x = b;
                        int info = 0;
                        Lapack::Dtptrs('L', m_transposeFlag, 'N', n, 1, A.get(), x.GetRawPtr(), n, info);

                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dtrtrs";
                            ASSERTL0(false, message.c_str());
                        }
                        else if( info > 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th diagonal element of A is 0 for dtrtrs";
                            ASSERTL0(false, message.c_str());
                        }
                    }
                    break;
                case eSYMMETRIC:
                    {
                        x = b;
                        int info = 0;
                        Lapack::Dsptrs('U', n, 1, A.get(), m_ipivot.get(), x.GetRawPtr(), x.GetRows(), info);
                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dsptrs";
                            ASSERTL0(false, message.c_str());
                        }
                    }
                    break;
                case eBANDED:
                    {
                        x = b;
                        int KL = m_numberOfSubDiagonals;
                        int KU = m_numberOfSuperDiagonals;
                        int info = 0;
                        
                        Lapack::Dgbtrs(m_transposeFlag, n, KL, KU, 1, A.get(), 2*KL+KU+1, m_ipivot.get(), x.GetRawPtr(), n, info);
                        
                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dgbtrs";
                            ASSERTL0(false, message.c_str());
                        }
                    }
                    break;
                case eSYMMETRIC_BANDED:
                    NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                    break;
                case eUPPER_TRIANGULAR_BANDED:
                    NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                    break;
                case eLOWER_TRIANGULAR_BANDED:
                    NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                    break;
                    
                default:
                    NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
            }
            
        }

        template<typename BVectorType, typename XVectorType>
        static void SolveTranspose(const BVectorType& b, XVectorType& x, MatrixStorage m_matrixType,
                            const Array<OneD, const int>& m_ipivot, unsigned int n,
                            const Array<OneD, const double>& A,
                            char m_transposeFlag, unsigned int m_numberOfSubDiagonals,
                   unsigned int m_numberOfSuperDiagonals)
        {
            switch(m_matrixType)
            {
                case eFULL:
                    {
                        x = b;
                        int info = 0;
                        Lapack::Dgetrs('T',n,1,A.get(),n,(int *)m_ipivot.get(),x.GetRawPtr(), n,info);

                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dgetrs";
                            ASSERTL0(false, message.c_str());
                        }
                    }

                    break;
                case eDIAGONAL:
                    Solve(b, x, m_matrixType, m_ipivot, n, A, m_transposeFlag, m_numberOfSubDiagonals, m_numberOfSuperDiagonals);
                    break;
                case eUPPER_TRIANGULAR:
                    {
                        char trans = m_transposeFlag;
                        if( trans == 'N' )
                        {
                            trans = 'T';
                        }
                        else
                        {
                            trans = 'N';
                        }

                        x = b;
                        int info = 0;
                        Lapack::Dtptrs('U', trans, 'N', n, 1, A.get(), x.GetRawPtr(), n, info);

                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dtrtrs";
                            ASSERTL0(false, message.c_str());
                        }
                        else if( info > 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th diagonal element of A is 0 for dtrtrs";
                            ASSERTL0(false, message.c_str());
                        }
                    }

                    break;
                case eLOWER_TRIANGULAR:
                    {
                        char trans = m_transposeFlag;
                        if( trans == 'N' )
                        {
                            trans = 'T';
                        }
                        else
                        {
                            trans = 'N';
                        }
                        x = b;
                        int info = 0;
                        Lapack::Dtptrs('L', trans, 'N', n, 1, A.get(), x.GetRawPtr(), n, info);

                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dtrtrs";
                            ASSERTL0(false, message.c_str());
                        }
                        else if( info > 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th diagonal element of A is 0 for dtrtrs";
                            ASSERTL0(false, message.c_str());
                        }
                    }
                    break;
                case eSYMMETRIC:
                    Solve(b, x, m_matrixType, m_ipivot, n, A, m_transposeFlag, m_numberOfSubDiagonals, m_numberOfSuperDiagonals);
                    break;
                case eBANDED:
                    {
                        x = b;
                        int KL = m_numberOfSubDiagonals;
                        int KU = m_numberOfSuperDiagonals;
                        int info = 0;
                        
                        Lapack::Dgbtrs(m_transposeFlag, n, KL, KU, 1, A.get(), 2*KL+KU+1, m_ipivot.get(), x.GetRawPtr(), n, info);
                        
                        if( info < 0 )
                        {
                            std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dgbtrs";
                            ASSERTL0(false, message.c_str());
                        }
                    }
                    break;
                case eSYMMETRIC_BANDED:
                    NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                    break;
                case eUPPER_TRIANGULAR_BANDED:
                    NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                    break;
                case eLOWER_TRIANGULAR_BANDED:
                    NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                    break;
                    
                default:
                    NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
            }
        }
    };
    
    class LinearSystem
    {
        public:
            template<typename MatrixType>
            explicit LinearSystem(const boost::shared_ptr<MatrixType> &theA, PointerWrapper wrapperType = eCopy) :
                n(theA->GetRows()),
                A(theA->GetPtr(), eVECTOR_WRAPPER),
                m_ipivot(),
                m_numberOfSubDiagonals(theA->GetNumberOfSubDiagonals()),
                m_numberOfSuperDiagonals(theA->GetNumberOfSuperDiagonals()),
                m_matrixType(theA->GetType()),
                m_transposeFlag(theA->GetTransposeFlag())
            {
                // At some point we should fix this.  We should upate the copy of 
                // A to be transposd for this to work.
                ASSERTL0(theA->GetTransposeFlag() == 'N', "LinearSystem requires a non-transposed matrix.");
                ASSERTL0( (wrapperType == eWrapper && theA->GetType() != eBANDED) || wrapperType == eCopy , "Banded matrices can't be wrapped");
                
                if( wrapperType == eCopy )
                {
                    A = Array<OneD, double>(theA->GetPtr().num_elements());
                    CopyArray(theA->GetPtr(), A);
                }
                
                FactorMatrix(*theA);
            }
            
            template<typename MatrixType>
            explicit LinearSystem(const MatrixType& theA, PointerWrapper wrapperType = eCopy) :
                n(theA.GetRows()),
                A(theA.GetPtr(), eVECTOR_WRAPPER),
                m_ipivot(),
                m_numberOfSubDiagonals(theA.GetNumberOfSubDiagonals()),
                m_numberOfSuperDiagonals(theA.GetNumberOfSuperDiagonals()),
                m_matrixType(theA.GetType()),
                m_transposeFlag(theA.GetTransposeFlag())
            {
                // At some point we should fix this.  We should upate the copy of 
                // A to be transposd for this to work.
                ASSERTL0(theA.GetTransposeFlag() == 'N', "LinearSystem requires a non-transposed matrix.");
                ASSERTL0( (wrapperType == eWrapper && theA.GetType() != eBANDED) || wrapperType == eCopy, "Banded matrices can't be wrapped" );
                
                if( wrapperType == eCopy )
                {
                    A = Array<OneD, double>(theA.GetPtr().num_elements());
                    CopyArray(theA.GetPtr(), A);
                }

                FactorMatrix(theA);
            }

            LinearSystem(const LinearSystem& rhs) :
                n(rhs.n),
                A(rhs.A),
                m_ipivot(rhs.m_ipivot),
                m_numberOfSubDiagonals(rhs.m_numberOfSubDiagonals),
                m_numberOfSuperDiagonals(rhs.m_numberOfSuperDiagonals),
                m_matrixType(rhs.m_matrixType),
                m_transposeFlag(rhs.m_transposeFlag)
            {
            }

            LinearSystem& operator=(const LinearSystem& rhs)
            {
                LinearSystem temp(rhs);
                swap(temp);
                return *this;
            }

            ~LinearSystem() {}

            // In the following calls to Solve, VectorType must be a NekVector.
            // Anything else won't compile.        
            template<typename VectorType>
            typename RemoveVectorConst<typename RawType<VectorType>::type>::type Solve(const VectorType& b)
            {
                typename RemoveVectorConst<typename RawType<VectorType>::type>::type x(ConsistentObjectAccess<VectorType>::const_reference(b).GetRows());
                LinearSystemSolver::Solve(ConsistentObjectAccess<VectorType>::const_reference(b), x, m_matrixType,
                    m_ipivot, n, A, m_transposeFlag, m_numberOfSubDiagonals, m_numberOfSuperDiagonals);
                return x;
            }    

            template<typename BType, typename XType>
            void Solve(const BType& b, XType& x) const
            {
                LinearSystemSolver::Solve(ConsistentObjectAccess<BType>::const_reference(b), 
                      ConsistentObjectAccess<XType>::reference(x), m_matrixType,
                      m_ipivot, n, A, m_transposeFlag, m_numberOfSubDiagonals, m_numberOfSuperDiagonals);
            }

            // Transpose variant of solve
            template<typename VectorType>
            typename RemoveVectorConst<typename RawType<VectorType>::type>::type SolveTranspose(const VectorType& b)
            {
                typename RemoveVectorConst<typename RawType<VectorType>::type>::type x(ConsistentObjectAccess<VectorType>::const_reference(b).GetRows());
                LinearSystemSolver::SolveTranspose(ConsistentObjectAccess<VectorType>::const_reference(b), x, m_matrixType,
                    m_ipivot, n, A, m_transposeFlag, m_numberOfSubDiagonals, m_numberOfSuperDiagonals);
                return x;
            }    

            template<typename BType, typename XType>
            void SolveTranspose(const BType& b, XType& x) const
            {
                LinearSystemSolver::SolveTranspose(ConsistentObjectAccess<BType>::const_reference(b), 
                               ConsistentObjectAccess<XType>::reference(x), m_matrixType,
                               m_ipivot, n, A, m_transposeFlag, m_numberOfSubDiagonals, m_numberOfSuperDiagonals);
            }
            
                

            unsigned int GetRows() const { return n; }
            unsigned int GetColumns() const { return n; }
            
        private:
            template<typename MatrixType>
            void FactorMatrix(const MatrixType& theA)
            {
                switch(m_matrixType)
                {
                    case eFULL:
                        {
                            int m = theA.GetRows();
                            int n = theA.GetColumns();
                            
                            int pivotSize = std::max(1, std::min(m, n));
                            int info = 0;
                            m_ipivot = Array<OneD, int>(pivotSize);

                            Lapack::Dgetrf(m, n, A.get(), m, m_ipivot.get(), info);

                            if( info < 0 )
                            {
                                std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dgetrf";
                                ASSERTL0(false, message.c_str());
                            }
                            else if( info > 0 )
                            {
                                std::string message = "ERROR: Element u_" + boost::lexical_cast<std::string>(info) +   boost::lexical_cast<std::string>(info) + " is 0 from dgetrf";
                                ASSERTL0(false, message.c_str());
                            } 
                        }
                        break;
                    case eDIAGONAL:
                        for(unsigned int i = 0; i < theA.GetColumns(); ++i)
                        {
                            A[i] = 1.0/theA(i,i);
                        }
                        break;
                    case eUPPER_TRIANGULAR:
                    case eLOWER_TRIANGULAR:
                        break;
                    case eSYMMETRIC:
                        {
                            int info = 0;
                            int pivotSize = theA.GetRows();
                            m_ipivot = Array<OneD, int>(pivotSize);
                            
                            Lapack::Dsptrf('U', theA.GetRows(), A.get(), m_ipivot.get(), info);
                            
                            if( info < 0 )
                            {
                                std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dsptrf";
                                ASSERTL0(false, message.c_str());
                            }
                            else if( info > 0 )
                            {
                                std::string message = "ERROR: Element u_" + boost::lexical_cast<std::string>(info) +   boost::lexical_cast<std::string>(info) + " is 0 from dsptrf";
                                ASSERTL0(false, message.c_str());
                            }
                        }
                        break;
                    case eBANDED:
                        {
                            int M = n;
                            int N = n;
                            int KL = m_numberOfSubDiagonals;
                            int KU = m_numberOfSuperDiagonals;
                            
                            // The array we pass in to dgbtrf must have enough space for KL
                            // subdiagonals and KL+KU superdiagonals (see lapack users guide,
                            // in the section discussing band storage.
                            unsigned int requiredStorageSize = MatrixStoragePolicy<BandedMatrixTag>::
                                GetRequiredStorageSize(n, n, KL, KL+KU);
                            
                            unsigned int rawRows = KL+KU+1;
                            A = Array<OneD, double>(requiredStorageSize);

                            // Put the extra elements up front.
                            for(unsigned int i = 0; i < theA.GetColumns(); ++i)
                            {
                                std::copy(theA.GetRawPtr() + i*rawRows, theA.GetRawPtr() + (i+1)*rawRows,
                                    A.get() + (i+1)*KL + i*rawRows);
                            }
                                   
                            int info = 0;
                            int pivotSize = theA.GetRows();
                            m_ipivot = Array<OneD, int>(pivotSize);
                            
                            Lapack::Dgbtrf(M, N, KL, KU, A.get(), 2*KL+KU+1, m_ipivot.get(), info);
                          
                            if( info < 0 )
                            {
                                std::string message = "ERROR: The " + boost::lexical_cast<std::string>(-info) + "th parameter had an illegal parameter for dgbtrf";
                                ASSERTL0(false, message.c_str());
                            }
                            else if( info > 0 )
                            {
                                std::string message = "ERROR: Element u_" + boost::lexical_cast<std::string>(info) +   boost::lexical_cast<std::string>(info) + " is 0 from dgbtrf";
                                ASSERTL0(false, message.c_str());
                            }
                        }
                        break;
                    case eSYMMETRIC_BANDED:
                        NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                        break;
                    case eUPPER_TRIANGULAR_BANDED:
                        NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                        break;
                    case eLOWER_TRIANGULAR_BANDED:
                        NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                        break;
                        
                    default:
                        NEKERROR(ErrorUtil::efatal, "Unhandled matrix type");
                }
            }
        
            void swap(LinearSystem& rhs)
            {
                std::swap(n, rhs.n);
                std::swap(A, rhs.A);
                std::swap(m_ipivot,rhs.m_ipivot);
                std::swap(m_numberOfSubDiagonals, rhs.m_numberOfSubDiagonals);
                std::swap(m_numberOfSuperDiagonals, rhs.m_numberOfSuperDiagonals);
                std::swap(m_matrixType, rhs.m_matrixType);
                std::swap(m_transposeFlag, rhs.m_transposeFlag);
            }

            unsigned int n;
            Array<OneD, double> A;
            Array<OneD, int> m_ipivot;  
            unsigned int m_numberOfSubDiagonals;
            unsigned int m_numberOfSuperDiagonals;
            MatrixStorage m_matrixType;
            char m_transposeFlag;
        };
}

#endif //NEKTAR_LIB_UTILITIES_LINEAR_ALGEBRA_NEK_LINSYS_HPP
