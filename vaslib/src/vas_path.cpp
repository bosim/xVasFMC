///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Martin Boehme
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

#include "vas_path.h"

QString VasPath::m_path=".";

/////////////////////////////////////////////////////////////////////////////

QString VasPath::prependPath(const QString &relativePath)
{
    QString ret;
    if (relativePath.isEmpty()) ret = m_path;
    else ret = m_path + (relativePath[0] == '/' ? "" : "/") +  relativePath;
    return ret.trimmed();
}

/////////////////////////////////////////////////////////////////////////////
