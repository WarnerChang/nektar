///////////////////////////////////////////////////////////////////////////////
//
// File GlobalLinSys.cpp
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
// Description: GlobalLinSys definition
//
///////////////////////////////////////////////////////////////////////////////

#include <MultiRegions/GlobalLinSysDirectFull.h>
#include <MultiRegions/LocalToGlobalC0ContMap.h>

namespace Nektar
{
    namespace MultiRegions
    {
        /**
         * @class GlobalLinSysDirect
         *
         * Consider a linear system
         *   \f$\boldsymbol{M\hat{u}}_g=\boldsymbol{\hat{f}}\f$
         * to be solved, where \f$\boldsymbol{M}\f$ is a matrix of type
         * specified by \a mkey. This function assembles the global system
         * matrix \f$\boldsymbol{M}\f$ out of the elemental submatrices
         * \f$\boldsymbol{M}^e\f$. This is equivalent to:
         * \f[ \boldsymbol{M}=\boldsymbol{\mathcal{A}}^T
         * \underline{\boldsymbol{M}}^e\boldsymbol{\mathcal{A}}.\f]
         * where the matrix \f$\boldsymbol{\mathcal{A}}\f$ is a sparse
         * permutation matrix of size \f$N_{\mathrm{eof}}\times
         * N_{\mathrm{dof}}\f$. However, due to the size and sparsity of the
         * matrix \f$\boldsymbol{\mathcal{A}}\f$, it is more efficient to
         * assemble the global matrix using the mapping array \a
         * map\f$[e][i]\f$ contained in the input argument \a locToGloMap.
         * The global assembly is then evaluated as:
         * \f[ \boldsymbol{M}\left[\mathrm{\texttt{map}}[e][i]\right]
         * \left[\mathrm{\texttt{map}}[e][j]\right]
         *       =\mathrm{\texttt{sign}}[e][i]\cdot
         * \mathrm{\texttt{sign}}[e][j] \cdot\boldsymbol{M}^e[i][j]\f]
         * where the values \a sign\f$[e][i]\f$ ensure the correct connectivity.
         */

        /**
         * Registers the class with the Factory.
         */
        string GlobalLinSysDirectFull::className
                = GlobalLinSysFactory::RegisterCreatorFunction(
                    "DirectFull",
                    GlobalLinSysDirectFull::create,
                    "Direct Full.");


        /// Constructor for full direct matrix solve.
        GlobalLinSysDirectFull::GlobalLinSysDirectFull(const GlobalLinSysKey &pLinSysKey,
                     const boost::shared_ptr<LocalMatrixSystem> &pLocMatSys,
                     const boost::shared_ptr<LocalToGlobalBaseMap>
                                                            &pLocToGloMap)
                : GlobalLinSysDirect(pLinSysKey, pLocMatSys, pLocToGloMap)
        {

            ASSERTL1(m_linSysKey.GetGlobalSysSolnType()==eDirectFullMatrix,
                     "This routine should only be used when using a Full Direct"
                     " matrix solve");

            //m_blkMatrices[0] = Mat;
            m_locMatSys = pLocMatSys;
            AssembleFullMatrix(boost::dynamic_pointer_cast<LocalToGlobalC0ContMap>(pLocToGloMap));
        }


        GlobalLinSysDirectFull::~GlobalLinSysDirectFull()
        {

        }


        void GlobalLinSysDirectFull::Solve( const Array<OneD, const NekDouble> &pInput,
                          Array<OneD,       NekDouble> &pOutput,
                    const LocalToGlobalBaseMapSharedPtr &pLocToGloMap,
                    const Array<OneD, const NekDouble> &pDirForcing)
        {
            LocalToGlobalC0ContMapSharedPtr vLocToGloMap
                = boost::dynamic_pointer_cast<LocalToGlobalC0ContMap>(pLocToGloMap);
            DNekScalBlkMatSharedPtr vBlkMat = m_locMatSys->GetLocalSystem()[0];

            bool dirForcCalculated = (bool) pDirForcing.num_elements();
            int nDirDofs  = vLocToGloMap->GetNumGlobalDirBndCoeffs();

            if(nDirDofs)
            {
                // calculate the dirichlet forcing
                int nGlobDofs = vLocToGloMap->GetNumGlobalCoeffs();
                Array<OneD, NekDouble> tmp(nGlobDofs);
                if(dirForcCalculated)
                {
                    Vmath::Vsub(nGlobDofs, pInput.get(), 1,
                                pDirForcing.get(), 1,
                                tmp.get(), 1);
                }
                else
                {
                    // Calculate the dirichlet forcing and substract it
                    // from the rhs
                    DNekScalBlkMat &Mat = *vBlkMat;

                    int nLocDofs = vLocToGloMap->GetNumLocalCoeffs();
                    NekVector<NekDouble> V_dir(nGlobDofs,pOutput,eWrapper);
                    NekVector<NekDouble> V_glob(nGlobDofs,tmp,eWrapper);
                    NekVector<NekDouble> V_loc(nLocDofs);

                    vLocToGloMap->GlobalToLocal(V_dir,V_loc);
                    V_loc = Mat*V_loc;
                    vLocToGloMap->Assemble(V_loc,V_glob);

                    Vmath::Vsub(nGlobDofs,pInput.get(),1,tmp.get(),1,tmp.get(),1);
                }

                Array<OneD, NekDouble> offsetarray;
                GlobalLinSysDirect::Solve(tmp+nDirDofs,offsetarray = pOutput+nDirDofs);
            }
            else
            {
                GlobalLinSysDirect::Solve(pInput,pOutput);
            }
        }


