/**********************************************************************
 * $Id: avc.h,v 1.1.1.1 2001/06/27 20:10:54 vir Exp $
 *
 * Name:     avc.h
 * Project:  Arc/Info Vector coverage (AVC) BIN<->E00 conversion library
 * Language: ANSI C
 * Purpose:  Header file containing all definitions for the library.
 * Author:   Daniel Morissette, danmo@videotron.ca
 *
 **********************************************************************
 * Copyright (c) 1999, Daniel Morissette
 *
 * All rights reserved.  This software may be copied or reproduced, in
 * all or in part, without the prior written consent of its author,
 * Daniel Morissette (danmo@videotron.ca).  However, any material copied
 * or reproduced must bear the original copyright notice (above), this 
 * original paragraph, and the original disclaimer (below).
 * 
 * The entire risk as to the results and performance of the software,
 * supporting text and other information contained in this file
 * (collectively called the "Software") is with the user.  Although 
 * considerable efforts have been used in preparing the Software, the 
 * author does not warrant the accuracy or completeness of the Software.
 * In no event will the author be liable for damages, including loss of
 * profits or consequential damages, arising out of the use of the 
 * Software.
 * 
 **********************************************************************
 *
 * $Log: avc.h,v $
 * Revision 1.1.1.1  2001/06/27 20:10:54  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:11  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.6  1999/08/23 18:15:56  daniel
 * Added AVCE00DeleteCoverage()
 *
 * Revision 1.5  1999/06/08 22:07:28  daniel
 * Added AVCReadWrite in AVCAccess type
 *
 * Revision 1.4  1999/05/17 16:16:41  daniel
 * Added RXP + TXT/TX6/TX7 write support
 *
 * Revision 1.3  1999/05/11 02:15:04  daniel
 * Added coverage write support
 *
 * Revision 1.2  1999/02/25 03:39:39  daniel
 * Added TXT, TX6/TX7, RXP and RPL support
 *
 * Revision 1.1  1999/01/29 16:29:24  daniel
 * Initial revision
 *
 **********************************************************************/

#ifndef _AVC_H_INCLUDED_
#define _AVC_H_INCLUDED_

#include "cpl_conv.h"
#include "cpl_string.h"

CPL_C_START


/* Coverage precision
 */
#define AVC_DEFAULT_PREC   0
#define AVC_SINGLE_PREC    1
#define AVC_DOUBLE_PREC    2

/* Coverage file types
 */
typedef enum
{
    AVCFileUnknown = 0,
    AVCFileARC,
    AVCFilePAL,
    AVCFileCNT,
    AVCFileLAB,
    AVCFilePRJ,
    AVCFileTOL,
    AVCFileLOG,
    AVCFileTXT,  /* TXT and TX6 share the same binary format */
    AVCFileTX6,
    AVCFileRXP,
    AVCFileRPL,  /* RPL is a PAL for a region */
    AVCFileTABLE
}AVCFileType;


/* Read or Write access flag
 */
typedef enum
{
    AVCRead,
    AVCWrite,
    AVCReadWrite
} AVCAccess;



/*=====================================================================
                        Structures
 =====================================================================*/

/*---------------------------------------------------------------------
 * Structures defining various Arc/Info objects types.
 * These are shared by the Binary and the E00 functions.
 *--------------------------------------------------------------------*/

typedef struct AVCVertex_t
{
    double      x;      /* Even for single precision, we always         */
    double      y;      /* use doubles for the vertices in memory.      */
}AVCVertex;

/*---------------------------------------------------------------------
 * AVCArc: Information about an ARC
 *--------------------------------------------------------------------*/
typedef struct AVCArc_t
{
    GInt32      nArcId;
    GInt32      nUserId;
    GInt32      nFNode;
    GInt32      nTNode;
    GInt32      nLPoly;
    GInt32      nRPoly;
    GInt32      numVertices;
    AVCVertex   *pasVertices;   
}AVCArc;

/*---------------------------------------------------------------------
 * AVCPal: A PAL (Polygon Arc List) references all the arcs that 
 *         constitute a polygon.
 *--------------------------------------------------------------------*/
typedef struct AVCPalArc_t
{
    GInt32      nArcId;
    GInt32      nFNode;
    GInt32      nAdjPoly;
}AVCPalArc;

typedef struct AVCPal_t
{
    GInt32      nPolyId;
    AVCVertex   sMin;
    AVCVertex   sMax;
    GInt32      numArcs;
    AVCPalArc   *pasArcs;
}AVCPal;

