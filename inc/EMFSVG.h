/**
  @file uemf_print.h
  
  @brief Prototypes for functions for printing records from EMF files.
*/

/*
File:      uemf_print.h
Version:   0.0.5
Date:      14-FEB-2013
Author:    David Mathog, Biology Division, Caltech
email:     mathog@caltech.edu
Copyright: 2013 David Mathog and California Institute of Technology (Caltech)
*/

extern "C" {

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h> /* for offsetof() macro */
#include <string.h>
#include "uemf.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FLAG_SUPPORTED printf("   Status: %sSUPPORTED%s\n", KGRN, KNRM);
#define FLAG_IGNORED   printf("   Status: %sIGNORED%s\n", KRED, KNRM);
#define FLAG_PARTIAL   printf("   Status: %sPARTIAL SUPPORT%s\n", KYEL, KNRM);
#define FLAG_USELESS   printf("   Status  %sUSELESS%s\n", KCYN, KNRM);
#define FLAG_RESET printf("%s", KNRM);

typedef struct {
    char *nameSpace;
    char *lastFormId;
    bool verbose;
} drawingStates;

#define BUFFERSIZE 1024
//! \cond
/* prototypes for objects used in EMR records */
void hexbytes_print(uint8_t *buf,unsigned int num);
void colorref_print(U_COLORREF color);
void rgbquad_print(U_RGBQUAD color);
void rectl_print(U_RECTL rect);
void sizel_print(U_SIZEL sz);
void pointl_print(U_POINTL pt);
void point16_print(U_POINT16 pt, FILE * out);
void lcs_gamma_print(U_LCS_GAMMA lg);
void lcs_gammargb_print(U_LCS_GAMMARGB lgr);
void trivertex_print(U_TRIVERTEX tv);
void gradient3_print(U_GRADIENT3 g3);
void gradient4_print(U_GRADIENT4 g4);
void logbrush_print(U_LOGBRUSH lb);
void xform_print(U_XFORM xform);
void ciexyz_print(U_CIEXYZ ciexyz);
void ciexyztriple_print(U_CIEXYZTRIPLE cie3);
void logcolorspacea_print(U_LOGCOLORSPACEA lcsa);
void logcolorspacew_print(U_LOGCOLORSPACEW lcsa);
void panose_print(U_PANOSE panose);
void logfont_print(U_LOGFONT lf);
void logfont_panose_print(U_LOGFONT_PANOSE lfp); 
int  bitmapinfoheader_print(const char *Bmih);
void bitmapinfo_print(const char *Bmi);
void blend_print(U_BLEND blend);
void extlogpen_print(const PU_EXTLOGPEN elp);
void logpen_print(U_LOGPEN lp);
void logpltntry_print(U_LOGPLTNTRY lpny);
void logpalette_print(const PU_LOGPALETTE lp);
void rgndataheader_print(U_RGNDATAHEADER rdh);
void rgndata_print(const PU_RGNDATA rd);
void coloradjustment_print(U_COLORADJUSTMENT ca);
void pixelformatdescriptor_print(U_PIXELFORMATDESCRIPTOR pfd);
void emrtext_print(const char *emt, const char *record, int type);

/* prototypes for EMR records */
void U_EMRNOTIMPLEMENTED_print(const char *name, const char *contents, FILE *out);
void U_EMRHEADER_print(const char *contents, FILE *out);
void U_EMRPOLYBEZIER_print(const char *contents, FILE *out);
void U_EMRPOLYGON_print(const char *contents, FILE *out);
void U_EMRPOLYLINE_print(const char *contents, FILE *out);
void U_EMRPOLYBEZIERTO_print(const char *contents, FILE *out);
void U_EMRPOLYLINETO_print(const char *contents, FILE *out);
void U_EMRPOLYPOLYLINE_print(const char *contents, FILE *out);
void U_EMRPOLYPOLYGON_print(const char *contents, FILE *out);
void U_EMRSETWINDOWEXTEX_print(const char *contents, FILE *out);
void U_EMRSETWINDOWORGEX_print(const char *contents, FILE *out);
void U_EMRSETVIEWPORTEXTEX_print(const char *contents, FILE *out);
void U_EMRSETVIEWPORTORGEX_print(const char *contents, FILE *out);
void U_EMRSETBRUSHORGEX_print(const char *contents, FILE *out);
void U_EMREOF_print(const char *contents, FILE *out);
void U_EMRSETPIXELV_print(const char *contents, FILE *out);
void U_EMRSETMAPPERFLAGS_print(const char *contents, FILE *out);
void U_EMRSETMAPMODE_print(const char *contents, FILE *out);
void U_EMRSETBKMODE_print(const char *contents, FILE *out);
void U_EMRSETPOLYFILLMODE_print(const char *contents, FILE *out);
void U_EMRSETROP2_print(const char *contents, FILE *out);
void U_EMRSETSTRETCHBLTMODE_print(const char *contents, FILE *out);
void U_EMRSETTEXTALIGN_print(const char *contents, FILE *out);
void U_EMRSETCOLORADJUSTMENT_print(const char *contents, FILE *out);
void U_EMRSETTEXTCOLOR_print(const char *contents, FILE *out);
void U_EMRSETBKCOLOR_print(const char *contents, FILE *out);
void U_EMROFFSETCLIPRGN_print(const char *contents, FILE *out);
void U_EMRMOVETOEX_print(const char *contents, FILE *out);
void U_EMRSETMETARGN_print(const char *contents, FILE *out);
void U_EMREXCLUDECLIPRECT_print(const char *contents, FILE *out);
void U_EMRINTERSECTCLIPRECT_print(const char *contents, FILE *out);
void U_EMRSCALEVIEWPORTEXTEX_print(const char *contents, FILE *out);
void U_EMRSCALEWINDOWEXTEX_print(const char *contents, FILE *out);
void U_EMRSAVEDC_print(const char *contents, FILE *out);
void U_EMRRESTOREDC_print(const char *contents, FILE *out);
void U_EMRSETWORLDTRANSFORM_print(const char *contents, FILE *out);
void U_EMRMODIFYWORLDTRANSFORM_print(const char *contents, FILE *out);
void U_EMRSELECTOBJECT_print(const char *contents, FILE *out);
void U_EMRCREATEPEN_print(const char *contents, FILE *out);
void U_EMRCREATEBRUSHINDIRECT_print(const char *contents, FILE *out);
void U_EMRDELETEOBJECT_print(const char *contents, FILE *out);
void U_EMRANGLEARC_print(const char *contents, FILE *out);
void U_EMRELLIPSE_print(const char *contents, FILE *out);
void U_EMRRECTANGLE_print(const char *contents, FILE *out);
void U_EMRROUNDRECT_print(const char *contents, FILE *out);
void U_EMRARC_print(const char *contents, FILE *out);
void U_EMRCHORD_print(const char *contents, FILE *out);
void U_EMRPIE_print(const char *contents, FILE *out);
void U_EMRSELECTPALETTE_print(const char *contents, FILE *out);
void U_EMRCREATEPALETTE_print(const char *contents, FILE *out);
void U_EMRSETPALETTEENTRIES_print(const char *contents, FILE *out);
void U_EMRRESIZEPALETTE_print(const char *contents, FILE *out);
void U_EMRREALIZEPALETTE_print(const char *contents, FILE *out);
void U_EMREXTFLOODFILL_print(const char *contents, FILE *out);
void U_EMRLINETO_print(const char *contents, FILE *out);
void U_EMRARCTO_print(const char *contents, FILE *out);
void U_EMRPOLYDRAW_print(const char *contents, FILE *out);
void U_EMRSETARCDIRECTION_print(const char *contents, FILE *out);
void U_EMRSETMITERLIMIT_print(const char *contents, FILE *out);
void U_EMRBEGINPATH_print(const char *contents, FILE *out);
void U_EMRENDPATH_print(const char *contents, FILE *out);
void U_EMRCLOSEFIGURE_print(const char *contents, FILE *out);
void U_EMRFILLPATH_print(const char *contents, FILE *out);
void U_EMRSTROKEANDFILLPATH_print(const char *contents, FILE *out);
void U_EMRSTROKEPATH_print(const char *contents, FILE *out);
void U_EMRFLATTENPATH_print(const char *contents, FILE *out);
void U_EMRWIDENPATH_print(const char *contents, FILE *out);
void U_EMRSELECTCLIPPATH_print(const char *contents, FILE *out);
void U_EMRABORTPATH_print(const char *contents, FILE *out);
void U_EMRCOMMENT_print(const char *contents, FILE *out, const char *blimit, size_t off);
void U_EMRFILLRGN_print(const char *contents, FILE *out);
void U_EMRFRAMERGN_print(const char *contents, FILE *out);
void U_EMRINVERTRGN_print(const char *contents, FILE *out);
void U_EMRPAINTRGN_print(const char *contents, FILE *out);
void U_EMREXTSELECTCLIPRGN_print(const char *contents, FILE *out);
void U_EMRBITBLT_print(const char *contents, FILE *out);
void U_EMRSTRETCHBLT_print(const char *contents, FILE *out);
void U_EMRMASKBLT_print(const char *contents, FILE *out);
void U_EMRPLGBLT_print(const char *contents, FILE *out);
void U_EMRSETDIBITSTODEVICE_print(const char *contents, FILE *out);
void U_EMRSTRETCHDIBITS_print(const char *contents, FILE *out);
void U_EMREXTCREATEFONTINDIRECTW_print(const char *contents, FILE *out);
void U_EMREXTTEXTOUTA_print(const char *contents, FILE *out);
void U_EMREXTTEXTOUTW_print(const char *contents, FILE *out);
void U_EMRPOLYBEZIER16_print(const char *contents, FILE *out);
void U_EMRPOLYGON16_print(const char *contents, FILE *out);
void U_EMRPOLYLINE16_print(const char *contents, FILE *out);
void U_EMRPOLYBEZIERTO16_print(const char *contents, FILE *out);
void U_EMRPOLYLINETO16_print(const char *contents, FILE *out);
void U_EMRPOLYPOLYLINE16_print(const char *contents, FILE *out);
void U_EMRPOLYPOLYGON16_print(const char *contents, FILE *out);
void U_EMRPOLYDRAW16_print(const char *contents, FILE *out);
void U_EMRCREATEMONOBRUSH_print(const char *contents, FILE *out);
void U_EMRCREATEDIBPATTERNBRUSHPT_print(const char *contents, FILE *out);
void U_EMREXTCREATEPEN_print(const char *contents, FILE *out);
void U_EMRSETICMMODE_print(const char *contents, FILE *out);
void U_EMRCREATECOLORSPACE_print(const char *contents, FILE *out);
void U_EMRSETCOLORSPACE_print(const char *contents, FILE *out);
void U_EMRDELETECOLORSPACE_print(const char *contents, FILE *out);
void U_EMRPIXELFORMAT_print(const char *contents, FILE *out);
void U_EMRSMALLTEXTOUT_print(const char *contents, FILE *out);
void U_EMRALPHABLEND_print(const char *contents, FILE *out);
void U_EMRSETLAYOUT_print(const char *contents, FILE *out);
void U_EMRTRANSPARENTBLT_print(const char *contents, FILE *out);
void U_EMRGRADIENTFILL_print(const char *contents, FILE *out);
void U_EMRCREATECOLORSPACEW_print(const char *contents, FILE *out);
int U_emf_onerec_print(const char *contents, const char *blimit, int recnum, size_t off, FILE *out);
int emf2svg(char *contents, size_t length, char **out);
//! \endcond


}

