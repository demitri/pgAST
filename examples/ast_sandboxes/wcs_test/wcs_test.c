//
//  wcs_test.c
//
//  This simple script reads a FITS header, extracts the WCS from it via AST, and checks that it is a sky frame.
//
//  Created by Demitri Muna on 1/29/20.
//

// Use provided Makefile or compile with:
// gcc -I/usr/local/ast/include -L/usr/local/ast/lib -last  wcs_test.c 

#include <stdio.h>

/* include AST libraries */
#include "ast.h"

/* Header for WISE file 00720a001-w1-int-1b.fits */
char* header = "SIMPLE  =                    T / file does conform to FITS standard             BITPIX  =                  -32 / number of bits per data pixel                  NAXIS   =                    2 / number of data axes                            NAXIS1  =                 1016 / length of data axis 1                          NAXIS2  =                 1016 / length of data axis 2                          EXTEND  =                    T / FITS dataset may contain extensions            COMMENT   FITS (Flexible Image Transport System) format is defined in 'AstronomyCOMMENT   and Astrophysics', volume 376, page 359; bibcode: 2001A&A...376..359H TELESCOP= 'WISE    '                                                            BUNIT   = 'DN      '                                                            EQUINOX =                 2000                                                  RADECSYS= 'FK5     '                                                            FILETYPE= 'intensity image frame'                                               DATE_OBS= '2010-01-07T08:07:11.360' / Observation midpoint UTC                  WAVELEN =                3.368 / [micron] effective wavelength of band          BAND    =                    1 / Band number                                    SCAN    = '00720a  '           / Scan ID                                        FRNUM   =                    1 / Frame bin number in scan                       FRSETID = '00720a001'          / Frame set ID                                   SCANSTRT= '2010-007T08:07:04.509' / Scan start UTC                              SCANEND = '2010-007T08:52:40.369' / Scan end UTC                                SCANGRP = '0a      '           / Scan ID group                                  FRMTOFF =               -4.853 / [sec] Offset applied to packet VTC             VTC     =     316123614.974392 / Observation midpoint vehicle time code         UNIXT   =     1262851631.36002 / [sec] Observation midpoint UNIX time           EPHEMT  =     316123697.544426 / [sec] Observation midpoint TDT secs from J2000 UTC     = '2010-007T08:07:11.360' / Observation midpoint UTC date/time          DATIME  = '2010-01-07T08:07:11.360' / Observation midpoint calendar date/time   MJD_OBS =       55203.33832593 / [days] Obs. midpoint Modified Julian Day       JD_OBS  =     2455203.83832593 / [days] Observation midpoint Julian Day         HJD_OBS =     2455203.83844738 / [days] Obs. midpoint Heliocentric Julian Day   DTANNEAL=     2625.53931885958 / [sec] Time since last anneal                   UTANNEAL= '2010-007T07:23:25.821' / UTC of last anneal                          RAWFILE = 'none    '           / Raw input file name; usually none              TLMFILE = '/wise/fops/ingest/delivs/10007/10007T105729/WIS_HRP_PKT_FE1A_2010_0&'CONTINUE  '07_10_57_29_CAT.bin'                                                 TLMSEQNO=                  998 / Delivery image sequence number                 DELIVID = '10007T105729'       / Delivery ID                                    CMPRSRAT=                0.349 / Compression ratio                              INEVENTS= 'DESC   SCAN'        / Active orbit events                            EVDESC  =     189.360318243504 / [sec] Time since orbit event start             EVSCAN  =     6.85131824016571 / [sec] Time since orbit event start             L0FILE  = '/wise/fops/l0/0a/00720a/fr/001/00720a001-w1-int-0.fits.gz' / Level-0 CRPIX1  =                508.5 / Reference X pixel                              CRPIX2  =                508.5 / Reference Y pixel                              CRVAL1  =     244.968461965351 / [deg] Image center RA                          CRVAL2  =      62.169211613854 / [deg] Image center Dec                         CTYPE1  = 'RA---SIN-SIP'       / Sin projection with SIP coefficients           CTYPE2  = 'DEC--SIN-SIP'       / Sin projection with SIP coefficients           CD1_1   = -0.000422886848685716 / WCS rotation matrix element                   CD1_2   = 0.000635009613952676 / WCS rotation matrix element                    CD2_1   = 0.000639185340115771 / WCS rotation matrix element                    CD2_2   = 0.000420124176316281 / WCS rotation matrix element                    WCROTA2 =     303.488735266107 / [deg] CCW rotation of RA at CRPIX1,2           PA      =     56.5112647338927 / [deg] Rotation of +Y EofN at CRPIX1,2          WCDELT1 =        -0.0007664145 / [deg/pix] X-axis scale                         WCDELT2 =         0.0007614076 / [deg/pix] Y-axis scale                         PXSCAL1 =           -2.7590922 / [arcsec/pixel] X-axis scale                    PXSCAL2 =           2.74106736 / [arcsec/pixel] Y-axis scale                    CRDER1  = 3.52992764005061E-06 / [deg] Apriori RA error                         CRDER2  = 3.64518658549616E-06 / [deg] Apriori Dec error                        UNCRTPA = 0.000679332434597599 / [deg] Apriori PA error                         CSDRADEC= 1.57386280510341E-07 / Apriori RA/Dec cross-correlation               UNCRTS1 =              2.3E-07 / [deg/pix] Apriori X-axis scale error           UNCRTS2 =              2.3E-07 / [deg/pix] Apriori Y-axis scale error           RA0     =     245.032362010072 / Level-0 ADCS frame center RA                   DEC0    =     62.1342902853335 / Level-0 ADCS frame center Dec                  PA0     =     56.5731601085637 / Level-0 ADCS frame rot. of +Y EofN             PA0_SC  =     213.426839891436 / Level-0 ADCS raw rot. of S/C +Y EofN           ELON0   =     190.650636818608 / Level-0 ADCS frame center ecliptic lon         ELAT0   =     78.4191042139454 / Level-0 ADCS frame center ecliptic lat         GLON0   =     93.5650650365354 / Level-0 ADCS frame center galactic lon         GLAT0   =      41.164637217101 / Level-0 ADCS frame center galactic lat         L0XFORM = '-1,0,0,1'           / Raw->L0 frame transformation                   L0B2BDX =                -9999 / Raw->L0 band x offset                          L0B2BDY =                -9999 / Raw->L0 band y offset                          L0B2BDPA=                -9999 / Raw->L0 band rotation offset                   SCRATEX = -0.00354113471903081 / [arcmin/sec] Rot. rate about S/C CF X          SCRATEY =     3.70162542557345 / [arcmin/sec] Rot. rate about S/C CF Y          SCRATEZ = -0.00984817826600907 / [arcmin/sec] Rot. rate about S/C CF Z          EXPTIME =                  7.7 / [sec] Frame exposure time                      TSAMP   =                  1.1 / [sec] Sample time                              FRINT   =                   11 / [sec] Frame-to-frame interval                  DEBOFF  =                 1024 / DEB offset                                     DEBTRUNC=                    3 / Bits truncated by DEB                          DEBGAIN =                 3.75 / [e-/DEB ADU] DEB gain                          FEBGAIN =                 5.74 / [e-/SUR ADU] FEB gain                          SUN2SCX =   -0.283751892759924 / [AU] Sun-to-S/C vector X component, J2000      SUN2SCY =    0.863826900981703 / [AU] Sun-to-S/C vector Y component, J2000      SUN2SCZ =    0.374542869772644 / [AU] Sun-to-S/C vector Z component, J2000      SCVELX  =   -0.020851643403682 / [AU/day] SUN-to-S/C velocity vector X componentSCVELY  = -0.00546371175646724 / [AU/day] SUN-to-S/C velocity vector Y componentSCVELZ  = -0.00331885658728277 / [AU/day] SUN-to-S/C velocity vector Z componentERTH2SCX= -1.21971979464944E-05 / [AU] Earth-to-S/C vector X component          ERTH2SCY= -9.01614227023024E-06 / [AU] Earth-to-S/C vector Y component          ERTH2SCZ=  4.3544460366698E-05 / [AU] Earth-to-S/C vector Z component           SUNSEP  =       91.22797818411 / [deg] L0 FOV center to Sun angular sep         SUNPA   =     140.762974244083 / [deg] Direction of Sun, east of north          MOONSEP =     84.7409612228119 / [deg] L0 FOV center to Moon angular sep        MOONPA  =     231.705159245806 / [deg] Direction of Moon, east of north         MARSSEP =     79.1487723906949 / [deg] L0 FOV center to Mars angular sep        MARSPA  =     291.210618146322 / [deg] Direction of Mars, east of north         JUPSEP  =     99.3467016163806 / [deg] L0 FOV center to Jupiter angular sep     JUPPA   =     100.599091720373 / [deg] Direction of Jupiter, east of north      SATSEP  =     76.1695666259109 / [deg] L0 FOV center to Saturn angular sep      SATPA   =     243.118288723622 / [deg] Direction of Saturn, east of north       GEOLON  =           347.841368 / [deg] S/C geographic longitude                 GEOLAT  =            70.717514 / [deg] S/C geographic latitude                  GEOALT  =           538.983889 / [km] S/C altitude                              GCD2SAA =            77.288167 / [deg] Great circle angle to SAA boundary       GEO_DT  =    0.639493405818939 / [sec] Time since ground track sample           SURCOEF1=                    0 / Nominal static survey SUR coefficient #1       SURCOEF2=                   -7 / Nominal static survey SUR coefficient #2       SURCOEF3=                   -5 / Nominal static survey SUR coefficient #3       SURCOEF4=                   -3 / Nominal static survey SUR coefficient #4       SURCOEF5=                   -1 / Nominal static survey SUR coefficient #5       SURCOEF6=                    1 / Nominal static survey SUR coefficient #6       SURCOEF7=                    3 / Nominal static survey SUR coefficient #7       SURCOEF8=                    5 / Nominal static survey SUR coefficient #8       SURCOEF9=                    7 / Nominal static survey SUR coefficient #9       ATTEMGEN= 'true    '           / electromagnet control enable                   ATT_ADST= 'point   '           / current adcs state                             ATT_ERRX=         -2.46609E-05 / attitude error x                               ATT_ERRY=         -0.000168002 / attitude error y                               ATT_ERRZ=          2.21851E-05 / attitude error z                               ATTUPDMT= 'three_ax_rt_att'    / attitude update method                         ATT_FAQ1=             0.115113 / inertial to control attitude x                 ATT_FAQ2=             0.210025 / inertial to control attitude y                 ATT_FAQ3=            -0.968216 / inertial to control attitude z                 ATT_FAQ4=            0.0720862 / inertial to control attitude w                 ATT_FRTX=         -1.18474E-06 / [rad/sec] inertial to control rate x           ATT_FRTY=           0.00107435 / [rad/sec] inertial to control rate y           ATT_FRTZ=         -6.20435E-07 / [rad/sec] inertial to control rate z           ATT_METH= 'three_ax_rt_att'    / attitude determination method                  ATTCRCFG=                    5 / current config set                             ATTACCFG= 'w1234_sci'          / actuator config index                          GYRODTST= 'good    '           / Overall status of the gyro data                ICV_VLD = 'valid   '           / icv valid                                      IMU_PWR = 'on      '           / imu power on                                   MAG_PWR = 'on      '           / mag power on                                   MAGFDVST= 'good    '           / mag field vector status                        MNVRDONE= 'true    '           / maneuver done                                  MSUN_VST= 'good    '           / measured sun vector status                     MSUN_VX =            -0.102145 / measured sun x                                 MSUN_VY =             0.994498 / measured sun y                                 MSUN_VZ =           -0.0232405 / measured sun z                                 ORBSTVLD= 'valid   '           / orbital state valid                            RATERRX =          4.67474E-06 / [rad/sec] rate error x                         RATERRY =          2.95741E-05 / [rad/sec] rate error y                         RATERRZ =          6.20435E-07 / [rad/sec] rate error z                         WHL1_SPD=             -144.281 / [rad/sec] wheel 1 speed                        WHL2_SPD=             -112.865 / [rad/sec] wheel 2 speed                        WHL3_SPD=             -114.028 / [rad/sec] wheel 3 speed                        WHL4_SPD=             -151.262 / [rad/sec] wheel 4 speed                        SEL_SENS= 'st1_st2_rs'         / selected sensor                                INSHADOW= 'false   '           / in shadow                                      NMSUNUSD=                    6 / num sun sensors used                           ST1_ASCT=                -9999 / star tracker 1 att star count                  ST2_ASCT=                -9999 / star tracker 2 att star count                  ST1_BGHI= 'ok      '           / star tracker 1 background high                 ST2_BGHI= 'ok      '           / star tracker 2 background high                 ST1STRID=                    0 / star tracker 1 star id mode                    ST2STRID=                    0 / star tracker 2 star id mode                    ST_ATTDQ= 'good    '           / star tracker attitude data quality             ST_RTEDQ= 'good    '           / star tracker rate data quality                 ST_USED = 'both    '           / star tracker used                              STVLDLCT=                -9999 / st valid last cycle                            TQROD1ST= 'off     '           / torqrod1 current powered status                TQROD2ST= 'off     '           / torqrod2 current powered status                TQROD3ST= 'off     '           / torqrod3 current powered status                MDSCMODE= 'operate '           / Current spacecraft state                       APSHIMYT=              93.4589 / [K] Aperture Shade Inner Cone (-Y) Temperature APSHIPYT=              92.6536 / [K] Aperture Shade Inner Cone (+Y) Temperature APSHOMYT=                174.2 / [K] Aperture Shade Outer Cone (-Y) Temperature APSHOPYT=                179.9 / [K] Aperture Shade Outer Cone (+Y) Temperature B13NUMSA=                    9 / Bands 1&3 Number of Samples                    B1EOFERR= 'no      '           / Band 1 End of Frame Error                      B1SCIFSE=                  136 / Band 1 Science Frames Sent                     B1MLPERR= 'no      '           / Band 1 Missing Last Pixel Error                B1SATDET= 'enabled '           / Band 1 Saturation Detection Enable             B1COEFF1=                    0 / Band 1 Coefficient 1                           B1COEFF2=                   -7 / Band 1 Coefficient 2                           B1COEFF3=                   -5 / Band 1 Coefficient 3                           B1COEFF4=                   -3 / Band 1 Coefficient 4                           B1COEFF5=                   -1 / Band 1 Coefficient 5                           B1COEFF6=                    1 / Band 1 Coefficient 6                           B1COEFF7=                    3 / Band 1 Coefficient 7                           B1COEFF8=                    5 / Band 1 Coefficient 8                           B1COEFF9=                    7 / Band 1 Coefficient 9                           B1SUROFF=                 1024 / Band 1 SUR Offset                              B1SSYERR= 'no      '           / Band 1 Sample Sync Error                       B1PSATTH=                16000 / Band 1 Pixel Saturation Threshold              B1FPTERR= 'no      '           / Band 1 Focal Plane Timeout Error               B1TRBITS=                    3 / Band 1 Truncated Bits                          B24NUMSA=                    9 / Bands 2&4 Number of Samples                    B2EOFERR= 'no      '           / Band 2 End of Frame Error                      B2SCIFSE=                  136 / Band 2 Science Frames Sent                     B2MLPERR= 'no      '           / Band 2 Missing Last Pixel Error                B2SATDET= 'enabled '           / Band 2 Saturation Detection Enable             B2COEFF1=                    0 / Band 2 Coefficient 1                           B2COEFF2=                   -7 / Band 2 Coefficient 2                           B2COEFF3=                   -5 / Band 2 Coefficient 3                           B2COEFF4=                   -3 / Band 2 Coefficient 4                           B2COEFF5=                   -1 / Band 2 Coefficient 5                           B2COEFF6=                    1 / Band 2 Coefficient 6                           B2COEFF7=                    3 / Band 2 Coefficient 7                           B2COEFF8=                    5 / Band 2 Coefficient 8                           B2COEFF9=                    7 / Band 2 Coefficient 9                           B2SUROFF=                 1024 / Band 2 SUR Offset                              B2SSYERR= 'no      '           / Band 2 Sample Sync Error                       B2PSATTH=                14500 / Band 2 Pixel Saturation Threshold              B2FPTERR= 'no      '           / Band 2 Focal Plane Timeout Error               B2TRBITS=                    3 / Band 2 Truncated Bits                          B3EOFERR= 'no      '           / Band 3 End of Frame Error                      B3SCIFSE=                  136 / Band 3 Science Frames Sent                     B3MLPERR= 'no      '           / Band 3 Missing Last Pixel Error                B3SATDET= 'enabled '           / Band 3 Saturation Detection Enable             B3COEFF1=                   -4 / Band 3 Coefficient 1                           B3COEFF2=                   -3 / Band 3 Coefficient 2                           B3COEFF3=                   -2 / Band 3 Coefficient 3                           B3COEFF4=                   -1 / Band 3 Coefficient 4                           B3COEFF5=                    0 / Band 3 Coefficient 5                           B3COEFF6=                    1 / Band 3 Coefficient 6                           B3COEFF7=                    2 / Band 3 Coefficient 7                           B3COEFF8=                    3 / Band 3 Coefficient 8                           B3COEFF9=                    4 / Band 3 Coefficient 9                           B3SUROFF=                 1024 / Band 3 SUR Offset                              B3SSYERR= 'no      '           / Band 3 Sample Sync Error                       B3PSATTH=                16000 / Band 3 Pixel Saturation Threshold              B3FPTERR= 'no      '           / Band 3 Focal Plane Timeout Error               B3TRBITS=                    2 / Band 3 Truncated Bits                          B4EOFERR= 'no      '           / Band 4 End of Frame Error                      B4SCIFSE=                  136 / Band 4 Science Frames Sent                     B4MLPERR= 'no      '           / Band 4 Missing Last Pixel Error                B4SATDET= 'enabled '           / Band 4 Saturation Detection Enable             B4COEFF1=                   -4 / Band 4 Coefficient 1                           B4COEFF2=                   -3 / Band 4 Coefficient 2                           B4COEFF3=                   -2 / Band 4 Coefficient 3                           B4COEFF4=                   -1 / Band 4 Coefficient 4                           B4COEFF5=                    0 / Band 4 Coefficient 5                           B4COEFF6=                    1 / Band 4 Coefficient 6                           B4COEFF7=                    2 / Band 4 Coefficient 7                           B4COEFF8=                    3 / Band 4 Coefficient 8                           B4COEFF9=                    4 / Band 4 Coefficient 9                           B4SUROFF=                 1024 / Band 4 SUR Offset                              B4SSYERR= 'no      '           / Band 4 Sample Sync     Error                   B4PSATTH=                16000 / Band 4 Pixel Saturation Threshold              B4FPTERR= 'no      '           / Band 4 Focal Plane Timeout Error               B4TRBITS=                    2 / Band 4 Truncated Bits                          B3FPATEM=              7.27171 / [K] Back of Band 3 FPA Temperature             B4FPATEM=              7.30647 / [K] Back of Band 4 FPA Temperature             CHANAPSA=              -4.7791 / [arcmin] Channel A Position [A]                CHANAPSB=              2.40124 / [arcmin] Channel A Position [B]                CHANBPSA=                    0 / [arcmin] Channel B Position [A]                CHANBPSB=           -0.0012875 / [arcmin] Channel B Position [B]                CURAMSAM=                 4097 / Current Ramp Samples                           LFOLERRA= 'no      '           / Large Following Error [A]                      LFOLERRB= 'no      '           / Large Following Error [B]                      B1FEBFT1=              32.2319 / [K] FEB Band 1 FPA Temperature 1               B1FEBFT2=              32.0457 / [K] FEB Band 1 FPA Temperature 2               B1FEBOVO=              1.80394 / [V] FEB Band 1 Offset Voltage                  B2FEBFT1=              31.9615 / [K] FEB Band 2 FPA Temperature 1               B2FEBFT2=               31.627 / [K] FEB Band 2 FPA Temperature 2               B2FEBOVO=              1.84315 / [V] FEB Band 2 Offset Voltage                  B3FEBFT1=               7.0754 / [K] FEB Band 3 FPA Temperature 1               B3FEBFT2=              7.15144 / [K] FEB Band 3 FPA Temperature 2               B3FEBOVO=              1.74511 / [V] FEB Band 3 Offset Voltage                  B4FEBFT1=              7.07779 / [K] FEB Band 4 FPA Temperature 1               B4FEBFT2=              7.14261 / [K] FEB Band 4 FPA Temperature 2               B4FEBOVO=              1.68629 / [V] FEB Band 4 Offset Voltage                  ANNHEA_A= 'disabled'           / Annealing Heater A Enabled                     ANNHEAPA= 'off     '           / Annealing Heater A Power Indicator             ANNHEA_B= 'disabled'           / Annealing Heater B Enabled                     ANNHEAPB= 'off     '           / Annealing Heater B Power Indicator             ANNHEA_T=                    0 / [sec] Annealing Heater Timer                   IVCSADOT=              62.4925 / [K] IVCS Aft Dome Temperature                  IVCSLHET=              59.8316 / [K] IVCS LHe Cooling Loop Temperature          OVCSAFDT=               108.81 / [K] OVCS Aft Dome Temperature                  OVCSRART=              108.422 / [K] OVCS Radiator Ring Temperature             PRITBOTT=              7.33548 / [K] Primary Tank Bottom Temperature            PRITTOPT=              7.73094 / [K] Primary Tank Top Temperature               SCPOSMON= 'b       '           / Scanner selected for position monitoring       SCANVELA=                 3.65 / [arcmin/sec] Scan Velocity [A]                 SCANVELB=                 3.81 / [arcmin/sec] Scan Velocity [B]                 SECTBOTT=                11.69 / [K] Secondary Tank Bottom Temperature          SECTTOPT=              11.2545 / [K] Secondary Tank Top Temperature             PSEQEN_A= 'disabled'           / Position Sequencer Enabled [A]                 PSEQEN_B= 'enabled '           / Position Sequencer Enabled [B]                 SERVEN_A= 'disabled'           / Servo Enabled [A]                              SERVEN_B= 'enabled '           / Servo Enabled [B]                              BAFFRONT=              10.8182 / [K] Baffle Front Temperature                   BSPLMOUT=              11.0301 / [K] Beamsplitter Mount Temperature             TELINTFT=              10.8182 / [K] Telescope Interface Flange Temperature     SCAMIRRT=              10.8632 / [K] Scan Mirror Temperature                    SECMIRRT=              10.8032 / [K] Secondary Mirror Temperature               T12FPOIT=              107.453 / [K] Tube 1/2 Fold Point Temperature            T23FPOIT=              62.4925 / [K] Tube 2/3 Fold Point Temperature            SHNDSPTE=                181.4 / [K] Shell near Door Sep. Plane Temperature     SHNPVETE=                184.1 / [K] Shell near Primary Vent Temperature        SHARDOTE=                186.7 / [K] Shell at Rear Dome Temperature             TLCURMOD= 'nominal '           / Current telemetry mode                         PLPAMPRT=              21.8691 / [K] payload_preamp_redundant temperature       PLPAMP_T=              21.8302 / [K] payload_preamp temperature                 HK_MINDT=    -0.36000007390976 / [sec] Minimum H/K sample dt                    HK_MAXDT=     12.6399999260902 / [sec] Maximum H/K sample dt                    INGSTREV= '$Id: ingestpipe 6821 2010-01-01 06:31:25Z tim $' / Svn revision      L0CREATE= '2010-01-07T14:02:06.564Z' / Level-0 file creation date/time          CAL01   = '/wise/fops/cal/ifr/wise-meta-ingest.tbl'                             CK01    = '/wise/fops/ref/mos/naif/10007/WISE_CK_2010_01_07_05_04_28.bc'        CK02    = '/wise/fops/ref/mos/naif/10007/WISE_CK_2010_01_07_12_02_32.bc'        CK03    = '/wise/fops/ref/mos/naif/10007/WISE_CK_2010_01_07_13_09_53.bc'        CK-PRE01= '/wise/fops/ref/mos/naif/10004/WIS_PGEN_1002_1BwSSFwINSTR_2010_003_1&'CONTINUE  '8_29_49.bc'                                                          CK-PRE02= '/wise/fops/ref/mos/naif/10006/WIS_PGEN_1002_1B_OVRLYB_MERGE_2010_00&'CONTINUE  '5_16_20_48.bc'                                                       HK01    = '/wise/fops/ref/mos/hk/hk.db'                                         SCLK01  = 'WIS_SCLKSCET.00005.tsc'                                              SPK01   = '/wise/fops/ref/mos/naif/09350/WIS_NAV_SPK_WISEONLY_15Dec_LP24_2009_&'CONTINUE  '349_18_37_04.bsp'                                                    SPK02   = '/wise/fops/ref/mos/naif/09350/WIS_NAV_SPK_WISEONLY_16Dec_V01_2009_3&'CONTINUE  '50_19_46_38.bsp'                                                     SPK03   = '/wise/fops/ref/mos/naif/09351/WIS_NAV_SPK_WISEONLY_17Dec_V01_2009_3&'CONTINUE  '51_17_57_39.bsp'                                                     SPK04   = '/wise/fops/ref/mos/naif/09352/WIS_NAV_SPK_WISEONLY_18Dec_V01_2009_3&'CONTINUE  '52_19_52_30.bsp'                                                     SPK05   = '/wise/fops/ref/mos/naif/09353/WIS_NAV_SPK_WISEONLY_19Dec_V01_2009_3&'CONTINUE  '53_20_37_44.bsp'                                                     SPK06   = '/wise/fops/ref/mos/naif/09354/WIS_NAV_SPK_WISEONLY_20Dec_V01_2009_3&'CONTINUE  '54_20_25_13.bsp'                                                     SPK07   = '/wise/fops/ref/mos/naif/09355/WIS_NAV_SPK_WISEONLY_21Dec_V01_2009_3&'CONTINUE  '55_21_09_32.bsp'                                                     SPK08   = '/wise/fops/ref/mos/naif/09356/WIS_NAV_SPK_WISEONLY_22Dec_V01_2009_3&'CONTINUE  '56_18_47_41.bsp'                                                     SPK09   = '/wise/fops/ref/mos/naif/09357/WIS_NAV_SPK_WISEONLY_23Dec_V01_2009_3&'CONTINUE  '57_16_53_19.bsp'                                                     SPK10   = '/wise/fops/ref/mos/naif/09358/WIS_NAV_SPK_WISEONLY_24Dec_V01_2009_3&'CONTINUE  '58_16_53_07.bsp'                                                     TRK01   = '/wise/fops/ref/mos/track/WIS_NAV_WGT_31Dec_RadarOD_2009_365_20_26_1&'CONTINUE  '3.txt   '                                                            TRK02   = '/wise/fops/ref/mos/track/WIS_NAV_WGT_04Jan_RadarOD_2010_004_23_16_2&'CONTINUE  '0.txt   '                                                            TRK03   = '/wise/fops/ref/mos/track/WIS_NAV_WGT_06Jan_RadarOD_2010_006_20_26_3&'CONTINUE  '0.txt   '                                                            LONGSTRN= 'OGIP 1.0'                                                            WRELEASE= 'release-v3.2.2'     / WSDS s/w release tag                           HISTORY instruframecal, v.$Id on 2011-04-16 at 23:20:33                         ICALDIR = '/wise/fops/cal/ifr'                                                  MODEINT =     9.92237377166748 / [DN] pixel mode within lowest sigma partition  A_0_0   =     0.71839008934784 / axis 1 distortion coefficient of v0            A_0_1   =        -0.0001339348 / axis 1 distortion coefficient of v1            A_0_2   =        -7.865474E-06 / axis 1 distortion coefficient of v2            A_0_3   =        -2.197543E-10 / axis 1 distortion coefficient of v3            A_0_4   =        -1.650831E-13 / axis 1 distortion coefficient of v4            A_1_0   =     -0.0008364487736 / axis 1 distortion coefficient of u1            A_1_1   =         1.319662E-06 / axis 1 distortion coefficient of u.v           A_1_2   =          1.40211E-09 / axis 1 distortion coefficient of u.v2          A_1_3   =         -4.91699E-13 / axis 1 distortion coefficient of u.v3          A_2_0   =        -1.490523E-06 / axis 1 distortion coefficient of u2            A_2_1   =        -1.774975E-10 / axis 1 distortion coefficient of u2.v          A_2_2   =         1.378454E-12 / axis 1 distortion coefficient of u2.v2         A_3_0   =         3.075193E-09 / axis 1 distortion coefficient of u3            A_3_1   =        -4.456145E-13 / axis 1 distortion coefficient of u2.v          A_4_0   =         1.268865E-12 / axis 1 distortion coefficient of u4            A_DMAX  =                2.075 / [pixel] axis 1 maximum correction              A_ORDER =                    4 / polynomial order, axis 1, detector to sky      AP_0_0  =    -0.71892488934784 / axis 1 distortion coefficient of V0            AP_0_1  =         0.0001357412 / axis 1 distortion coefficient of V1            AP_0_2  =         7.867368E-06 / axis 1 distortion coefficient of V2            AP_0_3  =         1.974483E-10 / axis 1 distortion coefficient of V3            AP_0_4  =         2.082039E-13 / axis 1 distortion coefficient of V4            AP_1_0  =      0.0008361366736 / axis 1 distortion coefficient of U1            AP_1_1  =        -1.316141E-06 / axis 1 distortion coefficient of U.V           AP_1_2  =        -1.337672E-09 / axis 1 distortion coefficient of U.V2          AP_1_3  =         5.028983E-13 / axis 1 distortion coefficient of U.V3          AP_2_0  =         1.498243E-06 / axis 1 distortion coefficient of U2            AP_2_1  =         1.688004E-10 / axis 1 distortion coefficient of U2.V          AP_2_2  =        -1.462359E-12 / axis 1 distortion coefficient of U2.V2         AP_3_0  =        -3.081662E-09 / axis 1 distortion coefficient of U3            AP_3_1  =         4.586934E-13 / axis 1 distortion coefficient of U2.V          AP_4_0  =        -1.281481E-12 / axis 1 distortion coefficient of U4            AP_ORDER=                    4 / polynomial order, axis 1, sky to detector      B_0_0   =     -0.0800634760345 / axis 2 distortion coefficient of v0            B_0_1   =      0.0003275043621 / axis 2 distortion coefficient of v1            B_0_2   =         1.053036E-06 / axis 2 distortion coefficient of v2            B_0_3   =        -3.251883E-09 / axis 2 distortion coefficient of v3            B_0_4   =        -2.838759E-13 / axis 2 distortion coefficient of v4            B_1_0   = -0.000303693199999991 / axis 2 distortion coefficient of u1           B_1_1   =        -3.427523E-06 / axis 2 distortion coefficient of u.v           B_1_2   =        -2.803326E-10 / axis 2 distortion coefficient of u.v2          B_1_3   =          8.58959E-13 / axis 2 distortion coefficient of u.v3          B_2_0   =        -3.894091E-08 / axis 2 distortion coefficient of u2            B_2_1   =        -4.203492E-11 / axis 2 distortion coefficient of u2.v          B_2_2   =        -2.228216E-13 / axis 2 distortion coefficient of u2.v2         B_3_0   =         1.011286E-10 / axis 2 distortion coefficient of u3            B_3_1   =         8.828245E-13 / axis 2 distortion coefficient of u2.v          B_4_0   =         8.859376E-13 / axis 2 distortion coefficient of u4            B_DMAX  =                1.071 / [pixel] axis 2 maximum correction              B_ORDER =                    4 / polynomial order, axis 2, detector to sky      BP_0_0  =      0.0798377160345 / axis 2 distortion coefficient of V0            BP_0_1  =     -0.0003290875621 / axis 2 distortion coefficient of V1            BP_0_2  =        -1.048795E-06 / axis 2 distortion coefficient of V2            BP_0_3  =         3.275245E-09 / axis 2 distortion coefficient of V3            BP_0_4  =         2.705566E-13 / axis 2 distortion coefficient of V4            BP_1_0  = 0.000304400299999991 / axis 2 distortion coefficient of U1            BP_1_1  =         3.427776E-06 / axis 2 distortion coefficient of U.V           BP_1_2  =         2.689765E-10 / axis 2 distortion coefficient of U.V2          BP_1_3  =        -8.272042E-13 / axis 2 distortion coefficient of U.V3          BP_2_0  =         4.010036E-08 / axis 2 distortion coefficient of U2            BP_2_1  =         5.357017E-11 / axis 2 distortion coefficient of U2.V          BP_2_2  =         2.224483E-13 / axis 2 distortion coefficient of U2.V2         BP_3_0  =         -1.03665E-10 / axis 2 distortion coefficient of U3            BP_3_1  =        -8.917543E-13 / axis 2 distortion coefficient of U2.V          BP_4_0  =         -8.85461E-13 / axis 2 distortion coefficient of U4            BP_ORDER=                    4 / polynomial order, axis 2, sky to detector      SKEW    =                   0. / [deg] Y-axis shear                             DA_0_0  =      -8.11065216E-06 / Diff. abberation distortion delta              DAP_0_0 =       8.11065216E-06 / Diff. abberation distortion delta              DA_0_1  =       1.96108415E-20 / Diff. abberation distortion delta              DAP_0_1 =      -1.96108415E-20 / Diff. abberation distortion delta              DA_1_0  =       2.01850264E-05 / Diff. abberation distortion delta              DAP_1_0 =      -2.01850264E-05 / Diff. abberation distortion delta              DB_0_0  =      -7.54660345E-05 / Diff. abberation distortion delta              DBP_0_0 =       7.54660345E-05 / Diff. abberation distortion delta              DB_0_1  =       2.01848621E-05 / Diff. abberation distortion delta              DBP_0_1 =      -2.01848621E-05 / Diff. abberation distortion delta              DB_1_0  =       9.03887014E-18 / Diff. abberation distortion delta              DBP_1_0 =      -9.03887014E-18 / Diff. abberation distortion delta              DIFFABBR=                    1 / Distortion correction applied                  DISTSRC = 'B10221  '           / Distortion source reference code               B2BSRC  = 'B10221  '           / Band-to-band delta source reference code       L1ACOSRC= 'v3.5    '           / L1A position source                            DRA0_1B =    -107.396971591416 / [arc-secs] L0 to L1B delta RA                  DDEC0_1B=     125.769739283478 / [arc-secs] L0 to L1B delta Dec                 DPA0_1B =    -222.823348815561 / [arc-secs] L0 to L1B delta PA                  COMMENT OLD CRVAL1 = 244.968533585519                                           COMMENT OLD CRVAL2 = 62.1693226134402                                           COMMENT OLD WCROTA2 = 303.487788809697                                          COMMENT OLD PA = 56.512211190303                                                COMMENT OLD CRDER1 = 0.017                                                      COMMENT OLD CRDER2 = 0.017                                                      COMMENT OLD UNCRTPA = 0.017                                                     COMMENT OLD CSDRADEC = 0                                                        COMMENT OLD UNCRTS1 = 3.0E-06                                                   COMMENT OLD UNCRTS2 = 3.0E-06                                                   COMMENT OLD WCDELT1 = -0.0007664145                                             COMMENT OLD PXSCAL1 = -2.7590922                                                COMMENT OLD WCDELT2 = 0.0007614076                                              COMMENT OLD PXSCAL2 = 2.74106736                                                COMMENT OLD CD1_1 = -0.000422876290065633                                       COMMENT OLD CD1_2 = 0.000635016553805125                                        COMMENT OLD CD2_1 = 0.00063919232560363                                         COMMENT OLD CD2_2 = 0.000420113686674479                                        COMMENT OLD WCROTA2 = 303.487788809697                                          COMMENT OLD WCDELT1 = -0.0007664145                                             COMMENT OLD WCDELT2 = 0.0007614076                                              COMMENT OLD SKEW = 0.                                                           COMMENT OLD PA = 56.512211190303                                                COMMENT OLD PXSCAL1 = -2.7590922                                                COMMENT OLD PXSCAL2 = 2.74106736                                                MAGZP   =               20.752 / Relative single exposure photometric zero pointMAGZPUNC=                0.006 / 1-sigma uncertainty in MAGZP (-999=undefined)  ";

int main() {
	
	AstFitsChan *fitschan;
	AstFrameSet *wcsinfo;
	AstFrame *frame;
	int issky;
	
	astBegin;
	
	fitschan = astFitsChan( NULL, NULL, ""); /* create an empty channel */
	astPutCards( fitschan, header ); /* add all cards at once */

	/* read the WCS from the header */
	wcsinfo = astRead( fitschan );

	frame = astGetFrame( wcsinfo, AST__CURRENT );
	issky = astIsASkyFrame( frame );
	astAnnul( frame );
	
	printf("issky = %d\n", issky);
	
	astEnd;
	
}
