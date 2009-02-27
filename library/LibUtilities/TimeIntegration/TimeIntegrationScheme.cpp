///////////////////////////////////////////////////////////////////////////////
//
// File TimeIntegrationScheme.cpp
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
// Description: implementation of time integration key class 
//
///////////////////////////////////////////////////////////////////////////////
#include <LibUtilities/LibUtilities.h>
#include <iostream>
#include <LibUtilities/TimeIntegration/TimeIntegrationScheme.h>

namespace Nektar
{
    namespace LibUtilities 
    {  
        TimeIntegrationSchemeManagerT &TimeIntegrationSchemeManager(void)
        {
            TimeIntegrationSchemeManagerT& m = Loki::SingletonHolder<TimeIntegrationSchemeManagerT>::Instance();
            static bool reg = m.RegisterGlobalCreator(TimeIntegrationScheme::Create);
            return m;
        }
        

        bool operator==(const TimeIntegrationSchemeKey &lhs, const TimeIntegrationSchemeKey &rhs)
        {
            return (lhs.m_method == rhs.m_method);
        }

        bool operator<(const TimeIntegrationSchemeKey &lhs, const TimeIntegrationSchemeKey &rhs)
        {            
            return (lhs.m_method < rhs.m_method);
        }
        
        bool TimeIntegrationSchemeKey::opLess::operator()(const TimeIntegrationSchemeKey &lhs, const TimeIntegrationSchemeKey &rhs) const
        {
            return (lhs.m_method < rhs.m_method);
        }

        std::ostream& operator<<(std::ostream& os, const TimeIntegrationSchemeKey& rhs)
        {
            os << "Time Integration Scheme: " << TimeIntegrationMethodMap[rhs.GetIntegrationMethod()] << endl;

            return os;
        }

        TimeIntegrationSolution::TimeIntegrationSolution(TimeIntegrationMethod method, 
                                                         const DoubleArray& y, 
                                                         const DoubleArray& My, 
                                                         NekDouble t):
            m_method(method),
            m_sol(y),
            m_solVector(1),
            m_t(1)
        {        
            m_solVector[0] = My;
            m_t[0] = t;    
        }

        TimeIntegrationSolution::TimeIntegrationSolution(TimeIntegrationMethod method, 
                                                         const DoubleArray& y, 
                                                         const TripleArray& My, 
                                                         const Array<OneD, NekDouble>& t):
            m_method(method),
            m_sol(y),
            m_solVector(My),
            m_t(t)
        {        
        }

        TimeIntegrationSolution::TimeIntegrationSolution(TimeIntegrationMethod method, 
                                                         unsigned int nsteps,
                                                         unsigned int nvar,
                                                         unsigned int npoints):
            m_method(method),
            m_solVector(nsteps),
            m_sol(nvar),
            m_t(nsteps)
        {
            for(int i = 0; i < nsteps; i++)
            {
                m_solVector[i] = Array<OneD, Array<OneD, NekDouble> >(nvar);
                for(int j = 0; j < nvar; j++)
                {
                    m_solVector[i][j] = Array<OneD,NekDouble>(npoints);
                }
            }   
            for(int j = 0; j < nvar; j++)
            {
                m_sol[j] = Array<OneD,NekDouble>(npoints);
            }         
        }
        
        TimeIntegrationSchemeSharedPtr TimeIntegrationScheme::Create(const TimeIntegrationSchemeKey &key)
        {
            TimeIntegrationSchemeSharedPtr returnval(MemoryManager<TimeIntegrationScheme>::AllocateSharedPtr(key));
            return returnval;
        }

        TimeIntegrationScheme::TimeIntegrationScheme(const TimeIntegrationSchemeKey &key):
            m_schemeKey(key)
        {
            switch(key.GetIntegrationMethod())
            {
            case eForwardEuler:
            case eAdamsBashforthOrder1:
                {
                    m_numsteps = 1;
                    m_numstages = 1;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(1);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(1);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.0);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps ,m_numstages,1.0);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps, 1.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps ,m_numsteps, 1.0);

