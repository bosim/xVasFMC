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

/*! \file    checklist.cpp
    \author  Alexander Wemmer, alex@wemmer.at
*/

#include <QFile>

#include "fmc_sounds_handler.h"
#include "fmc_sounds.h"

#include "checklist.h"

/////////////////////////////////////////////////////////////////////////////

const ChecklistItem& Checklist::item(int index) const
{
    if (index < 0 || index >= m_checklist.count()) return m_empty_checklist_item;
    return m_checklist.at(index);
}


/////////////////////////////////////////////////////////////////////////////

void Checklist::incItemIndex() 
{
    if (!isAtLastItem() && 
        m_fmc_sounds_handler != 0 &&
        m_fmc_sounds_handler->fmcSounds() != 0 &&
        !item(m_current_item_index+1).checkSoundfile().isEmpty())
    {
        m_fmc_sounds_handler->fmcSounds()->addSoundToQueueDirectly(
            item(m_current_item_index+1).checkSoundfile(), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT, true);

        if (!item(m_current_item_index+1).resultSoundfile().isEmpty())
            m_fmc_sounds_handler->fmcSounds()->addSoundToQueueDirectly(
                item(m_current_item_index+1).resultSoundfile(), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT, true);
    }

    m_current_item_index = qMin(m_checklist.count(), ++m_current_item_index);
}

/////////////////////////////////////////////////////////////////////////////

void ChecklistManager::setSoundHandler(FMCSoundsHandler* fmc_sounds_handler) 
{ 
    m_fmc_sounds_handler = fmc_sounds_handler; 
    for(int index=0; index < count(); ++index) checklist(index).setSoundHandler(fmc_sounds_handler);
}

/////////////////////////////////////////////////////////////////////////////

bool ChecklistManager::loadFromFile(const QString& filename)
{
    clear();

    //----- open the file

    QString full_filename = m_base_dir + "/" + filename;
    QFile file(full_filename);
    if (!file.open(QIODevice::ReadOnly)) 
    {
        Logger::log(QString("ChecklistManager:loadFromFile: Could not open file %1\n%2").
                    arg(full_filename).arg(file.errorString()));
        return false;
    }

    //----- read in the checklists

    Logger::log(QString("ChecklistManager:loadFromFile: Reading file %1").arg(full_filename));

    uint cl_counter = 0;
    uint item_counter = 0;

    unsigned long line_counter = 0;
    bool error_while_reading = false;
    QTextStream instream(&file);

    while (!instream.atEnd()) 
    {
        ++line_counter;

        QString line = instream.readLine().trimmed();
        if (line.length() <= 0 || line.startsWith("#")) continue;

        if (line.startsWith("[") && line.endsWith("]"))
        {
            QString cl_name_string = line.mid(1, line.length()-2);
            QStringList items = cl_name_string.split("|", QString::KeepEmptyParts);

            m_checklist_list.append(Checklist(m_fmc_sounds_handler, items[0]));
            m_current_checklist_index = m_checklist_list.count() - 1;

            if (items.count() > 1) currentChecklist().setChecklistSoundfile(items[1]);
            ++cl_counter;
        }
        else
        {
            QStringList items = line.split("|", QString::KeepEmptyParts);

            ChecklistItem new_item;
            new_item.setCheckText(items[0]);
            if (items.count() >= 2) new_item.setResultText(items[1]);
            if (items.count() >= 3) new_item.setCheckSoundfile(items[2]);
            if (items.count() >= 4) new_item.setResultSoundfile(items[3]);

            currentChecklist().appendItem(new_item);
            ++item_counter;
        }      
        
    }

    Logger::log(QString("ChecklistManager:loadFromFile: Loaded %1 CLs with %2 items").arg(cl_counter).arg(item_counter));

    resetChecklistIndex();
    file.close();
    return !error_while_reading;
}

/////////////////////////////////////////////////////////////////////////////

void ChecklistManager::incChecklistIndex() 
{
    if (!isAtLastChecklist() && 
        m_fmc_sounds_handler != 0 &&
        m_fmc_sounds_handler->fmcSounds() != 0 &&
        !checklist(m_current_checklist_index+1).checklistSoundfile().isEmpty())
    {
        m_fmc_sounds_handler->fmcSounds()->addSoundToQueueDirectly(
            checklist(m_current_checklist_index+1).checklistSoundfile(), FMCSoundBase::SOUND_SOURCE_OTHER_PILOT, true);
    }

    m_current_checklist_index = qMin(m_checklist_list.count()-1, ++m_current_checklist_index); 
}

/////////////////////////////////////////////////////////////////////////////


bool ChecklistManager::decChecklistItemIndex()
{
    bool checklist_changed = false;

    if (currentChecklist().isAtFirstItem())
    {
        if (!isAtFirstChecklist())
        {
            decChecklistIndex();
            checklist_changed = true;
        }
        else
        {
            resetChecklistIndex();
            checklist_changed = true;
        }
    }
    else
    {
        currentChecklist().decItemIndex();
    }

    return checklist_changed;
}

/////////////////////////////////////////////////////////////////////////////

bool ChecklistManager::incChecklistItemIndex()
{
    bool checklist_changed = false;

    if (currentChecklist().isAtLastItem())
    {
        if (!isAtLastChecklist())
        {
            incChecklistIndex();
            currentChecklist().resetItemIndex();
            checklist_changed = true;
        }
        else
        {
            resetChecklistIndex();
            checklist_changed = true;
        }
    }
    else
    {
        currentChecklist().incItemIndex();
    }

    return checklist_changed;
}

/////////////////////////////////////////////////////////////////////////////

// End of file
