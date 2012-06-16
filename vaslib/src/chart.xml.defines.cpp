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

/*! \file    chart.xml.defines.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/


#include "chart.xml.defines.h"

/////////////////////////////////////////////////////////////////////////////

QDomElement createNode(QDomElement element, const QString& name)
{
    MYASSERT(!name.isEmpty());
    QDomDocument doc = element.ownerDocument();
    QDomElement child = doc.createElement(name);
    element.appendChild(child);
    MYASSERT(!child.isNull());
    return child;
}

/////////////////////////////////////////////////////////////////////////////

QDomElement checkAndCreateNode(QDomElement element, const QString& name)
{
    MYASSERT(!name.isEmpty());
    QDomElement child = element.firstChildElement(name);
    if (child.isNull()) child = createNode(element, name);
    return child;
}

/////////////////////////////////////////////////////////////////////////////

void writeDOMDoubleAttribute(QDomElement& element, 
                             const QString& attribute,
                             const double& value)
{
    element.setAttribute(attribute, QString::number(value, 'f', 8));
};

/////////////////////////////////////////////////////////////////////////////

void writeDOMIntAttribute(QDomElement& element, 
                          const QString& attribute,
                          const int& value)
{
    element.setAttribute(attribute, QString::number(value));
};

/////////////////////////////////////////////////////////////////////////////

// End of file