                    m_schemeType = eExplicit;
                }
                break;
            case eAdamsBashforthOrder2:
                {
                    m_numsteps = 2;
                    m_numstages = 1;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(1);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(1);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.0);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps ,m_numstages);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps,0.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps ,m_numsteps,0.0);

                    m_B[0][0][0] = 3.0/2.0;
                    m_B[0][1][0] = 1.0;

                    m_U[0][0] = 1.0;

                    m_V[0][0] = 1.0;
                    m_V[0][1] = -0.5;

                    m_schemeType = eExplicit;
                }
                break;
            case eBackwardEuler:
            case eAdamsMoultonOrder1:
                {
                    m_numsteps = 1;
                    m_numstages = 1;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(1);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(1);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,1.0);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps ,m_numstages,1.0);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps, 1.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps ,m_numsteps, 1.0);

                    m_schemeType = eDiagonallyImplicit;
                }
                break;
            case eAdamsMoultonOrder2:
                {
                    m_numsteps  = 2;
                    m_numstages = 1;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(1);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(1);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.5);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps, m_numstages,0.0);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps, 0.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps, m_numsteps, 0.0);

                    m_B[0][0][0] = 0.5;
                    m_B[0][1][0] = 1.0;

                    m_U[0][0] = 1.0;
                    m_U[0][1] = 0.5;

                    m_V[0][0] = 1.0;
                    m_V[0][1] = 0.5;


                    m_schemeType = eDiagonallyImplicit;
                }
                break;
            case eMidpoint:
                {
                    m_numsteps = 1;
                    m_numstages = 2;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(1);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(1);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.0);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps, m_numstages,0.0);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps, 1.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps, m_numsteps, 1.0);

                    m_A[0][1][0] = 0.5;
                    m_B[0][0][1] = 1.0;

                    m_schemeType = eExplicit;
                }
                break;
            case eClassicalRungeKutta4:
                {
                    m_numsteps = 1;
                    m_numstages = 4;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(1);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(1);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.0);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps, m_numstages,0.0);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps, 1.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps, m_numsteps, 1.0);

                    m_A[0][1][0] = 0.5;
                    m_A[0][2][1] = 0.5;
                    m_A[0][3][2] = 1.0;

                    m_B[0][0][0] = 1.0/6.0;
                    m_B[0][0][1] = 1.0/3.0;
                    m_B[0][0][2] = 1.0/3.0;
                    m_B[0][0][3] = 1.0/6.0;

                    m_schemeType = eExplicit;
                }
                break;
            case eDIRKOrder2:
                {
                    m_numsteps = 1;
                    m_numstages = 2;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(1);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(1);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.0);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps, m_numstages,0.0);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps, 1.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps, m_numsteps, 1.0);

                    NekDouble lambda = (2.0-sqrt(2.0))/2.0;

                    m_A[0][0][0] = lambda;
                    m_A[0][1][0] = 1.0 - lambda;
                    m_A[0][1][1] = lambda;

                    m_B[0][0][0] = 1.0 - lambda;
                    m_B[0][0][1] = lambda;

                    m_schemeType = eDiagonallyImplicit;
                }
                break;
            case eDIRKOrder3:
                {
                    m_numsteps = 1;
                    m_numstages = 3;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(1);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(1);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.0);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps, m_numstages,0.0);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps, 1.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps, m_numsteps, 1.0);

                    NekDouble lambda = 0.4358665215;

                    m_A[0][0][0] = lambda;
                    m_A[0][1][0] = 0.5  * (1.0 - lambda);
                    m_A[0][2][0] = 0.25 * (-6.0*lambda*lambda + 16.0*lambda - 1.0);
                    m_A[0][1][1] = lambda;
                    m_A[0][2][1] = 0.25 * ( 6.0*lambda*lambda - 20.0*lambda + 5.0);
                    m_A[0][2][2] = lambda;

                    m_B[0][0][0] = 0.25 * (-6.0*lambda*lambda + 16.0*lambda - 1.0);
                    m_B[0][0][1] = 0.25 * ( 6.0*lambda*lambda - 20.0*lambda + 5.0);
                    m_B[0][0][2] = lambda;

                    m_schemeType = eDiagonallyImplicit;
                }
                break;
            case eIMEXdirk_3_4_3:
                {
                    m_numsteps  = 1;
                    m_numstages = 4;

                    m_A = Array<OneD, Array<TwoD,NekDouble> >(2);
                    m_B = Array<OneD, Array<TwoD,NekDouble> >(2);

                    m_A[0] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.0);
                    m_B[0] = Array<TwoD,NekDouble>(m_numsteps, m_numstages,0.0);
                    m_A[1] = Array<TwoD,NekDouble>(m_numstages,m_numstages,0.0);
                    m_B[1] = Array<TwoD,NekDouble>(m_numsteps, m_numstages,0.0);
                    m_U    = Array<TwoD,NekDouble>(m_numstages,m_numsteps, 1.0);
                    m_V    = Array<TwoD,NekDouble>(m_numsteps, m_numsteps, 1.0);

                    NekDouble lambda = 0.4358665215;

                    m_A[0][1][1] = lambda;
                    m_A[0][2][1] = 0.5  * (1.0 - lambda);
                    m_A[0][3][1] = 0.25 * (-6.0*lambda*lambda + 16.0*lambda - 1.0);
                    m_A[0][2][2] = lambda;
                    m_A[0][3][2] = 0.25 * ( 6.0*lambda*lambda - 20.0*lambda + 5.0);
                    m_A[0][3][3] = lambda;

                    m_B[0][0][1] = 0.25 * (-6.0*lambda*lambda + 16.0*lambda - 1.0);
                    m_B[0][0][2] = 0.25 * ( 6.0*lambda*lambda - 20.0*lambda + 5.0);
                    m_B[0][0][3] = lambda;

                    m_A[1][1][0] = 0.4358665215;
                    m_A[1][2][0] = 0.3212788860;
                    m_A[1][2][1] = 0.3966543747;
                    m_A[1][3][0] =-0.105858296;
                    m_A[1][3][1] = 0.5529291479;
                    m_A[1][3][2] = 0.5529291479;

                    m_B[1][0][1] = 0.25 * (-6.0*lambda*lambda + 16.0*lambda - 1.0);
                    m_B[1][0][2] = 0.25 * ( 6.0*lambda*lambda - 20.0*lambda + 5.0);
                    m_B[1][0][3] = lambda;

                    m_schemeType = eIMEX;
                }
                break;
            default:
                {
                    NEKERROR(ErrorUtil::efatal,"Invalid Time Integration Scheme");
                }
            }

            m_firstStageEqualsOldSolution = CheckIfFirstStageEqualsOldSolution(m_A,m_B,m_U,m_V);
            m_lastStageEqualsNewSolution  = CheckIfLastStageEqualsNewSolution(m_A,m_B,m_U,m_V);

            ASSERTL1(VerifyIntegrationSchemeType(m_schemeType,m_A,m_B,m_U,m_V),
                     "Time integration scheme coefficients do not match its type");
        }


        bool TimeIntegrationScheme::
        VerifyIntegrationSchemeType(TimeIntegrationSchemeType type,
                                    const Array<OneD, const Array<TwoD, NekDouble> >& A,
                                    const Array<OneD, const Array<TwoD, NekDouble> >& B,
                                    const Array<TwoD, const NekDouble>& U,
                                    const Array<TwoD, const NekDouble>& V) const
        {
            int i;
            int j;
            int m;
            bool returnval = false;
            int  IMEXdim   = A.num_elements();
            int  dim       = A[0].GetRows();

            Array<OneD, TimeIntegrationSchemeType> vertype(IMEXdim,eExplicit);

            for(m = 0; m < IMEXdim; m++)
            {
                for(i = 0; i < dim; i++)
                {
                    if( fabs(A[m][i][i]) > NekConstants::kNekZeroTol )
                    {
                        vertype[m] = eDiagonallyImplicit;
                    }
                }
                
                for(i = 0; i < dim; i++)
                {
                    for(j = i+1; j < dim; j++)
                    {
                        if( fabs(A[m][i][j]) > NekConstants::kNekZeroTol )
                        {
                             vertype[m] = eImplicit;
                             ASSERTL1(false,"Fully Impplicit schemes cannnot be handled by the TimeIntegrationScheme class");
                        }
                    }
                }
            }

            if(IMEXdim == 2)
            {
                ASSERTL1(B.num_elements()==2,"Coefficient Matrix B should have an implicit and explicit part for IMEX schemes");
                if((vertype[0] == eDiagonallyImplicit) &&
                   (vertype[1] == eExplicit))
                {
                    vertype[0] = eIMEX;
                }
                else
                {
                    ASSERTL1(false,"This is not a proper IMEX scheme");
                }
            }

            return (vertype[0] == type);
        }

        TimeIntegrationSolutionSharedPtr 
        TimeIntegrationScheme::InitializeScheme(const NekDouble                      timestep,
                                                      NekDouble                      &time   ,
                                                      int                            &nsteps ,
                                                      ConstDoubleArray               &y_0    ,
                                                const TimeIntegrationSchemeOperators &op     ) const
        {
            TimeIntegrationSolutionSharedPtr y_out;
            TimeIntegrationMethod method = m_schemeKey.GetIntegrationMethod();
            
            switch(method)
            {
            case eForwardEuler:
            case eBackwardEuler:
            case eAdamsBashforthOrder1:
            case eAdamsMoultonOrder1:
            case eMidpoint:
            case eClassicalRungeKutta4:
            case eDIRKOrder2:
            case eDIRKOrder3:
            case eIMEXdirk_3_4_3:
                {
                    nsteps = 0;
                    
                    unsigned int nvar      = y_0.num_elements();
                    unsigned int npoints   = y_0[0].num_elements();
                    DoubleArray My_0(nvar);
                    for(int i = 0; i < nvar; i++)
                    {
                        My_0[i] = Array<OneD,NekDouble>(npoints);
                    }
                    op.DoOdeLhs(y_0,My_0,time);
                    y_out = MemoryManager<TimeIntegrationSolution>::AllocateSharedPtr(method,y_0,My_0,time);
                }
                break;
            case eAdamsBashforthOrder2:
                {
                    // To initialise, we do a 1st order Adams Bashforth step (=Forward euler) to calculate the
                    // solution at the first time level
                    TimeIntegrationSchemeKey         IntKey(eAdamsBashforthOrder1);
                    TimeIntegrationSchemeSharedPtr   IntScheme = TimeIntegrationSchemeManager()[IntKey];
                    TimeIntegrationSolutionSharedPtr u = IntScheme->InitializeScheme(timestep,time,nsteps,y_0,op);
                    IntScheme->TimeIntegrate(timestep,u,op);
                    
                    int i;
                    int nvar    = y_0.num_elements();
                    int npoints = y_0[0].num_elements();
                    DoubleArray f_y_0(nvar);
                    for(i = 0; i < nvar; i++)
                    {
                        f_y_0[i] = Array<OneD,NekDouble>(npoints);
                    }
                    op.DoOdeRhs(y_0,f_y_0,time);
                    
                    for(i = 0; i < nvar; i++)
                    {
                        Blas::Dscal(npoints,timestep,f_y_0[i].get(),1);
                    }
                    
                    TripleArray My(2);
                    My[0] = (u->GetSolutionVector())[0];
                    My[1] = f_y_0;
                    
                    Array<OneD,NekDouble> t(2);
                    t[0] = u->GetTime();
                    t[1] = timestep;
                    
                    time = u->GetTime();
                    nsteps = 1;
                    y_out = MemoryManager<TimeIntegrationSolution>::AllocateSharedPtr(method,u->GetSolution(),My,t);
                }
                break;
            default:
                {
                    NEKERROR(ErrorUtil::efatal,"Methods need implementation for specified integration scheme.");
                }
            }
           
            return y_out;
        }        
        
        TimeIntegrationScheme::ConstDoubleArray& 
        TimeIntegrationScheme::TimeIntegrate(const NekDouble                        timestep,          
                                                   TimeIntegrationSolutionSharedPtr &y      ,
                                             const TimeIntegrationSchemeOperators   &op     ) const
        {
            ASSERTL1(!(GetIntegrationSchemeType() == eImplicit),
                     "Fully Implicit integration scheme cannot be handled by this routine.");
            
            TimeIntegrationMethod method = y->GetIntegrationMethod();
            int nvar    = y->GetFirstDim ();
            int npoints = y->GetSecondDim();
            
            ASSERTL1(method == m_schemeKey.GetIntegrationMethod(),
                     "Input and output argument are constructed for a different integration scheme.");
            
            TimeIntegrationSolutionSharedPtr y_new = MemoryManager<TimeIntegrationSolution>::
                AllocateSharedPtr(method,m_numsteps,nvar,npoints); 

            TimeIntegrate(timestep,
                          y->GetSolutionVector(),
                          y->GetSolution(),
                          y->GetTimeVector(),
                          y_new->UpdateSolutionVector(),
                          y_new->UpdateSolution(),
                          y_new->UpdateTimeVector(),
                          op); 
            
                y = y_new;

                return y->GetSolution();
        }

        void TimeIntegrationScheme::TimeIntegrate(const NekDouble                      timestep,      
                                                        ConstTripleArray               &y_old  ,
                                                        ConstDoubleArray               &sol_old,
                                                        ConstSingleArray               &t_old  ,
                                                        TripleArray                    &y_new  ,
                                                        DoubleArray                    &sol_new,
                                                        SingleArray                    &t_new  ,
                                                  const TimeIntegrationSchemeOperators &op     ) const
        {
            ASSERTL1(CheckTimeIntegrateArguments(timestep,y_old,sol_old,t_old,y_new,sol_new,t_new,op),
                     "Arguments not well defined");    

            unsigned int i,j,k;    
            unsigned int nvar      = GetFirstDim (y_old);
            unsigned int npoints   = GetSecondDim(y_old);
            
            TimeIntegrationSchemeType type = GetIntegrationSchemeType();             
            
            // First, we are going to calculate the various stage values and stage derivatives
            // (this is the multi-stage part of the method)
            // - Y   corresponds to the stage values
            // - F   corresponds to the stage derivatives
            // - T   corresponds to the time at the different stages
            // - tmp corresponds to the right hand side of each stage equation
            DoubleArray Y  (nvar);        
            DoubleArray tmp(nvar);        
            TripleArray F  (m_numstages);  
            TripleArray F_IMEX;  // Used to store the Explicit stage derivative of IMEX schemes
                                 // The implicit part will be stored in F
            NekDouble   T;
            
            // Allocate memory for the arrays Y and F and tmp
            // The same storage will be used for every stage -> Y is a DoubleArray
            for(j = 0; j < nvar; j++)
            {
                Y[j]   = Array<OneD, NekDouble>(npoints);
            }
            // The same storage will be used for every stage -> tmp is a DoubleArray
            if( !m_firstStageEqualsOldSolution || (m_numstages>1) )
            {
                for(j = 0; j < nvar; j++)
                {
                    tmp[j] = Array<OneD, NekDouble>(npoints);
                }
            }
            // Different storage for every stage derivative as the data
            // will be re-used to update the solution -> F is a TripleArray
            for(i = 0; i < m_numstages; ++i)
            {    
                F[i]   = DoubleArray(nvar);
                for(j = 0; j < nvar; j++)
                {
                    F[i][j] = Array<OneD, NekDouble>(npoints);
                }
            }
            if(type == eIMEX)
            {
                F_IMEX = TripleArray(m_numstages);
                for(i = 0; i < m_numstages; ++i)
                {   
                    F_IMEX[i]   = DoubleArray(nvar); 
                    for(j = 0; j < nvar; j++)
                    {
                        F_IMEX[i][j] = Array<OneD, NekDouble>(npoints);
                    }      
                }
            }
            
            // The loop below calculates the stage values and derivatives
            for(i = 0; i < m_numstages; i++)
            {
                if( (i==0) && m_firstStageEqualsOldSolution )
                {
                    for(k = 0; k < nvar; k++)
                    {
                        Vmath::Vcopy(npoints,sol_old[k],1,Y[k],1);
                    }
                }
                else
                {
                    // The stage values Y are a linear combination of:
                    // 1: the stage derivatives
                    for(k = 0; k < nvar; k++)
                    {
                        Vmath::Smul(npoints,timestep*A(i,j),F[j][k],1,
                                    tmp[k],1);
                        if(type == eIMEX)       
                        {
                            Vmath::Svtvp(npoints,timestep*A_IMEX(i,j),F_IMEX[j][k],1,
                                         tmp[k],1,tmp[k],1);
                        }
                    }          
                    T = A(i,0)*timestep;

                    for( j = 1; j < i; j++ )
                    {
                        for(k = 0; k < nvar; k++)
                        {
                            Vmath::Svtvp(npoints,timestep*A(i,j),F[j][k],1,
                                     tmp[k],1,tmp[k],1);
                            if(type == eIMEX)       
                            {
                                Vmath::Svtvp(npoints,timestep*A_IMEX(i,j),F_IMEX[j][k],1,
                                             tmp[k],1,tmp[k],1);
                            }
                        }          
                        T += A(i,j)*timestep;
                    }
                    
                    // 2: the imported multi-step solution of the previous time level
                    for( j = 0; j < m_numsteps; j++)
                    {
                        for(k = 0; k < nvar; k++)
                        {
                            Vmath::Svtvp(npoints,U(i,j),y_old[j][k],1,
                                         tmp[k],1,tmp[k],1);
                        }
                        T += U(i,j)*t_old[j];
                    }
                    
                    // Now, solve for the stage values
                    if( (type == eDiagonallyImplicit) ||
                        (type == eIMEX) )
                    {
                        if( fabs(A(i,j)) > NekConstants::kNekZeroTol )
                        {
                            op.DoImplicitSolve(tmp, Y, T, A(i,i)*timestep);
                        }
                        else
                        {
                            op.DoOdeLhsSolve(tmp, Y, T);
                        }
                    }
                    else
                    {
                        op.DoOdeLhsSolve(tmp, Y, T);
                    }
                }
                
                // Calculate the stage derivative based upond the stage value
                if(type == eIMEX)
                {
                    op.DoOdeImplicitRhs(Y, F[i]     , T);
                    op.DoOdeExplicitRhs(Y, F_IMEX[i], T);
                }
                else
                {
                    op.DoOdeRhs(Y, F[i], T);
                }
            }
            
            // Next, the solution at the new time level will be calculated.
            // For multi-step methods, this includes updating the values 
            // of the auxiliary pare=ameters
            
            // The loop below calculates the solution at the new time level

            // If last stage equals new solution, the new solution needs not be 
            // calculated explicitely but can simply be copied. This saves
            // a solve as this normally would require a call to DoOdeLhsSolve
            int i_start = m_lastStageEqualsNewSolution?1:0; 
            for( i = i_start; i < m_numsteps; i++)
            {
                // The solution at the new time level is a linear combination of:
                // 1: the stage derivatives
                for(k = 0; k < nvar; k++)
                {
                    Vmath::Smul(npoints,timestep*B(i,0),F[0][k],1,y_new[i][k],1);
                    if(type == eIMEX)
                    {
                        Vmath::Svtvp(npoints,timestep*B_IMEX(i,0),F_IMEX[0][k],1,
                                     y_new[i][k],1,y_new[i][k],1);
                    }
                }
                t_new[i] = B(i,0)*timestep;   
                
                for( j = 1; j < m_numstages; j++ )
                {
                    for(k = 0; k < nvar; k++)
                    {
                        Vmath::Svtvp(npoints,timestep*B(i,j),F[j][k],1,
                                     y_new[i][k],1,y_new[i][k],1);
                        if(type == eIMEX)
                        {
                            Vmath::Svtvp(npoints,timestep*B_IMEX(i,j),F_IMEX[j][k],1,
                                         y_new[i][k],1,y_new[i][k],1);
                        }
                    }
                    t_new[i] += B(i,j)*timestep;                    
                }
                
                // 2: the imported multi-step solution of the previous time level
                for( j = 0; j < m_numsteps; j++ )
                {
                    for(k = 0; k < nvar; k++)
                    {
                        Vmath::Svtvp(npoints,V(i,j),y_old[j][k],1,
                                     y_new[i][k],1,y_new[i][k],1);
                    }
                    t_new[i] += V(i,j)*t_old[j];
                }
            }

            if(m_lastStageEqualsNewSolution)
            {
                for(k = 0; k < nvar; k++)
                {
                    Vmath::Vcopy(npoints,Y[k],1,sol_new[k],1);
                }

                t_new[0] = B(0,0)*timestep;
                for(j = 1; j < m_numstages; j++)
                {
                    t_new[0] += B(0,j)*timestep;
                }

                op.DoOdeLhs(sol_new,y_new[0],t_new[0]);

                i_start = 1;
            }
            else
            {
                op.DoOdeLhsSolve(y_new[0],sol_new,t_new[0]);
            }
        }        
        
        bool TimeIntegrationScheme::CheckIfFirstStageEqualsOldSolution(const Array<OneD, const Array<TwoD, NekDouble> >& A,
                                                                       const Array<OneD, const Array<TwoD, NekDouble> >& B,
                                                                       const Array<TwoD, const NekDouble>& U,
                                                                       const Array<TwoD, const NekDouble>& V) const
        {
            int i,m;
            // First stage equals old solution if:
            // 1. the first row of the coefficient matrix A consists of zeros
            // 2. U[0][0] is equal to one and all other first row entries of U are zero
 
            // 1. Check first condition
            for(m = 0; m < A.num_elements(); m++)
            {
                for(i = 0; i < m_numstages; i++)
                {
                    if( fabs(A[m][0][i]) > NekConstants::kNekZeroTol )
                    {
                        return false;
                    }
                }
            }

            // 2. Check second condition
            if( fabs(U[0][0] - 1.0) > NekConstants::kNekZeroTol )
            {
                return false;
            }
            for(i = 1; i < m_numsteps; i++)
            {
                if( fabs(U[0][i]) > NekConstants::kNekZeroTol )
                {
                    return false;
                }
            }

            return true;
        }
            
        bool TimeIntegrationScheme::CheckIfLastStageEqualsNewSolution(const Array<OneD, const Array<TwoD, NekDouble> >& A,
                                                                      const Array<OneD, const Array<TwoD, NekDouble> >& B,
                                                                      const Array<TwoD, const NekDouble>& U,
                                                                      const Array<TwoD, const NekDouble>& V) const
        {
            int i,m;
            // Last stage equals new solution if:
            // 1. the last row of the coefficient matrix A is equal to the first row of matrix B
            // 2. the last row of the coefficient matrix U is equal to the first row of matrix V
 
            // 1. Check first condition
            for(m = 0; m < A.num_elements(); m++)
            {
                for(i = 0; i < m_numstages; i++)
                {
                    if( fabs(A[m][m_numstages-1][i]-B[m][0][i]) > NekConstants::kNekZeroTol )
                    {
                        return false;
                    }
                }
            }

            // 2. Check second condition
            for(i = 0; i < m_numsteps; i++)
            {
                if( fabs(U[m_numstages-1][i]-V[0][i]) > NekConstants::kNekZeroTol )
                {
                    return false;
                }
            }

            return true;
        }
        
        bool TimeIntegrationScheme::CheckTimeIntegrateArguments(const NekDouble                      timestep,      
                                                                      ConstTripleArray               &y_old  ,
                                                                      ConstDoubleArray               &sol_old,
                                                                      ConstSingleArray               &t_old  ,
                                                                      TripleArray                    &y_new  ,
                                                                      DoubleArray                    &sol_new,
                                                                      SingleArray                    &t_new  ,
                                                                const TimeIntegrationSchemeOperators &op) const
        {
            // Check if arrays are all of consistent size
            ASSERTL1(y_old.num_elements()==m_numsteps,"Non-matching number of steps.");    
            ASSERTL1(y_new.num_elements()==m_numsteps,"Non-matching number of steps."); 
            
            ASSERTL1(y_old[0].   num_elements()==y_new[0].   num_elements(),"Non-matching number of variables.");  
            ASSERTL1(y_old[0][0].num_elements()==y_new[0][0].num_elements(),"Non-matching number of coefficients."); 
            
            ASSERTL1(sol_old.   num_elements()==sol_new.   num_elements(),"Non-matching number of variables.");  
            ASSERTL1(sol_old[0].num_elements()==sol_new[0].num_elements(),"Non-matching number of coefficients.");  
            
            ASSERTL1(t_old.num_elements()==m_numsteps,"Non-matching number of steps.");    
            ASSERTL1(t_new.num_elements()==m_numsteps,"Non-matching number of steps."); 

            return true;
        }
        
        std::ostream& operator<<(std::ostream& os, const TimeIntegrationSchemeSharedPtr& rhs)
        {
            return operator<<(os,*rhs);
        }

        std::ostream& operator<<(std::ostream& os, const TimeIntegrationScheme& rhs)
        {
            int i,j;
            int r = rhs.GetNsteps();
            int s = rhs.GetNstages();
            TimeIntegrationSchemeType type = rhs.GetIntegrationSchemeType();

            int oswidth = 9;
            int osprecision = 6;

            os << "Time Integration Scheme: " << TimeIntegrationMethodMap[rhs.GetIntegrationMethod()] << endl;
            os << "- number of steps:  " << r << endl;
            os << "- number of stages: " << s << endl;
            os << "- type of scheme:   " << TimeIntegrationSchemeTypeMap[rhs.GetIntegrationSchemeType()] << endl;
            os << "General linear method tableau: " << endl;

            for(i = 0; i < s; i++)
            {
                for(j = 0; j < s; j++)
                {
                    os.width(oswidth);
                    os.precision(osprecision);
                    os << right << rhs.A(i,j) << " ";
                }
                if(type == eIMEX)
                {
                    os << " '"; 
                    for(j = 0; j < s; j++)
                    {
                        os.width(oswidth);
                        os.precision(osprecision);
                        os << right << rhs.A_IMEX(i,j) << " ";
                    }
                }
                os << " |"; 

                for(j = 0; j < r; j++)
                {
                    os.width(oswidth);
                    os.precision(osprecision);
                    os << right << rhs.U(i,j);
                }
                os << endl;
            }
            int imexflag = (type == eIMEX)?2:1;
            for(int i = 0; i < (r+imexflag*s)*(oswidth+1)+imexflag*2-1; i++)
            {
                os << "-";
            }
            os << endl;
            for(i = 0; i < r; i++)
            {
                for(j = 0; j < s; j++)
                {
                    os.width(oswidth);
                    os.precision(osprecision);
                    os << right << rhs.B(i,j) << " ";
                }
                if(type == eIMEX)
                {
                    os << " '"; 
                    for(j = 0; j < s; j++)
                    {
                        os.width(oswidth);
                        os.precision(osprecision);
                        os << right << rhs.B_IMEX(i,j) << " ";
                    }
                }
                os << " |"; 

                for(j = 0; j < r; j++)
                {
                    os.width(oswidth);
                    os.precision(osprecision);
                    os << right << rhs.V(i,j);
                }
                os << endl;
            }
            return os;
        }


        /**
         * \page pageGeneralLinearMethods General Linear Methods 
         *  the implementation of time integration schemes in Nektar++
         *
         * \section sectionGeneralLinearMethodsIntroduction Introduction
         * General linear methods can be considered as the generalisation of a broad 
         * range of different numerical methods for ordinary differential equations. 
         * They were introduced by Butcher and they provide a unified formulation for 
         * traditional methods such as the Runge-Kutta methods and the linear multi-step methods. 
         * From an implementational point of view, this means that all these numerical methods 
         * can be abstracted in a similar way. As this allows a high level of generality, 
         * it is chosen in Nektar++ to cast all time integration schemes in the framework of 
         * general linear methods.
         *
         *
         * For background information about general linear methods, please consult the 
         * following references:<BR>
         * [1] Butcher, J.C. (2006) <em>General linear methods</em>, Acta Numerica 15, 157-256 <BR>
         * [2] http://www.math.auckland.ac.nz/~butcher/conferences.html
         *
         * \section  sectionGeneralLinearMethods General linear methods
         * The standard initial value problem can written in the form
         * \f[
         * \frac{d\boldsymbol{y}}{dt}=\boldsymbol{f}(t,\boldsymbol{y}),\quad \boldsymbol{y}(t_0)=\boldsymbol{y}_0
         * \f]
         * where \f$\boldsymbol{y}=[y^0\quad y^1\quad  \cdots \quad y^{N-1}]^T\f$. <BR>
         * In the formulation of general linear methods, it is more convenient to consider the ODE in autonomous form, i.e.
         * \f[
         * \frac{d\boldsymbol{\hat{y}}}{dt}=\boldsymbol{\hat{f}}(\boldsymbol{\hat{y}}),\quad \boldsymbol{\hat{y}}(t_0)=
         * \boldsymbol{\hat{y}}_0.
         * \f]
         *
         * When the original problem is presented in non-autonomous form, it can be written as an 
         * autonomous system in the following way:
         * \f[
         * \boldsymbol{\hat{y}}=
         * \left [ \begin{array}{c}
         * t \\
         * \boldsymbol{y}
         * \end{array}\right],\quad
         * \boldsymbol{\hat{f}}(\boldsymbol{\hat{y}})=
         * \left [ \begin{array}{c}
         * 1 \\
         * \boldsymbol{f}(\boldsymbol{\hat{y}})
         * \end{array}\right].
         * \f]
         *
         * \subsection subsectionGeneralLinearMethodsFomulation Formulation
         * Suppose the governing differential equation is given in autonomous form, 
         * the \f$n^{th}\f$ step of the general linear method comprising
         *  -  \f$r\f$ steps (as in a multi-step method)
         *  -  \f$s\f$ stages (as in a Runge-Kutta method)
         *
         * is formulated as:
         * \f{eqnarray*}
         * \boldsymbol{Y}_i = \Delta t\sum_{j=0}^{s-1}a_{ij}\boldsymbol{F}_j+\sum_{j=0}^{r-1}u_{ij}
         * \boldsymbol{\hat{y}}_{j}^{[n-1]}, \qquad i=0,1,\ldots,s-1 \\
         * \boldsymbol{\hat{y}}_{i}^{[n]}=\Delta t\sum_{j=0}^{s-1}b_{ij}\boldsymbol{F}_j+\sum_{j=0}^{r-1}v_{ij}
         * \boldsymbol{\hat{y}}_{j}^{[n-1]}, \qquad i=0,1,\ldots,r-1
         * \f}
         * where \f$\boldsymbol{Y}_i\f$ are referred to as the stage values and \f$\boldsymbol{F}_i\f$ as 
         * the stage derivatives. Both quantities are related by the differential equation:
         * \f{displaymath}
         * \boldsymbol{F}_i = \boldsymbol{\hat{f}}(\boldsymbol{Y}_i).
         * \f}
         * The matrices \f$A=[a_{ij}]\f$, \f$U=[u_{ij}]\f$, \f$B=[b_{ij}]\f$, \f$V=[v_{ij}]\f$ are 
         * characteristic of a specific method.
         * Furthermore, the following two quantities serve as input and output of step \f$n\f$:
         * - <b>output</b>: \f$\boldsymbol{\hat{y}}_{j}^{[n]}\f$ the new set of approximations.
         *    - \f$\boldsymbol{\hat{y}}_{0}^{[n]}\f$ refers to the approximation \f$\boldsymbol{\hat{y}}(t+n\Delta t)\f$. 
         *      This is the solution were are looking for.
         *    - \f$\boldsymbol{\hat{y}}_{j}^{[n]}\f$ (\f$j=1,\ldots,r-1\f$) refers to the approximation at
         *       the current time-level of an auxiliary set of parameters inherent to the method.
         * - <b>input</b>: \f$\boldsymbol{\hat{y}}_{j}^{[n-1]}\f$ the set of approximations calculated at step \f$n-1\f$.
         *    - \f$\boldsymbol{\hat{y}}_{0}^{[n-1]}\f$ refers to the approximation of the solution at time 
         *      level \f$n-1\f$ calculated during the step \f$n-1\f$.
         *    - \f$\boldsymbol{\hat{y}}_{j}^{[n-1]}\f$ (\f$j=1,\ldots,r-1\f$) refers to the approximation 
         *      of the auxiliary parameters calculated during the step \f$n-1\f$.
         *
         * \subsection subsectionGeneralLinearMethodsMatrixNotation Matrix notation
         * Adopting the notation:
         * \f{displaymath}
         * \boldsymbol{\hat{y}}^{[n-1]}=
         * \left[\begin{array}{c}
         * \boldsymbol{\hat{y}}^{[n-1]}_0\\
         * \boldsymbol{\hat{y}}^{[n-1]}_1\\
         * \vdots\\
         * \boldsymbol{\hat{y}}^{[n-1]}_{r-1}
         * \end{array}\right],\quad
         * \boldsymbol{\hat{y}}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{\hat{y}}^{[n]}_0\\
         * \boldsymbol{\hat{y}}^{[n]}_1\\
         * \vdots\\
         * \boldsymbol{\hat{y}}^{[n]}_{r-1}
         * \end{array}\right],\quad
         * \boldsymbol{Y}=
         * \left[\begin{array}{c}
         * \boldsymbol{Y}_0\\
         * \boldsymbol{Y}_1\\
         * \vdots\\
         * \boldsymbol{Y}_{s-1}
         * \end{array}\right],\quad
         * \boldsymbol{F}=
         * \left[\begin{array}{c}
         * \boldsymbol{F}_0\\
         * \boldsymbol{F}_1\\
         * \vdots\\
         * \boldsymbol{F}_{s-1}
         * \end{array}\right]\quad
         * \f}
         * the general linear method can be written more compactly in the following form:
         * \f{displaymath}
         * \left[ \begin{array}{c}
         * \boldsymbol{Y}\\
         * \boldsymbol{\hat{y}}^{[n]}
         * \end{array}\right] =
         * \left[ \begin{array}{cc}
         * A\otimes I & U\otimes I \\
         * B\otimes I & V\otimes I
         * \end{array}\right]
         * \left[ \begin{array}{c}
         * \Delta t\boldsymbol{F}\\
         * \boldsymbol{\hat{y}}^{[n-1]}
         * \end{array}\right]
         * \f}
         * where \f$I\f$ is the identity matrix of dimension \f$N\times N\f$.
         *
         * \subsection subsectionGeneralLinearMethodsEvaluation Evaluation of an General Linear Method
         * Although the General linear method is essentially presented for ODE's in its autonomous form, in 
         * Nektar++ it will be used to solve ODE's formulated in non-autonomous form.
         * Given the ODE,
         * \f[
         * \frac{d\boldsymbol{y}}{dt}=\boldsymbol{f}(t,\boldsymbol{y}),\quad \boldsymbol{y}(t_0)=\boldsymbol{y}_0
         * \f]
         * a single step of General Linear Method can then be evaluated in the following way:<BR><BR>
         * - <b>input</b> 
         *   - \f$\boldsymbol{y}^{[n-1]}\f$, i.e. the \f$r\f$ subvectors comprising \f$\boldsymbol{y}^{[n-1]}_i\f$
         *   - \f$t^{[n-1]}\f$, i.e. the equivalent of \f$\boldsymbol{y}^{[n-1]}\f$ for the time variable \f$t\f$
         * - <b>step 1:</b> The stage values \f$\boldsymbol{Y}_i\f$, \f$\boldsymbol{T}_i\f$ 
         *   and the stage derivatives \f$\boldsymbol{F}_i\f$ are calculated through the relations:
         * \f{eqnarray*}
         * \boldsymbol{Y}_i &=& \Delta t\sum_{j=0}^{s-1}a_{ij}\boldsymbol{F}_j+\sum_{j=0}^{r-1}u_{ij}\boldsymbol{y}_{j}^{[n-1]}, \qquad i=0,1,\ldots,s-1\\
         * T_i &=& \Delta t\sum_{j=0}^{s-1}a_{ij}+\sum_{j=0}^{r-1}u_{ij}
         * t_{j}^{[n-1]}, \qquad i=0,1,\ldots,s-1\\
         * \boldsymbol{F}_i &=& f(T_i,\boldsymbol{Y}_i), \qquad i=0,1,\ldots,s-1
         * \f}        
         * - <b>step 2:</b> The approximation at the new time level \f$\boldsymbol{y}^{[n]}\f$ is calculated 
         * as a linear combination of the stage derivatives \f$\boldsymbol{F}_i\f$ and the input vector \f$\boldsymbol{y}^{[n-1]}\f$.
         * In addition, the time vector \f$t^{[n]}\f$ is also updated
         * \f{eqnarray*}
         * \boldsymbol{y}_{i}^{[n]}&=&\Delta t\sum_{j=0}^{s-1}b_{ij}\boldsymbol{F}_j+
         * \sum_{j=0}^{r-1}v_{ij}\boldsymbol{y}_{j}^{[n-1]}, \qquad i=0,1,\ldots,r-1\\
         * t_{i}^{[n]}&=&\Delta t\sum_{j=0}^{s-1}b_{ij}+
         * \sum_{j=0}^{r-1}v_{ij}t_{j}^{[n-1]}, \qquad i=0,1,\ldots,r-1\\
         * \f}
         * - <b>output</b> 
         *   - \f$\boldsymbol{y}^{[n]}\f$, i.e. the \f$r\f$ subvectors comprising \f$\boldsymbol{y}^{[n]}_i\f$. 
         *     \f$\boldsymbol{y}^{[n-1]}_0\f$ corresponds to the actual approximation at the new time level.
         *   - \f$t^{[n]}\f$ where \f$t^{[n]}_0\f$ is equal to the new time level \f$t+\Delta t\f$
         *
         * \section sectionGeneralLinearMethodsNektar General Linear Methods in Nektar++
         * In Nektar++, we do not use the standard General Linear Methods formulation. First of all as mentioned above, 
         * we will use the non-autonomous form as the explicit treatment of the time variable \f$t\f$
         * allows for more flexibility. And secondly, we generalise the concept a General Linear Methods
         * in order to solve ODE's of the form:
         * \f[
         * \boldsymbol{m}(t,\frac{d\boldsymbol{y}}{dt})=\boldsymbol{l}(t,\boldsymbol{y})
         * \f]
         * This 
         * - is a more natural formulation for ODE's originating from time-dependent PDE's
         * - allows for an easier treatment of strongly imposed essential boundary conditions for such PDE's
         *
         * \subsection sectionGeneralLinearMethodsNektarFormulation Formulation
         * Applied to the ODE above, the General Linear Method can now be formulated as
         * \f{eqnarray*}
         * \boldsymbol{m}(T_i,\boldsymbol{Y}_i) &=& \Delta t\sum_{j=0}^{s-1}a_{ij}\boldsymbol{F}_j+\sum_{j=0}^{r-1}u_{ij}\boldsymbol{y}_{j}^{[n-1]}, \qquad i=0,1,\ldots,s-1\\
         * T_i &=& \Delta t\sum_{j=0}^{s-1}a_{ij}+\sum_{j=0}^{r-1}u_{ij}
         * t_{j}^{[n-1]}, \qquad i=0,1,\ldots,s-1\\
         * \boldsymbol{F}_i &=& l(T_i,\boldsymbol{Y}_i), \qquad i=0,1,\ldots,s-1\\
         * \boldsymbol{y}_{i}^{[n]}&=&\Delta t\sum_{j=0}^{s-1}b_{ij}\boldsymbol{F}_j+
         * \sum_{j=0}^{r-1}v_{ij}\boldsymbol{y}_{j}^{[n-1]}, \qquad i=0,1,\ldots,r-1\\
         * t_{i}^{[n]}&=&\Delta t\sum_{j=0}^{s-1}b_{ij}+
         * \sum_{j=0}^{r-1}v_{ij}t_{j}^{[n-1]}, \qquad i=0,1,\ldots,r-1\\
         * \f}
         * It is very important to note that this has the following implications to the output vector (and similarly
         * for the input vector):
         * - <b>output</b>: \f$\boldsymbol{y}_{j}^{[n]}\f$ the new set of approximations.
         *    - \f$\boldsymbol{y}_{0}^{[n]}\f$ now refers to the approximation of \f$\boldsymbol{m}(t_n,\boldsymbol{y}(t_n))\f$. 
         *      This is <i>NOT</i> the solution were are looking for.
         *    - \f$\boldsymbol{y}_{j}^{[n]}\f$ (\f$j=1,\ldots,r-1\f$) refers to the approximation at
         *       the current time-level of an auxiliary set of parameters inherent to the method. If this parameter
         *       in the original method represents the approximation at an earlier time level, say \f$\boldsymbol{y}(t_{n-2})\f$,
         *       it will now represent \f$\boldsymbol{m}(t_{n-2},\boldsymbol{y}(t_{n-2}))\f$ in the generalised formulation.
         * 
         * This means that an extra step is required to calculate the actual approximation at the new time level. This can be done by solving 
         * the the type system of below for \f$\boldsymbol{y}\f$:
         * \f[
         * \boldsymbol{m}(t,\boldsymbol{y}) = \boldsymbol{b}
         * \f]
         *
         * \subsection sectionGeneralLinearMethodsNektarSchemes Type of Time Integration Schemes
         * Nektar++ contains various classes and methods which implement the concept of the General
         * Linear Methods. This toolbox is capable of numerically solving the generalised ODE using a broad range
         * of different time-stepping methods. We distinguish the following types of General Linear Methods:
         * - <b>Formally Explicit Methods</b><BR>
         *   These types of methods are considered explicit from an ODE point of view. They are characterised by a lower
         *   triangular coefficient matrix \f$A\f$, i.e. \f$a_{ij} = 0\f$ for \f$j\geq i\f$. To avoid confusion, we
         *   make a further distinction:
         *   - <i>direct explicit method</i>: \f$\boldsymbol{m}(t,\frac{d\boldsymbol{y}}{dt})=\frac{d\boldsymbol{y}}{dt}\f$
         *     Only forward operators are required.
         *   - <i>indirect explicit method</i>: The inverse operator \f$\boldsymbol{m}^{-1}\f$ is required.
         * - <b>Diagonally Implicit Methods</b><BR>
         *   Compared to the explicit methods, the coefficient matrix \f$A\f$ has now non-zero entries on the diagonal.
         *   This means that each stage value depend on the stage derivative at the same stage, requiring an implicit
         *   step. However, the calculation of the different stage values is still uncoupled. Best known are the
         *   DIRK schemes.
         * - <b>IMEX schemes</b><BR>
         *  <i>information to be added</i>
         * - <b>Fully Implicit Methods Methods</b><BR>
         *   The coefficient matrix has a non-zero upper triangular part. The calculation of all stages values is fully coupled. 
         *
         * The aim in Nektar++ is to fully support the first two (three?) types of General Linear Methods. 
         * Fully implicit methods are currently not implemented.
         *
         * \subsection sectionGeneralLinearMethodsNektarSchemesHowTo How to use
         * The goal of abstracting the concept of General Linear Methods is to provide users with a single interface 
         * for time-stepping, independent of the chosen method. The TimeIntegrationScheme class allow the user to numerically
         * integrate ODE's using high-order complex schemes, as if it were done using the forward euler method.
         * Switching between time-stepping schemes should be as easy as changing a parameter in an input file.
         * The only thing the user should provide, is an implementation of the left and right hand side operation
         * of the generalised ODE to be solved. 
         *
         * To introduce the implementation of time stepping schemes in Nektar++, consider the following example:
         * \code
         * NekDouble timestepsize = 0.1;
         * NekDouble time         = 0.0;
         * Array<OneD, Array<OneD, NekDouble> >  y_t0;
         * Array<OneD, Array<OneD, NekDouble> >  y_0;
         * SomeClass object;
         *        
         * LibUtilities::TimeIntegrationSchemeKey       IntKey(LibUtilities::eClassicalRungeKutta4);
         * LibUtilities::TimeIntegrationSchemeSharedPtr IntScheme = LibUtilities::TimeIntegrationSchemeManager()[IntKey];
         *  
         * int nInitSteps;
         * LibUtilities::TimeIntegrationSolutionSharedPtr y = IntScheme->InitializeScheme(timestepsize,time,nInitSteps,object,y_t0);
         *
         * for(n = nInitSteps; n < nsteps; ++n)
         * {
         *  y0 = IntScheme->ExplicitIntegration(m_timestep,object,y);
         *  time += timestepsize;
         * }
         * \endcode
         * <BR>
         * We can distinguish four different sections in the code above:
         * - <b>Initialisation</b>
         * \code
         * NekDouble timestepsize = 0.1;
         * NekDouble time         = 0.0;
         * Array<OneD, Array<OneD, NekDouble> >  y_t0;
         * Array<OneD, Array<OneD, NekDouble> >  y_0;
         * Foo bar;
         * \endcode
         * The object <code>y_t0</code> should contain the solution at the initial time t0.
         * The object <code>y_0</code> will later be used to store the solution at every time level.
         * It corresponds to the vector \f$\boldsymbol{y}^{[n]}_0\f$ defined above.
         * Both <code>y_t0</code> and <code>y_0</code> are Array of Arrays where the first dimension
         * corresponds to the number of variables (eg. u,v,w) and
         * the second dimension corresponds to the number length of the variables
         * (e.g. the number of modes).<BR>
         * The variable <code>bar</code> is the object of some class <code>Foo</code>. This class
         * Foo should have some a series of methods called representing the ODE. The user should make sure
         * his class contains a proper implementation of these necessary methods. For more information
         * about these methods, click \ref sectionGeneralLinearMethodsNektarSchemesRequiredMethods "here".
         * - <b>Loading the time integration scheme</b>   
         * \code
         * LibUtilities::TimeIntegrationSchemeKey       IntKey(LibUtilities::eClassicalRungeKutta4);
         * LibUtilities::TimeIntegrationSchemeSharedPtr IntScheme = LibUtilities::TimeIntegrationSchemeManager()[IntKey];
         * \endcode
         * First, a key which uniquely defines the desired time integration scheme is created. Then, Using this key, 
         * the TimeIntegrationSchemeManager is called which will return you the requested time integration scheme. 
         * The concept of a time integration scheme is abstracted as the 
         * class TimeIntegrationScheme. Objects of this class will hold the proper
         * integration coefficients and know how to perform integration.
         * - <b>Initialising the time integration scheme</b>        
         * \code 
         * int nInitSteps;
         * LibUtilities::TimeIntegrationSolutionSharedPtr y = IntScheme->InitializeScheme(timestepsize,time,nInitSteps,bar,y_t0);
         * \endcode
         * Given the initial solution and some additional parameters, this method constructs and returns an object <code>y</code> which is
         * the abstraction of the vectors \f$\boldsymbol{y}^{[n]}\f$ and \f$t^{[n]}\f$. This abstraction is done through the class
         * <code>TimeIntegrationSolution</code>. This initialisation is essential in case of multi-step schemes, where the vector
         * \f$\boldsymbol{y}^{[n]}\f$ consist of more than one entry. The object <code>y</code> can than later be passed to the actual 
         * integration method, as it contains all necessary input.
         * - <b>Perform the time integration</b>         
         * \code
         * for(n = nInitSteps; n < nsteps; ++n)
         * {
         *  y0 = IntScheme->ExplicitIntegration(m_timestep,object,y);
         *  time += timestepsize;
         * }
         * \endcode
         * The actual explicit time integration can now be done by looping over the function call above.
         * This function updates the object <code>y</code> every time step to hold the vectors
         * \f$\boldsymbol{y}^{[n]}\f$ and \f$t^{[n]}\f$ at every time level <code>n</code>. In addition, it also 
         * returns the actual solution \f$\boldsymbol{y}^{[n]}_0\f$ (which in fact is also embedded in the 
         * object <code>y</code>)
         *
         *
         * \subsection sectionGeneralLinearMethodsNektarSchemesRequiredMethods Required Methods
         * An object <code>bar</code> of the class <code>Foo</code> should be passed to the TimeIntegrationScheme
         * class routines. This class <code>Foo</code> should have the following public member functions:
         * - <code>Foo::ODErhs</code>
         *   \code    
         *   void Foo::ODErhs(const Array<OneD, const Array<OneD, NekDouble> >& inarray,  
         *                          Array<OneD,       Array<OneD, NekDouble> >& outarray, 
         *                    const NekDouble time);
         *   \endcode
         *   This function should which evaluate the right hand side operator \f$\boldsymbol{l}(t,\boldsymbol{y})\f$ 
         *   of the generalised ODE. 
         *   - Input Parameters
         *     - <code>inarray</code>: the vector \f$\boldsymbol{y}\f$
         *     - <code>time</code>: the time \f$t\f$ 
         *   - Output Parameters
         *     - <code>outarray</code>: the result \f$\boldsymbol{l}(t,\boldsymbol{y})\f$ 
         *     .
         *   .
         *   Both <code>inarray</code> and <code>outarray</code> are Array of Arrays where the first dimension
         *   corresponds to the number of variables (eg. u,v,w) and
         *   the second dimension corresponds to the number length of the variables
         *   (e.g. the number of modes).
         * - <code>Foo::ODElhs</code>
         *   \code    
         *   void Foo::ODElhs(const Array<OneD, const Array<OneD, NekDouble> >& inarray,  
         *                          Array<OneD,       Array<OneD, NekDouble> >& outarray, 
         *                    const NekDouble time);
         *   \endcode
         *   This function should which evaluate the left hand side operator \f$\boldsymbol{m}(t,\boldsymbol{y})\f$ 
         *   of the generalised ODE. 
         *   - Input Parameters
         *     - <code>inarray</code>: the vector \f$\boldsymbol{y}\f$
         *     - <code>time</code>: the time \f$t\f$ 
         *   - Output Parameters
         *     - <code>outarray</code>: the result \f$\boldsymbol{m}(t,\boldsymbol{y})\f$ 
         *     .
         *   .
         *   Both <code>inarray</code> and <code>outarray</code> are Array of Arrays where the first dimension
         *   corresponds to the number of variables (eg. u,v,w) and
         *   the second dimension corresponds to the number length of the variables
         *   (e.g. the number of modes).
         * - <code>Foo::ODElhsSolve</code>
         *   \code    
         *   void Foo::ODElhsSolve(const Array<OneD, const Array<OneD, NekDouble> >& inarray,  
         *                               Array<OneD,       Array<OneD, NekDouble> >& outarray, 
         *                         const NekDouble time);
         *   \endcode
         *   This function should solve the system \f$\boldsymbol{m}(t,\boldsymbol{y}) = \boldsymbol{b}\f$
         *   for the unknown vector \f$\boldsymbol{y}\f$. For linear operators, this can be thought of
         *   as the inverse matrix operator, i.e.
         *   \f[
         *   \boldsymbol{y} = \boldsymbol{M}^{-1}\boldsymbol{b}
         *   \f]
         *   - Input Parameters
         *     - <code>inarray</code>: the vector \f$\boldsymbol{b}\f$
         *     - <code>time</code>: the time \f$t\f$ 
         *   - Output Parameters
         *     - <code>outarray</code>: the result \f$\boldsymbol{y}\f$
         *     .
         *   .
         *   Both <code>inarray</code> and <code>outarray</code> are Array of Arrays where the first dimension
         *   corresponds to the number of variables (eg. u,v,w) and
         *   the second dimension corresponds to the number length of the variables
         *   (e.g. the number of modes).
         * - <code>Foo::ODEdirkSolve</code>
         *   \code    
         *   void Foo::ODEdirkSolve(const Array<OneD, const Array<OneD, NekDouble> >& inarray,  
         *                                Array<OneD,       Array<OneD, NekDouble> >& outarray, 
         *                          const NekDouble time,
         *                          const NekDouble lambda);
         *   \endcode
         *   This method will be needed for diagonally implicit methods. The stage values in these methods
         *   should be calculated through the relation:
         *   \f[
         *   \boldsymbol{m}(T_i,\boldsymbol{Y}_i) = \Delta t a_{ii} \boldsymbol{l}(T_i,\boldsymbol{Y}_i)+ \Delta t\sum_{j=0}^{i-1}a_{ij}\boldsymbol{l}(T_j,\boldsymbol{Y}_j)+\sum_{j=0}^{r-1}u_{ij}\boldsymbol{y}_{j}^{[n-1]}, \qquad i=0,1,\ldots,s-1
         *   \f]
         *   Moving the terms in \f$\boldsymbol{Y}_i\f$ to the left hand side of the equation, it can be appreciated
         *   that we need a method which can solve:
         *   \f[
         *   \boldsymbol{m}(t,\boldsymbol{y}) - \lambda \boldsymbol{l}(t,\boldsymbol{y}) = \boldsymbol{b}
         *   \f]
         *   for the unknown vector \f$\boldsymbol{y}\f$. <code>Foo::ODEdirkSolve</code> is the method
         *   designed to do this. For linear operators, this can be thought of
         *   as the inverse matrix operator, i.e.
         *   \f[
         *   \boldsymbol{y} = \left(\boldsymbol{M}-\lambda \boldsymbol{L}\right)^{-1}\boldsymbol{b}
         *   \f]
         *   - Input Parameters
         *     - <code>inarray</code>: the vector \f$\boldsymbol{b}\f$
         *     - <code>time</code>: the time \f$t\f$ 
         *     - <code>lambda</code>: the coefficient \f$\lambda\f$ 
         *   - Output Parameters
         *     - <code>outarray</code>: the result \f$\boldsymbol{y}\f$
         *     .
         *   .
         *   Both <code>inarray</code> and <code>outarray</code> are Array of Arrays where the first dimension
         *   corresponds to the number of variables (eg. u,v,w) and
         *   the second dimension corresponds to the number length of the variables
         *   (e.g. the number of modes).
         * 
         * \subsection sectionGeneralLinearMethodsImplementationEssentialBC Strongly imposed essential boundary conditions.
         * Dirichlet boundary conditions can be strongly imposed by lifting the known Dirichlet solution.
         * This is equivalent to decompose the approximate solution \f$y\f$ into an known lifted function, 
         * \f$y^{\mathcal{D}}\f$, which satisfies the Dirichlet boundary conditions, and an unknown 
         * homogeneous function, \f$y^{\mathcal{D}}\f$, which
         * is zero on the Dirichlet boundaries, i.e.
         * \f[
         * y = y^{\mathcal{D}} + y^{\mathcal{H}}
         * \f]
         * In a Finite Element discretisation, this corresponds to splitting the solution vector of coefficients \f$\boldsymbol{y}\f$
         * into the known Dirichlet degrees of freedom \f$\boldsymbol{y}^{\mathcal{D}}\f$ and the unknown homogeneous 
         * degrees of freedom \f$\boldsymbol{y}^{\mathcal{H}}\f$. If ordering the known coefficients first, this corresponds to:
         * \f[
         * \boldsymbol{y} = \left[ \begin{array}{c}
         * \boldsymbol{y}^{\mathcal{D}} \\
         * \boldsymbol{y}^{\mathcal{H}} \end{array} \right]
         * \f]
         * The generalised formulation of the General Linear Method (i.e. the introduction of a left hand side operator)
         * allows for an easier treatment of these types of boundary conditions. To better appreciate this, consider the 
         * equation for the stage values for an explicit general linear method where both the left and right hand side operator 
         * are linear operators,
         * i.e. they can be represented by a matrix.
         * \f[
         * \boldsymbol{M}\boldsymbol{Y}_i = \Delta t\sum_{j=0}^{i-1}a_{ij}\boldsymbol{L}\boldsymbol{Y}_j+\sum_{j=0}^{r-1}u_{ij}\boldsymbol{y}_{j}^{[n-1]}, \qquad i=0,1,\ldots,s-1
         * \f] 
         * In case of a lifted known solution, this can be written as:
         * \f[
         * \left[ \begin{array}{cc}
         * \boldsymbol{M}^{\mathcal{DD}} & \boldsymbol{M}^{\mathcal{DH}} \\
         * \boldsymbol{M}^{\mathcal{HD}} & \boldsymbol{M}^{\mathcal{HH}} \end{array} \right]
         * \left[ \begin{array}{c}
         * \boldsymbol{Y}^{\mathcal{D}}_i \\
         * \boldsymbol{Y}^{\mathcal{H}}_i \end{array} \right]
         * = \Delta t\sum_{j=0}^{i-1}a_{ij}
         * \left[ \begin{array}{cc}
         * \boldsymbol{L}^{\mathcal{DD}} & \boldsymbol{L}^{\mathcal{DH}} \\
         * \boldsymbol{L}^{\mathcal{HD}} & \boldsymbol{L}^{\mathcal{HH}} \end{array} \right]
         * \left[ \begin{array}{c}
         * \boldsymbol{Y}^{\mathcal{D}}_j \\
         * \boldsymbol{Y}^{\mathcal{H}}_j \end{array} \right]
         * +\sum_{j=0}^{r-1}u_{ij}
         * \left[ \begin{array}{c}
         * \boldsymbol{y}^{\mathcal{D}[n-1]}_j \\
         * \boldsymbol{y}^{\mathcal{H}[n-1]}_j \end{array} \right], \qquad i=0,1,\ldots,s-1
         * \f] 
         * In order to calculate the stage values correctly, the \ref sectionGeneralLinearMethodsNektarSchemesRequiredMethods
         *  should now be implemented to do the following: 
         * 
         * - <code>Foo::ODElhsSolve</code>
         *   \code    
         *   void Foo::ODElhsSolve(const Array<OneD, const Array<OneD, NekDouble> >& inarray,  
         *                               Array<OneD,       Array<OneD, NekDouble> >& outarray, 
         *                         const NekDouble time);
         *   \endcode
         *   This function should now solve the system:
         *   \f[
         *   \left[ \begin{array}{cc}
         *   \boldsymbol{M}^{\mathcal{DD}} & \boldsymbol{M}^{\mathcal{DH}} \\
         *   \boldsymbol{M}^{\mathcal{HD}} & \boldsymbol{M}^{\mathcal{HH}} \end{array} \right]
         *   \left[ \begin{array}{c}
         *   \boldsymbol{y}^{\mathcal{D}} \\
         *   \boldsymbol{y}^{\mathcal{H}} \end{array} \right]
         *   = 
         *   \left[ \begin{array}{c}
         *   \boldsymbol{b}^{\mathcal{D}} \\
         *   \boldsymbol{b}^{\mathcal{H}} \end{array} \right]
         *   \f] 
         *   for the unknown vector \f$\boldsymbol{y}\f$. This can be done in three steps:
         *   -# Set the known solution \f$\boldsymbol{y}^{\mathcal{D}}\f$
         *   -# Calculate the modified right hand side term:
         *      \f[  \boldsymbol{b}^{\mathcal{H}} - \boldsymbol{M}^{\mathcal{HD}}\boldsymbol{y}^{\mathcal{D}} \f]
         *   -# Solve the system below for the unknown \f$\boldsymbol{y}^{\mathcal{H}}\f$,
         *      \f[ \boldsymbol{M}^{\mathcal{HH}}\boldsymbol{y}^{\mathcal{H}} = \boldsymbol{b}^{\mathcal{H}} - \boldsymbol{M}^{\mathcal{HD}}\boldsymbol{y}^{\mathcal{D}} \f]
         * - <code>Foo::ODErhs</code>
         *   \code    
         *   void Foo::ODErhs(const Array<OneD, const Array<OneD, NekDouble> >& inarray,  
         *                          Array<OneD,       Array<OneD, NekDouble> >& outarray, 
         *                    const NekDouble time);
         *   \endcode
         *   This function should now evaluate the right hand side operator in order to calculate:
         *   \f[
         *   \left[ \begin{array}{c}
         *   \boldsymbol{b}^{\mathcal{D}} \\
         *   \boldsymbol{b}^{\mathcal{H}} \end{array} \right]
         *    =
         *   \left[ \begin{array}{cc}
         *   \boldsymbol{L}^{\mathcal{DD}} & \boldsymbol{L}^{\mathcal{DH}} \\
         *   \boldsymbol{L}^{\mathcal{HD}} & \boldsymbol{L}^{\mathcal{HH}} \end{array} \right]
         *   \left[ \begin{array}{c}
         *   \boldsymbol{y}^{\mathcal{D}} \\
         *   \boldsymbol{y}^{\mathcal{H}} \end{array} \right]
         *   \f] 
         *   Note that only the homogeneous part \f$\boldsymbol{b}^{\mathcal{H}}\f$ will be used to 
         *   calculate the stage values, as seen in the <code>Foo::ODElhsSolve</code> method. This means 
         *   essentially, only the bottom part of the operation above, i.e.
         *   \f[ \boldsymbol{L}^{\mathcal{HD}}\boldsymbol{y}^{\mathcal{D}} + \boldsymbol{L}^{\mathcal{HH}}\boldsymbol{y}^{\mathcal{H}} \f]
         *   is required. However, sometimes it might be more convenient to use/implement routines for the
         *   <code>Foo::ODErhs</code> method that also calculate \f$\boldsymbol{b}^{\mathcal{D}}\f$.
         * - <code>Foo::ODElhs</code>
         *   \code    
         *   void Foo::ODElhs(const Array<OneD, const Array<OneD, NekDouble> >& inarray,  
         *                          Array<OneD,       Array<OneD, NekDouble> >& outarray, 
         *                    const NekDouble time);
         *   \endcode
         *   This function should now evaluate the left hand side operator in order to calculate:
         *   \f[
         *   \left[ \begin{array}{c}
         *   \boldsymbol{b}^{\mathcal{D}} \\
         *   \boldsymbol{b}^{\mathcal{H}} \end{array} \right]
         *    =
         *   \left[ \begin{array}{cc}
         *   \boldsymbol{M}^{\mathcal{DD}} & \boldsymbol{M}^{\mathcal{DH}} \\
         *   \boldsymbol{M}^{\mathcal{HD}} & \boldsymbol{M}^{\mathcal{HH}} \end{array} \right]
         *   \left[ \begin{array}{c}
         *   \boldsymbol{y}^{\mathcal{D}} \\
         *   \boldsymbol{y}^{\mathcal{H}} \end{array} \right]
         *   \f] 
         *   Note that only the homogeneous part \f$\boldsymbol{b}^{\mathcal{H}}\f$ will be used in the
         *   time stepping algorithms, as seen in the <code>Foo::ODElhsSolve</code> method. This means 
         *   essentially, only the bottom part of the operation above, i.e.
         *   \f[ \boldsymbol{M}^{\mathcal{HD}}\boldsymbol{y}^{\mathcal{D}} + \boldsymbol{M}^{\mathcal{HH}}\boldsymbol{y}^{\mathcal{H}} \f]
         *   is required. However, sometimes it might be more convenient to use/implement routines for the
         *   <code>Foo::ODElhs</code> method that also calculate \f$\boldsymbol{b}^{\mathcal{D}}\f$.
         * - <code>Foo::ODEdirkSolve</code>
         *   \code    
         *   void Foo::ODEdirkSolve(const Array<OneD, const Array<OneD, NekDouble> >& inarray,  
         *                                Array<OneD,       Array<OneD, NekDouble> >& outarray, 
         *                          const NekDouble time,
         *                          const NekDouble lambda);
         *   \endcode
         *   This method, needed for diagonally implicit methods, should now solve the system:
         *   \f[
         *   \left(\left[ \begin{array}{cc}
         *   \boldsymbol{M}^{\mathcal{DD}} & \boldsymbol{M}^{\mathcal{DH}} \\
         *   \boldsymbol{M}^{\mathcal{HD}} & \boldsymbol{M}^{\mathcal{HH}} \end{array} \right]
         *   - \lambda \left[ \begin{array}{cc}
         *   \boldsymbol{L}^{\mathcal{DD}} & \boldsymbol{L}^{\mathcal{DH}} \\
         *   \boldsymbol{L}^{\mathcal{HD}} & \boldsymbol{L}^{\mathcal{HH}} \end{array} \right]\right)
         *   \left[ \begin{array}{c}
         *   \boldsymbol{y}^{\mathcal{D}} \\
         *   \boldsymbol{y}^{\mathcal{H}} \end{array} \right]
         *   = 
         *   \left[ \begin{array}{cc}
         *   \boldsymbol{H}^{\mathcal{DD}} & \boldsymbol{H}^{\mathcal{DH}} \\
         *   \boldsymbol{H}^{\mathcal{HD}} & \boldsymbol{H}^{\mathcal{HH}} \end{array} \right]
         *   \left[ \begin{array}{c}
         *   \boldsymbol{y}^{\mathcal{D}} \\
         *   \boldsymbol{y}^{\mathcal{H}} \end{array} \right]
         *   = 
         *   \left[ \begin{array}{c}
         *   \boldsymbol{b}^{\mathcal{D}} \\
         *   \boldsymbol{b}^{\mathcal{H}} \end{array} \right]
         *   \f] 
         *   for the unknown vector \f$\boldsymbol{y}\f$. This can be done in three steps:
         *   -# Set the known solution \f$\boldsymbol{y}^{\mathcal{D}}\f$
         *   -# Calculate the modified right hand side term:
         *      \f[  \boldsymbol{b}^{\mathcal{H}} - \boldsymbol{H}^{\mathcal{HD}}\boldsymbol{y}^{\mathcal{D}} \f]
         *   -# Solve the system below for the unknown \f$\boldsymbol{y}^{\mathcal{H}}\f$,
         *      \f[ \boldsymbol{H}^{\mathcal{HH}}\boldsymbol{y}^{\mathcal{H}} = \boldsymbol{b}^{\mathcal{H}} - \boldsymbol{H}^{\mathcal{HD}}\boldsymbol{y}^{\mathcal{D}} \f]
         * 
         *
         * \subsection sectionGeneralLinearMethodsImplementationSchemes Implemented integration schemes
         * Currently the time integration schemes below are implemented in Nektar++. We will list their coefficients here
         * and we will also indicate what the auxiliary parameters \f$\boldsymbol{\hat{y}}_{j}^{[n]}\f$ (\f$j=1,\ldots,r-1\f$) 
         * of the multistep methods represent:
         * - Forward Euler (or 1st order Adams Bashfort)
         *   - enum value: <code>eForwardEuler</code>, <code>eAdamsBashforthOrder1</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{c|c}
         * A & U \\
         * \hline 
         * B & V
         * \end{array}\right] = 
         * \left[\begin{array}{c|c}
         * 0 & 1 \\
         * \hline
         * 1 & 1 
         * \end{array}\right],\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)
         * \end{array}\right]
         * \f]
         * - Backward Euler (or 1st order Adams Moulton)
         *   - enum value: <code>eBackwardEuler</code>, <code>eAdamsMoultonOrder1</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{c|c}
         * A & U \\
         * \hline 
         * B & V
         * \end{array}\right] = 
         * \left[\begin{array}{c|c}
         * 1 & 1 \\
         * \hline
         * 1 & 1 
         * \end{array}\right],\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)
         * \end{array}\right]
         * \f]
         * - 2nd order Adams Bashforth
         *   - enum value: <code>eAdamsBashforthOrder2</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{c|c}
         * A & U \\
         * \hline 
         * B & V
         * \end{array}\right] = 
         * \left[\begin{array}{c|cc}
         * 0 & 1 & 0 \\
         * \hline 
         * \frac{3}{2} & 1 & \frac{-1}{2}  \\
         * 1 & 0 & 0 
         * \end{array}\right],\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0\\
         * \boldsymbol{y}^{[n]}_1\\
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)\\
         * \Delta t \boldsymbol{l}(t^{n-1},\boldsymbol{y}^{n-1})
         * \end{array}\right]
         * \f]
         * - 2nd order Adams Moulton
         *   - enum value: <code>eAdamsMoultonOrder2</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{c|c}
         * A & U \\
         * \hline 
         * B & V
         * \end{array}\right] = 
         * \left[\begin{array}{c|cc}
         * \frac{1}{2} & 1 & \frac{1}{2} \\
         * \hline
         * \frac{1}{2} & 1 & \frac{1}{2} \\
         * 1 & 0 & 0 
         * \end{array}\right],\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0\\
         * \boldsymbol{y}^{[n]}_1\\
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)\\
         * \Delta t \boldsymbol{l}(t^{n},\boldsymbol{y}^{n})
         * \end{array}\right]
         * \f]
         * - the midpoint method 
         *   - enum value: <code>eMidpoint</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{c|c}
         * A & U \\
         * \hline 
         * B & V
         * \end{array}\right] = 
         * \left[\begin{array}{cc|c}
         * 0 & 0 & 1 \\
         * \frac{1}{2} & 0 & 1 \\
         * \hline
         * 0 & 1 & 1 
         * \end{array}\right],\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)
         * \end{array}\right]
         * \f]
         * - RK4: the standard fourth order Runge-Kutta scheme 
         *   - enum value: <code>eClassicalRungeKutta4</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{c|c}
         * A & U \\
         * \hline 
         * B & V
         * \end{array}\right] = 
         * \left[\begin{array}{cccc|c}
         * 0 & 0 & 0 & 0 & 1 \\
         * \frac{1}{2} & 0 & 0 & 0 & 1 \\
         * 0 & \frac{1}{2} & 0 & 0 & 1 \\
         * 0 & 0 & 1 & 0 & 1 \\
         * \hline
         * \frac{1}{6} & \frac{1}{3} & \frac{1}{3} & \frac{1}{6} & 1 
         * \end{array}\right],\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)
         * \end{array}\right]
         * \f]
         * - 2nd order Diagonally Implicit Runge Kutta (DIRK) 
         *   - enum value: <code>eDIRKOrder2</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{c|c}
         * A & U \\
         * \hline 
         * B & V
         * \end{array}\right] = 
         * \left[\begin{array}{cc|c}
         * \lambda & 0 & 1 \\
         * \left(1-\lambda\right) & \lambda & 1 \\
         * \hline 
         * \left(1-\lambda\right) & \lambda & 1 
         * \end{array}\right]\quad \mathrm{with}\quad \lambda=\frac{2-\sqrt{2}}{2},\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)
         * \end{array}\right]
         * \f]
         * - 3rd order Diagonally Implicit Runge Kutta (DIRK) 
         *   - enum value: <code>eDIRKOrder3</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{c|c}
         * A & U \\
         * \hline 
         * B & V
         * \end{array}\right] = 
         * \left[\begin{array}{ccc|c}
         * \lambda & 0 & 0 & 1 \\
         * \frac{1}{2}\left(1-\lambda\right) & \lambda & 0 & 1 \\
         * \frac{1}{4}\left(-6\lambda^2+16\lambda-1\right) & \frac{1}{4}\left(6\lambda^2-20\lambda+5\right) & \lambda & 1 \\
         * \hline
         * \frac{1}{4}\left(-6\lambda^2+16\lambda-1\right) & \frac{1}{4}\left(6\lambda^2-20\lambda+5\right) & \lambda & 1 
         * \end{array}\right]\quad \mathrm{with}\quad \lambda=0.4358665215,\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)
         * \end{array}\right]
         * \f]
         * - 3rd order L-stable, three stage IMEX DIRK(3,4,3) 
         *   - enum value: <code>eIMEXdirk_3_4_3</code>
         *   - coefficients and parameters:
         * \f[
         * \left[\begin{array}{cc|c}
         * A^{\mathrm{IM}} & A^{\mathrm{EM}} & U \\
         * \hline 
         * B^{\mathrm{IM}} & B^{\mathrm{EM}} & V
         * \end{array}\right] = 
         * \left[\begin{array}{cc|c}
         * \left[\begin{array}{cccc}
         * 0 & 0 & 0 & 0 \\
         * 0 & \lambda & 0 & 0 \\
         * 0 & \frac{1}{2}\left(1-\lambda\right) & \lambda & 0 \\
         * 0 & \frac{1}{4}\left(-6\lambda^2+16\lambda-1\right) & \frac{1}{4}\left(6\lambda^2-20\lambda+5\right) & \lambda
         * \end{array}\right] &
         * \left[\begin{array}{cccc}
         * 0 & 0 & 0 & 0 \\
         * \lambda & 0 & 0 & 0 \\
         * 0.3212788860 & 0.3966543747 & 0 & 0 \\
         * -0.105858296 & 0.5529291479 & 0.5529291479 & 0 
         * \end{array}\right] &
         * \left[\begin{array}{c}
         * 1\\
         * 1\\
         * 1\\
         * 1
         * \end{array}\right] \\
         * \hline
         * \left[\begin{array}{cccc}
         * 0 & \frac{1}{4}\left(-6\lambda^2+16\lambda-1\right) & \frac{1}{4}\left(6\lambda^2-20\lambda+5\right) & \lambda
         * \end{array}\right] &
         * \left[\begin{array}{cccc}
         * 0 & \frac{1}{4}\left(-6\lambda^2+16\lambda-1\right) & \frac{1}{4}\left(6\lambda^2-20\lambda+5\right) & \lambda
         * \end{array}\right] &
         * \left[\begin{array}{c}
         * 1
         * \end{array}\right]
         * \end{array}\right]\quad \mathrm{with}\quad \lambda=0.4358665215,\qquad
         * \boldsymbol{y}^{[n]}=
         * \left[\begin{array}{c}
         * \boldsymbol{y}^{[n]}_0
         * \end{array}\right]=
         * \left[\begin{array}{c}
         * \boldsymbol{m}\left(t^n,\boldsymbol{y}^{n}\right)
         * \end{array}\right]
         * \f]
         * \subsection sectionGeneralLinearMethodsImplementationAddMethods How to add a method
         * To add a new time integration scheme, follow the steps below:
         * - Choose a name for the method and add it to the ::TimeIntegrationMethod enum list.
         * - Populate the switch statement in the TimeIntegrationScheme constructor 
         *   with the coefficients of the new method.
         * - Populate the switch statement in the function TimeIntegrationScheme::InitializeScheme  
         *   with a proper initialisation strategy for the method.
         * - Add documentation for the method (especially indicating what the auxiliary parameters
         *   of the input and output vectors of the multi-step method represent)
         */
    }
}