/*---------------------------------------------------------------------
 * AVCCnt: Information about a CNT (polygon centroid)
 *--------------------------------------------------------------------*/
typedef struct AVCCnt_t
{
    GInt32      nPolyId;
    AVCVertex   sCoord;
    GInt32      numLabels;      /* 0 or 1 */
    GInt32      *panLabelIds;
}AVCCnt;


/*---------------------------------------------------------------------
 * AVCLab: Information about a LAB (polygon Label)
 *--------------------------------------------------------------------*/
typedef struct AVCLab_t
{
    GInt32      nValue;
    GInt32      nPolyId;
    AVCVertex   sCoord1;
    AVCVertex   sCoord2;
    AVCVertex   sCoord3;
}AVCLab;

/*---------------------------------------------------------------------
 * AVCTol: Information about a TOL record (coverage tolerances)
 *--------------------------------------------------------------------*/
typedef struct AVCTol_t
{
    GInt32      nIndex;
    GInt32      nFlag;
    double      dValue;
}AVCTol;

/*---------------------------------------------------------------------
 * AVCTxt: Information about a TXT/TX6/TX7 record (annotations)
 *--------------------------------------------------------------------*/
typedef struct AVCTxt_t
{
    GInt32      nTxtId;
    GInt32      nUserId;
    GInt32      nLevel;
    float       f_1e2;	/* Always (float)-1e+20, even for double precision! */
    GInt32      nSymbol;
    GInt32      numVerticesLine;
    GInt32      n28;    /* Unknown value at byte 28 */
    GInt32      numChars;
    GInt32      numVerticesArrow;

    GInt16      anJust1[20];
    GInt16      anJust2[20];

    double      dHeight;
    double      dV2;    /* ??? */
    double      dV3;    /* ??? */

    char        *pszText;

    AVCVertex   *pasVertices;
}AVCTxt;

/*---------------------------------------------------------------------
 * AVCRxp: Information about a RXP record (something related to regions...)
 *--------------------------------------------------------------------*/
typedef struct AVCRxp_t
{
    GInt32      n1;
    GInt32      n2;
}AVCRxp;


/*---------------------------------------------------------------------
 * AVCTableDef: Definition of an INFO table's structure.  
 *               This info is read from several files: 
 *                   info/arc.dir
 *                   info/arc####.dat
 *                   info/arc####.nit
 *
 *               And the data for the table itself is stored in a binary
 *               file in the coverage directory.
 *--------------------------------------------------------------------*/

typedef struct AVCFieldInfo_t
{
    char        szName[17];
    GInt16      nSize;
    GInt16      v2;
    GInt16      nOffset;
    GInt16      v4;
    GInt16      v5;
    GInt16      nFmtWidth;
    GInt16      nFmtPrec;
    GInt16      nType1;
    GInt16      nType2;
    GInt16      v10;
    GInt16      v11;
    GInt16      v12;
    GInt16      v13;
    char        szAltName[17];
    GInt16      nIndex;         /* >0 if valid, or -1 if field is deleted */
}AVCFieldInfo;

#define AVC_FT_DATE     10
#define AVC_FT_CHAR     20
#define AVC_FT_FIXINT   30
#define AVC_FT_FIXNUM   40
#define AVC_FT_BININT   50
#define AVC_FT_BINFLOAT 60


typedef struct AVCTableDef_t
{
    /* Stuff read from the arc.dir file 
     * (1 record, corresponding to this table, from the arc.dir file)
     */
    char        szTableName[33];
    char        szInfoFile[8];
    GInt16      numFields;
    GInt16      nRecSize;
    GInt32      numRecords;
    char        szExternal[3];  /* "XX" or "  " */

    /* Data file path read from the arc####.dat file
     */
    char        szDataFile[81];

    /* Field information read from the arc####.nit file
     */
    AVCFieldInfo *pasFieldDef;

}AVCTableDef;

typedef struct AVCField_t
{
    GInt16      nInt16;
    GInt32      nInt32;
    float       fFloat;
    double      dDouble;
    char        *pszStr;
}AVCField;

/*---------------------------------------------------------------------
 * Stuff related to buffered reading of raw binary files
 *--------------------------------------------------------------------*/

#define AVCRAWBIN_READBUFSIZE 1024

