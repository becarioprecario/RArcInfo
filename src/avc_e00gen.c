/**********************************************************************
 * $Id: avc_e00gen.c,v 1.1.1.1 2001/06/27 20:10:52 vir Exp $
 *
 * Name:     avc_e00gen.c
 * Project:  Arc/Info vector coverage (AVC)  BIN->E00 conversion library
 * Language: ANSI C
 * Purpose:  Functions to generate ASCII E00 lines form binary structures.
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
 * $Log: avc_e00gen.c,v $
 * Revision 1.1.1.1  2001/06/27 20:10:52  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:09  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.7  1999/08/23 18:20:49  daniel
 * Fixed support for attribute fields type 40
 *
 * Revision 1.6  1999/05/17 16:19:39  daniel
 * Made sure ACVE00GenTableRec() removes all spaces at the end of a
 * table record line (it used to leave one space)
 *
 * Revision 1.5  1999/05/11 02:08:17  daniel
 * Simple changes related to the addition of coverage write support.
 *
 * Revision 1.4  1999/03/03 02:06:38  daniel
 * Properly handle 8 bytes floats inside single precision tables.
 *
 * Revision 1.3  1999/02/25 17:01:58  daniel
 * Added support for 16 bit integers in INFO tables (type=50, size=2)
 *
 * Revision 1.2  1999/02/25 04:17:51  daniel
 * Added TXT, TX6/TX7, RXP and RPL support + some minor changes
 *
 * Revision 1.1  1999/01/29 16:28:52  daniel
 * Initial revision
 *
 **********************************************************************/

#include "avc.h"

#include <ctype.h>      /* toupper() */

/**********************************************************************
 *                          AVCE00GenInfoAlloc()
 *
 * Allocate and initialize a new AVCE00GenInfo structure.
 *
 * The structure will eventually have to be freed with AVCE00GenInfoFree().
 **********************************************************************/
AVCE00GenInfo  *AVCE00GenInfoAlloc(int nCoverPrecision)
{
    AVCE00GenInfo       *psInfo;

    psInfo = (AVCE00GenInfo*)CPLCalloc(1,sizeof(AVCE00GenInfo));

    /* Allocate output buffer.  
     * 2k should be enough... the biggest thing we'll need to store
     * in it will be 1 complete INFO table record.
     */
    psInfo->nBufSize = 2048;
    psInfo->pszBuf = (char *)CPLMalloc(psInfo->nBufSize*sizeof(char));

    psInfo->nPrecision = nCoverPrecision;

    return psInfo;
}

/**********************************************************************
 *                          AVCE00GenInfoFree()
 *
 * Free any memory associated with a AVCE00GenInfo structure.
 **********************************************************************/
void    AVCE00GenInfoFree(AVCE00GenInfo  *psInfo)
{
    if (psInfo)
        CPLFree(psInfo->pszBuf);
    CPLFree(psInfo);
}


/**********************************************************************
 *                          AVCE00GenReset()
 *
 * Reset the fields in the AVCE00GenInfo structure so that further calls
 * with bCont = TRUE (ex: AVCE00GenArc(psInfo, TRUE)) would return NULL.
 **********************************************************************/
void    AVCE00GenReset(AVCE00GenInfo  *psInfo)
{
    /* Reinitialize counters so that further calls with bCont = TRUE,
     * like AVCE00GenArc(psInfo, TRUE) would return NULL.
     */
    psInfo->iCurItem = psInfo->numItems = 0;
}

/**********************************************************************
 *                          _PrintRealValue()
 *
 * Format a floating point value according to the specified coverage
 * precision (AVC_SINGLE/DOUBLE_PREC),  and append the formatted value 
 * to the end of the pszBuf buffer.
 *
 * The function returns the number of characters added to the buffer.
 **********************************************************************/
static int  _PrintRealValue(char *pszBuf, int nPrecision, AVCFileType eType,
                            double dValue)
{
    static int numExpDigits=-1;
    int        nLen = 0;

    /* WIN32 systems' printf for floating point output generates 3
     * digits exponents (ex: 1.23E+012), but E00 files must have 2 digits
     * exponents (ex: 1.23E+12).
     * Run a test (only once per prg execution) to establish the number
     * of exponent digits on the current platform.
     */
    if (numExpDigits == -1)
    {
        char szBuf[50];
        int  i;

        sprintf(szBuf, "%10.7E", 123.45);
        numExpDigits = 0;
        for(i=strlen(szBuf)-1; i>0; i--)
        {
            if (szBuf[i] == '+' || szBuf[i] == '-')
                break;
            numExpDigits++;
        }
    }

    /* We will append the value at the end of the current buffer contents.
     */
    pszBuf = pszBuf+strlen(pszBuf);

    if (dValue < 0.0)
    {
        *pszBuf = '-';
        dValue = -1.0*dValue;
    }
    else
        *pszBuf = ' ';


    /* Just to make things more complicated, double values are 
     * output in a different format in attribute tables than in 
     * the other files!
     */
    if (nPrecision == AVC_DOUBLE_PREC && eType == AVCFileTABLE)
    {
        sprintf(pszBuf+1, "%20.17E", dValue);
        nLen = 24;
    }
    else if (nPrecision == AVC_DOUBLE_PREC)
    {
        sprintf(pszBuf+1, "%17.14E", dValue);
        nLen = 21;
    }
    else
    {
        sprintf(pszBuf+1, "%10.7E", dValue);
        nLen = 14;
    }

    /* Adjust number of exponent digits if necessary
     */
    if (numExpDigits > 2)
    {
        int n;
        n = strlen(pszBuf);
        
        pszBuf[n - numExpDigits]    = pszBuf[n-2];
        pszBuf[n - numExpDigits +1] = pszBuf[n-1];
        pszBuf[n - numExpDigits +2] = '\0';
    }

    /* Just make sure that the actual output length is what we expected.
     */
    CPLAssert(strlen(pszBuf) == nLen);

    return nLen;
}


