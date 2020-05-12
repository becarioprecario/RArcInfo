/**********************************************************************
 * $Id: avctest.c,v 1.1.1.1 2001/06/27 20:10:56 vir Exp $
 *
 * Name:     avctest.c
 * Project:  Arc/Info Vector coverage (AVC) BIN->E00 conversion library
 * Language: ANSI C
 * Purpose:  Test various part of the lib., and generate binary dumps.
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
 * $Log: avctest.c,v $
 * Revision 1.1.1.1  2001/06/27 20:10:56  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:13  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.4  1999/05/17 16:25:44  daniel
 * Detect .pal (RPL) files and handle them as PAL files
 *
 * Revision 1.3  1999/05/11 02:13:49  daniel
 * Added tests for some write functions and made major changes to
 * command-line args.
 *
 * Revision 1.2  1999/02/25 03:38:36  daniel
 * Modified bin. dump to include doubles, and added TXT/TX6 test functions
 *
 **********************************************************************/


#include "avc.h"

/* Prototypes for some internal functions not included in the lib
 * headers but that we want to test in this program.
 */
int _AVCBinReadHeader(AVCRawBinFile *psFile, AVCBinHeader *psHdr);
int _AVCBinReadNextArc(AVCRawBinFile *psFile, AVCArc *psArc, int nPrecision);
int _AVCBinReadNextPal(AVCRawBinFile *psFile, AVCPal *psPal, int nPrecision);
int _AVCBinReadNextCnt(AVCRawBinFile *psFile, AVCCnt *psCnt, int nPrecision);
int _AVCBinReadNextLab(AVCRawBinFile *psFile, AVCLab *psLab, int nPrecision);
int _AVCBinReadNextTxt(AVCRawBinFile *psFile, AVCTxt *psTxt, int nPrecision);
int _AVCBinReadNextArcDir(AVCRawBinFile *psInfo, AVCTableDef *psArcDir);
int _AVCBinReadNextArcNit(AVCRawBinFile *psInfo, AVCFieldInfo *psField);


#include <ctype.h>

static int  DumpHeader(AVCRawBinFile *hFile);
static void DumpArcFile(AVCRawBinFile *hFile);
static void DumpPalFile(AVCRawBinFile *psFile);
static void DumpCntFile(AVCRawBinFile *psFile);
static void DumpLabFile(AVCRawBinFile *psFile);
static void DumpTxtFile(AVCRawBinFile *psFile);
static void DumpArcDirFile(AVCRawBinFile *psFile);
static void DumpArcNitFile(AVCRawBinFile *psFile);

static void DumpBytes(AVCRawBinFile *psFile);

static void ConvertArcFile(AVCRawBinFile *psFile);
static void ConvertPalFile(AVCRawBinFile *psFile);
static void ConvertCntFile(AVCRawBinFile *psFile);
static void ConvertLabFile(AVCRawBinFile *psFile);
static void ConvertTxtFile(AVCRawBinFile *psFile, AVCFileType eType,
                           const char *pszFilename);

static void ConvertCover(const char *pszFname);
static void ConvertAATOnly(const char *pszFname);

static void DupBinFile(const char *pszSrcFile, const char *pszDstFile);


#define AVCTEST_USAGE "Usage: avctest -b|-bindump  <filename>\n" \
                      "or     avctest -e|-e00dump  <filename>\n" \
                      "or     avctest -c|-cov2e00  <coverage>\n" \
                      "or     avctest -a|-aatonly  <coverage>\n" \
                      "or     avctest -d|-dupfile  <src_file> <dst_filename>\n"

/**********************************************************************
 *                          main()
 *
 * This program is used to dump binary files (default behavior), and to
 * test some parts of the lib. during the development process. 
 **********************************************************************/
