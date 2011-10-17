///////////////////////////////////////////////////////////////////////////////
//
// File MultiRegionsDemoTests.cpp
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
// Description: Run a series of tests on the Incompressible Navier Stokes Solver
//
///////////////////////////////////////////////////////////////////////////////
#include "../../Auxiliary/RegressBase.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <boost/filesystem.hpp>

void RunL2RegressionTest(std::string demo, std::string input, std::string info);
void MakeOkFile(std::string demo, std::string input, std::string info);

#ifdef MAKE_OK_FILE
#define Execute($1,$2,$3)  MakeOkFile($1,$2,$3)
#else
#define Execute($1,$2,$3)  RunL2RegressionTest($1,$2,$3)
#endif

#ifdef _WINDOWS
#define COPY_COMMAND "copy "
#else
#define COPY_COMMAND "cp "
#endif

static int tests_total = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static bool verbose = false;
static bool quiet = false;

int main(int argc, char* argv[])
{
    if (argc > 2)
    {
        std::cout << "Usage: " << argv[0] << " [-v|-q]" << std::endl;
        return -1;
    }
    if (argc == 2 && std::string(argv[1]) == "-v")
    {
        verbose = true;
    }
    if (argc == 2 && std::string(argv[1]) == "-q")
    {
        quiet = true;
    }
    
    //Test Channel Flow
    Execute("IncNavierStokesSolver","Test_ChanFlow_m3.xml","Channel Flow P=3");
    Execute("IncNavierStokesSolver","Test_ChanFlow_m8.xml","Channel Flow P=8");
    Execute("IncNavierStokesSolver","Test_ChanFlow_m8_BodyForce.xml","Channel Flow P=8 BodyForce");    
    Execute("IncNavierStokesSolver","Test_ChanFlow_m8_singular.xml","Channel Flow P=8 Singularity Check");
    Execute("IncNavierStokesSolver","Test_ChanFlow2D_bcsfromfiles.xml","Channel Flow P=5 Boundary Conditions from files");  
    
    //Test Kovasznay Flow
    Execute("IncNavierStokesSolver","Test_KovaFlow_m3.xml","Kovasznay Flow P=3");
    Execute("IncNavierStokesSolver","Test_KovaFlow_m8.xml","Kovasznay Flow P=8");
    
    //Test Decaying Vortex
    Execute("IncNavierStokesSolver","Test_TaylorVor_dt1.xml","Convergence: Taylor Vortex IMEXOrder2 dt=0.01");
    Execute("IncNavierStokesSolver","Test_TaylorVor_dt2.xml","Convergence: Taylor Vortex IMEXOrder2 dt=0.001");

    //Test Coupled LinearNS Kovasznay Flow
    Execute("IncNavierStokesSolver","Test_KovaFlow_m8.xml","Steady Oseen Kovasznay flow P=6");
    Execute("IncNavierStokesSolver","Test_Kovas_Quad6_Tri4_mixedbcs.xml","Steady Oseen Kovasznay flow, mixed elements and bcs P=7");

    //Test Coupled LinearNS unsteady Channel Flow
    Execute("IncNavierStokesSolver","Test_ChanFlow_LinNS_m8.xml","Unsteady channel flow with coupled solver , P=8");

    Execute("IncNavierStokesSolver","Test_SinCos_LinNS_3DHom1D.xml","Steady Linearised NavierStokes, 3D Soln with coupled solver , P=6");
    
    //Test Modified Arnoldi direct stability (VelCorrectionScheme)
    Execute("IncNavierStokesSolver","ChanStability.xml","Linear stability (Mod. Arnoldi): Channel");
	
    //Test Modified Arnoldi adjoint stability (VelCorrectionScheme)
    Execute("IncNavierStokesSolver","ChanStability_adj.xml","Adjoint stability (Mod. Arnoldi): Channel");
	
    //Test Modified Arnoldi Transient growth  (VelCorrectionScheme)
    Execute("IncNavierStokesSolver","bfs_tg.xml","Transient Growth (Modified Arnoldi): Backward-facing step");
	
	//Test 3D homogeneous 1D approach, velocity correction scheme, Laminar Channel Flow
	Execute("IncNavierStokesSolver","Test_ChanFlow_3DH1D_MVM.xml","Laminar Channel Flow 3D homogeneous 1D, P=3, 20 Fourier modes (MVM)");
	Execute("IncNavierStokesSolver","Test_ChanFlow_3DH2D_MVM.xml","Laminar Channel Flow 3D homogeneous 2D, P=3, 8x8 Fourier modes (MVM)");
#ifdef NEKTAR_USING_FFTW
	Execute("IncNavierStokesSolver","Test_ChanFlow_3DH1D_FFT.xml","Laminar Channel Flow 3D homogeneous 1D, P=3, 20 Fourier modes (FFT)");
	Execute("IncNavierStokesSolver","Test_ChanFlow_3DH2D_FFT.xml","Laminar Channel Flow 3D homogeneous 2D, P=3, 8x8 Fourier modes (FFT)");
#endif
	
#ifdef NEKTAR_USING_ARPACK
    //same stability tests with Arpack
	/// @todo Fix ChanStability_Ar regression test to work on all architectures
//	Execute("IncNavierStokesSolver","ChanStability_Ar.xml","Linear stability (Arpack): Channel");
//	Execute("IncNavierStokesSolver","ChanStability_adj_Ar.xml","Adjoint stability (Arpack): Channel");
	Execute("IncNavierStokesSolver","bfs_tg-AR.xml","Transient Growth (Arpack): Backward-facing step");

	//Test Modified Arnoldi direct stability  (CoupledSolver)
	Execute("IncNavierStokesSolver","ChanStability_Coupled.xml","Linear stability with coupled solver (Arpack): Channel");
#endif
    if (tests_failed && !quiet)
    {
        std::cout << "WARNING: " << tests_failed << " test(s) failed." << std::endl;
    }
    else if (verbose)
    {
        std::cout << "All tests passed successfully!" << std::endl;
    }
    return tests_failed;
}

