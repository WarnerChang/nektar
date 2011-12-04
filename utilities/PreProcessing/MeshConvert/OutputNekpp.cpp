////////////////////////////////////////////////////////////////////////////////
//
//  File: OutputNekpp.cpp
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
//  Description: Nektar++ file format output.
//
////////////////////////////////////////////////////////////////////////////////

#include <string>
using namespace std;

#include "MeshElements.h"
#include "OutputNekpp.h"

namespace Nektar
{
    namespace Utilities
    {
        ModuleKey OutputNekpp::className = 
            GetModuleFactory().RegisterCreatorFunction(
                ModuleKey("xml", eOutputModule), OutputNekpp::create);

        OutputNekpp::OutputNekpp(MeshSharedPtr m) : OutputModule(m)
        {
            
        }

        OutputNekpp::~OutputNekpp()
        {

        }
        
        void OutputNekpp::Process()
        {
            TiXmlDocument doc;
            TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "utf-8", "");
            doc.LinkEndChild( decl );

            TiXmlElement * root = new TiXmlElement( "NEKTAR" );
            doc.LinkEndChild( root );

            // Begin <GEOMETRY> section
            TiXmlElement * geomTag = new TiXmlElement( "GEOMETRY" );
            geomTag->SetAttribute("DIM", m->expDim);
            geomTag->SetAttribute("SPACE", m->spaceDim);
            root->LinkEndChild( geomTag );

            WriteXmlNodes     (geomTag);
            WriteXmlEdges     (geomTag);
            WriteXmlFaces     (geomTag);
            WriteXmlElements  (geomTag);
            WriteXmlCurves    (geomTag);
            WriteXmlComposites(geomTag);
            WriteXmlDomain    (geomTag);
            WriteXmlExpansions(root);
            WriteXmlConditions(root);
            
            // Save the XML file.
            mshFile.close();
            doc.SaveFile(m->outFilename);
        }        

        void OutputNekpp::WriteXmlNodes(TiXmlElement * pRoot)
        {
            TiXmlElement* verTag = new TiXmlElement( "VERTEX" );
            NodeSet::iterator it;

            for (it = m->vertexSet.begin(); it != m->vertexSet.end(); ++it)
            {
                NodeSharedPtr n = *it;
                stringstream s;
                s << scientific << setprecision(3) 
                  << n->x << " " << n->y << " " << n->z;
                TiXmlElement * v = new TiXmlElement( "V" );
                v->SetAttribute("ID",n->id);
                v->LinkEndChild(new TiXmlText(s.str()));
                verTag->LinkEndChild(v);
            }
            pRoot->LinkEndChild(verTag);
        }

        void OutputNekpp::WriteXmlEdges(TiXmlElement * pRoot)
        {
            if (m->expDim >= 2)
            {
                int edgecnt = 0;
                TiXmlElement* verTag = new TiXmlElement( "EDGE" );
                EdgeSet::iterator it;
                
                for (it = m->edgeSet.begin(); it != m->edgeSet.end(); ++it)
                {
                    EdgeSharedPtr ed = *it;
                    stringstream s;

                    s << setw(5) << ed->n1->id << "  " << ed->n2->id << "   ";
                    TiXmlElement * e = new TiXmlElement( "E" );
                    e->SetAttribute("ID",ed->id);
                    e->LinkEndChild( new TiXmlText(s.str()) );
                    verTag->LinkEndChild(e);
                }
                pRoot->LinkEndChild( verTag );
            }
        }

        void OutputNekpp::WriteXmlFaces(TiXmlElement * pRoot)
        {
            if (m->expDim == 3)
            {
                TiXmlElement* verTag = new TiXmlElement( "FACE" );
                FaceSet::iterator it;

                for (it = m->faceSet.begin(); it != m->faceSet.end(); ++it)
                {
                    stringstream s;
                    FaceSharedPtr fa = *it;

                    for (int j = 0; j < fa->edgeList.size(); ++j)
                    {
                        s << setw(10) << fa->edgeList[j]->id;
                    }
                    TiXmlElement * f;
                    switch(fa->vertexList.size())
                    {
                        case 3:
                            f = new TiXmlElement("T");
                            break;
                        case 4:
                            f = new TiXmlElement("Q");
                            break;
                        default:
                            abort();
                    }
                    f->SetAttribute("ID", fa->id);
                    f->LinkEndChild( new TiXmlText(s.str()));
                    verTag->LinkEndChild(f);
                }
                pRoot->LinkEndChild( verTag );
            }
        }

        void OutputNekpp::WriteXmlElements(TiXmlElement * pRoot)
        {
            TiXmlElement* verTag = new TiXmlElement( "ELEMENT" );
            vector<ElementSharedPtr> &elmt = m->element[m->expDim];

            for(int i = 0; i < elmt.size(); ++i)
            {
                TiXmlElement *elm_tag = new TiXmlElement(elmt[i]->GetTag());
                elm_tag->SetAttribute("ID", elmt[i]->GetId());
                elm_tag->LinkEndChild(new TiXmlText(elmt[i]->GetXmlString()));
                verTag->LinkEndChild(elm_tag);
            }
            pRoot->LinkEndChild(verTag);
        }