int main(int argc, char *argv[])
{
    const char  *pszFname;
    AVCRawBinFile  *hFile;

/*---------------------------------------------------------------------
 *      Read program arguments.
 *--------------------------------------------------------------------*/
    if (argc<2)
    {
        printf("%s", AVCTEST_USAGE);
        return 1;
    }
    else
    {
        pszFname = argv[2];
    }
    
    if (EQUALN(argv[1], "-cov2e00", 2))
    {
/*---------------------------------------------------------------------
 *     With option -c <coverage_name> ... 
 *     convert the whole coverage to E00
 *--------------------------------------------------------------------*/
        ConvertCover(pszFname);
    }
    else if (EQUALN(argv[1], "-aatonly", 2))
    {
/*---------------------------------------------------------------------
 *     With option -aatonly <coverage_name> ... 
 *     use AVCE00ReadGotoSection() to read only the .AAT table if there
 *     is one.
 *--------------------------------------------------------------------*/
        ConvertAATOnly(pszFname);
    }
/*---------------------------------------------------------------------
 *      With option -bindump <filename>
 *      Open file, and dump binary contents.
 *--------------------------------------------------------------------*/
    else if (EQUALN(argv[1], "-bindump", 2) &&
             (hFile = AVCRawBinOpen(pszFname, "r"))!=NULL)
    {
        DumpBytes(hFile);

        AVCRawBinClose(hFile);
    } 
/*---------------------------------------------------------------------
 *      With option -e00dump <filename>
 *      Open file, and dump its contents in E00 format.
 *      If file name is not recognized, then the binary contents is returned.
 *--------------------------------------------------------------------*/
    else if (EQUALN(argv[1], "-e00dump", 2) &&
             (hFile = AVCRawBinOpen(pszFname, "r"))!=NULL)
    {

        if (strstr(pszFname, "arc.adf") != NULL)
            ConvertArcFile(hFile);
/*            DumpArcFile(hFile);*/
        else if (strstr(pszFname, "pal.adf") != NULL ||
                 strstr(pszFname, ".pal") != NULL )
            ConvertPalFile(hFile);
/*            DumpPalFile(hFile);*/
        else if (strstr(pszFname, "cnt.adf") != NULL)
            ConvertCntFile(hFile);
/*            DumpCntFile(hFile);*/
        else if (strstr(pszFname, "lab.adf") != NULL)
            ConvertLabFile(hFile);
/*            DumpLabFile(hFile);*/
        else if (strstr(pszFname, "txt.adf") != NULL)
            ConvertTxtFile(hFile, AVCFileTXT, pszFname);
        else if (strstr(pszFname, ".txt") != NULL )
            ConvertTxtFile(hFile, AVCFileTX6, pszFname);
        else if (strstr(pszFname, "arc.dir") != NULL)
            DumpArcDirFile(hFile);
        else if (strstr(pszFname, ".nit") != NULL)
            DumpArcNitFile(hFile);
        else
            DumpBytes(hFile);

        AVCRawBinClose(hFile);
    }
/*---------------------------------------------------------------------
 *      avctest -dupfile <src_file> <dst_filename>
 *
 *      Read specified binary file, and write it back to a new file.
 *--------------------------------------------------------------------*/
    else if (argc==4 && EQUALN(argv[1], "-dupfile", 2))
    {
        DupBinFile(argv[2], argv[3]);
    }
    else
    {
        printf("Cannot process file %s\n\n", pszFname);
        printf("%s", AVCTEST_USAGE);
        
        return 1;
    }

    return 0;
}


/**********************************************************************
 *                          DumpHeader()
 *
 * Read and dump the header contents.
 *
 * Returns the coverage precision: AVC_SINGLE/DOUBLE_PREC
 **********************************************************************/
static int DumpHeader(AVCRawBinFile *hFile)
{
    AVCBinHeader sHdr;
    int          nStatus;

    nStatus = _AVCBinReadHeader(hFile, &sHdr);

    if (nStatus == 0)
    {
        printf("\nFile Header:\n");
        printf("  Signature:  0x%8.8x (%d)\n", sHdr.nSignature, 
                                               sHdr.nSignature);
        printf("  Precision:  0x%8.8x (%d)\n", sHdr.nPrecision, 
                                               sHdr.nPrecision);
        printf("  Length:     0x%8.8x (%d)\n", sHdr.nLength, 
                                               sHdr.nLength);
        printf("\n");
    }

    if (sHdr.nPrecision < 0)
        return AVC_DOUBLE_PREC;
    else
        return AVC_SINGLE_PREC;

}


