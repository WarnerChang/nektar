///////////////////////////////////////////////////////////////////////////////
//
// File AdvectionDiffusionReactionSolver.cpp
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
// Description: Advection Diffusion Reaction solver
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cmath> 

#include <AdvectionDiffusionReactionSolver/AdvectionDiffusionReaction.h>


using namespace Nektar;


int main(int argc, char *argv[])
{
    
    ASSERTL0(argc == 2,"\n \t Usage: AdvectionDiffusionReactionSolver  meshfile \n");

    string fileNameString(argv[1]);
    
    //----------------------------------------------------------------
    // Read the mesh and construct container class
    AdvectionDiffusionReaction dom(fileNameString);
    
    int nsteps = dom.GetSteps();
    NekDouble lambda = 0.0;

    dom.Summary(cout);
    
    dom.ZeroPhysFields(); // Zero phys field so that following switch is consistent
    
    switch(dom.GetEquationType())
    {
    case eHelmholtz: case eSteadyDiffusionReaction:
        lambda = dom.GetParameter("Lambda");
    case ePoisson: case eSteadyDiffusion: // lambda is zero 
        dom.SetPhysForcingFunctions(dom.UpdateFields());        
    case eLaplace:                        // forcing function is zero
        // Solve the appropriate Helmholtz problem 
        dom.SolveHelmholtz(lambda);
        break;
    case eAdvection: case eDiffusion:
        // Set up the intial conditions 
        dom.SetInitialConditions();
        
        // Integrate from start time to end time
        dom.ExplicitlyIntegrateAdvection(nsteps);
        break;
    case eNoEquationType:
    default:
        ASSERTL0(false,"Unknown or undefined equation type");
    }
     // Dump output
    dom.Output();

    // Evaluate L2 Error
    for(int i = 0; i < dom.GetNvariables(); ++i)
    {
        cout << "L2 Error (variable "<< dom.GetVariable(i) <<"): " << dom.L2Error(i) << endl;
    }

}

/**
* $Log $
**/
