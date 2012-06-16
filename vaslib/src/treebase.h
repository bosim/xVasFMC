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

/*! \file    treebase.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __TREEBASE_H__
#define __TREEBASE_H__

#include <QObject>
#include <QMap>
#include <QString>

class TreeBase;
typedef QMapIterator<QString,TreeBase*> TreeIterator;

//! Base class of a tree
class TreeBase : public QObject
{
    Q_OBJECT

public:
    //! Standard Constructor
    TreeBase(TreeBase* parent);

    //! Destructor
    virtual ~TreeBase();
    
    //! must be implemented by derived classes to enable manual unlinking from
    //! any kind of structure (e.g. unlink the leaf from an XML DOM, etc.)
    virtual void unlink() = 0;
    
    //! Clears the tree deleting all leaves
    virtual void clear();

    //! returns the number of leafs
    virtual unsigned int count() const { return m_leaf_map.count(); }

    //! Adds the given leaf to the tree with autodelete = true
    virtual void addLeaf(const QString& name, TreeBase* leaf);

    //! Removes the given leaf from the tree and deletes it
    virtual void removeAndDeleteLeaf(const QString &name);

    //! Removes the given leaf from the tree and deletes it
    virtual void removeAndDeleteLeaf(TreeBase* leaf);

    //! Returns true if this node contains a leaf with the given name
    virtual bool containsLeaf(const QString& name) const;

    //! Renames the leaf with the given name to the new name.
    //! ATTENTION: If a leaf with the new name already exists, an assertion will be thrown!
    virtual bool renameLeaf(const QString& name, const QString& new_name, QString& err_msg);

    //! Returns a pointer to the leaf with the given name
    virtual const TreeBase* getLeaf(const QString& name) const;

    //! Returns a pointer to the leaf with the given name
    virtual TreeBase* getLeaf(const QString& name);

    //! Returns an iterator to the leafs.
    virtual TreeIterator iterator() const {  return TreeIterator(m_leaf_map); }

    //! Returns the parent of this leaf.
    virtual TreeBase* getParent() const { return m_parent; }

protected:
    
    QMap<QString, TreeBase*> m_leaf_map;

    TreeBase* m_parent;

private:
    //! Hidden copy-constructor
    TreeBase(const TreeBase&);
    //! Hidden assignment operator
    const TreeBase& operator = (const TreeBase&);
};



#endif /* __TREEBASE_H__ */

// End of file