/**********************************************************************
 *                          DumpArcFile()
 *
 * Read and dump the contents of an arc.adf file.
 **********************************************************************/
static void DumpArcFile(AVCRawBinFile *psFile)
{
    AVCArc      sArc;
    int         i, nPrec;

    sArc.pasVertices = NULL;
    sArc.numVertices = 0;

    nPrec = DumpHeader(psFile);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextArc(psFile, &sArc, nPrec);

        printf("%d  %d  %d  %d  %d  %d  %d\n", sArc.nArcId, sArc.nUserId,
                                           sArc.nFNode, sArc.nTNode,
                                           sArc.nLPoly, sArc.nRPoly,
                                           sArc.numVertices);

        for(i=0; i<sArc.numVertices; i++)
        {
            printf("%e  %e\n", sArc.pasVertices[i].x, sArc.pasVertices[i].y);
        }
    }

    CPLFree(sArc.pasVertices);
}

/**********************************************************************
 *                          DumpPalFile()
 *
 * Read and dump the contents of an pal.adf file.
 **********************************************************************/
static void DumpPalFile(AVCRawBinFile *psFile)
{
    AVCPal      sPal;
    int         i, nPrec;

    sPal.pasArcs = NULL;
    sPal.numArcs = 0;

    nPrec = DumpHeader(psFile);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextPal(psFile, &sPal, nPrec);

        printf("(%d)\n", sPal.nPolyId); 
        printf("%d  %e  %e  %e  %e\n", sPal.numArcs, 
                                       sPal.sMin.x, sPal.sMin.y,
                                       sPal.sMax.x, sPal.sMax.y);

        for(i=0; i<sPal.numArcs; i++)
        {
            printf("%d  %d  %d\n", sPal.pasArcs[i].nArcId,
                                   sPal.pasArcs[i].nFNode,
                                   sPal.pasArcs[i].nAdjPoly);
        }
    }

    CPLFree(sPal.pasArcs);
}


/**********************************************************************
 *                          DumpCntFile()
 *
 * Read and dump the contents of an cnt.adf file.
 **********************************************************************/
static void DumpCntFile(AVCRawBinFile *psFile)
{
    AVCCnt      sCnt;
    int         i, nPrec;

    nPrec = DumpHeader(psFile);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextCnt(psFile, &sCnt, nPrec);

        printf("(%d)\n", sCnt.nPolyId); 
        printf("%d  %e  %e\n", sCnt.numLabels, 
                               sCnt.sCoord.x, sCnt.sCoord.y);

        for(i=0; i<10 && i < sCnt.numLabels; i++)
            printf("  %d", sCnt.panLabelIds[i]);

        if (sCnt.numLabels > 10)
            printf("...");

        printf("\n");
    }
}


/**********************************************************************
 *                          DumpLabFile()
 *
 * Read and dump the contents of an lab.adf file.
 **********************************************************************/
static void DumpLabFile(AVCRawBinFile *psFile)
{
    AVCLab      sLab;
    int         nPrec;

    nPrec = DumpHeader(psFile);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextLab(psFile, &sLab, nPrec);

        printf("%d  %d  %e  %e\n", sLab.nValue, sLab.nPolyId,
                                   sLab.sCoord1.x, sLab.sCoord1.y);
        printf("%e  %e  %e  %e\n", sLab.sCoord2.x, sLab.sCoord2.y,
                                   sLab.sCoord3.x, sLab.sCoord3.y);

    }
}


/**********************************************************************
 *                          DumpTxtFile()
 *
 * Read and dump the contents of a txt.adf file.
 **********************************************************************/
