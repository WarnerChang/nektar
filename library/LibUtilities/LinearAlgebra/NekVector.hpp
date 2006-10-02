///////////////////////////////////////////////////////////////////////////////
//
// File: NekVector.hpp
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
// Description: Generic N-Dimensional Vector.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NEKTAR_LIB_UTILITIES_NEK_VECTOR_HPP
#define NEKTAR_LIB_UTILITIES_NEK_VECTOR_HPP

#include <LibUtilities/BasicUtils/ErrorUtil.hpp>
#include <LibUtilities/LinearAlgebra/NekPoint.hpp>
#include <LibUtilities/ExpressionTemplates/ExpressionTemplates.hpp>
#include <LibUtilities/LinearAlgebra/NekVectorMetadata.hpp>

#include <functional>
#include <algorithm>
#include <math.h>

#include <boost/call_traits.hpp>
#include <boost/shared_array.hpp>
#include <boost/utility/enable_if.hpp>

namespace Nektar
{
    // \param DataType The type of data held by each element of the vector.
    // \param dim The number of elements in the vector.  If set to 0, the vector
    //            will have a variable number of elements.
    // \param space The space of the vector.
    template<typename DataType, unsigned int dim = 0, unsigned int space = 0>
    class NekVector
    {
        public:
            NekVector() :
                m_impl()
            {
            }

            // \brief Creates a vector with given size and initial value.
            //        This constructor is only valid for variable sized vectors.
            NekVector(unsigned int size, typename boost::call_traits<DataType>::param_type a) :
                m_impl(size, a)
            {
            }

            explicit NekVector(const std::string& vectorValues) :
                m_impl()
            {
                BOOST_STATIC_ASSERT(dim > 0 );
                bool result = fromString(vectorValues, *this);
                ASSERTL1(result, "Error converting string values to vector");
            }

            NekVector(typename boost::call_traits<DataType>::param_type x,
                        typename boost::call_traits<DataType>::param_type y,
                        typename boost::call_traits<DataType>::param_type z) :
                m_impl(x,y,z)
            {
            }

            explicit NekVector(typename boost::call_traits<DataType>::param_type a) :
                m_impl(a)
            {
            }

            NekVector(const NekVector<DataType, 0, space>& rhs) :
                m_impl(rhs.m_impl)
            {
            }

            NekVector(unsigned int size, const DataType* const ptr) :
                m_impl(size, ptr)
            {
            }

            explicit NekVector(const DataType* const ptr) :
                m_impl(ptr)
            {
            }

            template<typename ExpressionPolicyType>
            NekVector(const expt::Expression<ExpressionPolicyType>& rhs) :
                m_impl(rhs.GetMetadata().Rows, DataType())
            {
                BOOST_MPL_ASSERT(( boost::is_same<typename expt::Expression<ExpressionPolicyType>::ResultType, NekVector<DataType, dim, space> > ));
                rhs.Apply(*this);
            }

            ~NekVector()
            {
            }

            template<typename ExpressionPolicyType>
            NekVector<DataType, dim, space>& operator=(const expt::Expression<ExpressionPolicyType>& rhs)
            {
                BOOST_MPL_ASSERT(( boost::is_same<typename expt::Expression<ExpressionPolicyType>::ResultType, NekVector<DataType, dim, space> > ));
                rhs.Apply(*this);
                return *this;
            }

            NekVector<DataType, dim, space>& operator=(const NekVector<DataType, dim, space>& rhs)
            {
                m_impl = rhs.m_impl;

                return *this;
            }

            /// \brief Returns the number of dimensions for the point.
            unsigned int GetDimension() const
            {
                return m_impl.dimension();
            }

            unsigned int GetRows() const
            {
                return m_impl.dimension();
            }

            /// \brief Returns i^{th} element.
            /// \param i The element to return.
            /// \pre i < dim
            /// \return A reference to the i^{th} element.
            ///
            /// Retrieves the i^{th} element.  Since it returns a reference you may
            /// assign a new value (i.e., p(2) = 3.2;)
            ///
            /// This operator performs range checking.
            typename boost::call_traits<DataType>::reference operator()(unsigned int i)
            {
                ASSERTL1((i >= 0) && (i < dimension()), "Invalid access to m_data via parenthesis operator");
                return m_impl.data[i];
            }

            typename boost::call_traits<DataType>::const_reference operator()(unsigned int i) const
            {
                ASSERTL1(( i >= 0) && (i < dimension()), "Invalid access to m_data via parenthesis operator");
                return m_impl.data[i];
            }

            typename boost::call_traits<DataType>::reference operator[](unsigned int i)
            {
                return m_impl.data[i];
            }

            typename boost::call_traits<DataType>::const_reference operator[](unsigned int i) const
            {
                return m_impl.data[i];
            }

