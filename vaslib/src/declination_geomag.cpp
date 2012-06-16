
#include <QString>

#include "declination_geomag.h"

#define NaN log(-1.0)

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define IEXT 0

// Originally RECL was 81, incremented for CRLF instead of LF alone
#define RECL 82

#define MAXINBUFF RECL+14

/** Max size of in buffer **/

#define MAXREAD MAXINBUFF-2
/** Max to read 2 less than total size (just to be safe) **/

#define MAXMOD 30
/** Max number of models in a file **/

#define PATH MAXREAD
/** Max path and filename length **/

#define EXT_COEFF1 (float)0
#define EXT_COEFF2 (float)0
#define EXT_COEFF3 (float)0

#define MAXDEG 13
#define MAXCOEFF (MAXDEG*(MAXDEG+2)+1) /* index starts with 1!, (from old Fortran?) */

float gh1[MAXCOEFF];
float gh2[MAXCOEFF];
float gha[MAXCOEFF];              /* Geomag global variables */
float ghb[MAXCOEFF];
float d=0,f=0,h=0,i=0;
float dtemp,ftemp,htemp,itemp;
float x=0,y=0,z=0;
float xtemp,ytemp,ztemp;
    
FILE *stream = NULL;                /* Pointer to specified model data file */

/****************************************************************************/
/*                                                                          */
/*                           Subroutine getshc                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Reads spherical harmonic coefficients from the specified             */
/*     model into an array.                                                 */
/*                                                                          */
/*     Input:                                                               */
/*           stream     - Logical unit number                               */
/*           iflag      - Flag for SV equal to ) or not equal to 0          */
/*                        for designated read statements                    */
/*           strec      - Starting record number to read from model         */
/*           nmax_of_gh - Maximum degree and order of model                 */
/*                                                                          */
/*     Output:                                                              */
/*           gh1 or 2   - Schmidt quasi-normal internal spherical           */
/*                        harmonic coefficients                             */
/*                                                                          */
/*     FORTRAN                                                              */
/*           Bill Flanagan                                                  */
/*           NOAA CORPS, DESDIS, NGDC, 325 Broadway, Boulder CO.  80301     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 15, 1988                                                */
/*                                                                          */
/****************************************************************************/