static void DumpTxtFile(AVCRawBinFile *psFile)
{
    AVCTxt      sTxt;
    int         i, nPrec, numVerticesTotal;

    sTxt.numVerticesLine = sTxt.numVerticesArrow = 0;
    sTxt.pasVertices = NULL;

    sTxt.numChars = 0;
    sTxt.pszText = NULL;

    nPrec = DumpHeader(psFile);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextTxt(psFile, &sTxt, nPrec);

        printf("(%d)  %d  %d  %d  %d  %d  %d  %d  (0!)\n", 
                                           sTxt.nTxtId, sTxt.nUserId,
                                           sTxt.nLevel, sTxt.numVerticesLine,
                                           sTxt.numVerticesArrow, sTxt.nSymbol,
                                           sTxt.n28, sTxt.numChars );

        printf("Just = %d / %d\n", sTxt.anJust2[0], sTxt.anJust1[0]);
        printf("%e\n", sTxt.f_1e2);
        printf("%e  %e  %e\n", sTxt.dHeight, sTxt.dV2, sTxt.dV3);

        numVerticesTotal = ABS(sTxt.numVerticesLine) +
                           ABS(sTxt.numVerticesArrow);

        for(i=0; i<numVerticesTotal; i++)
        {
            printf("%e  %e\n", sTxt.pasVertices[i].x, sTxt.pasVertices[i].y);
        }

        printf("%s\n", sTxt.pszText);
    }

    CPLFree(sTxt.pszText);
    CPLFree(sTxt.pasVertices);
}



/**********************************************************************
 *                          DumpArcDirFile()
 *
 * Read and dump the contents of an arc.dir file.
 **********************************************************************/
static void DumpArcDirFile(AVCRawBinFile *psFile)
{
    AVCTableDef   sArcDir;

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextArcDir(psFile, &sArcDir);

        printf("(%s) %s %s %4d %4d %4d\n", sArcDir.szInfoFile,
                                            sArcDir.szTableName,
                                            sArcDir.szExternal,
                                            sArcDir.numFields,
                                            sArcDir.nRecSize,
                                            sArcDir.numRecords);
    }
}

/**********************************************************************
 *                          DumpArcNitFile()
 *
 * Read and dump the contents of an arc####.nit file.
 **********************************************************************/
static void DumpArcNitFile(AVCRawBinFile *psFile)
{
    AVCFieldInfo   sField;

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextArcNit(psFile, &sField);

        printf("%s%4d%2d%4d%1d%2d%4d%2d%2d%1d%2d%4d%4d%2d%s%4d\n",
               sField.szName,
               sField.nSize,
               sField.v2,
               sField.nOffset,
               sField.v4,
               sField.v5,
               sField.nFmtWidth,
               sField.nFmtPrec,
               sField.nType1,
               sField.nType2,
               sField.v10,
               sField.v11,
               sField.v12,
               sField.v13,
               sField.szAltName,
               sField.nIndex );
    }
}


/**********************************************************************
 *                          DumpBytes()
 *
 * Read and dump the contents of an Binary file.
 **********************************************************************/
static void DumpBytes(AVCRawBinFile *psFile)
{
    int         nOffset=0;
    GInt32      nValue, anVal[2];
    GInt16      *pn16Val1, *pn16Val2;
    float       *pfValue;
    char        *pcValue;
    double      *pdValue;

    pfValue = (float*)(&nValue);
    pcValue = (char*)(&nValue);
    pdValue = (double*)anVal;

    pn16Val1 = (GInt16*)(pcValue+2);
    pn16Val2 = (GInt16*)(pcValue);

    anVal[0] = anVal[1] = 0;

    while(!AVCRawBinEOF(psFile))
    {
        nValue = AVCRawBinReadInt32(psFile);
        
        /* For double precision values, we only use the first half 
         * of the height bytes... and leave the other 4 bytes as zeros!
         * It's a bit of a hack, but it seems to be enough for the 
         * precision of the values we print!
         */
#ifdef CPL_MSB
        anVal[0] = nValue;
#else
        anVal[1] = nValue;
#endif

        printf("%d\t0x%8.8x  %-5d\t%-6d %-6d %5.3e  d=%5.3e",
                                  nOffset, nValue, nValue,
                                  *pn16Val1, *pn16Val2, *pfValue, *pdValue);

        printf("\t[%c%c%c%c]\n", isprint(pcValue[3])?pcValue[3]:'.',
                                 isprint(pcValue[2])?pcValue[2]:'.',
                                 isprint(pcValue[1])?pcValue[1]:'.',
                                 isprint(pcValue[0])?pcValue[0]:'.');
        nOffset += 4;
    }
}