            typename boost::call_traits<DataType>::const_reference x() const
            {
                ASSERTL1( dimension() >= 1, "Invalid use of NekVector::x");
                return (*this)(0);
            }

            typename boost::call_traits<DataType>::const_reference y() const
            {
                ASSERTL1( dimension() >= 2, "Invalid use of NekVector::y");
                return (*this)(1);
            }

            typename boost::call_traits<DataType>::const_reference z() const
            {
                ASSERTL1( dimension() >= 3, "Invalid use of NekVector::z");
                return (*this)(2);
            }

            typename boost::call_traits<DataType>::reference x()
            {
                ASSERTL1(dimension() >= 1, "Invalid use of NekVector::x");
                return (*this)(0);
            }

            typename boost::call_traits<DataType>::reference y()
            {
                ASSERTL1(dimension() >= 2, "Invalid use of NekVector::y");
                return (*this)(1);
            }

            typename boost::call_traits<DataType>::reference z()
            {
                ASSERTL1(dimension() >= 3, "Invalid use of NekVector::z");
                return (*this)(2);
            }

            void SetX(typename boost::call_traits<DataType>::const_reference val)
            {
                ASSERTL1(dimension() >= 1, "Invalid use of NekVector::SetX");
                m_impl.data[0] = val;
            }

            void SetY(typename boost::call_traits<DataType>::const_reference val)
            {
                ASSERTL1(dimension() >= 2, "Invalid use of NekVector::SetX");
                m_impl.data[1] = val;
            }

            void SetZ(typename boost::call_traits<DataType>::const_reference val)
            {
                ASSERTL1(dimension() >= 3, "Invalid use of NekVector::SetX");
                m_impl.data[2] = val;
            }

            bool operator==(const NekVector<DataType, dim, space>& rhs) const
            {
                if( GetDimension() != rhs.GetDimension() )
                {
                    return false;
                }

                for(unsigned int i = 0; i < GetDimension(); ++i)
                {
                    // If you get a compile error here then you have to
                    // add a != operator to the DataType class.
                    if( m_impl.data[i] != rhs[i] )
                    {
                        return false;
                    }
                }
                return true;
            }

            bool operator!=(const NekVector<DataType, dim, space>& rhs) const
            {
                return !(*this == rhs);
            }

            /// Arithmetic Routines

            // Unitary operators
            NekVector<DataType, dim, space> operator-() const
            {
                NekVector<DataType, dim, space> temp(*this);
                for(unsigned int i=0; i < GetDimension(); ++i)
                {
                    temp(i) = -temp(i);
                }
                return temp;
            }


            NekVector<DataType, dim, space>& operator+=(const NekVector<DataType, dim, space>& rhs)
            {
                ASSERTL1( GetDimension() == rhs.GetDimension(), "NekVector::operator+=, dimension of the two operands must be identical.");
                for(unsigned int i=0; i < GetDimension(); ++i)
                {
                    (*this)[i] += rhs[i];
                }
                return *this;
            }

            NekVector<DataType, dim, space>& operator-=(const NekVector<DataType, dim, space>& rhs)
            {
                ASSERTL1( GetDimension() == rhs.GetDimension(), "NekVector::operator-=, dimension of the two operands must be identical.");
                for(unsigned int i=0; i < GetDimension(); ++i)
                {
                    (*this)[i] -= rhs[i];
                }
                return *this;
            }

            NekVector<DataType, dim, space>& operator*=(typename boost::call_traits<DataType>::param_type rhs)
            {
                for(unsigned int i=0; i < GetDimension(); ++i)
                {
                    (*this)[i] *= rhs;
                }
                return *this;
            }

            DataType magnitude() const
            {
                DataType result = DataType(0);

                for(unsigned int i = 0; i < GetDimension(); ++i)
                {
                    result += (*this)[i]*(*this)[i];
                }
                return sqrt(result);
            }

            DataType dot(const NekVector<DataType, dim, space>& rhs) const
            {
                ASSERTL1( GetDimension() == rhs.GetDimension(), "NekVector::dot, dimension of the two operands must be identical.");

                DataType result = DataType(0);
                for(unsigned int i = 0; i < GetDimension(); ++i)
                {
                    result += (*this)[i]*rhs[i];
                }

                return result;
            }

            void normalize()
            {
                DataType m = magnitude();
                if( m > DataType(0) )
                {
                    for(unsigned int i = 0; i < GetDimension(); ++i)
                    {
                        (*this)[i] /= m;
                    }
                }
            }

