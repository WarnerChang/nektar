///////////////////////////////////////////////////////////////////////////////
//
// File ContField2D.cpp
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
// Description: Field definition for 2D domain with boundary conditions
//
///////////////////////////////////////////////////////////////////////////////

#include <MultiRegions/ContField2D.h>

namespace Nektar
{
    namespace MultiRegions
    {

        ContField2D::ContField2D(void):
            DisContField2D(),
            m_locToGloMap(),
            m_contNcoeffs(0),
            m_contCoeffs(),
            m_globalMat()
        {
        }

        ContField2D::ContField2D(const ContField2D &In):
            DisContField2D(In),
            m_locToGloMap(In.m_locToGloMap),
            m_contNcoeffs(In.m_contNcoeffs),
            m_contCoeffs(m_contNcoeffs,0.0),
            m_globalMat(In.m_globalMat),
            m_globalLinSys(In.m_globalLinSys)            
        {
        }
        
        ContField2D::ContField2D(const ContField2D &In, 
                                 SpatialDomains::MeshGraph2D &graph2D,
                                 SpatialDomains::BoundaryConditions &bcs, 
                                 const int bc_loc):
            DisContField2D(In),
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            // Set up boundary conditions for this variable. 
            GenerateBoundaryConditionExpansion(graph2D,bcs,bcs.GetVariable(bc_loc));
            EvaluateBoundaryConditions();
                        
            if(!SameTypeOfBoundaryConditions(In))
            {
                map<int,int> periodicEdges;
                vector<map<int,int> >periodicVertices;
                GetPeriodicEdges(graph2D,bcs,bcs.GetVariable(bc_loc),periodicVertices,periodicEdges);

                m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>::AllocateSharedPtr(m_ncoeffs,*m_exp, m_bndCondExpansions, m_bndConditions, periodicVertices, periodicEdges);
                
            }
            else
            {
                m_locToGloMap = In.m_locToGloMap;
            }

            m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
	    m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }

