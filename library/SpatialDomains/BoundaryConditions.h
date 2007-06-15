////////////////////////////////////////////////////////////////////////////////
//
//  File:  $Source: /usr/sci/projects/Nektar/cvs/Nektar++/library/SpatialDomains/BoundaryConditions.h,v $
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
//  Description:
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef NEKTAR_SPATIALDOMAINS_BOUNDARYCONDITIONS_H
#define NEKTAR_SPATIALDOMAINS_BOUNDARYCONDITIONS_H

#include <string>
#include <map>

#include <SpatialDomains/Equation.hpp>
#include <LibUtilities/BasicConst/NektarUnivTypeDefs.hpp>
#include <SpatialDomains/MeshGraph.h>

class TiXmlElement;
class TiXmlDocument;

class MeshGraph;

namespace Nektar
{
    namespace SpatialDomains
    {

        struct BoundaryConditionBase
        {
            std::string m_variable;
        };

        struct DirichletBoundaryCondition : public BoundaryConditionBase
        {
            DirichletBoundaryCondition(const std::string &eqn):
                m_DirichletCondition(eqn)
            {
            };

            Equation<NekDouble> m_DirichletCondition;
        };

        struct NeumannBoundaryCondition : public BoundaryConditionBase
        {
            NeumannBoundaryCondition(const std::string &eqn):
                m_NeumannCondition(eqn)
            {
            };

            Equation<NekDouble> m_NeumannCondition;
        };

        struct RobinBoundaryCondition : public BoundaryConditionBase
        {
            RobinBoundaryCondition(const std::string &a, const std::string &b):
                m_a(a), m_b(b)
            {
            }

            // u = a(x,y,z) + b(x,y,z)*\frac{\partial{u}}{\partial{n}}
            Equation<NekDouble> m_a;
            Equation<NekDouble> m_b;
        };

        class BoundaryConditions
        {
        public:
            typedef std::map<std::string, NekDouble> ParamMapType;
            typedef std::vector<std::string> VariableType;
            typedef std::vector<Composite> BoundaryRegionType;
            typedef boost::shared_ptr<BoundaryRegionType> BoundaryRegionShPtrType;
            typedef std::vector<BoundaryRegionShPtrType> BoundaryRegionCollectionType;
            typedef boost::shared_ptr<BoundaryConditionBase> BoundaryConditionShPtrType;
            typedef std::map<std::string, BoundaryConditionShPtrType> BoundaryConditionCollectionType;

            BoundaryConditions(const MeshGraph *meshGraph);
            ~BoundaryConditions();

            void Read(std::string &infilename);
            void Read(TiXmlDocument &doc);

            const ParamMapType &GetParameters(void)
            {
                return m_Parameters;
            }

        protected:
            void ReadParameters(TiXmlElement *conditions);
            void ReadVariables(TiXmlElement *conditions);
            void ReadBoundaryRegions(TiXmlElement *conditions);
            void ReadExpansionTypes(TiXmlElement *conditions);
            void ReadBoundaryConditions(TiXmlElement *conditions);
            void ReadForcingFunctions(TiXmlElement *conditions);
            void ReadInitialConditions(TiXmlElement *conditions);

            // Containers to hold conditions and associated data
            ParamMapType m_Parameters;
            VariableType m_Variables;
            BoundaryRegionCollectionType m_BoundaryRegions;
            BoundaryConditionCollectionType m_BoundaryConditions;

            /// The mesh graph to use for referencing geometry info.
            const MeshGraph *m_MeshGraph;

        private:
            BoundaryConditions();
        };
    };
}

#endif NEKTAR_SPATIALDOMAINS_BOUNDARYCONDITIONS_H