        /**
         * Assemble a full matrix from the block matrix stored in
         * #m_blkMatrices and the given local to global mapping information.
         * @param   locToGloMap Local to global mapping information.
         */
        void GlobalLinSysDirectFull::AssembleFullMatrix(
                        const LocalToGlobalC0ContMapSharedPtr& pLocToGloMap)
        {
            int i,j,n,cnt,gid1,gid2;
            NekDouble sign1,sign2,value;
            int totDofs     = pLocToGloMap->GetNumGlobalCoeffs();
            int NumDirBCs   = pLocToGloMap->GetNumGlobalDirBndCoeffs();

            unsigned int rows = totDofs - NumDirBCs;
            unsigned int cols = totDofs - NumDirBCs;
            NekDouble zero = 0.0;

            DNekMatSharedPtr Gmat;
            int bwidth = pLocToGloMap->GetFullSystemBandWidth();
            MatrixStorage matStorage;

            switch(m_linSysKey.GetMatrixType())
            {
                // case for all symmetric matices
            case StdRegions::eMass:
            case StdRegions::eHelmholtz:
            case StdRegions::eLaplacian:
                {
                    if( (2*(bwidth+1)) < rows)
                    {
                        matStorage = ePOSITIVE_DEFINITE_SYMMETRIC_BANDED;
                        Gmat = MemoryManager<DNekMat>
                                        ::AllocateSharedPtr(rows, cols, zero,
                                                            matStorage,
                                                            bwidth, bwidth);
                    }
                    else
                    {
                        matStorage = ePOSITIVE_DEFINITE_SYMMETRIC;
                        Gmat = MemoryManager<DNekMat>
                                        ::AllocateSharedPtr(rows, cols, zero,
                                                            matStorage);
                    }
                }
                break;
            case StdRegions::eLinearAdvectionReaction:
            case StdRegions::eLinearAdvectionDiffusionReaction:
                {
                    matStorage = eFULL;
                    Gmat = MemoryManager<DNekMat>
                                        ::AllocateSharedPtr(rows, cols, zero,
                                                            matStorage);
                }
                break;
            default:
                {
                    NEKERROR(ErrorUtil::efatal, "Add MatrixType to switch "
                                                "statement");
                }
            }

            // fill global matrix
            DNekScalMatSharedPtr loc_mat;
            DNekScalBlkMatSharedPtr vBlkMat = m_locMatSys->GetLocalSystem()[0];

            int loc_lda;
            for(n = cnt = 0; n < vBlkMat->GetNumberOfBlockRows(); ++n)
            {
                loc_mat = vBlkMat->GetBlock(n,n);
                loc_lda = loc_mat->GetRows();

                for(i = 0; i < loc_lda; ++i)
                {
                    gid1 = pLocToGloMap->GetLocalToGlobalMap(cnt + i)-NumDirBCs;
                    sign1 =  pLocToGloMap->GetLocalToGlobalSign(cnt + i);
                    if(gid1 >= 0)
                    {
                        for(j = 0; j < loc_lda; ++j)
                        {
                            gid2 = pLocToGloMap->GetLocalToGlobalMap(cnt + j)
                                                                    - NumDirBCs;
                            sign2 = pLocToGloMap->GetLocalToGlobalSign(cnt + j);
                            if(gid2 >= 0)
                            {
                                // When global matrix is symmetric,
                                // only add the value for the upper
                                // triangular part in order to avoid
                                // entries to be entered twice
                                if((matStorage == eFULL)||(gid2 >= gid1))
                                {
                                    value = Gmat->GetValue(gid1,gid2)
                                                + sign1*sign2*(*loc_mat)(i,j);
                                    Gmat->SetValue(gid1,gid2,value);
                                }
                            }
                        }
                    }
                }
                cnt   += loc_lda;
            }

            if(rows)
            {
                PointerWrapper w = eWrapper;
                m_linSys = MemoryManager<DNekLinSys>::AllocateSharedPtr(Gmat,w);
            }
        }
    }
}