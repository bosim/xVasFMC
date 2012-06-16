///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2009 Alexander Wemmer 
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

/*! \file    checklist.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __CHECKLIST_H__
#define __CHECKLIST_H__

#include <QString>
#include <QList>

#include "logger.h"

class FMCSoundsHandler;

/////////////////////////////////////////////////////////////////////////////

//! check list item
class ChecklistItem 
{
public:
    //! Standard Constructor
    ChecklistItem() {}

    //! Destructor
    virtual ~ChecklistItem() {}

    //-----

    const QString& checkText() const { return m_check_text; }
    void setCheckText(const QString& check_text) { m_check_text = check_text; }

    const QString& resultText() const { return m_result_text; }
    void setResultText(const QString& result_text) { m_result_text = result_text; }

    const QString& checkSoundfile() const { return m_check_soundfile; }
    void setCheckSoundfile(const QString& filename) { m_check_soundfile = filename; }

    const QString& resultSoundfile() const { return m_result_soundfile; }
    void setResultSoundfile(const QString& filename) { m_result_soundfile = filename; }

protected:

    QString m_check_text;
    QString m_result_text;

    QString m_check_soundfile;
    QString m_result_soundfile;
};

/////////////////////////////////////////////////////////////////////////////

//! checklist class
class Checklist
{
public:
    //! Standard Constructor
    Checklist(FMCSoundsHandler* fmc_sounds_handler = 0,
              const QString& name = QString::null) : 
        m_fmc_sounds_handler(fmc_sounds_handler), m_name(name), m_current_item_index(-1) {}

    //! Destructor
    virtual ~Checklist() {}

    //-----

    void setSoundHandler(FMCSoundsHandler* fmc_sounds_handler) { m_fmc_sounds_handler = fmc_sounds_handler; }

    int count() const { return m_checklist.count(); }

    const QString& name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    void resetItemIndex() { m_current_item_index = -1; }
    void incItemIndex();
    void decItemIndex() { m_current_item_index = qMax(-1, --m_current_item_index); }

    int currentItemIndex() const { return m_current_item_index; }

    const ChecklistItem& item(int index) const;
    const ChecklistItem& currentItem() const { return item(currentItemIndex()); }

    void appendItem(const ChecklistItem& item) { m_checklist.append(item); }

    bool isAtFirstItem() const { return currentItemIndex() < 0; }
    bool isAtLastItem() const { return currentItemIndex() >= count() || (count() == 0 && isAtFirstItem()); }

    const QString& checklistSoundfile() const { return m_check_list_soundfile; }
    void setChecklistSoundfile(const QString& filename) { m_check_list_soundfile = filename; }

protected:

    FMCSoundsHandler* m_fmc_sounds_handler;
    QString m_name;
    QList<ChecklistItem> m_checklist;
    ChecklistItem m_empty_checklist_item;
    int m_current_item_index;
    QString m_check_list_soundfile;
};

/////////////////////////////////////////////////////////////////////////////

//! ChecklistManager
class ChecklistManager
{
public:
    //! Standard Constructor
    ChecklistManager(const QString& checklist_base_dir) :
        m_base_dir(checklist_base_dir), m_fmc_sounds_handler(0), m_current_checklist_index(-1) 
    {
    }

    //! Destructor
    virtual ~ChecklistManager() {}

    //-----

    void setSoundHandler(FMCSoundsHandler* fmc_sounds_handler);

    void clear()
    {
        m_checklist_list.clear();
        resetChecklistIndex();
    }

    int count() const { return m_checklist_list.count(); }

    //! loads the checklists from the given file. returns true on success, false otherwise.
    bool loadFromFile(const QString& filename);

    void resetChecklistIndex() 
    {
        m_current_checklist_index = -1;
        for(int index=0; index < count(); ++index) checklist(index).resetItemIndex();
    }

    void setCurrentChecklistIndex(int index) { m_current_checklist_index = qMax(-1, qMin(m_checklist_list.count(), index)); }
    int currentChecklistIndex() const { return m_current_checklist_index; }

    const Checklist& currentChecklist() const { return checklist(currentChecklistIndex()); }
    const Checklist& checklist(int index) const 
    {
        if (index < 0 || index >= m_checklist_list.count()) return m_empty_checklist;
        return m_checklist_list.at(index);
    }

    Checklist& currentChecklist() { return checklist(currentChecklistIndex()); }
    Checklist& checklist(int index)
    {
        if (index < 0 || index >= m_checklist_list.count()) return m_empty_checklist;
        return m_checklist_list[index];
    }

    void incChecklistIndex();
    void decChecklistIndex() { m_current_checklist_index = qMax(-1, --m_current_checklist_index); }

    bool isAtFirstChecklist() const { return currentChecklistIndex() <= 0; }
    bool isAtLastChecklist() const { return currentChecklistIndex() >= count()-1 || (count() == 0 && isAtFirstChecklist()); }

    //! increases the index of the current checklist item of the current
    //! checklist. if the checklist is complete, this call will jump to the
    //! first item of the next checklist (if any). If the last item of the
    //! last checklist is reached, this call will reset the whole checklist state.
    //! Returns true if a jump to the next checklist occured, false otherwise.
    bool incChecklistItemIndex();

    //! decreases the index of the current checklist item of the current
    //! checklist. if the first checklist item is reached, this call will jump to the
    //! last item of the previous checklist (if any). If the first item of the
    //! first checklist is reached, this call will reset the whole checklist state.
    //! Returns true if a jump to the previous checklist occured, false otherwise.
    bool decChecklistItemIndex();

protected:
                                
    QString m_base_dir;
    FMCSoundsHandler* m_fmc_sounds_handler;
    QList<Checklist> m_checklist_list;
    Checklist m_empty_checklist;
    int m_current_checklist_index;

private:
    //! Hidden copy-constructor
    ChecklistManager(const ChecklistManager&);
    //! Hidden assignment operator
    const ChecklistManager& operator = (const ChecklistManager&);
};

#endif /* __CHECKLIST_H__ */

// End of file

