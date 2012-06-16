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

/*! \file    treebase_xml.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __TREEBASE_XML_H__
#define __TREEBASE_XML_H__

#include <QDomDocument>
#include <QDomElement>

#include "treebase.h"

//! Base class of a tree with a DOM element member
class TreeBaseXML  : public TreeBase
{
    Q_OBJECT

public:
    //! Standard Constructor
    TreeBaseXML(TreeBaseXML* parent);

    //! Destructor
    virtual ~TreeBaseXML();

    //! unlinks this element from it's DOM document
    virtual void unlink();

    //! returns a pointer to the internal DOM element
    virtual QDomElement& getDomElement() { return m_dom_element; }

    virtual QString getTagName() const { return m_dom_element.tagName(); }

    //! set the internal DOM element pointer, returns true on success
    virtual bool loadFromDomElement(QDomElement& dom_element, 
                                    QDomDocument& dom_doc,
                                    QString& err_msg);

    virtual bool saveToDomElement();
    
protected:

    QDomElement m_dom_element;

private:
    //! Hidden copy-constructor
    TreeBaseXML(const TreeBaseXML&);
    //! Hidden assignment operator
    const TreeBaseXML& operator = (const TreeBaseXML&);
};

#endif /* __TREEBASE_H__ */

// End of file

