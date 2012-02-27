////////////////////////////////////////////////////////////////////////////////
//
//  File:  $Source: /usr/sci/projects/Nektar/cvs/Nektar++/library/SpatialDomains/Equation.cpp,v $
//
//  For more information, please see: http://www.nektar.info/
//
//  The MIT License
//
//  Copyright (c) 2006 Division of Applied Mathematics, Brown University (USA),
//  Department of Aeronautics, Imperial College London (UK), and Scientific
//  Computing and Imaging Institute, University of Utah (USA).
//
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//  Description:  Wrapper to ExpressionEvaluator class
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NEKTAR_SPATIALDOMAINS_EQUATION_CPP
#define NEKTAR_SPATIALDOMAINS_EQUATION_CPP

#include <LibUtilities/BasicUtils/Equation.h>

namespace Nektar
{
    namespace LibUtilities
    {
        /**  @class Equation
        *    This class stores a string form of a symbolic expression
        *    to be evaluated e.g. for the boundary conditions, the unique
        *    numeric ID of that expression and a reference to the unique
        *    static instance of ExpressionEvaluator.
        *
        *    Typical scenario is that for multiple copies of
        *    Equation class holding their symbolic expressions in the
        *    std::string form, there is a unique instance of
        *    ExpressionEvaluator which holds the set of pre-processed
        *    symbolic expressions in the form of ASTs ready for fast
        *    evaluation. ExpressionEvaluator also keeps all constants
        *    and parameters specified in an XML file. There should be
        *    only one copy of Equation class per each symbolic expression
        *    specified in XML file, modulo possible bugs. Classes Equation
        *    and ExpressionEvaluator live symbiotic in a sense that the
        *    expression id stored in Equation class is generated by
        *    ExpressionEvaluator which holds ordered container of
        *    pre-processed expressions.
        *
        *    However there is also not that typical scenario: Equation
        *    class is used as a wrapper to std::string holding a link
        *    to file with boundary conditions. As a result, Equation
        *    is instantiated with nonparsable string -> ExpressionEvaluator
        *    throws an exception when Equation tries to call DefineFunction
        *    with that string parameter -> Equation catches an exception
        *    and let DisContField2D (the only part of the code using
        *    files to store boundary conditions) run smoothly without
        *    further attempts to evaluate an expression.
        *
        */
        LibUtilities::AnalyticExpressionEvaluator Equation::m_evaluator; 
    }
}

#endif //NEKTAR_SPATIALDOMAINS_EQUATION_CPP