        void OutputNekpp::WriteXmlCurves(TiXmlElement * pRoot)
        {
            int edgecnt = 0;
            int facecnt = 0;

            bool curve = false;
            EdgeSet::iterator it;
            for (it = m->edgeSet.begin(); it != m->edgeSet.end(); ++it)
            {
                if ((*it)->edgeNodes.size() > 0) 
                {
                    curve = true;
                    break;
                }
            }
            if (!curve) return;

            TiXmlElement * curved = new TiXmlElement ("CURVED" );

            for (it = m->edgeSet.begin(); it != m->edgeSet.end(); ++it)
            {
                if ((*it)->edgeNodes.size() > 0)
                {
                    TiXmlElement * e = new TiXmlElement( "E" );
                    e->SetAttribute("ID",        edgecnt++);
                    e->SetAttribute("EDGEID",    (*it)->id);
                    e->SetAttribute("NUMPOINTS", (*it)->GetNodeCount());
                    e->SetAttribute("TYPE", 
                                    LibUtilities::kPointsTypeStr[(*it)->curveType]);
                    TiXmlText * t0 = new TiXmlText((*it)->GetXmlCurveString());
                    e->LinkEndChild(t0);
                    curved->LinkEndChild(e);
                }
            }

            FaceSet::iterator it2;
            for (it2 = m->faceSet.begin(); it2 != m->faceSet.end(); ++it2)
            {
                if ((*it2)->faceNodes.size() > 0)
                {
                    TiXmlElement * f = new TiXmlElement( "F" );
                    f->SetAttribute("ID",       facecnt++);
                    f->SetAttribute("FACEID",   (*it2)->id);
                    f->SetAttribute("NUMPOINTS",(*it2)->GetNodeCount());
                    f->SetAttribute("TYPE",
                                    LibUtilities::kPointsTypeStr[(*it2)->curveType]);
                    TiXmlText * t0 = new TiXmlText((*it2)->GetXmlCurveString());
                    f->LinkEndChild(t0);
                    curved->LinkEndChild(f);
                }
            }
            pRoot->LinkEndChild( curved );
        }

        void OutputNekpp::WriteXmlComposites(TiXmlElement * pRoot)
        {
            TiXmlElement* verTag = new TiXmlElement("COMPOSITE");
            CompositeMap::iterator it;

            for (it = m->composite.begin(); it != m->composite.end(); ++it)
            {
                if (it->second->items.size() > 0) 
                {
                    TiXmlElement *comp_tag = new TiXmlElement("C"); // Composite
                    TiXmlElement *elm_tag;

                    comp_tag->SetAttribute("ID", it->second->id);
                    comp_tag->LinkEndChild( new TiXmlText(it->second->GetXmlString()) );
                    verTag->LinkEndChild(comp_tag);
                }
                else
                {
                    cout << "Composite " << it->second->id << " contains nothing." << endl;
                }
            }

            pRoot->LinkEndChild( verTag );
        }

        void OutputNekpp::WriteXmlDomain(TiXmlElement * pRoot)
        {
            // Write the <DOMAIN> subsection.
            TiXmlElement * domain = new TiXmlElement ("DOMAIN" );
            std::string list;
            CompositeMap::iterator it;
            
            for (it = m->composite.begin(); it != m->composite.end(); ++it)
            {
                if (it->second->items[0]->GetDim() == m->expDim)
                {
                    if (list.length() > 0)
                    {
                        list += ",";
                    }
                    list += boost::lexical_cast<std::string>(it->second->id);
                }
            }
            domain->LinkEndChild( new TiXmlText(" C[" + list + "] "));
            pRoot->LinkEndChild( domain );
        }

        void OutputNekpp::WriteXmlExpansions(TiXmlElement * pRoot)
        {
            // Write a default <EXPANSIONS> section.
            TiXmlElement * expansions = new TiXmlElement ("EXPANSIONS");
            CompositeMap::iterator it;
            
            for (it = m->composite.begin(); it != m->composite.end(); ++it)
            {
                if (it->second->items[0]->GetDim() == m->expDim)
                {
                    TiXmlElement * exp = new TiXmlElement ( "E");
                    exp->SetAttribute("COMPOSITE", "C["
                        + boost::lexical_cast<std::string>(it->second->id)
                        + "]");
                    exp->SetAttribute("NUMMODES",7);
                    exp->SetAttribute("FIELDS","u");
                    exp->SetAttribute("TYPE","MODIFIED");
                    expansions->LinkEndChild(exp);
                }
            }
            pRoot->LinkEndChild(expansions);
        }
        
        void OutputNekpp::WriteXmlConditions(TiXmlElement * pRoot)
        {
            TiXmlElement * conditions = new TiXmlElement ("CONDITIONS");
            pRoot->LinkEndChild(conditions);
        }   
    }
}