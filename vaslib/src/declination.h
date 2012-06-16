/* -------------------------------------------------------------- */
/* Simple main program for EMM model evaluation                   */
/*                                                                */
/* The program asks for time, location and magnetic indices.      */
/* In case the magnetic indices are unavailable, their most       */
/* likely value is zero.                                          */
/* Contributions to the model can be turned on and off using      */
/* control parameters. These can also used to set the desired     */
/* degree of the internal and external field.                     */
/*                                          Stefan Maus, Nov-2006 */
/* -------------------------------------------------------------- */

#ifndef DECLINATION_H
#define DECLINATION_H

#include "logger.h"
#include "waypoint.h"

/////////////////////////////////////////////////////////////////////////////

//! declination calculator
class Declination 
{
public:

    Declination(const QString& declination_datafile);
    virtual ~Declination();

    //! Filename shall be specified as a relative path (relative to the vasFMC directory)
    double declination(const Waypoint& location) const;

    static const Declination* globalDeclination() { return m_global_declination; }

protected:

    QString m_declination_datafile;

    static const Declination* m_global_declination;

private:
    //! Hidden copy-constructor
    Declination(const Declination&);
    //! Hidden assignment operator
    const Declination& operator = (const Declination&);
};

#endif /* DECLINATION_H */

// End of file