/**********************************************************************
 *                          AVCE00GenStartSection()
 *
 * Generate the first line of an E00 section.
 * pszName can be one of "ARC", "PAL", "CNT", or "LAB".
 **********************************************************************/
const char *AVCE00GenStartSection(AVCE00GenInfo *psInfo, AVCFileType eType, 
                                  const char *pszFilename)
{
    char *pszName;

    AVCE00GenReset(psInfo);

    if (eType == AVCFileTX6 || eType == AVCFileRXP || eType == AVCFileRPL)
    {
        /* TX6/RXP/RPL sections start with the name of the file in uppercase.
         * ex:  The section for "cities.txt" would start with "CITIES"
         */
        int i;
        for(i=0; pszFilename[i] != '\0'; i++)
        {
            if (pszFilename[i] == '.')
            {
                psInfo->pszBuf[i] = '\0';
                break;
            }
            psInfo->pszBuf[i] = toupper(pszFilename[i]);
        }

    }
    else
    {
        /* In most cases, the section starts with a 3 letters code followed
         * by the precision code (2 or 3)
         */
        switch(eType)
        {
          case AVCFileARC:
            pszName = "ARC";
            break;
          case AVCFilePAL:
            pszName = "PAL";
            break;
          case AVCFileCNT:
            pszName = "CNT";
            break;
          case AVCFileLAB:
            pszName = "LAB";
            break;
          case AVCFileTOL:
            pszName = "TOL";
            break;
          case AVCFilePRJ:
            pszName = "PRJ";
            break;
          case AVCFileTXT:
            pszName = "TXT";
            break;
          default:
            CPLError(CE_Failure, CPLE_NotSupported,
                     "Unsupported E00 section type!");
        }

        if (psInfo->nPrecision == AVC_DOUBLE_PREC)
            sprintf(psInfo->pszBuf, "%s  3", pszName);
        else
            sprintf(psInfo->pszBuf, "%s  2", pszName);
    }

    return psInfo->pszBuf;
}

/**********************************************************************
 *                          AVCE00GenEndSection()
 *
 * Generate the last line(s) of an E00 section.
 *
 * This function should be called once with bCont=FALSE to get the
 * first "end of section" line for the current section, and then call 
 * with bCont=TRUE to get all the other lines.
 *
 * The function returns NULL when there are no more lines to generate
 * for this "end of section".
 **********************************************************************/
const char *AVCE00GenEndSection(AVCE00GenInfo *psInfo, AVCFileType eType,
                                GBool bCont)
{
    if (bCont == FALSE)
    {
        /*------------------------------------------------------------- 
         * Most section types end with only 1 line.
         *------------------------------------------------------------*/
        AVCE00GenReset(psInfo);
        psInfo->iCurItem = 0;

        if (eType == AVCFileARC ||
            eType == AVCFilePAL ||
            eType == AVCFileRPL ||
            eType == AVCFileCNT ||
            eType == AVCFileTOL ||
            eType == AVCFileTXT ||
            eType == AVCFileTX6 )
        {
            sprintf(psInfo->pszBuf, 
    "        -1         0         0         0         0         0         0");
        }
        else if (eType == AVCFileLAB)
        {
            if (psInfo->nPrecision == AVC_DOUBLE_PREC)
                sprintf(psInfo->pszBuf, 
          "        -1         0 0.00000000000000E+00 0.00000000000000E+00");
            else
                sprintf(psInfo->pszBuf, 
          "        -1         0 0.0000000E+00 0.0000000E+00");
        }
        else if (eType == AVCFilePRJ)
        {
            sprintf(psInfo->pszBuf, "EOP");
        }
        else if (eType == AVCFileRXP )
        {
            sprintf(psInfo->pszBuf,"        -1         0");
        }
        else
        {
            CPLError(CE_Failure, CPLE_NotSupported,
                     "Unsupported E00 section type!");
            return NULL;
        }
    }
    else if ( psInfo->iCurItem == 0 &&
              psInfo->nPrecision == AVC_DOUBLE_PREC &&
              (eType == AVCFilePAL || eType == AVCFileRPL)     )
    {
        /*--------------------------------------------------------- 
         * Return the 2nd line for the end of a PAL or RPL section.
         *--------------------------------------------------------*/
        sprintf(psInfo->pszBuf, 
                " 0.00000000000000E+00 0.00000000000000E+00");

        psInfo->iCurItem++;
    }
    else
    {
        /*----------------------------------------------------- 
         * All other section types end with only one line, and thus
         * we return NULL when bCont==TRUE
         *----------------------------------------------------*/
        return NULL;
    }

    return psInfo->pszBuf;
}