int getshc(char file[PATH], int iflag, long strec, int nmax_of_gh, int gh)
{
    char  inbuff[MAXINBUFF];
    char irat[9];
    int ii,m,n,mm,nn;
    int ios = 0;
    int line_num;
    float g,hh;
    float trash;

    stream = fopen(file, "rt");
    if (stream == NULL)
    {
        printf("\nError on opening file %s", file);
    }
    else
    {
        ii = 0;
        ios = 0;
        fseek(stream,strec,SEEK_SET);
        for ( nn = 1; nn <= nmax_of_gh; ++nn)
        {
            for (mm = 0; mm <= nn; ++mm)
            {
                if (iflag == 1)
                {
                    fgets(inbuff, 3, stream);
                    inbuff[3]='\0';
                    sscanf(inbuff, "%d", &m);
                    fgets(inbuff, 3, stream);
                    inbuff[3]='\0';
                    sscanf(inbuff, "%d", &n);
                    fgets(inbuff, MAXREAD-4, stream);
                    sscanf(inbuff, "%f%f%f%f%s%d",
                           &g, &hh, &trash, &trash, irat, &line_num);
                }
                else
                {
                    fgets(inbuff, 3, stream);
                    inbuff[3]='\0';
                    sscanf(inbuff, "%d", &m);
                    fgets(inbuff, 3, stream);
                    inbuff[3]='\0';
                    sscanf(inbuff, "%d", &n);
                    fgets(inbuff, MAXREAD-4, stream);
                    sscanf(inbuff, "%f%f%f%f%s%d",
                           &trash, &trash, &g, &hh, irat, &line_num);
                }
                if ((nn != n) || (mm != m))
                {
                    ios = -2;
                    fclose(stream);
                    return(ios);
                }
                ii = ii + 1;
                switch(gh)
                {
                    case 1:  gh1[ii] = g;
                        break;
                    case 2:  gh2[ii] = g;
                        break;
                    default: printf("\nError in subroutine getshc");
                        break;
                }
                if (m != 0)
                {
                    ii = ii+ 1;
                    switch(gh)
                    {
                        case 1:  gh1[ii] = hh;
                            break;
                        case 2:  gh2[ii] = hh;
                            break;
                        default: printf("\nError in subroutine getshc");
                            break;
                    }
                }
            }
        }
    }
    fclose(stream);
    return(ios);
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine extrapsh                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Extrapolates linearly a spherical harmonic model with a              */
/*     rate-of-change model.                                                */
/*                                                                          */
/*     Input:                                                               */
/*           date     - date of resulting model (in decimal year)           */
/*           dte1     - date of base model                                  */
/*           nmax1    - maximum degree and order of base model              */
/*           gh1      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of base model                 */
/*           nmax2    - maximum degree and order of rate-of-change model    */
/*           gh2      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of rate-of-change model       */
/*                                                                          */
/*     Output:                                                              */
/*           gha or b - Schmidt quasi-normal internal spherical             */
/*                    harmonic coefficients                                 */
/*           nmax   - maximum degree and order of resulting model           */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 16, 1988                                                */
/*                                                                          */
/****************************************************************************/

int extrapsh(float date, float dte1, int nmax1, int nmax2, int gh)
{
    int   nmax;
    int   k, l;
    int   ii;
    float factor;

    factor = date - dte1;
    if (nmax1 == nmax2)
    {
        k =  nmax1 * (nmax1 + 2);
        nmax = nmax1;
    }
    else
    {
        if (nmax1 > nmax2)
        {
            k = nmax2 * (nmax2 + 2);
            l = nmax1 * (nmax1 + 2);
            switch(gh)
            {
                case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                    gha[ii] = gh1[ii];
                }
                    break;
                case 4:  for ( ii = k + 1; ii <= l; ++ii)
                    {
                        ghb[ii] = gh1[ii];
                    }
                    break;
                default: printf("\nError in subroutine extrapsh");
                    break;
            }
            nmax = nmax1;
        }
        else
        {
            k = nmax1 * (nmax1 + 2);
            l = nmax2 * (nmax2 + 2);
            switch(gh)
            {
                case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                    gha[ii] = factor * gh2[ii];
                }
                    break;
                case 4:  for ( ii = k + 1; ii <= l; ++ii)
                    {
                        ghb[ii] = factor * gh2[ii];
                    }
                    break;
                default: printf("\nError in subroutine extrapsh");
                    break;
            }
            nmax = nmax2;
        }
    }
    switch(gh)
    {
        case 3:  for ( ii = 1; ii <= k; ++ii)
        {
            gha[ii] = gh1[ii] + factor * gh2[ii];
        }
            break;
        case 4:  for ( ii = 1; ii <= k; ++ii)
            {
                ghb[ii] = gh1[ii] + factor * gh2[ii];
            }
            break;
        default: printf("\nError in subroutine extrapsh");
            break;
    }
    return(nmax);
}

/****************************************************************************/
/*                                                                          */
/*                           Subroutine interpsh                            */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Interpolates linearly, in time, between two spherical harmonic       */
/*     models.                                                              */
/*                                                                          */
/*     Input:                                                               */
/*           date     - date of resulting model (in decimal year)           */
/*           dte1     - date of earlier model                               */
/*           nmax1    - maximum degree and order of earlier model           */
/*           gh1      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of earlier model              */
/*           dte2     - date of later model                                 */
/*           nmax2    - maximum degree and order of later model             */
/*           gh2      - Schmidt quasi-normal internal spherical             */
/*                      harmonic coefficients of internal model             */
/*                                                                          */
/*     Output:                                                              */
/*           gha or b - coefficients of resulting model                     */
/*           nmax     - maximum degree and order of resulting model         */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 17, 1988                                                */
/*                                                                          */
/****************************************************************************/

