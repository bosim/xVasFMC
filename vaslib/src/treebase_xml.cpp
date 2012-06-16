///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2006 Alexander Wemmer 
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
///////////////////////////////////////////////////////////////////////////////

/*! \file    treebase_xml.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "treebase_xml.h"
#include "assert.h"
/////////////////////////////////////////////////////////////////////////////

TreeBaseXML::TreeBaseXML(TreeBaseXML* parent) : TreeBase(parent)
{
}

/////////////////////////////////////////////////////////////////////////////

TreeBaseXML::~TreeBaseXML()
{
}

/////////////////////////////////////////////////////////////////////////////

void TreeBaseXML::unlink()
{
    if (m_dom_element.isNull()) return;
    QDomNode parent = m_dom_element.parentNode();
    if (parent.isNull()) return;
    parent.removeChild(m_dom_element);
}

/////////////////////////////////////////////////////////////////////////////

bool TreeBaseXML::loadFromDomElement(QDomElement& dom_element,
                                     QDomDocument& dom_doc,
                                     QString& err_msg)
{
    MYASSERT(!dom_element.isNull());
    m_dom_element = dom_element; 
    err_msg = QString::null;
    return false;
}

/////////////////////////////////////////////////////////////////////////////

bool TreeBaseXML::saveToDomElement()
{
    TreeIterator iter = iterator();
    while(iter.hasNext())
    {
        iter.next();
        TreeBase* leaf = iter.value();
        MYASSERT(leaf != 0);
        if (!leaf->inherits("TreeBaseXML")) continue;
        ((TreeBaseXML*)leaf)->saveToDomElement();
    }    
    return true;
}

// End of file