/**********************************************************************
 *                          AVCE00GenObject()
 *
 * Cover function on top of AVCE00GenArc/Pal/Cnt/Lab() that will
 * call the right function according to argument eType.
 *
 * Since there is no compiler type checking on psObj, you have to
 * be very careful to make sure you pass an object of the right type 
 * when you use this function!
 *
 * The function returns NULL when there are no more lines to generate
 * for this ARC.
 **********************************************************************/
const char *AVCE00GenObject(AVCE00GenInfo *psInfo, 
                            AVCFileType eType, void *psObj, GBool bCont)
{
    const char *pszLine;

    switch(eType)
    {
      case AVCFileARC:
        pszLine = AVCE00GenArc(psInfo, (AVCArc*)psObj, bCont);
        break;
      case AVCFilePAL:
      case AVCFileRPL:
        pszLine = AVCE00GenPal(psInfo, (AVCPal*)psObj, bCont);
        break;
      case AVCFileCNT:
        pszLine = AVCE00GenCnt(psInfo, (AVCCnt*)psObj, bCont);
        break;
      case AVCFileLAB:
        pszLine = AVCE00GenLab(psInfo, (AVCLab*)psObj, bCont);
        break;
      case AVCFileTOL:
        pszLine = AVCE00GenTol(psInfo, (AVCTol*)psObj, bCont);
        break;
      case AVCFileTXT:
        pszLine = AVCE00GenTxt(psInfo, (AVCTxt*)psObj, bCont);
        break;
      case AVCFileTX6:
        pszLine = AVCE00GenTx6(psInfo, (AVCTxt*)psObj, bCont);
        break;
      case AVCFilePRJ:
        pszLine = AVCE00GenPrj(psInfo, (char**)psObj, bCont);
        break;
      case AVCFileRXP:
        pszLine = AVCE00GenRxp(psInfo, (AVCRxp*)psObj, bCont);
        break;
      default:
        CPLError(CE_Failure, CPLE_NotSupported,
                 "AVCE00GenObject(): Unsupported file type!");
    }

    return pszLine;
}


/*=====================================================================
                            ARC stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenArc()
 *
 * Generate the next line of an E00 ARC.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current ARC, and then call with bCont=TRUE
 * to get all the other lines for this ARC.
 *
 * The function returns NULL when there are no more lines to generate
 * for this ARC.
 **********************************************************************/