int interpsh(float date, float dte1, int nmax1, float dte2, int nmax2, int gh)
{
    int   nmax;
    int   k, l;
    int   ii;
    float factor;

    factor = (date - dte1) / (dte2 - dte1);
    if (nmax1 == nmax2)
    {
        k =  nmax1 * (nmax1 + 2);
        nmax = nmax1;
    }
    else
    {
        if (nmax1 > nmax2)
        {
            k = nmax2 * (nmax2 + 2);
            l = nmax1 * (nmax1 + 2);
            switch(gh)
            {
                case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                    gha[ii] = gh1[ii] + factor * (-gh1[ii]);
                }
                    break;
                case 4:  for ( ii = k + 1; ii <= l; ++ii)
                    {
                        ghb[ii] = gh1[ii] + factor * (-gh1[ii]);
                    }
                    break;
                default: printf("\nError in subroutine extrapsh");
                    break;
            }
            nmax = nmax1;
        }
        else
        {
            k = nmax1 * (nmax1 + 2);
            l = nmax2 * (nmax2 + 2);
            switch(gh)
            {
                case 3:  for ( ii = k + 1; ii <= l; ++ii)
                {
                    gha[ii] = factor * gh2[ii];
                }
                    break;
                case 4:  for ( ii = k + 1; ii <= l; ++ii)
                    {
                        ghb[ii] = factor * gh2[ii];
                    }
                    break;
                default: printf("\nError in subroutine extrapsh");
                    break;
            }
            nmax = nmax2;
        }
    }
    switch(gh)
    {
        case 3:  for ( ii = 1; ii <= k; ++ii)
        {
            gha[ii] = gh1[ii] + factor * (gh2[ii] - gh1[ii]);
        }
            break;
        case 4:  for ( ii = 1; ii <= k; ++ii)
            {
                ghb[ii] = gh1[ii] + factor * (gh2[ii] - gh1[ii]);
            }
            break;
        default: printf("\nError in subroutine extrapsh");
            break;
    }
    return(nmax);
}

/****************************************************************************/
/*                                                                          */
/*                           Subroutine shval3                              */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Calculates field components from spherical harmonic (sh)             */
/*     models.                                                              */
/*                                                                          */
/*     Input:                                                               */
/*           igdgc     - indicates coordinate system used; set equal        */
/*                       to 1 if geodetic, 2 if geocentric                  */
/*           latitude  - north latitude, in degrees                         */
/*           longitude - east longitude, in degrees                         */
/*           elev      - WGS84 altitude above mean sea level (igdgc=1), or  */
/*                       radial distance from earth's center (igdgc=2)      */
/*           a2,b2     - squares of semi-major and semi-minor axes of       */
/*                       the reference spheroid used for transforming       */
/*                       between geodetic and geocentric coordinates        */
/*                       or components                                      */
/*           nmax      - maximum degree and order of coefficients           */
/*           iext      - external coefficients flag (=0 if none)            */
/*           ext1,2,3  - the three 1st-degree external coefficients         */
/*                       (not used if iext = 0)                             */
/*                                                                          */
/*     Output:                                                              */
/*           x         - northward component                                */
/*           y         - eastward component                                 */
/*           z         - vertically-downward component                      */
/*                                                                          */
/*     based on subroutine 'igrf' by D. R. Barraclough and S. R. C. Malin,  */
/*     report no. 71/1, institute of geological sciences, U.K.              */
/*                                                                          */
/*     FORTRAN                                                              */
/*           Norman W. Peddie                                               */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 17, 1988                                                */
/*                                                                          */
/****************************************************************************/

