///////////////////////////////////////////////////////////////////////////////
//
// File MatrixKey.cpp
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
// Description: Definition of MatrixKey based on StdMatrixKey
//
///////////////////////////////////////////////////////////////////////////////


#include <LocalRegions/MatrixKey.h>

namespace Nektar
{
    namespace LocalRegions
    {
        // Register Mass Matrix creator. 
        MatrixKey::MatrixKey(StdRegions::MatrixType matrixType,
                             StdRegions::ShapeType shapeType,
                             StdRegions::StdExpansion &stdExpansion,
                             NekDouble    scalefactor,
                             LibUtilities::PointsType nodalType)
        {
            m_stdMatKey =  MemoryManager<StdRegions::StdMatrixKey>::AllocateSharedPtr(matrixType,shapeType,stdExpansion,nodalType);

            m_scalefactor = scalefactor;
            m_metricinfo  = stdExpansion.GetMetricInfo(); 
        }

        bool MatrixKey::opLess::operator()(const MatrixKey &lhs, const MatrixKey &rhs)
        {	    
            {
                return (lhs.GetMatrixType() < rhs.GetMatrixType());
            }
        }

        bool operator<(const MatrixKey &lhs, const MatrixKey &rhs)
        {
            if(lhs.GetMatrixType() < rhs.GetMatrixType())
            {
                return true;
            }

            if(lhs.GetMatrixType() > rhs.GetMatrixType())
            {
                return false;
            }

            if(lhs.GetNcoeffs() < rhs.GetNcoeffs())
            {
                return true;
            }

            if(lhs.GetNcoeffs() > rhs.GetNcoeffs())
            {
                return false;
            }

            if(lhs.m_metricinfo < rhs.m_metricinfo)
            {
                return true;
            }

            for(unsigned int i = 0; i < StdRegions::ShapeTypeDimMap[lhs.GetShapeType()]; ++i)
            {
                if(lhs.GetBasis(i) < rhs.GetBasis(i))
                {
                    return true;
                }
            }

            if(lhs.m_scalefactor > rhs.m_scalefactor)
            {
                return false;
            }

            if(lhs.m_scalefactor < rhs.m_scalefactor)
            {
                return true;
            }

            return false;
        }

        std::ostream& operator<<(std::ostream& os, const MatrixKey& rhs)
        {
            os << *rhs.GetStdMatKey();

            return os;
        }
    }
}

/**
* $Log: MatrixKey.cpp,v $
* Revision 1.8  2007/07/10 17:17:22  sherwin
* Introduced Scaled Matrices into the MatrixManager
*
* Revision 1.7  2007/05/27 16:10:28  bnelson
* Update to new Array type.
*
* Revision 1.6  2007/04/08 03:33:30  jfrazier
* Minor reformatting and fixing SharedArray usage.
*
* Revision 1.5  2007/04/04 21:49:23  sherwin
* Update for SharedArray
*
* Revision 1.4  2007/03/20 09:13:37  kirby
* new geomfactor routines; update for metricinfo; update style
*
* Revision 1.3  2007/03/09 20:41:50  jfrazier
* *** empty log message ***
*
* Revision 1.2  2007/03/05 08:06:07  sherwin
* Updated to use MemoryManager
*
* Revision 1.1  2007/03/04 20:42:14  sherwin
* Keys for matrix managers
*
***/

