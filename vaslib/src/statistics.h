///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2008 Alexander Wemmer 
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

/*! \file    statistics.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

class QFile;
class QString;

/////////////////////////////////////////////////////////////////////////////

//! date collector
class Statistics
{
public:
    //! Standard Constructor
    Statistics(const QString& filename, const QString& separator = " ");

    //! Destructor
    virtual ~Statistics();

    void putItemInLine(const QString& item);

    void nextLine();

protected:

    QFile *m_stat_file;
    QString m_separator;

private:
    //! Hidden copy-constructor
    Statistics(const Statistics&);
    //! Hidden assignment operator
    const Statistics& operator = (const Statistics&);
};



#endif /* __STATISTICS_H__ */

// End of file