int shval3(
    int   igdgc,
    float flat,
    float flon,
    float elev,
    int   nmax,
    int   gh,
    int   iext,
    float ext1,
    float ext2,
    float ext3)
{
    float earths_radius = 6371.2;
    float dtr = 0.01745329;
    float slat;
    float clat;
    float ratio;
    float aa, bb, cc, dd;
    float sd;
    float cd;
    float r;
    float a2;
    float b2;
    float rr = 0.0;
    float fm = 0.0,fn = 0.0;
    float sl[14];
    float cl[14];
    float p[119];
    float q[119];
    int ii,j,k,l,m,n;
    int npq;
    int ios;
    double arguement;
    double power;
    a2 = 40680631.59;            /* WGS84 */
    b2 = 40408299.98;            /* WGS84 */
    ios = 0;
    r = elev;
    arguement = flat * dtr;
    slat = sin( arguement );
    if ((90.0 - flat) < 0.001)
    {
        aa = 89.999;            /*  300 ft. from North pole  */
    }
    else
    {
        if ((90.0 + flat) < 0.001)
        {
            aa = -89.999;        /*  300 ft. from South pole  */
        }
        else
        {
            aa = flat;
        }
    }
    arguement = aa * dtr;
    clat = cos( arguement );
    arguement = flon * dtr;
    sl[1] = sin( arguement );
    cl[1] = cos( arguement );
    switch(gh)
    {
        case 3:  x = 0;
            y = 0;
            z = 0;
            break;
        case 4:  xtemp = 0;
            ytemp = 0;
            ztemp = 0;
            break;
        default: printf("\nError in subroutine shval3");
            break;
    }
    sd = 0.0;
    cd = 1.0;
    l = 1;
    n = 0;
    m = 1;
    npq = (nmax * (nmax + 3)) / 2;
    if (igdgc == 1)
    {
        aa = a2 * clat * clat;
        bb = b2 * slat * slat;
        cc = aa + bb;
        arguement = cc;
        dd = sqrt( arguement );
        arguement = elev * (elev + 2.0 * dd) + (a2 * aa + b2 * bb) / cc;
        r = sqrt( arguement );
        cd = (elev + dd) / r;
        sd = (a2 - b2) / dd * slat * clat / r;
        aa = slat;
        slat = slat * cd - clat * sd;
        clat = clat * cd + aa * sd;
    }
    ratio = earths_radius / r;
    arguement = 3.0;
    aa = sqrt( arguement );
    p[1] = 2.0 * slat;
    p[2] = 2.0 * clat;
    p[3] = 4.5 * slat * slat - 1.5;
    p[4] = 3.0 * aa * clat * slat;
    q[1] = -clat;
    q[2] = slat;
    q[3] = -3.0 * clat * slat;
    q[4] = aa * (slat * slat - clat * clat);
    for ( k = 1; k <= npq; ++k)
    {
        if (n < m)
        {
            m = 0;
            n = n + 1;
            arguement = ratio;
            power =  n + 2;
            rr = pow(arguement,power);
            fn = n;
        }
        fm = m;
        if (k >= 5)
        {
            if (m == n)
            {
                arguement = (1.0 - 0.5/fm);
                aa = sqrt( arguement );
                j = k - n - 1;
                p[k] = (1.0 + 1.0/fm) * aa * clat * p[j];
                q[k] = aa * (clat * q[j] + slat/fm * p[j]);
                sl[m] = sl[m-1] * cl[1] + cl[m-1] * sl[1];
                cl[m] = cl[m-1] * cl[1] - sl[m-1] * sl[1];
            }
            else
            {
                arguement = fn*fn - fm*fm;
                aa = sqrt( arguement );
                arguement = ((fn - 1.0)*(fn-1.0)) - (fm * fm);
                bb = sqrt( arguement )/aa;
                cc = (2.0 * fn - 1.0)/aa;
                ii = k - n;
                j = k - 2 * n + 1;
                p[k] = (fn + 1.0) * (cc * slat/fn * p[ii] - bb/(fn - 1.0) * p[j]);
                q[k] = cc * (slat * q[ii] - clat/fn * p[ii]) - bb * q[j];
            }
        }
        switch(gh)
        {
            case 3:  aa = rr * gha[l];
                break;
            case 4:  aa = rr * ghb[l];
                break;
            default: printf("\nError in subroutine shval3");
                break;
        }
        if (m == 0)
        {
            switch(gh)
            {
                case 3:  x = x + aa * q[k];
                    z = z - aa * p[k];
                    break;
                case 4:  xtemp = xtemp + aa * q[k];
                    ztemp = ztemp - aa * p[k];
                    break;
                default: printf("\nError in subroutine shval3");
                    break;
            }
            l = l + 1;
        }
        else
        {
            switch(gh)
            {
                case 3:  bb = rr * gha[l+1];
                    cc = aa * cl[m] + bb * sl[m];
                    x = x + cc * q[k];
                    z = z - cc * p[k];
                    if (clat > 0)
                    {
                        y = y + (aa * sl[m] - bb * cl[m]) *
                            fm * p[k]/((fn + 1.0) * clat);
                    }
                    else
                    {
                        y = y + (aa * sl[m] - bb * cl[m]) * q[k] * slat;
                    }
                    l = l + 2;
                    break;
                case 4:  bb = rr * ghb[l+1];
                    cc = aa * cl[m] + bb * sl[m];
                    xtemp = xtemp + cc * q[k];
                    ztemp = ztemp - cc * p[k];
                    if (clat > 0)
                    {
                        ytemp = ytemp + (aa * sl[m] - bb * cl[m]) *
                                fm * p[k]/((fn + 1.0) * clat);
                    }
                    else
                    {
                        ytemp = ytemp + (aa * sl[m] - bb * cl[m]) *
                                q[k] * slat;
                    }
                    l = l + 2;
                    break;
                default: printf("\nError in subroutine shval3");
                    break;
            }
        }
        m = m + 1;
    }
    if (iext != 0)
    {
        aa = ext2 * cl[1] + ext3 * sl[1];
        switch(gh)
        {
            case 3:   x = x - ext1 * clat + aa * slat;
                y = y + ext2 * sl[1] - ext3 * cl[1];
                z = z + ext1 * slat + aa * clat;
                break;
            case 4:   xtemp = xtemp - ext1 * clat + aa * slat;
                ytemp = ytemp + ext2 * sl[1] - ext3 * cl[1];
                ztemp = ztemp + ext1 * slat + aa * clat;
                break;
            default:  printf("\nError in subroutine shval3");
                break;
        }
    }
    switch(gh)
    {
        case 3:   aa = x;
            x = x * cd + z * sd;
            z = z * cd - aa * sd;
            break;
        case 4:   aa = xtemp;
            xtemp = xtemp * cd + ztemp * sd;
            ztemp = ztemp * cd - aa * sd;
            break;
        default:  printf("\nError in subroutine shval3");
            break;
    }
    return(ios);
}