typedef struct AVCRawBinFile_t
{
    FILE        *fp;
    char        *pszFname;
    AVCAccess   eAccess;
    GByte       abyBuf[AVCRAWBIN_READBUFSIZE];
    int         nOffset;        /* Location of current buffer in the file */
    int         nCurSize;       /* Nbr of bytes currently loaded        */
    int         nCurPos;        /* Next byte to read from abyBuf[]      */
}AVCRawBinFile;


/*---------------------------------------------------------------------
 * Stuff related to reading and writing binary coverage files
 *--------------------------------------------------------------------*/

typedef struct AVCBinHeader_t
{
    GUInt32     nSignature;
    GInt32      nPrecision;     /* <0 for double prec., >0 for single prec. */
    GInt32      nRecordSize;    /* nbr of 2 byte words, 0 for var. length   */
    GInt32      nLength;        /* Overall file length, in 2 byte words     */
}AVCBinHeader;

typedef struct AVCBinFile_t
{
    AVCRawBinFile *psRawBinFile;
    char          *pszFilename;
    AVCRawBinFile *psIndexFile;   /* Index file, Write mode only */

    AVCFileType   eFileType;
    int           nPrecision;     /* AVC_SINGLE/DOUBLE_PREC  */

    union
    {
        AVCTableDef  *psTableDef;
    }hdr;

    /* cur.* : temp. storage used to read one record (ARC, PAL, ... or 
     *         Table record) from the file.
     */
    union
    {
        AVCArc       *psArc;
        AVCPal       *psPal;
        AVCCnt       *psCnt;
        AVCLab       *psLab;
        AVCTol       *psTol;
        AVCTxt       *psTxt;
        AVCRxp       *psRxp;
        AVCField     *pasFields;
        char         **papszPrj;
    }cur;

}AVCBinFile;

/*---------------------------------------------------------------------
 * Stuff related to the generation of E00
 *--------------------------------------------------------------------*/

/*---------------------------------------------------------------------
 *                        AVCE00GenInfo structure
 * This structure is used by the E00 generator functions to store
 * their buffer and their current state in case they need to be
 * called more than once for a given object type (i.e. ARC, PAL and IFO).
 *--------------------------------------------------------------------*/

typedef struct AVCE00GenInfo_t
{
    char        *pszBuf;
    int         nBufSize;
    
    int         nPrecision;     /* AVC_SINGLE/DOUBLE_PREC       */
    int         iCurItem;
    int         numItems;
}AVCE00GenInfo;

/*---------------------------------------------------------------------
 * Stuff related to the parsing of E00
 *--------------------------------------------------------------------*/

/*---------------------------------------------------------------------
 *                        AVCE00ParseInfo structure
 * This structure is used by the E00 parser functions to store
 * their buffer and their current state while parsing an object.
 *--------------------------------------------------------------------*/

typedef struct AVCE00ParseInfo_t
{
    AVCFileType eFileType;
    int         nPrecision;     /* AVC_SINGLE/DOUBLE_PREC       */
    int         iCurItem;
    int         numItems;

    int         nCurObjectId;
    GBool       bForceEndOfSection;  /* For sections that don't have an */
                                     /* explicit end-of-section line.   */
    AVCFileType eSuperSectionType;/* For sections containing several files*/
    char        *pszSectionHdrLine;  /* Used by supersection types      */

    union
    {
        AVCTableDef  *psTableDef;
    }hdr;
    GBool       bTableHdrComplete;   /* FALSE until table header is */
                                     /* finished parsing */
    int         nTableE00RecLength;

    /* cur.* : temp. storage used to store current object (ARC, PAL, ... or 
     *         Table record) from the file.
     */
    union
    {
        AVCArc       *psArc;
        AVCPal       *psPal;
        AVCCnt       *psCnt;
        AVCLab       *psLab;
        AVCTol       *psTol;
        AVCTxt       *psTxt;
        AVCRxp       *psRxp;
        AVCField     *pasFields;
        char         **papszPrj;
    }cur;

    char        *pszBuf;        /* Buffer used only for TABLEs  */
    int         nBufSize;
}AVCE00ParseInfo;


/*---------------------------------------------------------------------
 * Stuff related to the transparent binary -> E00 conversion
 *--------------------------------------------------------------------*/
typedef struct AVCE00Section_t
{
    AVCFileType eType;     /* File Type          */
    char        *pszName;  /* File or Table Name */
}AVCE00Section;