const char *AVCE00GenArc(AVCE00GenInfo *psInfo, AVCArc *psArc, GBool bCont)
{
    if (bCont == FALSE)
    {
        /* Initialize the psInfo structure with info about the
         * current ARC.
         */
        psInfo->iCurItem = 0;
        if (psInfo->nPrecision == AVC_DOUBLE_PREC)
            psInfo->numItems = psArc->numVertices;
        else
            psInfo->numItems = (psArc->numVertices+1)/2;

        /* And return the ARC header line
         */
        sprintf(psInfo->pszBuf, "%10d%10d%10d%10d%10d%10d%10d",
                psArc->nArcId, psArc->nUserId,
                psArc->nFNode, psArc->nTNode,
                psArc->nLPoly, psArc->nRPoly,
                psArc->numVertices);
    }
    else if (psInfo->iCurItem < psInfo->numItems)
    {
        int iVertex;

        /* return the next set of vertices for the ARC.
         */
        if (psInfo->nPrecision == AVC_DOUBLE_PREC)
        {
            iVertex = psInfo->iCurItem;

            psInfo->pszBuf[0] = '\0';
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileARC,
                            psArc->pasVertices[iVertex].x);
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileARC,
                            psArc->pasVertices[iVertex].y);
        }
        else
        {
            iVertex = psInfo->iCurItem*2;

            psInfo->pszBuf[0] = '\0';
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileARC,
                            psArc->pasVertices[iVertex].x);
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileARC,
                            psArc->pasVertices[iVertex].y);

            /* Check because if we have a odd number of vertices then
             * the last line contains only one pair of vertices.
             */
            if (iVertex+1 < psArc->numVertices)
            {
                _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileARC,
                                psArc->pasVertices[iVertex+1].x);
                _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileARC,
                                psArc->pasVertices[iVertex+1].y);
            }
        }
        psInfo->iCurItem++;
    }
    else
    {
        /* No more lines to generate for this ARC.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}

/*=====================================================================
                            PAL stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenPal()
 *
 * Generate the next line of an E00 PAL (Polygon Arc List) entry.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current PAL, and then call with bCont=TRUE
 * to get all the other lines for this PAL.
 *
 * The function returns NULL when there are no more lines to generate
 * for this PAL entry.
 **********************************************************************/
const char *AVCE00GenPal(AVCE00GenInfo *psInfo, AVCPal *psPal, GBool bCont)
{
    if (bCont == FALSE)
    {
        /* Initialize the psInfo structure with info about the
         * current PAL.  (Number of lines excluding header)
         */
        psInfo->numItems = (psPal->numArcs+1)/2;


        /* And return the PAL header line.
         */
        sprintf(psInfo->pszBuf, "%10d", psPal->numArcs);

        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFilePAL,
                        psPal->sMin.x);
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFilePAL,
                        psPal->sMin.y);

        /* Double precision PAL entries have their header on 2 lines!
         */
        if (psInfo->nPrecision == AVC_DOUBLE_PREC)
        {
            psInfo->iCurItem = -1;      /* Means 1 line left in header */
        }
        else
        {
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFilePAL,
                            psPal->sMax.x);
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFilePAL,
                            psPal->sMax.y);
            psInfo->iCurItem = 0;       /* Next thing = first Arc entry */
        }

    }
    else if (psInfo->iCurItem == -1)
    {
        /* Second (and last) header line for double precision coverages
         */
        psInfo->pszBuf[0] = '\0';
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFilePAL,
                        psPal->sMax.x);
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFilePAL,
                        psPal->sMax.y);

        psInfo->iCurItem = 0;       /* Next thing = first Arc entry */
    }
    else if (psInfo->iCurItem < psInfo->numItems)
    {
        /* Return PAL Arc entries...
         */
        int iArc;

        iArc = psInfo->iCurItem*2;

        /* If we have a odd number of arcs then
         * the last line contains only one arc entry.
         */
        if (iArc+1 < psPal->numArcs)
        {
            sprintf(psInfo->pszBuf, "%10d%10d%10d%10d%10d%10d",
                                    psPal->pasArcs[iArc].nArcId,
                                    psPal->pasArcs[iArc].nFNode,
                                    psPal->pasArcs[iArc].nAdjPoly,
                                    psPal->pasArcs[iArc+1].nArcId,
                                    psPal->pasArcs[iArc+1].nFNode,
                                    psPal->pasArcs[iArc+1].nAdjPoly);

        }
        else
        {
            sprintf(psInfo->pszBuf, "%10d%10d%10d", 
                                    psPal->pasArcs[iArc].nArcId,
                                    psPal->pasArcs[iArc].nFNode,
                                    psPal->pasArcs[iArc].nAdjPoly);
        }
        psInfo->iCurItem++;
    }
    else
    {
        /* No more lines to generate for this PAL.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}


/*=====================================================================
                            CNT stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenCnt()
 *
 * Generate the next line of an E00 CNT (Polygon Centroid) entry.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current CNT, and then call with bCont=TRUE
 * to get all the other lines for this CNT.
 *
 * The function returns NULL when there are no more lines to generate
 * for this CNT entry.
 **********************************************************************/
const char *AVCE00GenCnt(AVCE00GenInfo *psInfo, AVCCnt *psCnt, GBool bCont)
{
    if (bCont == FALSE)
    {
        /* Initialize the psInfo structure with info about the
         * current CNT.
         */
        psInfo->iCurItem = 0;
        psInfo->numItems = (psCnt->numLabels+7)/8;

        /* And return the CNT header line.
         */
        sprintf(psInfo->pszBuf, "%10d", psCnt->numLabels);

        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileCNT,
                        psCnt->sCoord.x);
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileCNT,
                        psCnt->sCoord.y);

    }
    else if (psInfo->iCurItem < psInfo->numItems)
    {
        /* Return CNT Label Ids, 8 label Ids per line... 
         */
        int i, nFirstLabel, numLabels;

        nFirstLabel = psInfo->iCurItem * 8;
        numLabels = MIN(8, (psCnt->numLabels-nFirstLabel));

        psInfo->pszBuf[0] = '\0';
        for(i=0; i < numLabels; i++)
        {
            sprintf(psInfo->pszBuf + strlen(psInfo->pszBuf), "%10d", 
                                        psCnt->panLabelIds[nFirstLabel+i] );
        }

        psInfo->iCurItem++;
    }
    else
    {
        /* No more lines to generate for this CNT.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}


/*=====================================================================
                            LAB stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenLab()
 *
 * Generate the next line of an E00 LAB (Label) entry.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current LAB, and then call with bCont=TRUE
 * to get all the other lines for this LAB.
 *
 * The function returns NULL when there are no more lines to generate
 * for this LAB entry.
 **********************************************************************/
const char *AVCE00GenLab(AVCE00GenInfo *psInfo, AVCLab *psLab, GBool bCont)
{
    if (bCont == FALSE)
    {
        /* Initialize the psInfo structure with info about the
         * current LAB. (numItems = Number of lines excluding header)
         */
        psInfo->iCurItem = 0;
        if (psInfo->nPrecision == AVC_DOUBLE_PREC)
            psInfo->numItems = 2;
        else
            psInfo->numItems = 1;

        /* And return the LAB header line.
         */
        sprintf(psInfo->pszBuf, "%10d%10d", psLab->nValue, psLab->nPolyId);

        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                        psLab->sCoord1.x);
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                        psLab->sCoord1.y);

    }
    else if (psInfo->iCurItem < psInfo->numItems)
    {
        /* Return next Label coordinates... 
         */
        if (psInfo->nPrecision != AVC_DOUBLE_PREC)
        {
            /* Single precision, all on the same line
             */
            psInfo->pszBuf[0] = '\0';
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                            psLab->sCoord2.x);
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                            psLab->sCoord2.y);
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                            psLab->sCoord3.x);
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                            psLab->sCoord3.y);

        }
        else if (psInfo->iCurItem == 0)
        {
            /* 2nd line, in a double precision coverage
             */
            psInfo->pszBuf[0] = '\0';
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                            psLab->sCoord2.x);
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                            psLab->sCoord2.y);
        }
        else
        {
            /* 3rd line, in a double precision coverage
             */
            psInfo->pszBuf[0] = '\0';
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                            psLab->sCoord3.x);
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileLAB,
                            psLab->sCoord3.y);
        }

        psInfo->iCurItem++;
    }
    else
    {
        /* No more lines to generate for this LAB.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}

/*=====================================================================
                            TOL stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenTol()
 *
 * Generate the next line of an E00 TOL (Tolerance) entry.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current TOL, and then call with bCont=TRUE
 * to get all the other lines for this TOL.
 *
 * The function returns NULL when there are no more lines to generate
 * for this TOL entry.
 **********************************************************************/
const char *AVCE00GenTol(AVCE00GenInfo *psInfo, AVCTol *psTol, GBool bCont)
{
    if (bCont == TRUE)
    {
        /*--------------------------------------------------------- 
         * TOL entries are only 1 line, we support the bCont flag
         * only for compatibility with the other AVCE00Gen*() functions.
         *--------------------------------------------------------*/
        return NULL;
    }

    sprintf(psInfo->pszBuf, "%10d%10d", psTol->nIndex, psTol->nFlag);
    _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileTOL,
                    psTol->dValue);

    return psInfo->pszBuf;
}

