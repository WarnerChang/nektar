///////////////////////////////////////////////////////////////////////////////
//
// File VelocityCorrection.cpp
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
// Description: Velocity Correction Scheme for the Incompressible
// Navier Stokes equations
///////////////////////////////////////////////////////////////////////////////

#include <IncNavierStokesSolver/EquationSystems/VelocityCorrectionScheme.h>

namespace Nektar
{
    string VelocityCorrectionScheme::className = GetEquationSystemFactory().RegisterCreatorFunction("VelocityCorrectionScheme", VelocityCorrectionScheme::create);
    
    
    /**
     * Constructor. Creates ...
     *
     * \param 
     * \param
     */
    VelocityCorrectionScheme::VelocityCorrectionScheme(
            const LibUtilities::SessionReaderSharedPtr& pSession):
        IncNavierStokes(pSession)
    {
        
    }

    void VelocityCorrectionScheme::v_InitObject()
    {
        IncNavierStokes::v_InitObject();
        // Set m_pressure to point to last field of m_fields; 
        if(NoCaseStringCompare(m_session->GetVariable(m_fields.num_elements()-1),"p") == 0)
        {
            m_nConvectiveFields = m_fields.num_elements()-1;
            m_pressure = m_fields[m_nConvectiveFields];
            m_pressureCalls = 1;
        }
        else
        {
            ASSERTL0(false,"Need to set up pressure field definition");
        }

        LibUtilities::TimeIntegrationMethod intMethod;
        std::string TimeIntStr = m_session->GetSolverInfo("TIMEINTEGRATIONMETHOD");
        int i;
        for(i = 0; i < (int) LibUtilities::SIZE_TimeIntegrationMethod; ++i)
        {
            if(NoCaseStringCompare(LibUtilities::TimeIntegrationMethodMap[i],TimeIntStr) == 0 )
            {
                intMethod = (LibUtilities::TimeIntegrationMethod)i; 
                break;
            }
        }

        ASSERTL0(i != (int) LibUtilities::SIZE_TimeIntegrationMethod, "Invalid time integration type.");
        
        // Set to 1 for first step and it will then be increased in
        // time advance routines
        switch(intMethod)
        {
        case LibUtilities::eIMEXOrder1: 
            {
                m_intSteps = 1;
                m_integrationScheme = Array<OneD, LibUtilities::TimeIntegrationSchemeSharedPtr> (m_intSteps);
                LibUtilities::TimeIntegrationSchemeKey       IntKey0(intMethod);
                m_integrationScheme[0] = LibUtilities::TimeIntegrationSchemeManager()[IntKey0];
            }
            break;
        case LibUtilities::eIMEXOrder2: 
            {
                m_intSteps = 2;
                m_integrationScheme = Array<OneD, LibUtilities::TimeIntegrationSchemeSharedPtr> (m_intSteps);
                LibUtilities::TimeIntegrationSchemeKey       IntKey0(LibUtilities::eIMEXOrder1);
                m_integrationScheme[0] = LibUtilities::TimeIntegrationSchemeManager()[IntKey0];
                LibUtilities::TimeIntegrationSchemeKey       IntKey1(intMethod);
                m_integrationScheme[1] = LibUtilities::TimeIntegrationSchemeManager()[IntKey1];
            }
            break;
        case LibUtilities::eIMEXOrder3: 
            {
                m_intSteps = 3;
                m_integrationScheme = Array<OneD, LibUtilities::TimeIntegrationSchemeSharedPtr> (m_intSteps);
                LibUtilities::TimeIntegrationSchemeKey       IntKey0(LibUtilities::eIMEXdirk_3_4_3);
                m_integrationScheme[0] = LibUtilities::TimeIntegrationSchemeManager()[IntKey0];
                LibUtilities::TimeIntegrationSchemeKey       IntKey1(LibUtilities::eIMEXdirk_3_4_3);
                m_integrationScheme[1] = LibUtilities::TimeIntegrationSchemeManager()[IntKey1];
                LibUtilities::TimeIntegrationSchemeKey       IntKey2(intMethod);
                m_integrationScheme[2] = LibUtilities::TimeIntegrationSchemeManager()[IntKey2];
            }
            break;
        default:
            ASSERTL0(0,"Integration method not suitable: Options include IMEXOrder1, IMEXOrder2 or IMEXOrder3");
            break;
        }
		
		if(m_HomogeneousType != eHomogeneous3D)
		{
			// Set up mapping from pressure boundary condition to pressure
			// element details.
			m_pressure->GetBoundaryToElmtMap(m_pressureBCtoElmtID,
                                         m_pressureBCtoTraceID);
			// Storage array for high order pressure BCs
			m_pressureHBCs = Array<OneD, Array<OneD, NekDouble> > (m_intSteps);

			// Count number of HBC conditions
			Array<OneD, const SpatialDomains::BoundaryConditionShPtr > PBndConds = m_pressure->GetBndConditions();
			Array<OneD, MultiRegions::ExpListSharedPtr>  PBndExp = m_pressure->GetBndCondExpansions();

			int n,cnt;
			for(cnt = n = 0; n < PBndConds.num_elements(); ++n)
			{
				// High order boundary condition;
				if(PBndConds[n]->GetUserDefined().GetEquation() == "H")
				{
					cnt += PBndExp[n]->GetNcoeffs();
				}
			}
		

			for(n = 0; n < m_intSteps; ++n)
			{
				m_pressureHBCs[n] = Array<OneD, NekDouble>(cnt);
			}
		}
		
        
        m_integrationOps.DefineOdeRhs(&VelocityCorrectionScheme::EvaluateAdvection_SetPressureBCs, this);
        
        m_integrationOps.DefineImplicitSolve(&VelocityCorrectionScheme::SolveUnsteadyStokesSystem,this);

    }