/**********************************************************************
 *                          ConvertArcFile()
 *
 * Read and convert the contents of an arc.adf file to E00.
 **********************************************************************/
static void ConvertArcFile(AVCRawBinFile *psFile)
{
    AVCArc      sArc;
    AVCE00GenInfo *psGenInfo;
    const char  *pszLine;
    int         nPrec;

    sArc.pasVertices = NULL;
    sArc.numVertices = 0;

    nPrec = DumpHeader(psFile);

    psGenInfo = AVCE00GenInfoAlloc(nPrec);

    pszLine = AVCE00GenStartSection(psGenInfo, AVCFileARC, "");
    printf("%s\n", pszLine);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextArc(psFile, &sArc, nPrec);

        pszLine = AVCE00GenArc(psGenInfo, &sArc, FALSE);

        while(pszLine != NULL)
        {
            printf("%s\n", pszLine);
            pszLine = AVCE00GenArc(psGenInfo, &sArc, TRUE);
        }
    }

    pszLine = AVCE00GenEndSection(psGenInfo, AVCFileARC, FALSE);
    printf("%s\n", pszLine);

    CPLFree(sArc.pasVertices);
    AVCE00GenInfoFree(psGenInfo);
}


/**********************************************************************
 *                          ConvertPalFile()
 *
 * Read and conver the contents of a pal.adf file to E00.
 **********************************************************************/
static void ConvertPalFile(AVCRawBinFile *psFile)
{
    AVCPal      sPal;
    AVCE00GenInfo *psGenInfo;
    const char  *pszLine;
    int         nPrec;

    sPal.pasArcs = NULL;
    sPal.numArcs = 0;

    nPrec = DumpHeader(psFile);

    psGenInfo = AVCE00GenInfoAlloc(nPrec);

    pszLine = AVCE00GenStartSection(psGenInfo, AVCFilePAL, "");
    printf("%s\n", pszLine);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextPal(psFile, &sPal, nPrec);

        pszLine = AVCE00GenPal(psGenInfo, &sPal, FALSE);

        while(pszLine != NULL)
        {
            printf("%s\n", pszLine);
            pszLine = AVCE00GenPal(psGenInfo, &sPal, TRUE);
        }
    }

    pszLine = AVCE00GenEndSection(psGenInfo, AVCFilePAL, FALSE);
    printf("%s\n", pszLine);

    CPLFree(sPal.pasArcs);
    AVCE00GenInfoFree(psGenInfo);
}


/**********************************************************************
 *                          ConvertCntFile()
 *
 * Read and conver the contents of a cnt.adf file to E00.
 **********************************************************************/
static void ConvertCntFile(AVCRawBinFile *psFile)
{
    AVCCnt      sCnt;
    AVCE00GenInfo *psGenInfo;
    const char  *pszLine;
    int         nPrec;

    sCnt.panLabelIds = NULL;

    nPrec = DumpHeader(psFile);

    psGenInfo = AVCE00GenInfoAlloc(nPrec);

    pszLine = AVCE00GenStartSection(psGenInfo, AVCFileCNT, "");
    printf("%s\n", pszLine);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextCnt(psFile, &sCnt, nPrec);

        pszLine = AVCE00GenCnt(psGenInfo, &sCnt, FALSE);

        while(pszLine != NULL)
        {
            printf("%s\n", pszLine);
            pszLine = AVCE00GenCnt(psGenInfo, &sCnt, TRUE);
        }
    }

    pszLine = AVCE00GenEndSection(psGenInfo, AVCFileCNT, FALSE);
    printf("%s\n", pszLine);

    AVCE00GenInfoFree(psGenInfo);
}

/**********************************************************************
 *                          ConvertLabFile()
 *
 * Read and conver the contents of a cnt.adf file to E00.
 **********************************************************************/
