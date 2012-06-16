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

/*! \file    treebase.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include "treebase.h"

/////////////////////////////////////////////////////////////////////////////

TreeBase::TreeBase(TreeBase* parent) : m_parent(parent)
{
}

/////////////////////////////////////////////////////////////////////////////

TreeBase::~TreeBase()
{
    clear();
}

/////////////////////////////////////////////////////////////////////////////

void TreeBase::clear()
{
    qDeleteAll(m_leaf_map);
    m_leaf_map.clear(); 
}

/////////////////////////////////////////////////////////////////////////////

void TreeBase::addLeaf(const QString& name, TreeBase* leaf)
{
    MYASSERT(!name.isEmpty());
    MYASSERT(leaf != 0);
    MYASSERT(!m_leaf_map.contains(name));
    m_leaf_map.insert(name, leaf);
}

/////////////////////////////////////////////////////////////////////////////

void TreeBase::removeAndDeleteLeaf(const QString& name)
{
    MYASSERT(!name.isEmpty());
    MYASSERT(m_leaf_map.contains(name));
    TreeBase* leaf = m_leaf_map.take(name);
    if (leaf != 0) delete leaf;
}

/////////////////////////////////////////////////////////////////////////////

void TreeBase::removeAndDeleteLeaf(TreeBase* leaf)
{
    MYASSERT(leaf != 0);
    QMap<QString, TreeBase*>::iterator iter = m_leaf_map.begin();
    for(; iter != m_leaf_map.end(); ++iter)
    {
        if (static_cast<TreeBase*>(iter.value()) == leaf)
        {
            MYASSERT(iter.value() != 0);
            iter.value()->unlink();
            delete iter.value();
            m_leaf_map.erase(iter);
            return;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

bool TreeBase::containsLeaf(const QString& name) const
{
    MYASSERT(!name.isEmpty());
    return m_leaf_map.contains(name);
}

/////////////////////////////////////////////////////////////////////////////

bool TreeBase::renameLeaf(const QString& name, const QString& new_name, QString& err_msg)
{
    MYASSERT(!name.isEmpty());
    MYASSERT(!new_name.isEmpty());

    if (containsLeaf(new_name))
    {
        err_msg = QString("Tree already contains a leaf with name %1").arg(new_name);
        return false;
    }
    
    TreeBase* leaf = m_leaf_map.take(name);
    m_leaf_map.insert(new_name, leaf);

    MYASSERT(!containsLeaf(name));
    MYASSERT(containsLeaf(new_name));
    return true;
}

/////////////////////////////////////////////////////////////////////////////

const TreeBase* TreeBase::getLeaf(const QString& name) const
{
    if (!m_leaf_map.contains(name)) return 0;
    return m_leaf_map.find(name).value();
}

/////////////////////////////////////////////////////////////////////////////

TreeBase* TreeBase::getLeaf(const QString& name)
{
    if (!m_leaf_map.contains(name)) return 0;
    return m_leaf_map.find(name).value();
}

// End of file
