///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2005-2007 Alexander Wemmer 
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

/*! \file    vroute.h
    \author  Alexander Wemmer, alex@wemmer.at
*/

#ifndef VROUTE_H
#define VROUTE_H

#include <QObject>
#include <QString>

class QHttp;

/////////////////////////////////////////////////////////////////////////////

//! compact route
class CompactRoute 
{
public:
    //! Standard Constructor
    CompactRoute() {};

    //! Destructor
    virtual ~CompactRoute() {};

    //-----

    //! returns true if ADEP, ADES and ROUTE are set
    inline bool isValid() { return !m_adep.isEmpty() && !m_ades.isEmpty() && !m_route.isEmpty(); }

public:

    QString m_adep;
    QString m_ades;
    QString m_route;
    QString m_distance;
    QString m_max_fl;
    QString m_min_fl;
};

typedef QList<CompactRoute> CompactRouteList;

/////////////////////////////////////////////////////////////////////////////

//! vroute access
class VRoute : public QObject
{
    Q_OBJECT

public:
    
    //! Standard Constructor
    VRoute(const QString& vroute_host = "www.euroutepro.com", 
           const QString& vroute_dir = 
           "/internal/query.php?type=query&dep=%1&arr=%2&auth_code=2404585ebe3cd4d3f5cbebdcdea45fe5");
    
    //! Destructor
    virtual ~VRoute();

    void setAiracRestiction(const QString& airac) { m_airac_restriction_string = airac; }

    void requestFP(const QString& adep, const QString& ades);

signals:

    void signalGotRoute(const CompactRouteList& compact_route_list);
    void signalError(const QString& error_string);

protected slots:

    void slotCmdFin(int id, bool error);

protected:
    
    QString m_vroute_host;
    QString m_vroute_dir;
    
    QString m_airac_restriction_string;

    QHttp* m_http;
    int m_last_id;

private:
    //! Hidden copy-constructor
    VRoute(const VRoute&);
    //! Hidden assignment operator
    const VRoute& operator = (const VRoute&);
};

#endif /* VROUTE_H */

// End of file