            NekVector<DataType, dim, space> cross(typename boost::call_traits<NekVector<DataType, dim, space> >::param_type rhs) const
            {
                BOOST_STATIC_ASSERT(dim==3 || dim == 0);
                ASSERTL1(dimension() == 3 && rhs.dimension() == 3, "NekVector::cross is only valid for 3D vectors.");

                DataType first = y()*rhs.z() - z()*rhs.y();
                DataType second = z()*rhs.x() - x()*rhs.z();
                DataType third = x()*rhs.y() - y()*rhs.x();

                NekVector<DataType, dim, space> result;
                result[0] = first;
                result[1] = second;
                result[2] = third;
                return result;
            }

            std::string AsString() const
            {
                unsigned int d = GetRows();
                std::string result = "(";
                for(unsigned int i = 0; i < d; ++i)
                {
                    result += boost::lexical_cast<std::string>((*this)[i]);
                    if( i < dim-1 )
                    {
                        result += ", ";
                    }
                }
                result += ")";
                return result;
            }

        private:
            template<typename ImplDataType, unsigned int ImplSize, unsigned int ImplSpace>
            class VectorImpl
            {
                public:
                    VectorImpl() :
                        data()
                    {
                        for(unsigned int i = 0; i < ImplSize; ++i)
                        {
                            data[i] = ImplDataType(0);
                        }
                    }

                    explicit VectorImpl(typename boost::call_traits<ImplDataType>::param_type a) :
                        data()
                    {
                        for(unsigned int i = 0; i < ImplSize; ++i)
                        {
                            data[i] = a;
                        }
                    }

                    VectorImpl(typename boost::call_traits<DataType>::param_type x,
                            typename boost::call_traits<DataType>::param_type y,
                            typename boost::call_traits<DataType>::param_type z)
                    {
                        BOOST_STATIC_ASSERT(ImplSize == 3);
                        data[0] = x;
                        data[1] = y;
                        data[2] = z;
                    }

                    VectorImpl(unsigned int size, typename boost::call_traits<ImplDataType>::param_type a) :
                        data()
                    {
                        unsigned int end = std::min(size, ImplSize);
                        for(unsigned int i = 0; i < end; ++i)
                        {
                            data[i] = a;
                        }

                        for(unsigned int i = end; i < ImplSize; ++i)
                        {
                            data[i] = ImplDataType(0);
                        }
                    }

                    VectorImpl(const DataType* const ptr) :
                        data()
                    {
                        for(unsigned int i = 0; i < ImplSize; ++i)
                        {
                            data[i] = ptr[i];
                        }
                    }

                    VectorImpl(const VectorImpl<ImplDataType, ImplSize, ImplSpace>& rhs) :
                        data()
                    {
                        for(unsigned int i = 0; i < ImplSize; ++i)
                        {
                            data[i] = rhs.data[i];
                        }
                    }

                    VectorImpl<ImplDataType, ImplSize, ImplSpace>& operator=(const VectorImpl<ImplDataType, ImplSize, ImplSpace>& rhs)
                    {
                        for(unsigned int i = 0; i < ImplSize; ++i)
                        {
                            data[i] = rhs.data[i];
                        }
                        return *this;
                    }

                    ~VectorImpl() {}

                    unsigned int dimension() const { return ImplSize; }
                    ImplDataType data[ImplSize];
            };

            // \brief Specialization for variable sized vectors.
            template<typename ImplDataType, unsigned int ImplSpace>
            class VectorImpl<ImplDataType, 0, ImplSpace>
            {
                public:
                    VectorImpl() :
                        data(new ImplDataType[1]),
                        size(1)
                    {
                        data[0] = ImplDataType(0);
                    }

                    VectorImpl(unsigned int s, typename boost::call_traits<ImplDataType>::param_type a) :
                        data(new DataType[s]),
                        size(s)
                    {
                        for(unsigned int i = 0; i < size; ++i)
                        {
                            data[i] = a;
                        }
                    }

                    explicit VectorImpl(typename boost::call_traits<ImplDataType>::param_type a) :
                        data(new DataType[1]),
                        size(1)
                    {
                        data[0] = a;
                    }

                    VectorImpl(unsigned int s, const DataType* const ptr) :
                        data(new DataType[s]),
                        size(s)
                    {
                        for(unsigned int i = 0; i < size; ++i)
                        {
                            data[i] = ptr[i];
                        }
                    }


                    VectorImpl(const VectorImpl<ImplDataType, 0, ImplSpace>& rhs) :
                            data(new ImplDataType[rhs.size]),
                        size(rhs.size)
                    {
                        for(unsigned int i = 0; i < size; ++i)
                        {
                            data[i] = rhs.data[i];
                        }
                    }

                    ~VectorImpl() {}

                    VectorImpl<ImplDataType, 0, ImplSpace>& operator=(const VectorImpl<ImplDataType, 0, ImplSpace>& rhs)
                    {
                        boost::shared_array<ImplDataType> temp = boost::shared_array<DataType>(new ImplDataType[rhs.size]);
                        for(unsigned int i = 0; i < rhs.size; ++i)
                        {
                            temp[i] = rhs.data[i];
                        }

                        data = temp;
                        size = rhs.size;
                        return *this;
                    }