static void ConvertLabFile(AVCRawBinFile *psFile)
{
    AVCLab      sLab;
    AVCE00GenInfo *psGenInfo;
    const char  *pszLine;
    int         nPrec;

    nPrec = DumpHeader(psFile);

    psGenInfo = AVCE00GenInfoAlloc(nPrec);

    pszLine = AVCE00GenStartSection(psGenInfo, AVCFileLAB, "");
    printf("%s\n", pszLine);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextLab(psFile, &sLab, nPrec);

        pszLine = AVCE00GenLab(psGenInfo, &sLab, FALSE);

        while(pszLine != NULL)
        {
            printf("%s\n", pszLine);
            pszLine = AVCE00GenLab(psGenInfo, &sLab, TRUE);
        }
    }

    pszLine = AVCE00GenEndSection(psGenInfo, AVCFileLAB, FALSE);
    printf("%s\n", pszLine);

    AVCE00GenInfoFree(psGenInfo);
}

/**********************************************************************
 *                          ConvertTxtFile()
 *
 * Read and conver the contents of a txt.adf file to E00.
 *
 * This function supports both  TXT and TX6 type of files.
 **********************************************************************/
static void ConvertTxtFile(AVCRawBinFile *psFile, AVCFileType eType,
                           const char *pszFilename)
{
    AVCTxt      sTxt;
    AVCE00GenInfo *psGenInfo;
    const char  *pszLine;
    int         nPrec;

    /* Initialize structure fields to NULL ... VERY IMPORTANT!
     */
    sTxt.numVerticesLine = sTxt.numVerticesArrow = 0;
    sTxt.pasVertices = NULL;

    sTxt.numChars = 0;
    sTxt.pszText = NULL;

    nPrec = DumpHeader(psFile);

    psGenInfo = AVCE00GenInfoAlloc(nPrec);

    pszLine = AVCE00GenStartSection(psGenInfo, eType, pszFilename);
    printf("%s\n", pszLine);

    while(!AVCRawBinEOF(psFile))
    {
        _AVCBinReadNextTxt(psFile, &sTxt, nPrec);

        if (eType == AVCFileTXT)
            pszLine = AVCE00GenTxt(psGenInfo, &sTxt, FALSE);
        else
            pszLine = AVCE00GenTx6(psGenInfo, &sTxt, FALSE);

        while(pszLine != NULL)
        {
            printf("%s\n", pszLine);
            if (eType == AVCFileTXT)
                pszLine = AVCE00GenTxt(psGenInfo, &sTxt, TRUE);
            else
                pszLine = AVCE00GenTx6(psGenInfo, &sTxt, TRUE);
        }
    }

    pszLine = AVCE00GenEndSection(psGenInfo, eType, FALSE);
    printf("%s\n", pszLine);

    AVCE00GenInfoFree(psGenInfo);

    CPLFree(sTxt.pszText);
    CPLFree(sTxt.pasVertices);
}



/**********************************************************************
 *                          ConvertCover()
 *
 * Convert a complete coverage to E00.
 **********************************************************************/
static void ConvertCover(const char *pszFname)
{
    AVCE00ReadPtr hReadInfo;
    const char *pszLine;

    hReadInfo = AVCE00ReadOpen(pszFname);

    if (hReadInfo)
    {
        while ((pszLine = AVCE00ReadNextLine(hReadInfo)) != NULL)
        {
            printf("%s\n", pszLine);
        }

        AVCE00ReadClose(hReadInfo);
    }
}


/**********************************************************************
 *                          ConvertAATonly()
 *
 * Look for a .AAT table in the coverage, and if we find one then 
 * convert it to E00...
 *
 * This function exercises the AVCE00ReadSectionsList() and 
 * AVCE00ReadGotoSection() functions.
 **********************************************************************/