    VelocityCorrectionScheme::~VelocityCorrectionScheme(void)
    {
        
    }


    void VelocityCorrectionScheme::v_PrintSummary(std::ostream &out)
    {
        cout <<  "\tSolver Type     : Velocity Correction" <<endl;
        TimeParamSummary(out);
        cout << "\tTime integ.     : " << LibUtilities::TimeIntegrationMethodMap[m_integrationScheme[m_intSteps-1]->GetIntegrationMethod()] << endl;
    }


    void VelocityCorrectionScheme::v_DoInitialise(void)
    {
        // Set initial condition using time t=0
        SetInitialConditions(0.0);
    }

    void VelocityCorrectionScheme::v_DoSolve(void)
    {

        switch(m_equationType)
        {
        case eUnsteadyStokes: 
        case eUnsteadyNavierStokes:
		case eUnsteadyLinearisedNS:
            {   
                // Integrate from start time to end time
                AdvanceInTime(m_steps);
                break;
            }
        case eNoEquationType:
        default:
            ASSERTL0(false,"Unknown or undefined equation type for VelocityCorrectionScheme");
        }
    }

	
	void VelocityCorrectionScheme:: v_TransCoeffToPhys(void)
	{
		int nfields = m_fields.num_elements() - 1;
		for (int k=0 ; k < nfields; ++k)
		{
			//Backward Transformation in physical space for time evolution
			m_fields[k]->BwdTrans_IterPerExp(m_fields[k]->GetCoeffs(),
										     m_fields[k]->UpdatePhys());
			
		}
	}
	
	void VelocityCorrectionScheme:: v_TransPhysToCoeff(void)
	{
		int nfields = m_fields.num_elements() - 1;
		for (int k=0 ; k < nfields; ++k)
		{
			//Forward Transformation in physical space for time evolution
			m_fields[k]->FwdTrans_IterPerExp(m_fields[k]->GetPhys(),
										     m_fields[k]->UpdateCoeffs());
			
		}
	}
	
	
	
    Array<OneD, bool> VelocityCorrectionScheme::v_GetSystemSingularChecks()
    {
        int vVar = m_session->GetVariables().size();
        Array<OneD, bool> vChecks(vVar, false);
        vChecks[vVar-1] = true;
        return vChecks;
    }

    int VelocityCorrectionScheme::v_GetForceDimension()
    {
        return m_session->GetVariables().size() - 1;
    }

    void VelocityCorrectionScheme::EvaluateAdvection_SetPressureBCs(const Array<OneD, const Array<OneD, NekDouble> > &inarray, 
                                                                    Array<OneD, Array<OneD, NekDouble> > &outarray, 
                                                                    const NekDouble time)
    {
        // evaluate convection terms
        m_advObject->DoAdvection(m_fields, m_nConvectiveFields, m_velocity, 
                                 inarray, outarray);

        //add the force
        if(m_session->DefinesFunction("BodyForce"))
        {
            int nqtot      = m_fields[0]->GetTotPoints();
            for(int i = 0; i < m_nConvectiveFields; ++i)
            {
                Vmath::Vadd(nqtot,outarray[i],1,(m_forces[i]->GetPhys()),1,outarray[i],1);
            }
        }        

		if(m_HomogeneousType != eHomogeneous3D)
		{
			// Set pressure BCs
			EvaluatePressureBCs(inarray, outarray);
		}
    }