typedef struct AVCE00ReadInfo_t
{
    char        *pszCoverPath;
    char        *pszInfoPath;
    char        *pszCoverName;

    /* pasSections is built when the coverage is opened and describes
     * the squeleton of the E00 file.
     */
    AVCE00Section *pasSections;
    int            numSections;

    /* If bReadAllSections=TRUE then reading automatically continues to
     * the next section when a section finishes. (This is the default)
     * Otherwise, you can use AVCE00ReadGotoSection() to read one section
     * at a time... this will set bReadAllSections=FALSE.
     */ 
    GBool         bReadAllSections;

    /* Info about the file (or E00 section) currently being processed
     */
    int           iCurSection;
    AVCBinFile    *hFile;

    int           iCurStep;  /* AVC_GEN_* values, see below */
    AVCE00GenInfo *hGenInfo;

} *AVCE00ReadPtr;

/* E00 generation steps... tells the AVCE00Read*() functions which
 * parts of the given E00 file are currently being processed.
 */
#define AVC_GEN_NOTSTARTED      0
#define AVC_GEN_DATA            1
#define AVC_GEN_ENDSECTION      2
#define AVC_GEN_TABLEHEADER     3
#define AVC_GEN_TABLEDATA       4

/*---------------------------------------------------------------------
 * Stuff related to the transparent E00 -> binary conversion
 *--------------------------------------------------------------------*/
typedef struct AVCE00WriteInfo_t
{
    char        *pszCoverPath;
    char        *pszInfoPath;
    char        *pszCoverName;

    /* Info about the file (or E00 section) currently being processed
     */
    AVCFileType  eCurFileType;
    AVCBinFile  *hFile;

    /* Requested precision for the new coverage... may differ from the
     * precision of the E00 input lines.  (AVC_SINGLE_PREC or AVC_DOUBLE_PREC)
     */
    int          nPrecision;

    AVCE00ParseInfo *hParseInfo;

} *AVCE00WritePtr;

/* Coverage generation steps... used to store the current state of the
 * AVCE00WriteNextLine() function.
 */
#define AVC_WR_TOPLEVEL         0
#define AVC_WR_READING_SECTION

/*=====================================================================
              Function prototypes (lower-level lib. functions)
 =====================================================================*/

/*---------------------------------------------------------------------
 * Functions related to buffered reading of raw binary files (and writing)
 *--------------------------------------------------------------------*/
AVCRawBinFile *AVCRawBinOpen(const char *pszFname, const char *pszAccess);
void        AVCRawBinClose(AVCRawBinFile *psInfo);
void        AVCRawBinFSeek(AVCRawBinFile *psInfo, int nOffset, int nFrom);
GBool       AVCRawBinEOF(AVCRawBinFile *psInfo);

void        AVCRawBinReadBytes(AVCRawBinFile *psInfo, int nBytesToRead, 
                               GByte *pBuf);
GInt16      AVCRawBinReadInt16(AVCRawBinFile *psInfo);
GInt32      AVCRawBinReadInt32(AVCRawBinFile *psInfo);
float       AVCRawBinReadFloat(AVCRawBinFile *psInfo);
double      AVCRawBinReadDouble(AVCRawBinFile *psInfo);

void        AVCRawBinWriteBytes(AVCRawBinFile *psFile, int nBytesToWrite,
                                GByte *pBuf);
void        AVCRawBinWriteInt16(AVCRawBinFile *psFile, GInt16 n16Value);
void        AVCRawBinWriteInt32(AVCRawBinFile *psFile, GInt32 n32Value);
void        AVCRawBinWriteFloat(AVCRawBinFile *psFile, float fValue);
void        AVCRawBinWriteDouble(AVCRawBinFile *psFile, double dValue);
void        AVCRawBinWriteZeros(AVCRawBinFile *psFile, int nBytesToWrite);
void        AVCRawBinWritePaddedString(AVCRawBinFile *psFile, int nFieldSize, 
                                       const char *pszString);

/*---------------------------------------------------------------------
 * Functions related to reading the binary coverage files
 *--------------------------------------------------------------------*/

AVCBinFile *AVCBinReadOpen(const char *pszPath, const char *pszName, 
                           AVCFileType eType);
void        AVCBinReadClose(AVCBinFile *psFile);

int         AVCBinReadRewind(AVCBinFile *psFile);