static void ConvertAATOnly(const char *pszFname)
{
    AVCE00ReadPtr   hReadInfo;
    AVCE00Section  *pasSect;
    const char     *pszLine;
    int             i, numSect;
    GBool           bFound;

    hReadInfo = AVCE00ReadOpen(pszFname);

    if (hReadInfo)
    {
        /* Fetch the list of E00 sections for the coverage, and 
         * try to find a .AAT table in it.
         */
        pasSect = AVCE00ReadSectionsList(hReadInfo, &numSect);
        bFound = FALSE;
        for(i=0; i<numSect; i++)
        {
            if (pasSect[i].eType == AVCFileTABLE &&
                strstr(pasSect[i].pszName, ".AAT") != NULL)
            {
                /* Found it!  Move the read pointer to the beginning 
                 * of the .AAT table, and tell the lib to stop reading
                 * at the end of table (3rd argument=FALSE)
                 */
                bFound = TRUE;
                AVCE00ReadGotoSection(hReadInfo, &(pasSect[i]), FALSE);
                break;
            }
        }

        if (bFound)
        {
            /* Convert the .AAT table to E00.  AVCE00ReadNextLine()
             * will return NULL at the end of the table.
             */
            while ((pszLine = AVCE00ReadNextLine(hReadInfo)) != NULL)
            {
                printf("%s\n", pszLine);
            }
        }
        else
        {
            printf("No .AAT table found in this coverage!\n");
        }

        AVCE00ReadClose(hReadInfo);
    }
}



/**********************************************************************
 *                          DupBinFile()
 *
 * Read and duplicate the contents of a binary coverage file.
 **********************************************************************/
static void DupBinFile(const char *pszSrcFile, const char *pszDstFile)
{
    AVCBinFile *psSrc=NULL, *psDst=NULL;
    void       *psObj;
    AVCFileType eType = AVCFileUnknown;

    /*-----------------------------------------------------------------
     * Open source file and create destination file
     *----------------------------------------------------------------*/
    if (strstr(pszSrcFile, "arc.adf") != NULL)
        eType = AVCFileARC;
    else if (strstr(pszSrcFile, "pal.adf") != NULL)
        eType = AVCFilePAL;
    else if (strstr(pszSrcFile, "cnt.adf") != NULL)
        eType = AVCFileCNT;
    else if (strstr(pszSrcFile, "lab.adf") != NULL)
        eType = AVCFileLAB;
    else if (strstr(pszSrcFile, "txt.adf") != NULL)
        eType = AVCFileTXT;
    else if (strstr(pszSrcFile, ".txt") != NULL )
        eType = AVCFileTX6;
/*
    else if (strstr(pszSrcFile, "arc.dir") != NULL)
        eType = AVCFile???;
    else if (strstr(pszSrcFile, ".nit") != NULL)
        eType = AVCFile???;
*/
    else
    {
        CPLError(CE_Failure, CPLE_NotSupported,
                 "Support for this file type is not implemented yet.");
        return;
    }

    if ((psSrc = AVCBinReadOpen("", pszSrcFile, eType)) == NULL)
        return;

    if ((psDst = AVCBinWriteCreate("", pszDstFile, 
                                   eType, psSrc->nPrecision)) == NULL)
    {
        AVCBinReadClose(psSrc);
        return;
    }

    /*-----------------------------------------------------------------
     * Copy objects
     *----------------------------------------------------------------*/
    while((psObj = AVCBinReadNextObject(psSrc)) != NULL)
    {
        switch(eType)
        {
          case AVCFileARC:
            AVCBinWriteArc(psDst, (AVCArc*)psObj);
            break;
          case AVCFilePAL:
            AVCBinWritePal(psDst, (AVCPal*)psObj);
            break;
          case AVCFileCNT:
            AVCBinWriteCnt(psDst, (AVCCnt*)psObj);
            break;
          case AVCFileLAB:
            AVCBinWriteLab(psDst, (AVCLab*)psObj);
            break;
          case AVCFileTOL:
            AVCBinWriteTol(psDst, (AVCTol*)psObj);
            break;
          default:
            CPLError(CE_Failure, CPLE_NotSupported,
                     "Support for this file type is not implemented yet.");
            AVCBinReadClose(psSrc);
            AVCBinWriteClose(psDst);
            return ;
        }
    }

    /*-----------------------------------------------------------------
     * Close files, cleanup and exit.
     *----------------------------------------------------------------*/
    AVCBinReadClose(psSrc);
    AVCBinWriteClose(psDst);
}