    void VelocityCorrectionScheme::SolveUnsteadyStokesSystem(const Array<OneD, const Array<OneD, NekDouble> > &inarray, 
                                                             Array<OneD, Array<OneD, NekDouble> > &outarray, 
                                                             const NekDouble time, 
                                                             const NekDouble aii_Dt)
    {

        int i,n;
        int phystot = m_fields[0]->GetTotPoints();
        int ncoeffs = m_fields[0]->GetNcoeffs();
        Array<OneD, Array< OneD, NekDouble> > F(m_nConvectiveFields);
        NekDouble  lambda = 1.0/aii_Dt/m_kinvis; 

        for(n = 0; n < m_nConvectiveFields; ++n)
        {
           F[n] = Array<OneD, NekDouble> (phystot);
        }
		
        SetBoundaryConditions(time);
		
        // Pressure Forcing = Divergence Velocity; 
        SetUpPressureForcing(inarray, F, aii_Dt);
        
        // Solver Pressure Poisson Equation 
#ifdef UseContCoeffs
        m_pressure->HelmSolve(F[0], m_pressure->UpdateContCoeffs(),0.0,true);
#else
		m_pressure->HelmSolve(F[0], m_pressure->UpdateCoeffs(),0.0);
#endif
		

        // Viscous Term forcing
        SetUpViscousForcing(inarray, F, aii_Dt);
    
        // Solve Helmholtz system and put in Physical space
        for(i = 0; i < m_nConvectiveFields; ++i)
        {
#ifdef UseContCoeffs
            m_fields[i]->HelmSolve(F[i], m_fields[i]->UpdateContCoeffs(), lambda,true);
            m_fields[i]->BwdTrans(m_fields[i]->GetContCoeffs(),outarray[i],true);
#else
            m_fields[i]->HelmSolve(F[i], m_fields[i]->UpdateCoeffs(), lambda);
            m_fields[i]->BwdTrans(m_fields[i]->GetCoeffs(),outarray[i]);
#endif
        }
    }

    
    void VelocityCorrectionScheme::EvaluatePressureBCs(const Array<OneD, const Array<OneD, NekDouble> >  &fields, const Array<OneD, const Array<OneD, NekDouble> >  &N)
    {
        Array<OneD, NekDouble> tmp;
        Array<OneD, const SpatialDomains::BoundaryConditionShPtr > PBndConds;
        Array<OneD, MultiRegions::ExpListSharedPtr>  PBndExp;
        int  n,cnt;
        int  nint    = min(m_pressureCalls++,m_intSteps);
        int  nlevels = m_pressureHBCs.num_elements();

        PBndConds   = m_pressure->GetBndConditions();
        PBndExp     = m_pressure->GetBndCondExpansions();

        // Reshuffle Bc Storage vector
        tmp = m_pressureHBCs[nlevels-1];
        for(n = nlevels-1; n > 0; --n)
        {
            m_pressureHBCs[n] = m_pressureHBCs[n-1];
        }
        m_pressureHBCs[0] = tmp;

        // Calculate BCs at current level
        CalcPressureBCs(fields,N);
        
        // Copy High order values into storage array 
        for(cnt = n = 0; n < PBndConds.num_elements(); ++n)
        {
            // High order boundary condition;
            if(PBndConds[n]->GetUserDefined().GetEquation() == "H")
            {
                int nq = PBndExp[n]->GetNcoeffs();
                Vmath::Vcopy(nq,&(PBndExp[n]->GetCoeffs()[0]),1,&(m_pressureHBCs[0])[cnt],1);
                cnt += nq;
            }
        }

        // Extrapolate to n+1
        Vmath::Smul(cnt,kHighOrderBCsExtrapolation[nint-1][nint-1],
                    m_pressureHBCs[nint-1],1,m_pressureHBCs[nlevels-1],1);
        for(n = 0; n < nint-1; ++n)
        {
            Vmath::Svtvp(cnt,kHighOrderBCsExtrapolation[nint-1][n],
                          m_pressureHBCs[n],1,m_pressureHBCs[nlevels-1],1,
                          m_pressureHBCs[nlevels-1],1);
        }

        // Reset Values into Pressure BCs
        for(cnt = n = 0; n < PBndConds.num_elements(); ++n)
        {
            // High order boundary condition;
            if(PBndConds[n]->GetUserDefined().GetEquation() == "H")
            {
                int nq = PBndExp[n]->GetNcoeffs();
                Vmath::Vcopy(nq,&(m_pressureHBCs[nlevels-1])[cnt],1,&(PBndExp[n]->UpdateCoeffs()[0]),1);
                cnt += nq;
            }
        }
    }
	
    void VelocityCorrectionScheme::CalcPressureBCs(const Array<OneD, const Array<OneD, NekDouble> > &fields, const Array<OneD, const Array<OneD, NekDouble> >  &N)
    {
        switch(m_nConvectiveFields)
        {
        case 1:
            ASSERTL0(false,"Velocity correction scheme not designed to have just one velocity component");
            break;
        case 2:
            CalcPressureBCs2D(fields,N);
            break;
        case 3:
            CalcPressureBCs3D(fields,N);
            break;
        }
    }
    
