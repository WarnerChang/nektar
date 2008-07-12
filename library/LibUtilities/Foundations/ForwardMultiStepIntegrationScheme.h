///////////////////////////////////////////////////////////////////////////////
//
// File ForwardMultiStepIntegrationScheme.h
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
// Description: Header file of forward multi-step time integration schemes
//
///////////////////////////////////////////////////////////////////////////////

#ifndef NEKTAR_LIB_UTILITIES_FOUNDATIONS_FORWARDMULTISTEPINTEGRATIONSCHEME_H
#define NEKTAR_LIB_UTILITIES_FOUNDATIONS_FORWARDMULTISTEPINTEGRATIONSCHEME_H

#include <math.h>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <LibUtilities/Foundations/Foundations.hpp>
#include <LibUtilities/BasicUtils/ErrorUtil.hpp>
#include <LibUtilities/Foundations/TimeIntegrationScheme.h>
#include <LibUtilities/Memory/NekMemoryManager.hpp>

namespace Nektar
{
    namespace LibUtilities 
    {
        class ForwardMultiStepIntegrationScheme: public TimeIntegrationScheme
        {
        public:

            virtual ~ForwardMultiStepIntegrationScheme()
            {
            }   
 
            static boost::shared_ptr<TimeIntegrationScheme> Create(const TimeIntegrationSchemeKey &tkey);        

            virtual const Array<OneD, const NekDouble>& GetBetaCoefficients(void) const
            {
                return m_beta;
            }

        protected:
            Array<OneD,NekDouble> m_beta;

        private:
            ForwardMultiStepIntegrationScheme(const TimeIntegrationSchemeKey &tkey);

            /// These should not be called.  All creation is done
            /// using the constructor requiring the key, declared
            /// above.
            ForwardMultiStepIntegrationScheme(const ForwardMultiStepIntegrationScheme &in)
            {
                NEKERROR(ErrorUtil::efatal,"Copy Constructor for the ForwardMultiStepIntegrationScheme class should not be called");
            }

            ForwardMultiStepIntegrationScheme():
                TimeIntegrationScheme()
            {
                NEKERROR(ErrorUtil::efatal,"Default Constructor for the ForwardMultiStepIntegrationScheme class should not be called");
            }
        };
    } // end of namespace
} // end of namespace 

#endif //NEKTAR_LIB_UTILITIES_FOUNDATIONS_FORWARDMULTISTEPINTEGRATIONSCHEME_H