/****************************************************************************/
/*                                                                          */
/*                           Subroutine dihf                                */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     Computes the geomagnetic d, i, h, and f from x, y, and z.            */
/*                                                                          */
/*     Input:                                                               */
/*           x  - northward component                                       */
/*           y  - eastward component                                        */
/*           z  - vertically-downward component                             */
/*                                                                          */
/*     Output:                                                              */
/*           d  - declination                                               */
/*           i  - inclination                                               */
/*           h  - horizontal intensity                                      */
/*           f  - total intensity                                           */
/*                                                                          */
/*     FORTRAN                                                              */
/*           A. Zunde                                                       */
/*           USGS, MS 964, box 25046 Federal Center, Denver, CO.  80225     */
/*                                                                          */
/*     C                                                                    */
/*           C. H. Shaffer                                                  */
/*           Lockheed Missiles and Space Company, Sunnyvale CA              */
/*           August 22, 1988                                                */
/*                                                                          */
/****************************************************************************/

int dihf (int gh)
{
    int ios;
    int j;
    float sn;
    float h2;
    float hpx;
    double arguement, arguement2;
    double rad, pi;

    ios = gh;
    sn = 0.0001;
    rad = 57.29577951;
    pi = 3.141592654;
    switch(gh)
    {
        case 3:   for (j = 1; j <= 1; ++j)
		{
            h2 = x*x + y*y;
            arguement = h2;
            h = sqrt(arguement);       /* calculate horizontal intensity */
            arguement = h2 + z*z;
            f = sqrt(arguement);      /* calculate total intensity */
            if (f < sn)
            {
                d = NaN;        /* If d and i cannot be determined, */
                i = NaN;        /*       set equal to NaN         */
            }
            else
            {
                arguement = z;
                arguement2 = h;
                i = atan2(arguement,arguement2);
                if (h < sn)
                {
                    d = NaN;
                }
                else
                {
                    hpx = h + x;
                    if (hpx < sn)
                    {
                        d = pi;
                    }
                    else
                    {
                        arguement = y;
                        arguement2 = hpx;
                        d = 2.0 * atan2(arguement,arguement2);
                    }
                }
            }
		}
            break;
        case 4:   for (j = 1; j <= 1; ++j)
            {
                h2 = xtemp*xtemp + ytemp*ytemp;
                arguement = h2;
                htemp = sqrt(arguement);
                arguement = h2 + ztemp*ztemp;
                ftemp = sqrt(arguement);
                if (ftemp < sn)
                {
                    dtemp = NaN;    /* If d and i cannot be determined, */
                    itemp = NaN;    /*       set equal to 999.0         */
                }
                else
                {
                    arguement = ztemp;
                    arguement2 = htemp;
                    itemp = atan2(arguement,arguement2);
                    if (htemp < sn)
                    {
                        dtemp = NaN;
                    }
                    else
                    {
                        hpx = htemp + xtemp;
                        if (hpx < sn)
                        {
                            dtemp = pi;
                        }
                        else
                        {
                            arguement = ytemp;
                            arguement2 = hpx;
                            dtemp = 2.0 * atan2(arguement,arguement2);
                        }
                    }
                }
            }
            break;
        default:  printf("\nError in subroutine dihf");
            break;
    }
    return(ios);
}