void RunL2RegressionTest(std::string Demo, std::string input, std::string info)
{
    tests_total++;
    if (!quiet)
    {
        std::cout << "TESTING: " << Demo << std::flush;
    }
	std::string NektarSolverDir =std::string("") +  NEKTAR_SOLVER_DIR + "/bin/";
    RegressBase Test(NektarSolverDir.c_str(),Demo,input,"Solvers/IncNavierStokesSolver/OkFiles/");
    int fail;
    
	std::string basename = input;
	basename.erase(basename.end()-4,basename.end());
	boost::filesystem::path filePath(std::string(REG_PATH) + "Solvers/IncNavierStokesSolver/InputFiles/" + basename);
	std::vector<std::string> extensions;
	extensions.push_back(".xml");
	extensions.push_back(".rst");
	extensions.push_back(".bse");
	extensions.push_back("_u_1.bc");
	extensions.push_back("_u_3.bc");
	extensions.push_back("_v_3.bc");

	for (unsigned int i = 0; i < extensions.size(); ++i)
	{
	    struct stat vFileInfo;
	    std::string source  = filePath.file_string() + extensions[i];
	    std::string command = std::string(COPY_COMMAND) + filePath.file_string() + extensions[i] + " .";
	    int vNotPresent = stat(source.c_str(), &vFileInfo);
	    if (!vNotPresent)
	    {
	        int status = system(command.c_str());
	        if(status)
	        {
	            std::cerr << "Unable to copy file:" << source << " to current location" << std::endl;
	            exit(2);
	        }
	    }
	}

    if(fail = Test.TestL2()) // test failed
    {
        if (!quiet)
        {
            std::cout << "\rFAILED: " << Demo << " " << input << " (" << info << ")" << std::endl;
            std::cout << "===========================================================\n";
            // Explain cause of error if available
            Test.PrintTestError(fail);
            std::cout << "===========================================================\n";
        }
        tests_failed++;
    }
    else
    {
        if (verbose)
        {
            std::cout << "\rPASSED: " << Demo << " " << input << " (" << info << ")" << std::endl;
        }
        else if (quiet)
        {
            // print nothing
        }
        else {
            std::cout << "\rPASSED: " << Demo << " (" << info << ")" << std::endl;
        }
        tests_passed++;
    }

#ifdef _WINDOWS
	std::string cleanup = "del /Q *.xml *.fld *.chk *.rst *.bc";
#else
    std::string cleanup = "rm -f *.xml *.fld *.chk *.rst *.bc";
#endif

    system(cleanup.c_str());
};

void MakeOkFile(std::string Demo, std::string input,				std::string info)
{
    tests_total++;
	std::string NektarSolverDir =std::string("") +  NEKTAR_SOLVER_DIR + "/bin/";
    RegressBase Test(NektarSolverDir.c_str(),Demo,input,"Solvers/IncNavierStokesSolver/OkFiles/");
    int fail;
    
    std::string basename = input;
    basename.erase(basename.end()-4,basename.end());
    boost::filesystem::path filePath(std::string(REG_PATH) + "Solvers/IncNavierStokesSolver/InputFiles/" + basename);
    std::vector<std::string> extensions;
    extensions.push_back(".xml");
    extensions.push_back(".rst");
    extensions.push_back(".bse");
    extensions.push_back("_u_1.bc");
    extensions.push_back("_u_3.bc");
    extensions.push_back("_v_3.bc");

    for (unsigned int i = 0; i < extensions.size(); ++i)
    {
        struct stat vFileInfo;
        std::string source  = filePath.file_string() + extensions[i];
        std::string command = std::string(COPY_COMMAND) + filePath.file_string() + extensions[i] + " .";
        int vNotPresent = stat(source.c_str(), &vFileInfo);
        if (!vNotPresent)
        {
            int status = system(command.c_str());
            if(status)
            {
                std::cerr << "Unable to copy file:" << source << " to current location" << std::endl;
                exit(2);
            }
        }
    }

    if(fail = Test.MakeOkFile())
    {
        std::cout << "Failed to make OK file\n";
        // Explain cause of error if available
        std::cout << "===========================================================\n";
        Test.PrintTestError(fail);
        std::cout << "===========================================================\n";
        tests_failed++;
	}
#ifdef _WINDOWS
	std::string cleanup = "del /Q *.xml *.fld *.chk *.rst *.bc";
#else
    std::string cleanup = "rm -f *.xml *.fld *.chk *.rst *.bc";
#endif

    system(cleanup.c_str());
}