                    unsigned int dimension() const { return size; }

                    boost::shared_array<ImplDataType> data;
                    unsigned int size;
            };

            VectorImpl<DataType, dim, space> m_impl;
    };    

    namespace expt
    {
        template<typename DataType, unsigned int dim, unsigned int space>
        class ExpressionTraits<NekVector<DataType, dim, space> >
        {
            public:
                typedef NekVectorMetadata MetadataType;
        };
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    NekVector<DataType, dim, space>
    operator+(const NekVector<DataType, dim, space>& lhs, const NekVector<DataType, dim>& rhs)
    {
        NekVector<DataType, dim, space> result(lhs);
        result += rhs;
        return result;
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    NekVector<DataType, dim, space>
    operator-(const NekVector<DataType, dim, space>& lhs, const NekVector<DataType, dim>& rhs)
    {
        NekVector<DataType, dim, space> result(lhs);
        result -= rhs;
        return result;
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    NekVector<DataType, dim, space>
    operator*(const NekVector<DataType, dim, space>& lhs, typename boost::call_traits<DataType>::param_type rhs)
    {
        NekVector<DataType, dim, space> result(lhs);
        result *= rhs;
        return result;
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    NekVector<DataType, dim, space>
    operator*(typename boost::call_traits<DataType>::param_type lhs, const NekVector<DataType, dim, space>& rhs)
    {
        NekVector<DataType, dim, space> result(rhs);
        result *= lhs;
        return result;
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    NekVector<DataType, dim, space>
    normalize(const NekVector<DataType, dim, space>& rhs)
    {
        NekVector<DataType, dim, space> result(rhs);
        result.normalize();
        return result;
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    DataType dot(const NekVector<DataType, dim, space>& lhs, const NekVector<DataType, dim>& rhs)
    {
        return lhs.dot(rhs);
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    NekVector<DataType, dim, space> cross(const NekVector<DataType, dim, space>& lhs,
                                            const NekVector<DataType, dim, space>& rhs)
    {
        return lhs.cross(rhs);
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    std::ostream& operator<<(std::ostream& os, const NekVector<DataType, dim, space>& rhs)
    {
        os << rhs.AsString();
        return os;
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    NekVector<DataType, dim, space> createVectorFromPoints(const NekPoint<DataType, dim, space>& source,
                                                        const NekPoint<DataType, dim, space>& dest)
    {
        NekVector<DataType, dim, space> result;
        for(unsigned int i = 0; i < dim; ++i)
        {
            result[i] = dest[i]-source[i];
        }
        return result;
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    NekPoint<DataType, dim, space> findPointAlongVector(const NekVector<DataType, dim, space>& lhs,
        typename boost::call_traits<DataType>::param_type t)
    {
        NekPoint<DataType, dim, space> result;
        for(unsigned int i = 0; i < dim; ++i)
        {
            result[i] = lhs[i]*t;
        }

        return result;
    }

    template<typename DataType, unsigned int dim, unsigned int space>
    bool fromString(const std::string& str, NekVector<DataType, dim, space>& result)
    {
        try
        {
            typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
            boost::char_separator<char> sep("(<,>) ");
            tokenizer tokens(str, sep);
            unsigned int i = 0;
            for(tokenizer::iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
            {
                result[i] = boost::lexical_cast<DataType>(*iter);
                ++i;
            }

            return i == dim;
        }
        catch(boost::bad_lexical_cast&)
        {
            return false;
        }
    }
}

#endif // NEKTAR_LIB_UTILITIES_NEK_VECTOR_HPP

/**
    $Log: NekVector.hpp,v $
    Revision 1.7  2006/09/30 15:18:37  bnelson
    no message

    Revision 1.6  2006/09/21 01:00:38  bnelson
    Fixed an expression template problem.

    Revision 1.5  2006/09/14 02:06:16  bnelson
    Fixed gcc compiler errors.

    Revision 1.4  2006/08/25 01:22:01  bnelson
    no message

    Revision 1.3  2006/08/14 02:29:49  bnelson
    Updated points, vectors, and matrix classes to work with ElVis.  Added a variety of methods to all of these classes.

    Revision 1.2  2006/06/01 13:44:29  kirby
    *** empty log message ***

    Revision 1.1  2006/06/01 09:12:42  kirby
    *** empty log message ***

    Revision 1.2  2006/05/25 03:02:40  bnelson
    Added Matrix/Vector multiplication.

    Revision 1.1  2006/05/04 18:57:44  kirby
    *** empty log message ***

    Revision 1.1  2006/04/11 02:00:43  bnelson
    Initial Revision


**/