    void VelocityCorrectionScheme::CalcPressureBCs2D(const Array<OneD, const Array<OneD, NekDouble> > &fields, const Array<OneD, const Array<OneD, NekDouble> >  &N)
    {
        int  i,n;
        
        Array<OneD, const SpatialDomains::BoundaryConditionShPtr > PBndConds;
        Array<OneD, MultiRegions::ExpListSharedPtr>  PBndExp;
	
        PBndConds = m_pressure->GetBndConditions();
        PBndExp   = m_pressure->GetBndCondExpansions();
		
        StdRegions::StdExpansionSharedPtr elmt;
        StdRegions::StdExpansion1DSharedPtr Pbc;
        
        Array<OneD, NekDouble> Pvals;
        
        int maxpts = 0,cnt;
        int elmtid,nq,offset, boundary;
	
        bool NegateNormals;
	
        // find the maximum values of points 
        for(cnt = n = 0; n < PBndConds.num_elements(); ++n)
        {
            for(i = 0; i < PBndExp[n]->GetExpSize(); ++i)
            {
                maxpts = max(maxpts, m_fields[0]->GetExp(m_pressureBCtoElmtID[cnt++])->GetTotPoints());
            }
        }
		
        Array<OneD, const NekDouble> U,V,Nu,Nv;
        Array<OneD, NekDouble> Uy(maxpts);
        Array<OneD, NekDouble> Vx(maxpts);
        Array<OneD, NekDouble> Qx(maxpts);  
        Array<OneD, NekDouble> Qy(maxpts);
        Array<OneD, NekDouble> Q(maxpts); 
		
        for(cnt = n = 0; n < PBndConds.num_elements(); ++n)
        {
            
            string type = PBndConds[n]->GetUserDefined().GetEquation(); 
            
            if(type == "H")
            {
                for(i = 0; i < PBndExp[n]->GetExpSize(); ++i,cnt++)
                {
                    // find element and edge of this expansion. 
                    // calculate curl x curl v;
                    elmtid = m_pressureBCtoElmtID[cnt];
                    elmt   = m_fields[0]->GetExp(elmtid);
                    nq     = elmt->GetTotPoints();
                    offset = m_fields[0]->GetPhys_Offset(elmtid);
                    
                    U = fields[m_velocity[0]] + offset;
                    V = fields[m_velocity[1]] + offset; 
                    
                    // Calculating vorticity Q = (dv/dx - du/dy)
                    elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],V,Vx);
                    elmt->PhysDeriv(MultiRegions::DirCartesianMap[1],U,Uy);					
                    
                    Vmath::Vsub(nq,Vx,1,Uy,1,Q,1);
                    
                    // Calculate  Curl(Q) = Qy i - Qx j 
                    elmt->PhysDeriv(Q,Qx,Qy);
                    
                    Nu = N[0] + offset;
                    Nv = N[1] + offset; 
                    
                    // Evaluate [N - kinvis Curlx Curl V].n
                    // x-component (stored in Qy)
                    Vmath::Svtvp(nq,-m_kinvis,Qy,1,Nu,1,Qy,1);
                    
                    // y-component (stored in Qx )
                    Vmath::Svtvp(nq,m_kinvis,Qx,1,Nv,1,Qx,1);
                    
                    Pbc =  boost::dynamic_pointer_cast<StdRegions::StdExpansion1D> (PBndExp[n]->GetExp(i));
                    
                    boundary = m_pressureBCtoTraceID[cnt];
                    
                    // Get edge values and put into Uy, Vx
                    elmt->GetEdgePhysVals(boundary,Pbc,Qy,Uy);
                    elmt->GetEdgePhysVals(boundary,Pbc,Qx,Vx);
                    
                    // calcuate (phi, dp/dn = [N-kinvis curl x curl v].n) 
                    Pvals = PBndExp[n]->UpdateCoeffs()+PBndExp[n]->GetCoeff_Offset(i);
                    // Decide if normals facing outwards
                    NegateNormals = (elmt->GetEorient(boundary) == StdRegions::eForwards)? false:true;
                    
                    Pbc->NormVectorIProductWRTBase(Uy,Vx,Pvals,NegateNormals); 
                }
            }
            else if(type == "" || type == "TimeDependent")  // setting if just standard BC no High order
            {
                cnt += PBndExp[n]->GetExpSize();
            }
            else
            {
                ASSERTL0(false,"Unknown USERDEFINEDTYPE in pressure boundary condition");
            }
        }
    }
				
	void VelocityCorrectionScheme::CalcPressureBCs3D(const Array<OneD, const Array<OneD, NekDouble> > &fields, const Array<OneD, const Array<OneD, NekDouble> >  &N)
	{
		Array<OneD, const SpatialDomains::BoundaryConditionShPtr > PBndConds;
		Array<OneD, MultiRegions::ExpListSharedPtr>  PBndExp;
		
		PBndConds = m_pressure->GetBndConditions();
		PBndExp   = m_pressure->GetBndCondExpansions();
		
		int elmtid,nq,offset, boundary,cnt,n;
		bool NegateNormals;
		int maxpts = 0;
		
		Array<OneD, NekDouble> Pvals;
		
		// find the maximum values of points 
        for(cnt = n = 0; n < PBndConds.num_elements(); ++n)
        {
            for(int i = 0; i < PBndExp[n]->GetExpSize(); ++i)
            {
                maxpts = max(maxpts, m_fields[0]->GetExp(m_pressureBCtoElmtID[cnt++])->GetTotPoints());
            }
        }
		
		Array<OneD, const NekDouble> U,V,W,Nu,Nv,Nw;
		Array<OneD, NekDouble> Uy(maxpts);
		Array<OneD, NekDouble> Uz(maxpts);
		Array<OneD, NekDouble> Vx(maxpts);
		Array<OneD, NekDouble> Vz(maxpts);
		Array<OneD, NekDouble> Wx(maxpts);
		Array<OneD, NekDouble> Wy(maxpts);
		Array<OneD, NekDouble> Qx(maxpts);  
		Array<OneD, NekDouble> Qy(maxpts);
		Array<OneD, NekDouble> Qz(maxpts);
		
		
		if(m_HomogeneousType == eHomogeneous1D)
		{
			StdRegions::StdExpansionSharedPtr elmt;
			StdRegions::StdExpansion1DSharedPtr Pbc;
			
			int exp_size, exp_size_per_plane;
			
			Array<OneD, Array< OneD, NekDouble> > velocity(m_nConvectiveFields);
			Array<OneD, Array< OneD, NekDouble> > advection(m_nConvectiveFields);
			
			int phystot = m_fields[0]->GetTotPoints();
			
			for(int n = 0; n < m_nConvectiveFields; ++n)
			{
				velocity[n] = Array<OneD, NekDouble> (phystot);
				advection[n] = Array<OneD, NekDouble> (phystot);
			}
			
			
			for(int i = 0; i < fields.num_elements(); i++)
			{
				m_pressure->HomogeneousFwdTrans(fields[i],velocity[i]);
				m_pressure->HomogeneousFwdTrans(N[i],advection[i]);
			}
			
			int K;
			NekDouble WaveNumber;
			cnt = 0;
			
			for(int k = 0; k < m_npointsZ; k++)
			{
				K = k/2;
				WaveNumber = 2*M_PI*pow(-1.0,double(k))*(double(K));
				
				for(int n = 0 ; n < PBndConds.num_elements(); ++n)
				{
					string type = PBndConds[n]->GetUserDefined().GetEquation();
					
					exp_size = PBndExp[n]->GetExpSize();
					
					exp_size_per_plane = exp_size/m_npointsZ;
					
					if(type == "H")
					{
						for(int i = 0; i < exp_size_per_plane; ++i,cnt++)
						{
							// find element and edge of this expansion. 
							// calculate curl x curl v;
							elmtid = m_pressureBCtoElmtID[cnt];
							elmt   = m_fields[0]->GetExp(elmtid);
							nq     = elmt->GetTotPoints();
							offset = m_fields[0]->GetPhys_Offset(elmtid);
							
							U = velocity[m_velocity[0]] + offset;
							V = velocity[m_velocity[1]] + offset;
							W = velocity[m_velocity[2]] + offset;
							
							// Calculating vorticity Q = Qx i + Qy j + Qz k
							elmt->PhysDeriv(MultiRegions::DirCartesianMap[1],W,Wy);
							elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],W,Wx);
							elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],V,Vx);
							elmt->PhysDeriv(MultiRegions::DirCartesianMap[1],U,Uy);
							Vmath::Smul(nq,WaveNumber,V,1,Vz,1);
							Vmath::Smul(nq,WaveNumber,U,1,Uz,1);
							
							Vmath::Vsub(nq,Wy,1,Vz,1,Qx,1);
							Vmath::Vsub(nq,Uz,1,Wx,1,Qy,1);
							Vmath::Vsub(nq,Vx,1,Uy,1,Qz,1);
							
							// Calculate  NxQ = Curl(Q) = (Qzy-Qyz) i + (Qxz-Qzx) j + (Qyx-Qxy) k
							// NxQ = NxQ_x i + NxQ_y j + NxQ_z k
							// Using the memory space assocaited with the velocity derivatives to
							// store the vorticity derivatives to save space.
							// Qzy => Uy // Qyz => Uz // Qxz => Vx // Qzx => Vz // Qyx => Wx // Qxy => Wy 
							elmt->PhysDeriv(MultiRegions::DirCartesianMap[1],Qz,Uy);
							elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],Qz,Vz);
							Vmath::Smul(nq,WaveNumber,Qy,1,Uz,1);
							Vmath::Smul(nq,WaveNumber,Qx,1,Vx,1);
							
							// Using the storage space associated with the 3 components of the vorticity
							// to store the 3 components od the vorticity curl to save space
							// Qx = Qzy-Qyz = Uy-Uz // Qy = Qxz-Qzx = Vx-Vz // Qz= Qyx-Qxy = Wx-Wy 
							// The last one is not required becasue the third component of the normal vector is always zero.
							Vmath::Vsub(nq,Uy,1,Uz,1,Qx,1);
							Vmath::Vsub(nq,Vx,1,Vz,1,Qy,1);
							
							Nu = advection[0] + offset;
							Nv = advection[1] + offset;
							
							// Evaluate [N - kinvis Curlx Curl V]
							// x-component (stored in Qx)
							Vmath::Svtvp(nq,-m_kinvis,Qx,1,Nu,1,Qx,1);
							// y-component (stored in Qy)
							Vmath::Svtvp(nq,m_kinvis,Qy,1,Nv,1,Qy,1);
							// z-component (stored in Qz) not required for this approach
							// the third component of the normal vector is always zero
							
							Pbc =  boost::dynamic_pointer_cast<StdRegions::StdExpansion1D> (PBndExp[n]->GetExp(i+k*exp_size_per_plane));
							
							boundary = m_pressureBCtoTraceID[cnt];
							
							// Get edge values and put into Uy, Vx
							elmt->GetEdgePhysVals(boundary,Pbc,Qy,Uy);
							elmt->GetEdgePhysVals(boundary,Pbc,Qx,Vx);
							
							// calcuate (phi, dp/dn = [N-kinvis curl x curl v].n) 
							Pvals = PBndExp[n]->UpdateCoeffs()+PBndExp[n]->GetCoeff_Offset(i+k*exp_size_per_plane);
							// Decide if normals facing outwards
							NegateNormals = (elmt->GetEorient(boundary) == StdRegions::eForwards)? false:true;
							
							Pbc->NormVectorIProductWRTBase(Vx,Uy,Pvals,NegateNormals);
						}
					}
					else if(type == "" || type == "TimeDependent")  // setting if just standard BC no High order
					{
						cnt += exp_size_per_plane;
					}
					else
					{
						ASSERTL0(false,"Unknown USERDEFINEDTYPE in pressure boundary condition");
					}
				}
			}
		}
		else if(m_HomogeneousType == eHomogeneous2D)
		{
			StdRegions::StdExpansionSharedPtr elmt;
			StdRegions::StdExpansion1DSharedPtr Pbc;
			
			int exp_size, exp_size_per_line;
			
			Array<OneD, Array< OneD, NekDouble> > velocity(m_nConvectiveFields);
			Array<OneD, Array< OneD, NekDouble> > advection(m_nConvectiveFields);
			
			int phystot = m_fields[0]->GetTotPoints();
			
			for(int n = 0; n < m_nConvectiveFields; ++n)
			{
				velocity[n] = Array<OneD, NekDouble> (phystot);
				advection[n] = Array<OneD, NekDouble> (phystot);
			}
			
			
			for(int i = 0; i < fields.num_elements(); i++)
			{
				m_pressure->HomogeneousFwdTrans(fields[i],velocity[i]);
				m_pressure->HomogeneousFwdTrans(N[i],advection[i]);
			}
			
			int Ky,Kz;
			NekDouble WaveNumber_y,WaveNumber_z;
			cnt = 0;
			
			for(int k1 = 0; k1 < m_npointsZ; k1++)
			{
				Kz = k1/2;
				WaveNumber_z =2*M_PI*pow(-1.0,double(k1))*(double(Kz));
				
				for(int k2 = 0; k2 < m_npointsY; k2++)
				{
					Ky = k2/2;
					WaveNumber_y = 2*M_PI*pow(-1.0,double(k2))*(double(Ky));
					
					for(int n = 0 ; n < PBndConds.num_elements(); ++n)
					{
						string type = PBndConds[n]->GetUserDefined().GetEquation();
						
						exp_size = PBndExp[n]->GetExpSize();
						
						exp_size_per_line = exp_size/(m_npointsZ*m_npointsY);
						
						if(type == "H")
						{
							for(int i = 0; i < exp_size_per_line; ++i,cnt++)
							{
								// find element and edge of this expansion. 
								// calculate curl x curl v;
								elmtid = m_pressureBCtoElmtID[cnt];
								elmt   = m_fields[0]->GetExp(elmtid);
								nq     = elmt->GetTotPoints();
								offset = m_fields[0]->GetPhys_Offset(elmtid);
								
								U = velocity[m_velocity[0]] + offset;
								V = velocity[m_velocity[1]] + offset;
								W = velocity[m_velocity[2]] + offset;
								
								// Calculating vorticity Q = Qx i + Qy j + Qz k
								elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],W,Wx);
								elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],V,Vx);
								Vmath::Smul(nq,WaveNumber_y,U,1,Uy,1);
								Vmath::Smul(nq,WaveNumber_z,U,1,Uz,1);
								
								Vmath::Vsub(nq,Uz,1,Wx,1,Qy,1);
								Vmath::Vsub(nq,Vx,1,Uy,1,Qz,1);
								
								// Calculate  NxQ = Curl(Q) = (Qzy-Qyz) i + (Qxz-Qzx) j + (Qyx-Qxy) k
								// NxQ = NxQ_x i + NxQ_y j + NxQ_z k
								// Using the memory space assocaited with the velocity derivatives to
								// store the vorticity derivatives to save space.
								// Qzy => Uy // Qyz => Uz // Qxz => Vx // Qzx => Vz // Qyx => Wx // Qxy => Wy 
								Vmath::Smul(nq,WaveNumber_y,Qz,1,Uy,1);
								Vmath::Smul(nq,WaveNumber_z,Qy,1,Uz,1);
								
								// Using the storage space associated with the 3 components of the vorticity
								// to store the 3 components od the vorticity curl to save space
								// Qx = Qzy-Qyz = Uy-Uz // Qy = Qxz-Qzx = Vx-Vz // Qz= Qyx-Qxy = Wx-Wy 
								Vmath::Vsub(nq,Uy,1,Uz,1,Qx,1);
								
								Nu = advection[0] + offset;
								
								// Evaluate [N - kinvis Curlx Curl V]
								// x-component (stored in Qx)
								Vmath::Svtvp(nq,-m_kinvis,Qx,1,Nu,1,Qx,1);
								
								boundary = m_pressureBCtoTraceID[cnt];
								
								if(boundary == 0)
								{
									(PBndExp[n]->UpdateCoeffs()+PBndExp[n]->GetCoeff_Offset(i+(k1*m_npointsY+k2)*exp_size_per_line))[0] = -1*Qx[0];
								}
								else if (boundary == 1)
								{
									(PBndExp[n]->UpdateCoeffs()+PBndExp[n]->GetCoeff_Offset(i+(k1*m_npointsY+k2)*exp_size_per_line))[0] = Qx[nq-1];
								}
								else 
								{
									ASSERTL0(false,"In the 3D homogeneous 2D approach BCs edge ID can be just 0 or 1 ");
								}
							}
						}
						else if(type == "" || type == "TimeDependent")  // setting if just standard BC no High order
						{
							cnt += exp_size_per_line;
						}
						else
						{
							ASSERTL0(false,"Unknown USERDEFINEDTYPE in pressure boundary condition");
						}
					}
				}
			}
		}
		else if(m_HomogeneousType == eHomogeneous3D)
		{
			ASSERTL0(false,"High Order Pressure BC not required for this approach");
		}
		// Full 3D		
                else
		{
                    int i, cnt;

                    StdRegions::StdExpansionSharedPtr elmt;
                    StdRegions::StdExpansion2DSharedPtr Pbc;

                    for(cnt = n = 0; n < PBndConds.num_elements(); ++n)
                    {

                        string type = PBndConds[n]->GetUserDefined().GetEquation(); 

                        if(type == "H")
                        {
                            for(i = 0; i < PBndExp[n]->GetExpSize(); ++i,cnt++)
                            {
                                // find element and face of this expansion. 
                                // calculate curl x curl v;
                                elmtid = m_pressureBCtoElmtID[cnt];
                                elmt   = m_fields[0]->GetExp(elmtid);
                                nq     = elmt->GetTotPoints();
                                offset = m_fields[0]->GetPhys_Offset(elmtid);
					
                                U = fields[m_velocity[0]] + offset;
				V = fields[m_velocity[1]] + offset; 
				W = fields[m_velocity[2]] + offset;
					
			        // Calculating vorticity Q = (dv/dx - du/dy)
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[1],U,Uy);
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[2],U,Uz);					
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],V,Vx);					
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[2],V,Vz);
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],W,Wx);					
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[1],W,Wy);					

                                Vmath::Vsub(nq,Wy,1,Vz,1,Qx,1);
                                Vmath::Vsub(nq,Uz,1,Wx,1,Qy,1);
                                Vmath::Vsub(nq,Vx,1,Uy,1,Qz,1);

                                // Calculate  NxQ = Curl(Q) = (Qzy-Qyz) i + (Qxz-Qzx) j + (Qyx-Qxy) k
                                // NxQ = NxQ_x i + NxQ_y j + NxQ_z k
                                // Using the velocity derivatives memory space to
                                // store the vorticity derivatives.
                                // Qzy => Uy // Qyz => Uz // Qxz => Vx // Qzx => Vz // Qyx => Wx // Qxy => Wy 
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[1],Qz,Uy);
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],Qz,Vz);
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[2],Qy,Uz);
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[2],Qx,Vx);
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[0],Qy,Wx);
                                elmt->PhysDeriv(MultiRegions::DirCartesianMap[1],Qx,Wy);
		
                                // Using the storage space associated with the 3 components of the vorticity
                                // to store the 3 components od the vorticity curl to save space
                                // Qx = Qzy-Qyz = Uy-Uz // Qy = Qxz-Qzx = Vx-Vz // Qz= Qyx-Qxy = Wx-Wy 
                                Vmath::Vsub(nq,Uy,1,Uz,1,Qx,1);
                                Vmath::Vsub(nq,Vx,1,Vz,1,Qy,1);
                                Vmath::Vsub(nq,Wx,1,Wy,1,Qz,1);

                                Nu = N[0] + offset;
                                Nv = N[1] + offset;
                                Nw = N[2] + offset;

                                // Evaluate [N - kinvis Curlx Curl V]
                                // x-component (stored in Qx)
                                Vmath::Svtvp(nq,-m_kinvis,Qx,1,Nu,1,Qx,1);
                                // y-component (stored in Qy)
                                Vmath::Svtvp(nq,m_kinvis,Qy,1,Nv,1,Qy,1);
                                // z-component (stored in Qz)
                                Vmath::Svtvp(nq,m_kinvis,Qz,1,Nw,1,Qz,1);		

                                Pbc =  boost::dynamic_pointer_cast<StdRegions::StdExpansion2D> (PBndExp[n]->GetExp(i));

                                boundary = m_pressureBCtoTraceID[cnt];

                                // Get face values and put into Uy, Vx and Wx
                                elmt->GetFacePhysVals(boundary,Qy+offset,Uy);
                                elmt->GetFacePhysVals(boundary,Qx+offset,Vx);
                                elmt->GetFacePhysVals(boundary,Qz+offset,Wx);

                                // calcuate (phi, dp/dn = [N-kinvis curl x curl v].n) 
                                Pvals = PBndExp[n]->UpdateCoeffs()+PBndExp[n]->GetCoeff_Offset(i);

                                // Decide if normals facing outwards
                                switch(boundary)
                                {
                                case 0:
                                    NegateNormals = (elmt->GetFaceorient(boundary) == StdRegions::eDir1FwdDir1_Dir2FwdDir2) 
                                                 || (elmt->GetFaceorient(boundary) == StdRegions::eDir1BwdDir1_Dir2BwdDir2) ? true:false; 
                                    break;
                                case 1:
                                    NegateNormals = (elmt->GetFaceorient(boundary) == StdRegions::eDir1BwdDir1_Dir2FwdDir2) 
                                                 || (elmt->GetFaceorient(boundary) == StdRegions::eDir1FwdDir1_Dir2BwdDir2) ? true:false;
                                    break;
                                case 2:
                                    NegateNormals = (elmt->GetFaceorient(boundary) == StdRegions::eDir1BwdDir1_Dir2FwdDir2) 
                                                 || (elmt->GetFaceorient(boundary) == StdRegions::eDir1FwdDir1_Dir2BwdDir2) ? true:false; 
                                    break;
                                case 3:
                                    NegateNormals = (elmt->GetFaceorient(boundary) == StdRegions::eDir1FwdDir1_Dir2FwdDir2) 
                                                 || (elmt->GetFaceorient(boundary) == StdRegions::eDir1BwdDir1_Dir2BwdDir2) ? true:false; 
                                    break;
                                case 4:
                                    NegateNormals = (elmt->GetFaceorient(boundary) == StdRegions::eDir1FwdDir1_Dir2FwdDir2) 
                                                 || (elmt->GetFaceorient(boundary) == StdRegions::eDir1BwdDir1_Dir2FwdDir2) ? true:false; 
                                    break;
                                case 5:
                                    NegateNormals = (elmt->GetFaceorient(boundary) == StdRegions::eDir1BwdDir1_Dir2FwdDir2) 
                                                 || (elmt->GetFaceorient(boundary) == StdRegions::eDir1FwdDir1_Dir2BwdDir2) ? true:false; 
                                    break;
                                default:
                                    ASSERTL0(false,"face value (> 5) is out of range");
                                    break;
                                }
                            
                                Pbc->NormVectorIProductWRTBase(Uy,Vx,Wx,Pvals,NegateNormals); 
                            }
                        }
                        else if(type == "" || type == "TimeDependent")  // setting if just standard BC no High order
                        {
                            cnt += PBndExp[n]->GetExpSize();
                        }
                        else
                        {
                            ASSERTL0(false,"Unknown USERDEFINEDTYPE in pressure boundary condition");
                        }
                    }
	        }
	}
	
	
    // Evaluate divergence of velocity field. 
    void   VelocityCorrectionScheme::SetUpPressureForcing(const Array<OneD, const Array<OneD, NekDouble> > &fields, Array<OneD, Array<OneD, NekDouble> > &Forcing, const NekDouble aii_Dt)
    {                
        int       i;
        int       physTot = m_fields[0]->GetTotPoints();
        Array<OneD, NekDouble> wk = Array<OneD, NekDouble>(physTot);
                
        Vmath::Zero(physTot,Forcing[0],1);
        
        for(i = 0; i < m_nConvectiveFields; ++i)
        {
			m_fields[i]->PhysDeriv(MultiRegions::DirCartesianMap[i],fields[i], wk);
            Vmath::Vadd(physTot,wk,1,Forcing[0],1,Forcing[0],1);
        }
        
        Vmath::Smul(physTot,1.0/aii_Dt,Forcing[0],1,Forcing[0],1);        
    }
    
    void   VelocityCorrectionScheme::SetUpViscousForcing(const Array<OneD, const Array<OneD, NekDouble> > &inarray, Array<OneD, Array<OneD, NekDouble> > &Forcing, const NekDouble aii_Dt)
    {
        NekDouble aii_dtinv = 1.0/aii_Dt;
        int ncoeffs = m_fields[0]->GetNcoeffs();
        int phystot = m_fields[0]->GetTotPoints();

        // Grad p
#ifdef UseContCoeffs
        m_pressure->BwdTrans(m_pressure->GetContCoeffs(),m_pressure->UpdatePhys(),true);
#else
        m_pressure->BwdTrans(m_pressure->GetCoeffs(),m_pressure->UpdatePhys());
#endif
        
        if(m_nConvectiveFields == 2)
        {
            m_pressure->PhysDeriv(m_pressure->GetPhys(), Forcing[0], Forcing[1]);
        }
        else
        {
            m_pressure->PhysDeriv(m_pressure->GetPhys(), Forcing[0], Forcing[1],Forcing[2]);
        }
        
        // Subtract inarray/(aii_dt) and divide by kinvis. Kinvis will
        // need to be updated for the convected fields.
        for(int i = 0; i < m_nConvectiveFields; ++i)
        {
            Blas::Daxpy(phystot,-aii_dtinv,inarray[i],1,Forcing[i],1);
            Blas::Dscal(phystot,1.0/m_kinvis,&(Forcing[i])[0],1);
        }
    }
            
} //end of namespace

/**
* $Log: VelocityCorrectionScheme.cpp,v $
* Revision 1.5  2010/01/28 15:17:59  abolis
* Time-Dependent boundary conditions
*
* Revision 1.4  2009/12/09 13:16:58  abolis
* Update related to regression test
*
* Revision 1.3  2009/09/10 10:42:49  pvos
* Modification to bind object pointer rather than object itself to time-integration functions.
* (Prevents unwanted copy-constructor calls)
*
* Revision 1.2  2009/09/06 22:31:16  sherwin
* First working version of Navier-Stokes solver and input files
*
* Revision 1.1  2009/03/14 16:43:21  sherwin
* First non-working version
*
*
**/
