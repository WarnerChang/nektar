///////////////////////////////////////////////////////////////////////////////
//
// File Basis.h
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
// Description: Header file of Basis definition 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BASIS_H
#define BASIS_H

#include <math.h>
#include <LibUtilities/Foundations/Foundations.hpp>
#include <Libutilities/Foundations/Points.hpp>

namespace Nektar
{
    namespace LibUtilities 
    {

        class BasisKey
        {
        public:
            BasisKey()
            {
                NEKERROR(ErrorUtil::efatal,"Default Constructor BasisKey should never be called");
            }

            BasisKey(const BasisType btype, const int nummodes, const PointsKey pkey):
		m_basistype(btype), 
                m_nummodes(nummodes),
                m_pointskey(pkey)
	    {
            }

            BasisKey(const BasisKey &B): m_basistype(B.m_basistype),
                m_nummodes(B.m_nummodes),
                m_pointskey(B.m_pointskey)
            {
            }

            ~BasisKey()
            {
            }

            /** \brief return order of basis */
            inline int GetBasisNumModes() const
            {
                return m_nummodes;
            }

            /** \brief return points order at which  basis is defined */
            inline int GetNumPoints() const
            {
                return m_pointskey.GetNumPoints();
            }

            /** \brief return type of expansion basis */
            inline BasisType GetBasisType() const
            {
                return m_basistype;
            }

            /** \brief return type of quadrature */
            inline PointsType GetPointsType() const
            {
                return m_pointskey.GetPointsType();
            }    

            /** \brief Check to see if the quadrature of expansions x is the
	     *  same as the calling basis
	     */
            inline bool SamePoints(const BasisKey &x) const
            {
                if(x.m_pointskey == m_pointskey)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            /** \brief Check to see if basis expansions x is the same as the
	     *  calling basis
	     */
            inline bool SameExp(const BasisKey &x) const
            {
                if((x.m_nummodes == m_nummodes)&&(x.m_basistype == m_basistype))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            //Overloaded Operators
            friend bool operator  == (const BasisKey& x, const BasisKey& y);
            friend bool operator  == (const BasisKey* x, const BasisKey& y);
            friend bool operator  == (const BasisKey& x, const BasisKey *y);
            friend bool operator  != (const BasisKey& x, const BasisKey& y);
            friend bool operator  != (const BasisKey* x, const BasisKey& y);
            friend bool operator  != (const BasisKey& x, const BasisKey *y);

        protected:
            int        m_nummodes;  /**< Expansion Order */
            BasisType  m_basistype;   /**< Expansion Type */
	    /** Point Key identifying points at which basis is evaluated */
            PointsKey   m_pointskey;

        private:
        };

        ///////////////////////////////////////////////////////////////////////////
        class Basis
        {
        public:    

            Basis()
            {
                NEKERROR(ErrorUtil::efatal,"Default Constructor for Basis should not be called");
            }

            Basis(const BasisKey &bkey): m_bkey(bkey)
            {

            }


            //Copy Constructor
            Basis(const Basis &B)
            {
                m_basistype   = B.m_basistype;
                m_nummodes    = B.m_nummodes;
                m_pointstype  = B.m_pointstype;
                m_pointsorder = B.m_pointsorder;
                m_alpha       = B.m_alpha;
                m_beta        = B.m_beta;

                int size = BasisMem();

                // Allocate Memory
                m_bdata  = new double [size];
                m_dbdata = new double [size];

                for(int i=0;i<size;i++)
                {
                    m_bdata[i]  = B.m_bdata[i];
                    m_dbdata[i] = B.m_dbdata[i];
                }
            };

            // default destructor()
            ~Basis()
            {
                if(m_bdata)
                {
                    delete [] m_bdata;  
                }

                if(m_dbdata)
                {
                    delete [] m_dbdata;  
                }

                m_bdata  = NULL;
                m_dbdata = NULL;
            };

            void ResetBasisNumModes(int nummodes)
            {
                m_nummodes = nummodes;

                if(m_bdata)
                {
                    delete[] m_bdata;  //delete old space
                    m_bdata = NULL;
                }

                if(m_dbdata)
                {
                    delete[] m_dbdata;  //delete old space
                    m_bdata = NULL;
                }

                m_bdata  = new double[BasisMem()]; //allocate new space
                m_dbdata = new double[BasisMem()]; //allocate new space

                GenBasis();
            }

            /** \brief return basis definition array m_bdata */
            inline const double * GetBdata() const
            {
                return m_bdata;
            }

            /** \brief return basis definition array m_dbdata */
            inline const double * GetDbdata() const
            {
                return m_dbdata;
            }




        protected:
            double    *m_bdata;       /**< Basis definition */
            double    *m_dbdata;      /**< Derivative Basis definition */


        private:

        private:

        };

	typedef boost::shared_ptr<Basis> BasisSharedPtr;

    } // end of namespace
} // end of namespace

#endif //BASIS_H