        ContField2D::ContField2D(SpatialDomains::MeshGraph2D &graph2D):
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            
            m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>::AllocateSharedPtr(m_ncoeffs,*m_exp);
            
            
	    m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
	    m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }

        ContField2D::ContField2D(SpatialDomains::MeshGraph2D &graph2D,
                                 SpatialDomains::BoundaryConditions &bcs, 
                                 const int bc_loc):
            DisContField2D(graph2D,bcs,bc_loc,false),
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            map<int,int> periodicEdges;
            vector<map<int,int> > periodicVertices;
            GetPeriodicEdges(graph2D,bcs,bcs.GetVariable(bc_loc),periodicVertices,periodicEdges);

            m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>::AllocateSharedPtr(m_ncoeffs,*m_exp, m_bndCondExpansions, m_bndConditions, periodicVertices, periodicEdges);
	    
	    m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
	    m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }

        ContField2D::ContField2D(SpatialDomains::MeshGraph2D &graph2D,
                                 SpatialDomains::BoundaryConditions &bcs, 
                                 const std::string variable):
            DisContField2D(graph2D,bcs,variable,false),
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            GenerateBoundaryConditionExpansion(graph2D,bcs,variable);
            EvaluateBoundaryConditions();

            map<int,int> periodicEdges;
            vector<map<int,int> >periodicVertices;
            GetPeriodicEdges(graph2D,bcs,variable,periodicVertices,periodicEdges);

            m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>::AllocateSharedPtr(m_ncoeffs,*m_exp,
                                                                                 m_bndCondExpansions,
                                                                                 m_bndConditions,
                                                                                 periodicVertices,
                                                                                 periodicEdges);
	    
	    m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
	    m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }

        ContField2D::ContField2D(const LibUtilities::BasisKey &TriBa, 
                                 const LibUtilities::BasisKey &TriBb, 
                                 const LibUtilities::BasisKey &QuadBa, 
                                 const LibUtilities::BasisKey &QuadBb,
                                 SpatialDomains::MeshGraph2D &graph2D,
                                 SpatialDomains::BoundaryConditions &bcs, 
                                 const int bc_loc,
                                 const LibUtilities::PointsType TriNb):
            DisContField2D(graph2D,bcs,bc_loc,false),
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            GenerateBoundaryConditionExpansion(graph2D,bcs,bcs.GetVariable(bc_loc));
            EvaluateBoundaryConditions();

            map<int,int> periodicEdges;
            vector<map<int,int> >periodicVertices;
            GetPeriodicEdges(graph2D,bcs,bcs.GetVariable(bc_loc),periodicVertices,periodicEdges);

            m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>::AllocateSharedPtr(m_ncoeffs,*m_exp,
                                                                                 m_bndCondExpansions,
                                                                                 m_bndConditions,
                                                                                 periodicVertices,
                                                                                 periodicEdges);
	    
	    m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
	    m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }

        ContField2D::ContField2D(const LibUtilities::BasisKey &TriBa, 
                                 const LibUtilities::BasisKey &TriBb, 
                                 const LibUtilities::BasisKey &QuadBa, 
                                 const LibUtilities::BasisKey &QuadBb, 
                                 SpatialDomains::MeshGraph2D &graph2D,
                                 SpatialDomains::BoundaryConditions &bcs, 
                                 const std::string variable,
                                 const LibUtilities::PointsType TriNb):
            DisContField2D(graph2D,bcs,variable,false),
            m_globalMat(MemoryManager<GlobalMatrixMap>::AllocateSharedPtr()),
            m_globalLinSys(MemoryManager<GlobalLinSysMap>::AllocateSharedPtr())
        {
            GenerateBoundaryConditionExpansion(graph2D,bcs,variable);
            EvaluateBoundaryConditions();

            map<int,int> periodicEdges;
            vector<map<int,int> >periodicVertices;
            GetPeriodicEdges(graph2D,bcs,variable,periodicVertices,periodicEdges);

            m_locToGloMap = MemoryManager<LocalToGlobalC0ContMap>::AllocateSharedPtr(m_ncoeffs,*m_exp,
                                                                                 m_bndCondExpansions,
                                                                                 m_bndConditions,
                                                                                 periodicVertices,
                                                                                 periodicEdges);
	    
	    m_contNcoeffs = m_locToGloMap->GetNumGlobalCoeffs();
	    m_contCoeffs  = Array<OneD,NekDouble>(m_contNcoeffs,0.0);
        }
        
        ContField2D::~ContField2D()
        {
        }


        bool ContField2D::SameTypeOfBoundaryConditions(const ContField2D &In)
        {
            int i;
            bool returnval = true;
            
            for(i = 0; i < m_bndConditions.num_elements(); ++i)
            {
             
                // check to see if boundary condition type is the same
                // and there are the same number of boundary
                // conditions in the boundary definition.
                if((m_bndConditions[i]->GetBoundaryConditionType()
                    != In.m_bndConditions[i]->GetBoundaryConditionType())||
                   (m_bndCondExpansions[i]->GetExpSize() != In.m_bndCondExpansions[i]->GetExpSize()))
                {
                    returnval = false;
                    break;
                }
            }
            
            return returnval;
        }
            
        void ContField2D::FwdTrans(const Array<OneD, const NekDouble> &inarray,
                                         Array<OneD,       NekDouble> &outarray,
                                   bool  UseContCoeffs)
        {            
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);  
            IProductWRTBase(inarray,wsp,true);
            
            // Solve the system
            GlobalLinSysKey key(StdRegions::eMass,m_locToGloMap);

            if(UseContCoeffs)
            {
                GlobalSolve(key,wsp,outarray);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp);
                GlobalToLocal(tmp,outarray);
            }
        }

        void ContField2D::MultiplyByInvMassMatrix(const Array<OneD, const NekDouble> &inarray,
                                                        Array<OneD,       NekDouble> &outarray,
                                                  bool  UseContCoeffs)
                                                  
        {
            GlobalLinSysKey key(StdRegions::eMass,m_locToGloMap);
            
            if(UseContCoeffs)
            {
                if(inarray.data() == outarray.data())
                {
                    Array<OneD, NekDouble> tmp(m_contNcoeffs,0.0);   
                    Vmath::Vcopy(m_contNcoeffs,inarray,1,tmp,1);
                    GlobalSolve(key,tmp,outarray);
                }
                else
                {
                    GlobalSolve(key,inarray,outarray);
                }
            }
            else
            {
                Array<OneD, NekDouble> globaltmp(m_contNcoeffs,0.0);

                if(inarray.data() == outarray.data())
                {
                    Array<OneD,NekDouble> tmp(inarray.num_elements()); 
                    Vmath::Vcopy(inarray.num_elements(),inarray,1,tmp,1);
                    Assemble(tmp,outarray);
                }
                else
                {
                    Assemble(inarray,outarray);
                }
                    
                GlobalSolve(key,outarray,globaltmp);
                GlobalToLocal(globaltmp,outarray);
            }
        }

        // Solve the helmholtz problem assuming that m_contCoeff vector 
        // contains an intial estimate for solution
        void ContField2D::HelmSolve(const Array<OneD, const NekDouble> &inarray,
                                          Array<OneD,       NekDouble> &outarray,
                                    NekDouble lambda,
                                    bool      UseContCoeffs,
                                    const Array<OneD, const NekDouble>& dirForcing)
        {
            //----------------------------------
            //  Setup RHS Inner product
            //----------------------------------
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);  
            IProductWRTBase(inarray,wsp,true);       
            // Note -1.0 term necessary to invert forcing function to
            // be consistent with matrix definition
            Vmath::Neg(m_contNcoeffs, wsp, 1);        

            // Forcing function with weak boundary conditions 
            int i,j;
            int bndcnt=m_locToGloMap->GetNumLocalDirBndCoeffs();
            for(i = m_numDirBndCondExpansions; i < m_bndCondExpansions.num_elements(); ++i)
            {
                for(j = 0; j < (m_bndCondExpansions[i])->GetNcoeffs(); j++)
                {
                    wsp[m_locToGloMap->GetBndCondCoeffsToGlobalCoeffsMap(bndcnt++)] +=  
                        (m_bndCondExpansions[i]->GetCoeffs())[j];
                }
            }

            // Solve the system
            GlobalLinSysKey key(StdRegions::eHelmholtz,m_locToGloMap,lambda);

            if(UseContCoeffs)
            {
                GlobalSolve(key,wsp,outarray,dirForcing);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp,dirForcing);
                GlobalToLocal(tmp,outarray);
            }
        }


        void ContField2D::LaplaceSolve(const Array<OneD, const NekDouble> &inarray,
                                             Array<OneD,       NekDouble> &outarray,
                                       const Array<OneD, const NekDouble> &dirForcing,
                                       const Array<OneD,       Array<OneD,NekDouble> >& variablecoeffs,
                                       NekDouble time,
                                       bool UseContCoeffs)
        {
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);  
            IProductWRTBase(inarray,wsp,true);       
            // Note -1.0 term necessary to invert forcing function to
            // be consistent with matrix definition
            Vmath::Neg(m_contNcoeffs, wsp, 1);    

            // Forcing function with weak boundary conditions 
            int i,j;
            int bndcnt=m_locToGloMap->GetNumLocalDirBndCoeffs();
            for(i = m_numDirBndCondExpansions; i < m_bndCondExpansions.num_elements(); ++i)
            {
                for(j = 0; j < (m_bndCondExpansions[i])->GetNcoeffs(); j++)
                {
                    wsp[m_locToGloMap->GetBndCondCoeffsToGlobalCoeffsMap(bndcnt++)] +=  
                        (m_bndCondExpansions[i]->GetCoeffs())[j];
                }
            }

            // Solve the system
            GlobalLinSysKey key(StdRegions::eLaplacian,m_locToGloMap,time,variablecoeffs);

            if(UseContCoeffs)
            {
                GlobalSolve(key,wsp,outarray,dirForcing);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp,dirForcing);
                GlobalToLocal(tmp,outarray);
            }
        }


        // Solve the linear advection problem assuming that m_contCoeff vector 
        // contains an intial estimate for solution
        void ContField2D::LinearAdvectionSolve(const Array<OneD, const NekDouble> &inarray,
                                               Array<OneD,       NekDouble> &outarray,
                                               NekDouble ax, NekDouble ay,
                                               bool        UseContCoeffs,
                                               const Array<OneD, const NekDouble>& dirForcing)
        {
            // Inner product of forcing
            Array<OneD,NekDouble> wsp(m_contNcoeffs);  
            IProductWRTBase(inarray,wsp,true);       


            // Solve the system
            GlobalLinSysKey key(StdRegions::eLinearAdvection,m_locToGloMap,
                                ax,ay,eDirectFullMatrix);

            if(UseContCoeffs)
            {
                GlobalSolve(key,wsp,outarray,dirForcing);
            }
            else
            {
                Array<OneD,NekDouble> tmp(m_contNcoeffs,0.0);
                GlobalSolve(key,wsp,tmp,dirForcing);
                GlobalToLocal(tmp,outarray);
            }
        }

        void ContField2D::LinearAdvectionEigs(const NekDouble ax, 
                                              const NekDouble ay,
                                              Array<OneD, NekDouble> &Real, 
                                              Array<OneD, NekDouble> &Imag, 
                                              Array<OneD, NekDouble> &Evecs)
        {
            // Solve the system
            GlobalLinSysKey key(StdRegions::eLinearAdvection,m_locToGloMap,
                                ax,ay,eDirectFullMatrix);
            
            DNekMatSharedPtr   Gmat = GenGlobalMatrixFull(key,m_locToGloMap);
            Gmat->EigenSolve(Real,Imag,Evecs);
        }

        // Note inout contains initial guess and final output. 
        void ContField2D::GlobalSolve(const GlobalLinSysKey &key, 
                                      const Array<OneD, const NekDouble>& rhs, 
                                            Array<OneD,       NekDouble>& inout,
                                      const Array<OneD, const NekDouble>& dirForcing)
        {
            int i,j;
            int bndcnt=0;
            int NumDirBcs = m_locToGloMap->GetNumGlobalDirBndCoeffs();
                  
            // STEP 1: SET THE DIRICHLET DOFS TO THE RIGHT VALUE
            //         IN THE SOLUTION ARRAY
            const Array<OneD,const int>& map = m_locToGloMap->GetBndCondCoeffsToGlobalCoeffsMap();
            for(i = 0; i < m_numDirBndCondExpansions; ++i)
            {
                const Array<OneD,const NekDouble>& coeffs = m_bndCondExpansions[i]->GetCoeffs();
                for(j = 0; j < (m_bndCondExpansions[i])->GetNcoeffs(); ++j)
                {
                    inout[map[bndcnt++]] = coeffs[j];
                }
            }  

            // STEP 2: CALCULATE THE HOMOGENEOUS COEFFICIENTS
            if(m_contNcoeffs - NumDirBcs > 0)
            {
                GlobalLinSysSharedPtr LinSys = GetGlobalLinSys(key);
                LinSys->Solve(rhs,inout,*m_locToGloMap,this,dirForcing);
            }
        }

       GlobalMatrixSharedPtr ContField2D::GetGlobalMatrix(const GlobalMatrixKey &mkey) 
        {
            ASSERTL1(mkey.LocToGloMapIsDefined(),
                     "To use method must have a LocalToGlobalBaseMap "
                     "attached to key");

            GlobalMatrixSharedPtr glo_matrix;
            GlobalMatrixMap::iterator matrixIter = m_globalMat->find(mkey);

            if(matrixIter == m_globalMat->end())
            {
                glo_matrix = GenGlobalMatrix(mkey,m_locToGloMap);
                (*m_globalMat)[mkey] = glo_matrix;
            }
            else
            {
                glo_matrix = matrixIter->second;
            }

            return glo_matrix;
        }

        GlobalLinSysSharedPtr ContField2D::GetGlobalLinSys(const GlobalLinSysKey &mkey) 
        {
            ASSERTL1(mkey.LocToGloMapIsDefined(),
                     "To use method must have a LocalToGlobalBaseMap "
                     "attached to key");

            GlobalLinSysSharedPtr glo_matrix;
            GlobalLinSysMap::iterator matrixIter = m_globalLinSys->find(mkey);

            if(matrixIter == m_globalLinSys->end())
            {
                glo_matrix = GenGlobalLinSys(mkey,m_locToGloMap);
                (*m_globalLinSys)[mkey] = glo_matrix;
            }
            else
            {
                glo_matrix = matrixIter->second;
            }

            return glo_matrix;
        }


    } // end of namespace
} //end of namespace