/////////////////////////////////////////////////////////////////////////////

// WMM2005.cof 2005 D m1 lat lon
double getDeclination(const Waypoint& location, const QString& declination_datafile)
{
  
    int   nmodel;             /* Number of models in file */
    int   max1[MAXMOD];
    int   max2[MAXMOD];
    int   max3[MAXMOD];
    int   nmax;
    int   igdgc=1;
    long  irec_pos[MAXMOD];
  
    char  mdfile[PATH];
    char  inbuff[MAXINBUFF];
    char  model[MAXMOD][9];
  
    float epoch[MAXMOD];
    float yrmin[MAXMOD];
    float yrmax[MAXMOD];
    float minyr = 0.0;
    float maxyr = 0.0;
    float altmin[MAXMOD];
    float altmax[MAXMOD];
    float minalt;
    float maxalt;
    float alt=1;
    float ddot;
    float fdot;
    float hdot;
    float idot;
    float xdot;
    float ydot;
    float zdot;

    //-----

    float sdate=2005.0;
    double latitude = location.lat();
    double longitude = location.lon();

    strcpy(mdfile, declination_datafile.toLatin1());
    stream=fopen(mdfile, "rt");
    MYASSERT(stream != 0);

    //-----

    /* Initializations. */
    
    inbuff[MAXREAD+1]='\0';  /* Just to protect mem. */
    inbuff[MAXINBUFF-1]='\0';  /* Just to protect mem. */
  
    /*  Obtain the desired model file and read the data  */
  
    int warn_H = 0;
    float warn_H_val = 99999.0;
    int warn_H_strong = 0;
    float warn_H_strong_val = 99999.0;
    int warn_P = 0;
        
    rewind(stream);
        
    int fileline = 0;                            /* First line will be 1 */
    int modelI = -1;                             /* First model will be 0 */
    while(fgets(inbuff,MAXREAD,stream))
    {
        fileline++;                           /* On new line */

		// The comparison should protect us from malformed lines.
		// Due to the introduction of CRLF line endings, there is a chance
		// that an error of one character can slip through.
        if ( (strlen(inbuff) != RECL) && (strlen(inbuff) != RECL-1) ) {	/* IF incorrect record size */
            printf("Corrupt record in file %s on line %d.\n", mdfile, fileline);
            fclose(stream);
            MYASSERT(false);
        }
      
        if(!strncmp(inbuff,"   ",3)){         /* If 1st 3 chars are spaces */
            modelI++;                           /* New model */
        
            if(modelI > MAXMOD){                /* If too many headers */
                printf("Too many models in file %s on line %d.", mdfile, fileline);
                fclose(stream);
                MYASSERT(false);
            }
        
            irec_pos[modelI]=ftell(stream);
            /* Get fields from buffer into individual vars.  */
            sscanf(inbuff, "%s%f%d%d%d%f%f%f%f", model[modelI], &epoch[modelI],
                   &max1[modelI], &max2[modelI], &max3[modelI], &yrmin[modelI],
                   &yrmax[modelI], &altmin[modelI], &altmax[modelI]);
        
            /* Compute date range for all models */
            if(modelI == 0){                    /*If first model */
                minyr=yrmin[0];
                maxyr=yrmax[0];
            } else {
                if(yrmin[modelI]<minyr){
                    minyr=yrmin[modelI];
                }
                if(yrmax[modelI]>maxyr){
                    maxyr=yrmax[modelI];
                }
            }
        }
    }

    nmodel = modelI + 1;
    fclose(stream);
    
    /* Pick model */
    for (modelI=0; modelI<nmodel; modelI++)
        if (sdate<yrmax[modelI]) break;
    if (modelI == nmodel) modelI--;           /* if beyond end of last model use last model */
      
    /* Get altitude min and max for selected model. */
    minalt=altmin[modelI];
    maxalt=altmax[modelI];
      
    // km <-> m
    minalt*=1000.0;
    maxalt*=1000.0;
    alt *= 0.001;
      
    /** This will compute everything needed for 1 point in time. **/

    if(max2[modelI] == 0) 
    {
        getshc(mdfile, 1, irec_pos[modelI], max1[modelI], 1);
        getshc(mdfile, 1, irec_pos[modelI+1], max1[modelI+1], 2);
        nmax = interpsh(sdate, yrmin[modelI], max1[modelI], yrmin[modelI+1], max1[modelI+1], 3);
        nmax = interpsh(sdate+1, yrmin[modelI] , max1[modelI], yrmin[modelI+1], max1[modelI+1],4);
    }
    else 
    {
        getshc(mdfile, 1, irec_pos[modelI], max1[modelI], 1);
        getshc(mdfile, 0, irec_pos[modelI], max2[modelI], 2);
        nmax = extrapsh(sdate, epoch[modelI], max1[modelI], max2[modelI], 3);
        nmax = extrapsh(sdate+1, epoch[modelI], max1[modelI], max2[modelI], 4);
    }

      
    /* Do the first calculations */
    shval3(igdgc, latitude, longitude, alt, nmax, 3, IEXT, EXT_COEFF1, EXT_COEFF2, EXT_COEFF3);
    dihf(3);
    shval3(igdgc, latitude, longitude, alt, nmax, 4, IEXT, EXT_COEFF1, EXT_COEFF2, EXT_COEFF3);
    dihf(4);
      
    ddot = ((dtemp - d)*57.29578);
    if (ddot > 180.0) ddot -= 360.0;
    if (ddot <= -180.0) ddot += 360.0;
    ddot *= 60.0;
      
    idot = ((itemp - i)*57.29578)*60;
    d = d*(57.29578);   i = i*(57.29578);
    hdot = htemp - h;   xdot = xtemp - x;
    ydot = ytemp - y;   zdot = ztemp - z;
    fdot = ftemp - f;

    /* deal with geographic and magnetic poles */
      
    if (h < 100.0) /* at magnetic poles */
    {
        d = NaN;
        ddot = NaN;
        /* while rest is ok */
    }

    if (h < 1000.0) 
    {
        warn_H = 0;
        warn_H_strong = 1;
        if (h<warn_H_strong_val) warn_H_strong_val = h;
    }
    else if (h < 5000.0 && !warn_H_strong) 
    {
        warn_H = 1;
        if (h<warn_H_val) warn_H_val = h;
    }
                  
    if (90.0-fabs(latitude) <= 0.001) /* at geographic poles */
    {
        x = NaN;
        y = NaN;
        d = NaN;
        xdot = NaN;
        ydot = NaN;
        ddot = NaN;
        warn_P = 1;
        warn_H = 0;
        warn_H_strong = 0;
        /* while rest is ok */
    }
      
    /** Above will compute everything for 1 point in time.  **/

    return -d;
}