/*=====================================================================
                            PRJ stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenPrj()
 *
 * Generate the next line of an E00 PRJ (Projection) section.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current PRJ, and then call with bCont=TRUE
 * to get all the other lines for this PRJ.
 *
 * The function returns NULL when there are no more lines to generate
 * for this PRJ entry.
 **********************************************************************/
const char *AVCE00GenPrj(AVCE00GenInfo *psInfo, char **papszPrj, GBool bCont)
{
    if (bCont == FALSE)
    {
        /*--------------------------------------------------------- 
         * Initialize the psInfo structure with info about the
         * current PRJ. (numItems = Number of lines to output)
         *--------------------------------------------------------*/
        psInfo->iCurItem = 0;
        psInfo->numItems = CSLCount(papszPrj) * 2;
    }

    if (psInfo->iCurItem < psInfo->numItems)
    {
        /*--------------------------------------------------------- 
         * Return the next PRJ section line.  Note that every
         * second line of the output is only a "~".
         *--------------------------------------------------------*/

        if (psInfo->iCurItem % 2 == 0)
        {
            /*-----------------------------------------------------
             * In theory we should split lines longer than 80 chars on
             * several lines, but I won't do it for now since I never
             * saw any projection line longer than 80 chars.
             *----------------------------------------------------*/
            sprintf(psInfo->pszBuf, "%s", papszPrj[psInfo->iCurItem/2]);
        }
        else
        {
            /*-----------------------------------------------------
             * Every second line in a PRJ section contains only a "~",
             * this is a way to tell that the previous line was complete.
             *----------------------------------------------------*/
            sprintf(psInfo->pszBuf, "~");
        }

        psInfo->iCurItem++;
    }
    else
    {
        /* No more lines to generate for this PRJ.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}


/*=====================================================================
                            TXT stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenTxt()
 *
 * Generate the next line of an E00 TXT (Annotation) entry.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current TXT, and then call with bCont=TRUE
 * to get all the other lines for this TXT.
 *
 * The function returns NULL when there are no more lines to generate
 * for this TXT entry.
 **********************************************************************/
const char *AVCE00GenTxt(AVCE00GenInfo *psInfo, AVCTxt *psTxt, GBool bCont)
{
    if (bCont == FALSE)
    {
        /*-------------------------------------------------------------
         * Initialize the psInfo structure with info about the
         * current TXT. (numItems = Number of lines excluding header)
         *------------------------------------------------------------*/
        psInfo->iCurItem = 0;
        if (psInfo->nPrecision == AVC_DOUBLE_PREC)
            psInfo->numItems = 7;
        else
            psInfo->numItems = 5;

        /* And return the TXT header line.
         */
        sprintf(psInfo->pszBuf, "%10d%10d%10d%10d%10d", 
                psTxt->nLevel, psTxt->numVerticesLine - 1, 
                psTxt->numVerticesArrow, psTxt->nSymbol, psTxt->numChars);
    }
    else if (psInfo->iCurItem < psInfo->numItems-2)
    {
        /*-------------------------------------------------------------
         * Return next line of coordinates... start by placing the coord. 
         * values in the order that they should appear, and then generate the 
         * current line
         * (This is a little bit less efficient, but will give much easier
         *  code to read ;-)
         *------------------------------------------------------------*/
        double  dXY[15];
        int     i, nFirstValue, numValuesPerLine;
        for(i=0; i<14; i++)
            dXY[i] = 0.0;

        dXY[14] = psTxt->dHeight;

        /* note that the first vertex in the vertices list is never exported
         */
        for(i=0; i < 4 && i< (psTxt->numVerticesLine-1); i++)
        {
            dXY[i] = psTxt->pasVertices[i+1].x;
            dXY[i+4] = psTxt->pasVertices[i+1].y;
        }
        for(i=0; i < 3 && i<ABS(psTxt->numVerticesArrow); i++)
        {
            dXY[i+8] = psTxt->pasVertices[i+psTxt->numVerticesLine].x;
            dXY[i+11] = psTxt->pasVertices[i+psTxt->numVerticesLine].y;
        }

        /* OK, now that we prepared the coord. values, return the next line
         * of coordinates.  The only difference between double and single
         * precision is the number of coordinates per line.
         */
        if (psInfo->nPrecision != AVC_DOUBLE_PREC)
        {
            /* Single precision
             */
            numValuesPerLine = 5;
        }
        else
        {
            /* Double precision
             */
            numValuesPerLine = 3;
        }       
     
        nFirstValue = psInfo->iCurItem*numValuesPerLine; 
        psInfo->pszBuf[0] = '\0';
        for(i=0; i<numValuesPerLine; i++)
        {
            _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileTXT,
                            dXY[nFirstValue+i] );
        }

        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem == psInfo->numItems-2)
    {
        /*-------------------------------------------------------------
         * Line with a -1.000E+02 value, ALWAYS SINGLE PRECISION !!!
         *------------------------------------------------------------*/
        psInfo->pszBuf[0] = '\0';
        _PrintRealValue(psInfo->pszBuf, AVC_SINGLE_PREC, AVCFileTXT,
                        psTxt->f_1e2 );
        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem == psInfo->numItems-1)
    {
        /*-------------------------------------------------------------
         * Last line, contains the text string
         *------------------------------------------------------------*/
        sprintf(psInfo->pszBuf, "%s", psTxt->pszText);
        psInfo->iCurItem++;
    }
    else
    {
        /* No more lines to generate for this TXT.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}

/*=====================================================================
                            TX6 stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenTx6()
 *
 * Generate the next line of an E00 TX6 (Annotation) entry.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current TX6, and then call with bCont=TRUE
 * to get all the other lines for this TX6.
 *
 * Note that E00 files can also contain TX7 sections, they seem identical
 * to TX6 sections, except for one value in each entry, and it was
 * impossible to find where this value comes from... so we will always
 * generate TX6 sections and not bother with TX7.
 *
 * The function returns NULL when there are no more lines to generate
 * for this TX6 entry.
 **********************************************************************/
const char *AVCE00GenTx6(AVCE00GenInfo *psInfo, AVCTxt *psTxt, GBool bCont)
{
    if (bCont == FALSE)
    {
        /*-------------------------------------------------------------
         * Initialize the psInfo structure with info about the
         * current TX6. (numItems = Number of lines excluding header)
         *------------------------------------------------------------*/
        psInfo->iCurItem = 0;
        psInfo->numItems = 9 + psTxt->numVerticesLine + 
                               ABS(psTxt->numVerticesArrow);

        /* And return the TX6 header line.
         */
        sprintf(psInfo->pszBuf, "%10d%10d%10d%10d%10d%10d%10d", 
                psTxt->nUserId, psTxt->nLevel, psTxt->numVerticesLine, 
                psTxt->numVerticesArrow, psTxt->nSymbol, psTxt->n28,
                psTxt->numChars);
    }
    else if (psInfo->iCurItem < psInfo->numItems && 
             psInfo->iCurItem < 6)
    {
        /*-------------------------------------------------------------
         * Text Justification stuff... 2 sets of 20 int16 values.
         *------------------------------------------------------------*/
        GInt16  *pValue;

        if (psInfo->iCurItem < 3)
            pValue = psTxt->anJust2 + psInfo->iCurItem * 7;
        else
            pValue = psTxt->anJust1 + (psInfo->iCurItem-3) * 7;

        if (psInfo->iCurItem == 2 || psInfo->iCurItem == 5)
        {
            sprintf(psInfo->pszBuf, "%10d%10d%10d%10d%10d%10d",
                                pValue[0], pValue[1], pValue[2], 
                                pValue[3], pValue[4], pValue[5]);
        }
        else
        {
            sprintf(psInfo->pszBuf, "%10d%10d%10d%10d%10d%10d%10d",
                                pValue[0], pValue[1], pValue[2], 
                                pValue[3], pValue[4], pValue[5], pValue[6]);
        }

        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem < psInfo->numItems && 
             psInfo->iCurItem == 6)
    {
        /*-------------------------------------------------------------
         * Line with a -1.000E+02 value, ALWAYS SINGLE PRECISION !!!
         *------------------------------------------------------------*/
        psInfo->pszBuf[0] = '\0';
        _PrintRealValue(psInfo->pszBuf, AVC_SINGLE_PREC, AVCFileTX6,
                        psTxt->f_1e2 );
        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem < psInfo->numItems && 
             psInfo->iCurItem == 7)
    {
        /*-------------------------------------------------------------
         * Line with 3 values, 1st value is probably text height.
         *------------------------------------------------------------*/
        psInfo->pszBuf[0] = '\0';
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileTX6,
                        psTxt->dHeight );
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileTX6,
                        psTxt->dV2 );
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileTX6,
                        psTxt->dV3 );
        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem < psInfo->numItems-1)
    {
        /*-------------------------------------------------------------
         * One line for each pair of X,Y coordinates
         *------------------------------------------------------------*/
        psInfo->pszBuf[0] = '\0';

        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileTX6,
                        psTxt->pasVertices[ psInfo->iCurItem-8 ].x );
        _PrintRealValue(psInfo->pszBuf, psInfo->nPrecision, AVCFileTX6,
                        psTxt->pasVertices[ psInfo->iCurItem-8 ].y );

        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem == psInfo->numItems-1)
    {
        /*-------------------------------------------------------------
         * Last line, contains the text string
         *------------------------------------------------------------*/
        sprintf(psInfo->pszBuf, "%s", psTxt->pszText);
        psInfo->iCurItem++;
    }
    else
    {
        /* No more lines to generate for this TX6.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}


/*=====================================================================
                            RXP stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenRxp()
 *
 * Generate the next line of an E00 RXP entry (RXPs are related to regions).
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current RXP, and then call with bCont=TRUE
 * to get all the other lines for this RXP.
 *
 * The function returns NULL when there are no more lines to generate
 * for this RXP entry.
 **********************************************************************/
const char *AVCE00GenRxp(AVCE00GenInfo *psInfo, AVCRxp *psRxp, GBool bCont)
{
    if (bCont == TRUE)
    {
        /*--------------------------------------------------------- 
         * RXP entries are only 1 line, we support the bCont flag
         * only for compatibility with the other AVCE00Gen*() functions.
         *--------------------------------------------------------*/
        return NULL;
    }

    sprintf(psInfo->pszBuf, "%10d%10d", psRxp->n1, psRxp->n2);

    return psInfo->pszBuf;
}



/*=====================================================================
                            TABLE stuff
 =====================================================================*/

/**********************************************************************
 *                          AVCE00GenTableHdr()
 *
 * Generate the next line of an E00 Table header.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current table header, and then call with 
 * bCont=TRUE to get all the other lines.
 *
 * The function returns NULL when there are no more lines to generate.
 **********************************************************************/
const char *AVCE00GenTableHdr(AVCE00GenInfo *psInfo, AVCTableDef *psDef,
                              GBool bCont)
{
    if (bCont == FALSE)
    {
        /* Initialize the psInfo structure with info about the
         * current Table Header
         */
        psInfo->iCurItem = 0;
        psInfo->numItems = psDef->numFields;

        /* And return the header's header line(!).
         */
        sprintf(psInfo->pszBuf, "%-32.32s%s%4d%4d%4d%10d", 
                                            psDef->szTableName,
                                            psDef->szExternal,
                                            psDef->numFields,
                                            psDef->numFields,
                                            psDef->nRecSize,
                                            psDef->numRecords);
    }
    else if (psInfo->iCurItem < psInfo->numItems)
    {
        /* Return next Field definition line
         */
        sprintf(psInfo->pszBuf,
                "%-16.16s%3d%2d%4d%1d%2d%4d%2d%2d%1d%2d%4d%4d%2d%-16.16s%4d-",
                psDef->pasFieldDef[psInfo->iCurItem].szName,
                psDef->pasFieldDef[psInfo->iCurItem].nSize,
                psDef->pasFieldDef[psInfo->iCurItem].v2,
                psDef->pasFieldDef[psInfo->iCurItem].nOffset,
                psDef->pasFieldDef[psInfo->iCurItem].v4,
                psDef->pasFieldDef[psInfo->iCurItem].v5,
                psDef->pasFieldDef[psInfo->iCurItem].nFmtWidth,
                psDef->pasFieldDef[psInfo->iCurItem].nFmtPrec,
                psDef->pasFieldDef[psInfo->iCurItem].nType1,
                psDef->pasFieldDef[psInfo->iCurItem].nType2,
                psDef->pasFieldDef[psInfo->iCurItem].v10,
                psDef->pasFieldDef[psInfo->iCurItem].v11,
                psDef->pasFieldDef[psInfo->iCurItem].v12,
                psDef->pasFieldDef[psInfo->iCurItem].v13,
                psDef->pasFieldDef[psInfo->iCurItem].szAltName,
                psDef->pasFieldDef[psInfo->iCurItem].nIndex );


        psInfo->iCurItem++;
    }
    else
    {
        /* No more lines to generate.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}

/**********************************************************************
 *                          AVCE00GenTableRec()
 *
 * Generate the next line of an E00 Table Data Record.
 *
 * This function should be called once with bCont=FALSE to get the
 * first E00 line for the current table record, and then call with 
 * bCont=TRUE to get all the other lines.
 *
 * The function returns NULL when there are no more lines to generate.
 **********************************************************************/
const char *AVCE00GenTableRec(AVCE00GenInfo *psInfo, int numFields,
                              AVCFieldInfo *pasDef, AVCField *pasFields,
                              GBool bCont)
{
    int     i, nSize, nType, nLen;
    char    *pszBuf2;

    if (bCont == FALSE)
    {
        /*------------------------------------------------------------- 
         * Initialize the psInfo structure to be ready to process this
         * new Table Record
         *------------------------------------------------------------*/
        psInfo->iCurItem = 0;
        psInfo->numItems = _AVCE00ComputeRecSize(numFields, pasDef);

        /*------------------------------------------------------------- 
         * First, we need to make sure that the output buffer is big 
         * enough to hold the whole record, plus 81 chars to hold
         * the line that we'll return to the caller.
         *------------------------------------------------------------*/
        nSize = psInfo->numItems + 1 + 81;

        if (psInfo->nBufSize < nSize)
        {
            psInfo->pszBuf = (char*)CPLRealloc(psInfo->pszBuf,
                                               nSize*sizeof(char));
            psInfo->nBufSize = nSize;
        }

        /*------------------------------------------------------------- 
         * Generate the whole record now, and we'll return it to the
         * caller by chunks of 80 chars.
         * The first 80 chars of the buffer will be used to return 
         * one line at a time, and the rest of the buffer is used to
         * hold the whole record.
         *------------------------------------------------------------*/
        pszBuf2 = psInfo->pszBuf+81;

        for(i=0; i<numFields; i++)
        {
            nType = pasDef[i].nType1*10;
            nSize = pasDef[i].nSize;

            if (nType ==  AVC_FT_DATE || nType == AVC_FT_CHAR ||
                nType == AVC_FT_FIXINT )
            {
                strncpy(pszBuf2, pasFields[i].pszStr, nSize);
                pszBuf2 += nSize;
            }
            else if (nType == AVC_FT_FIXNUM)
            {
                /* TYPE 40 attributes are stored with 1 byte per digit
                 * in binary format, and as single precision floats in
                 * E00 tables, even in double precision coverages.
                 */
                pszBuf2[0] = '\0';
                nLen = _PrintRealValue(pszBuf2, AVC_SINGLE_PREC, 
                                       AVCFileTABLE,
                                       atof(pasFields[i].pszStr));
                pszBuf2 += nLen;
            }
            else if (nType == AVC_FT_BININT && nSize == 4)
            {
                sprintf(pszBuf2, "%11d", pasFields[i].nInt32);
                pszBuf2 += 11;
            }
            else if (nType == AVC_FT_BININT && nSize == 2)
            {
                sprintf(pszBuf2, "%6d", pasFields[i].nInt16);
                pszBuf2 += 6;
            }
            else if (nType == AVC_FT_BINFLOAT && pasDef[i].nSize == 4)
            {
                pszBuf2[0] = '\0';
                /* NOTE: The E00 representation for a binary float is
                 * defined by its binary size, not by the coverage's
                 * precision.
                 */
                nLen = _PrintRealValue(pszBuf2, AVC_SINGLE_PREC, 
                                       AVCFileTABLE,
                                       pasFields[i].fFloat);
                pszBuf2 += nLen;
            }
            else if (nType == AVC_FT_BINFLOAT && pasDef[i].nSize == 8)
            {
                pszBuf2[0] = '\0';
                /* NOTE: The E00 representation for a binary float is
                 * defined by its binary size, not by the coverage's
                 * precision.
                 */
                nLen = _PrintRealValue(pszBuf2, AVC_DOUBLE_PREC,
                                       AVCFileTABLE,
                                       pasFields[i].dDouble);
                pszBuf2 += nLen;
            }
            else
            {
                /*-----------------------------------------------------
                 * Hummm... unsupported field type...
                 *----------------------------------------------------*/
                CPLError(CE_Failure, CPLE_NotSupported,
                         "Unsupported field type: (type=%d, size=%d)",
                         nType, pasDef[i].nSize);
                return NULL;
            }
        }

        *pszBuf2 = '\0';
    }

    if (psInfo->iCurItem < psInfo->numItems)
    {
        /*------------------------------------------------------------- 
         * Return the next 80 chars chunk.
         * The first 80 chars of the buffer is used to return 
         * one line at a time, and the rest of the buffer (chars 81+)
         * is used to hold the whole record.
         *------------------------------------------------------------*/
        nLen = psInfo->numItems - psInfo->iCurItem;

        if (nLen > 80)
            nLen = 80;

        strncpy(psInfo->pszBuf, psInfo->pszBuf+(81+psInfo->iCurItem), nLen);
        psInfo->pszBuf[nLen] = '\0';

        psInfo->iCurItem += nLen;

        /*------------------------------------------------------------- 
         * Arc/Info removes spaces at the end of the lines... let's 
         * remove them as well since it can reduce the E00 file size.
         *------------------------------------------------------------*/
        nLen--;
        while(nLen >= 0 && psInfo->pszBuf[nLen] == ' ')
        {
            psInfo->pszBuf[nLen] = '\0';
            nLen--;
        }
    }
    else
    {
        /* No more lines to generate.
         */
        return NULL;
    }

    return psInfo->pszBuf;
}