void       *AVCBinReadNextObject(AVCBinFile *psFile);
AVCArc     *AVCBinReadNextArc(AVCBinFile *psFile);
AVCPal     *AVCBinReadNextPal(AVCBinFile *psFile);
AVCCnt     *AVCBinReadNextCnt(AVCBinFile *psFile);
AVCLab     *AVCBinReadNextLab(AVCBinFile *psFile);
AVCTol     *AVCBinReadNextTol(AVCBinFile *psFile);
AVCTxt     *AVCBinReadNextTxt(AVCBinFile *psFile);
AVCRxp     *AVCBinReadNextRxp(AVCBinFile *psFile);
AVCField   *AVCBinReadNextTableRec(AVCBinFile *psFile);
char      **AVCBinReadPrj(AVCBinFile *psFile);

char      **AVCBinReadListTables(const char *pszInfoPath, 
                                 const char *pszCoverName,
                                 char ***ppapszArcDatFiles);

/*---------------------------------------------------------------------
 * Functions related to writing the binary coverage files
 *--------------------------------------------------------------------*/
AVCBinFile *AVCBinWriteCreate(const char *pszPath, const char *pszName, 
                              AVCFileType eType, int nPrecision);
AVCBinFile *AVCBinWriteCreateTable(const char *pszInfoPath, 
                                   AVCTableDef *psSrcTableDef,
                                   int nPrecision);
void        AVCBinWriteClose(AVCBinFile *psFile);

int         AVCBinWriteHeader(AVCBinFile *psFile);
int         AVCBinWriteObject(AVCBinFile *psFile, void *psObj);
int         AVCBinWriteArc(AVCBinFile *psFile, AVCArc *psArc);
int         AVCBinWritePal(AVCBinFile *psFile, AVCPal *psPal);
int         AVCBinWriteCnt(AVCBinFile *psFile, AVCCnt *psCnt);
int         AVCBinWriteLab(AVCBinFile *psFile, AVCLab *psLab);
int         AVCBinWriteTol(AVCBinFile *psFile, AVCTol *psTol);
int         AVCBinWritePrj(AVCBinFile *psFile, char **papszPrj);
int         AVCBinWriteTxt(AVCBinFile *psFile, AVCTxt *psTxt);
int         AVCBinWriteRxp(AVCBinFile *psFile, AVCRxp *psRxp);
int         AVCBinWriteTableRec(AVCBinFile *psFile, AVCField *pasFields);

/*---------------------------------------------------------------------
 * Functions related to the generation of E00
 *--------------------------------------------------------------------*/
AVCE00GenInfo  *AVCE00GenInfoAlloc(int nCoverPrecision);
void        AVCE00GenInfoFree(AVCE00GenInfo *psInfo);
void        AVCE00GenReset(AVCE00GenInfo  *psInfo);

const char *AVCE00GenStartSection(AVCE00GenInfo *psInfo, AVCFileType eType, 
                                  const char *pszFilename);
const char *AVCE00GenEndSection(AVCE00GenInfo *psInfo, AVCFileType eType,
                                GBool bCont);

const char *AVCE00GenObject(AVCE00GenInfo *psInfo, 
                            AVCFileType eType, void *psObj, GBool bCont);
const char *AVCE00GenArc(AVCE00GenInfo *psInfo, AVCArc *psArc, GBool bCont);
const char *AVCE00GenPal(AVCE00GenInfo *psInfo, AVCPal *psPal, GBool bCont);
const char *AVCE00GenCnt(AVCE00GenInfo *psInfo, AVCCnt *psCnt, GBool bCont);
const char *AVCE00GenLab(AVCE00GenInfo *psInfo, AVCLab *psLab, GBool bCont);
const char *AVCE00GenTol(AVCE00GenInfo *psInfo, AVCTol *psTol, GBool bCont);
const char *AVCE00GenTxt(AVCE00GenInfo *psInfo, AVCTxt *psTxt, GBool bCont);
const char *AVCE00GenTx6(AVCE00GenInfo *psInfo, AVCTxt *psTxt, GBool bCont);
const char *AVCE00GenPrj(AVCE00GenInfo *psInfo, char **papszPrj, GBool bCont);
const char *AVCE00GenRxp(AVCE00GenInfo *psInfo, AVCRxp *psRxp, GBool bCont);

const char *AVCE00GenTableHdr(AVCE00GenInfo *psInfo, AVCTableDef *psDef,
                              GBool bCont);
const char *AVCE00GenTableRec(AVCE00GenInfo *psInfo, int numFields,
                              AVCFieldInfo *pasDef, AVCField *pasFields,
                              GBool bCont);

/*---------------------------------------------------------------------
 * Functions related to parsing E00 lines
 *--------------------------------------------------------------------*/
AVCE00ParseInfo  *AVCE00ParseInfoAlloc();
void    AVCE00ParseInfoFree(AVCE00ParseInfo *psInfo);
void    AVCE00ParseReset(AVCE00ParseInfo  *psInfo);

AVCFileType AVCE00ParseSectionHeader(AVCE00ParseInfo  *psInfo, 
                                     const char *pszLine);
GBool   AVCE00ParseSectionEnd(AVCE00ParseInfo  *psInfo, const char *pszLine,
                              GBool bResetParseInfo);
AVCFileType AVCE00ParseSuperSectionHeader(AVCE00ParseInfo  *psInfo,
                                          const char *pszLine);
GBool   AVCE00ParseSuperSectionEnd(AVCE00ParseInfo  *psInfo,
                                   const char *pszLine );

void    *AVCE00ParseNextLine(AVCE00ParseInfo  *psInfo, const char *pszLine);
AVCArc  *AVCE00ParseNextArcLine(AVCE00ParseInfo *psInfo, const char *pszLine);
AVCPal  *AVCE00ParseNextPalLine(AVCE00ParseInfo *psInfo, const char *pszLine);
AVCCnt  *AVCE00ParseNextCntLine(AVCE00ParseInfo *psInfo, const char *pszLine);
AVCLab  *AVCE00ParseNextLabLine(AVCE00ParseInfo *psInfo, const char *pszLine);
AVCTol  *AVCE00ParseNextTolLine(AVCE00ParseInfo *psInfo, const char *pszLine);
AVCTxt  *AVCE00ParseNextTxtLine(AVCE00ParseInfo *psInfo, const char *pszLine);
AVCTxt  *AVCE00ParseNextTx6Line(AVCE00ParseInfo *psInfo, const char *pszLine);
char   **AVCE00ParseNextPrjLine(AVCE00ParseInfo *psInfo, const char *pszLine);
AVCRxp  *AVCE00ParseNextRxpLine(AVCE00ParseInfo *psInfo, const char *pszLine);
AVCTableDef *AVCE00ParseNextTableDefLine(AVCE00ParseInfo *psInfo, 
                                         const char *pszLine);
AVCField    *AVCE00ParseNextTableRecLine(AVCE00ParseInfo *psInfo, 
                                         const char *pszLine);


/*---------------------------------------------------------------------
 * Misc. functions shared by several parts of the lib.
 *--------------------------------------------------------------------*/
int _AVCE00ComputeRecSize(int numFields, AVCFieldInfo *pasDef);

void _AVCDestroyTableFields(AVCTableDef *psTableDef, AVCField *pasFields);
void _AVCDestroyTableDef(AVCTableDef *psTableDef);
AVCTableDef *_AVCDupTableDef(AVCTableDef *psSrcDef);

/*=====================================================================
              Function prototypes (THE PUBLIC ONES)
 =====================================================================*/

/*---------------------------------------------------------------------
 * Functions to make a binary coverage appear as E00
 *--------------------------------------------------------------------*/

AVCE00ReadPtr   AVCE00ReadOpen(const char *pszCoverPath);
void            AVCE00ReadClose(AVCE00ReadPtr psInfo);
const char     *AVCE00ReadNextLine(AVCE00ReadPtr psInfo);
int             AVCE00ReadRewind(AVCE00ReadPtr psInfo);

AVCE00Section  *AVCE00ReadSectionsList(AVCE00ReadPtr psInfo, int *numSect);
int             AVCE00ReadGotoSection(AVCE00ReadPtr psInfo, 
                                      AVCE00Section *psSect,
                                      GBool bContinue);

/*---------------------------------------------------------------------
 * Functions to write E00 lines to a binary coverage
 *--------------------------------------------------------------------*/

AVCE00WritePtr  AVCE00WriteOpen(const char *pszCoverPath, int nPrecision);
void            AVCE00WriteClose(AVCE00WritePtr psInfo);
int             AVCE00WriteNextLine(AVCE00WritePtr psInfo, 
                                    const char *pszLine);
int             AVCE00DeleteCoverage(const char *pszCoverPath);

CPL_C_END

#endif /* _AVC_H_INCLUDED_ */


