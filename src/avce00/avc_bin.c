/**********************************************************************
 * $Id: avc_bin.c,v 1.2 2005/02/17 13:03:40 vir Exp $
 *
 * Name:     avc_bin.c
 * Project:  Arc/Info vector coverage (AVC)  BIN->E00 conversion library
 * Language: ANSI C
 * Purpose:  Binary files access functions.
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
 * $Log: avc_bin.c,v $
 * Revision 1.2  2005/02/17 13:03:40  vir
 * Fixed a bug that wrongly converted E00 files into binary coverages when
 * optimization was used. It also procuded a few warnings, that do not appear
 * any more.
 *
 * Revision 1.1.1.1  2001/06/27 20:10:54  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:11  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.6  1999/08/23 18:17:16  daniel
 * Modified AVCBinReadListTables() to return INFO fnames for DeleteCoverage()
 *
 * Revision 1.5  1999/05/11 01:49:08  daniel
 * Simple changes required by addition of coverage write support
 *
 * Revision 1.4  1999/03/03 18:42:53  daniel
 * Fixed problem with INFO table headers (arc.dir) that sometimes contain an
 * invalid number of records.
 *
 * Revision 1.3  1999/02/25 17:01:53  daniel
 * Added support for 16 bit integers in INFO tables (type=50, size=2)
 *
 * Revision 1.2  1999/02/25 03:41:28  daniel
 * Added TXT, TX6/TX7, RXP and RPL support
 *
 * Revision 1.1  1999/01/29 16:28:52  daniel
 * Initial revision
 *
 **********************************************************************/

#include "avc.h"

#include <ctype.h>      /* for isspace() */

/*=====================================================================
 * Stuff related to reading the binary coverage files
 *====================================================================*/

AVCBinFile *_AVCBinReadOpenTable(const char *pszInfoPath,
                                 const char *pszTableName);
AVCBinFile *_AVCBinReadOpenPrj(const char *pszPath, const char *pszName);

/**********************************************************************
 *                          AVCBinReadOpen()
 *
 * Open a coverage file for reading, read the file header if applicable,
 * and initialize a temp. storage structure to be ready to read objects
 * from the file.
 *
 * pszPath is the coverage (or info directory) path, terminated by
 *         a '/' or a '\\'
 * pszName is the name of the file to open relative to this directory.
 *
 * Note: For most file types except tables, passing pszPath="" and 
 * including the coverage path as part of pszName instead would work.
 *
 * Returns a valid AVCBinFile handle, or NULL if the file could
 * not be opened.
 *
 * AVCBinClose() will eventually have to be called to release the 
 * resources used by the AVCBinFile structure.
 **********************************************************************/
AVCBinFile *AVCBinReadOpen(const char *pszPath, const char *pszName, 
                           AVCFileType eType)
{
    AVCBinFile   *psFile;

    /*-----------------------------------------------------------------
     * The case of INFO tables is a bit more complicated...
     * pass the control to a separate function.
     *----------------------------------------------------------------*/
    if (eType == AVCFileTABLE)
    {
        return _AVCBinReadOpenTable(pszPath, pszName);
    }

    /*-----------------------------------------------------------------
     * PRJ files are text files... we won't use the AVCRawBin*()
     * functions for them...
     *----------------------------------------------------------------*/
    if (eType == AVCFilePRJ)
    {
        return _AVCBinReadOpenPrj(pszPath, pszName);
    }

    /*-----------------------------------------------------------------
     * All other file types share a very similar opening method.
     *----------------------------------------------------------------*/
    psFile = (AVCBinFile*)CPLCalloc(1, sizeof(AVCBinFile));

    psFile->eFileType = eType;

    psFile->pszFilename = (char*)CPLMalloc((strlen(pszPath)+strlen(pszName)+1)*
                                           sizeof(char));
    sprintf(psFile->pszFilename, "%s%s", pszPath, pszName);

    psFile->psRawBinFile = AVCRawBinOpen(psFile->pszFilename, "r");

    if (psFile->psRawBinFile == NULL)
    {
        /* Failed to open file... just return NULL since an error message
         * has already been issued by AVCRawBinOpen()
         */
        CPLFree(psFile->pszFilename);
        CPLFree(psFile);
        return NULL;
    }

    /*-----------------------------------------------------------------
     * Read the header, and set the precision field if applicable
     *----------------------------------------------------------------*/
    AVCBinReadRewind(psFile);

    /*-----------------------------------------------------------------
     * Allocate a temp. structure to use to read objects from the file
     * (Using Calloc() will automatically initialize the struct contents
     *  to NULL... this is very important for ARCs and PALs)
     *----------------------------------------------------------------*/
    if (psFile->eFileType == AVCFileARC)
    {
        psFile->cur.psArc = (AVCArc*)CPLCalloc(1, sizeof(AVCArc));
    }
    else if (psFile->eFileType == AVCFilePAL ||
             psFile->eFileType == AVCFileRPL )
    {
        psFile->cur.psPal = (AVCPal*)CPLCalloc(1, sizeof(AVCPal));
    }
    else if (psFile->eFileType == AVCFileCNT)
    {
        psFile->cur.psCnt = (AVCCnt*)CPLCalloc(1, sizeof(AVCCnt));
    }
    else if (psFile->eFileType == AVCFileLAB)
    {
        psFile->cur.psLab = (AVCLab*)CPLCalloc(1, sizeof(AVCLab));
    }
    else if (psFile->eFileType == AVCFileTOL)
    {
        psFile->cur.psTol = (AVCTol*)CPLCalloc(1, sizeof(AVCTol));
    }
    else if (psFile->eFileType == AVCFileTXT ||
             psFile->eFileType == AVCFileTX6)
    {
        psFile->cur.psTxt = (AVCTxt*)CPLCalloc(1, sizeof(AVCTxt));
    }
    else if (psFile->eFileType == AVCFileRXP)
    {
        psFile->cur.psRxp = (AVCRxp*)CPLCalloc(1, sizeof(AVCRxp));
    }
    else
    {
        CPLError(CE_Failure, CPLE_IllegalArg,
                 "Unsupported file type or invalid file handle!");
    }

    return psFile;
}

/**********************************************************************
 *                          AVCBinReadClose()
 *
 * Close a coverage file, and release all memory (object strcut., buffers,
 * etc.) associated with this file.
 **********************************************************************/
void    AVCBinReadClose(AVCBinFile *psFile)
{

    AVCRawBinClose(psFile->psRawBinFile);
    psFile->psRawBinFile = NULL;

    CPLFree(psFile->pszFilename);
    psFile->pszFilename = NULL;

    if (psFile->eFileType == AVCFileARC)
    {
        CPLFree(psFile->cur.psArc->pasVertices);
        CPLFree(psFile->cur.psArc);
    }
    else if (psFile->eFileType == AVCFilePAL ||
             psFile->eFileType == AVCFileRPL )
    {
        CPLFree(psFile->cur.psPal->pasArcs);
        CPLFree(psFile->cur.psPal);
    }
    else if (psFile->eFileType == AVCFileCNT)
    {
        CPLFree(psFile->cur.psCnt->panLabelIds);
        CPLFree(psFile->cur.psCnt);
    }
    else if (psFile->eFileType == AVCFileLAB)
    {
        CPLFree(psFile->cur.psLab);
    }
    else if (psFile->eFileType == AVCFileTOL)
    {
        CPLFree(psFile->cur.psTol);
    }
    else if (psFile->eFileType == AVCFilePRJ)
    {
        CSLDestroy(psFile->cur.papszPrj);
    }
    else if (psFile->eFileType == AVCFileTXT || 
             psFile->eFileType == AVCFileTX6)
    {
        CPLFree(psFile->cur.psTxt->pasVertices);
        CPLFree(psFile->cur.psTxt->pszText);
        CPLFree(psFile->cur.psTxt);
    }
    else if (psFile->eFileType == AVCFileRXP)
    {
        CPLFree(psFile->cur.psRxp);
    }
    else if (psFile->eFileType == AVCFileTABLE)
    {
        _AVCDestroyTableFields(psFile->hdr.psTableDef, psFile->cur.pasFields);
        _AVCDestroyTableDef(psFile->hdr.psTableDef);
    }
    else
    {
        CPLError(CE_Failure, CPLE_IllegalArg,
                 "Unsupported file type or invalid file handle!");
    }

    CPLFree(psFile);
}

/**********************************************************************
 *                          _AVCBinReadHeader()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadRewind() instead)
 *
 * Read the first 100 bytes header of the file and fill the AVCHeader
 * structure.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadHeader(AVCRawBinFile *psFile, AVCBinHeader *psHeader)
{
    int nStatus = 0;

    AVCRawBinFSeek(psFile, 0, SEEK_SET);

    psHeader->nSignature = AVCRawBinReadInt32(psFile);

    if (AVCRawBinEOF(psFile))
        nStatus = -1;

    psHeader->nPrecision = AVCRawBinReadInt32(psFile);
    psHeader->nRecordSize= AVCRawBinReadInt32(psFile);
    AVCRawBinFSeek(psFile, 24, SEEK_SET);
    psHeader->nLength    = AVCRawBinReadInt32(psFile);

    /* Move the pointer at the end of the header
     */
    AVCRawBinFSeek(psFile, 100, SEEK_SET);

    return nStatus;
}


/**********************************************************************
 *                          AVCBinReadRewind()
 *
 * Rewind the read pointer, and read/skip the header if necessary so
 * that we are ready to read the data objects from the file after
 * this call.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int AVCBinReadRewind(AVCBinFile *psFile)
{
    AVCBinHeader sHeader;
    int          nStatus=0;

    AVCRawBinFSeek(psFile->psRawBinFile, 0, SEEK_SET);

    if ( psFile->eFileType == AVCFileARC ||
              psFile->eFileType == AVCFilePAL ||
              psFile->eFileType == AVCFileRPL ||
              psFile->eFileType == AVCFileCNT ||
              psFile->eFileType == AVCFileLAB ||
              psFile->eFileType == AVCFileTXT ||
              psFile->eFileType == AVCFileTX6  )
    {   
        nStatus = _AVCBinReadHeader(psFile->psRawBinFile, &sHeader);

        /* Store the precision information inside the file handle.
         */
        if (sHeader.nPrecision < 0)
            psFile->nPrecision = AVC_DOUBLE_PREC;
        else
            psFile->nPrecision = AVC_SINGLE_PREC;
    }
    else if (psFile->eFileType == AVCFileTOL)
    {
        /*-------------------------------------------------------------
         * For some reason, the tolerance files do not follow the 
         * general rules!
         * Single precision "tol.adf" have no header
         * Double precision "par.adf" have the usual 100 bytes header,
         *  but the 3rd field, which usually defines the precision has
         *  a positive value, even if the file is double precision!
         *------------------------------------------------------------*/
        int nSignature = 0;
        nSignature = AVCRawBinReadInt32(psFile->psRawBinFile);

        AVCRawBinFSeek(psFile->psRawBinFile, 0, SEEK_SET);

        if (nSignature == 9993)
        {
            /* We have a double precision par.adf... read the 100 bytes 
             * header and set the precision information inside the file 
             * handle.
             */
            nStatus = _AVCBinReadHeader(psFile->psRawBinFile, &sHeader);

            psFile->nPrecision = AVC_DOUBLE_PREC;
        }
        else
        {
            /* It's a single precision tol.adf ... just set the 
             * precision field.
             */
            psFile->nPrecision = AVC_SINGLE_PREC;
        }
    }

    return nStatus;
}


/**********************************************************************
 *                          AVCBinReadNextObject()
 *
 * Read the next structure from the file.  This function is just a generic
 * cover on top of the AVCBinReadNextArc/Lab/Pal/Cnt() functions.
 *
 * Returns a (void*) to a static structure with the contents of the object
 * that was read.  The contents of the structure will be valid only until
 * the next call.  
 * If you use the returned value, then make sure that you cast it to
 * the right type for the current file! (AVCArc, AVCPal, AVCCnt, ...)
 *
 * Returns NULL if an error happened or if EOF was reached.  
 **********************************************************************/
void *AVCBinReadNextObject(AVCBinFile *psFile)
{
    void *psObj = NULL;

    switch(psFile->eFileType)
    {
      case AVCFileARC:
        psObj = (void*)AVCBinReadNextArc(psFile);
        break;
      case AVCFilePAL:
      case AVCFileRPL:
        psObj = (void*)AVCBinReadNextPal(psFile);
        break;
      case AVCFileCNT:
        psObj = (void*)AVCBinReadNextCnt(psFile);
        break;
      case AVCFileLAB:
        psObj = (void*)AVCBinReadNextLab(psFile);
        break;
      case AVCFileTOL:
        psObj = (void*)AVCBinReadNextTol(psFile);
        break;
      case AVCFileTXT:
      case AVCFileTX6:
        psObj = (void*)AVCBinReadNextTxt(psFile);
        break;
      case AVCFileRXP:
        psObj = (void*)AVCBinReadNextRxp(psFile);
        break;
      case AVCFileTABLE:
        psObj = (void*)AVCBinReadNextTableRec(psFile);
        break;
      default:
        CPLError(CE_Failure, CPLE_IllegalArg,
                 "AVCBinReadNextObject(): Unsupported file type!");
    }

    return psObj;
}



/*=====================================================================
 *                              ARC
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadNextArc()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadNextArc() instead)
 *
 * Read the next Arc structure from the file.
 *
 * The contents of the psArc structure is assumed to be valid, and the
 * psArc->pasVertices buffer may be reallocated or free()'d if it is not
 * NULL.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextArc(AVCRawBinFile *psFile, AVCArc *psArc,
                              int nPrecision)
{
    int         i, numVertices;

    psArc->nArcId  = AVCRawBinReadInt32(psFile);
    if (AVCRawBinEOF(psFile))
        return -1;

                     AVCRawBinReadInt32(psFile);  /* Skip Record size field */
    psArc->nUserId = AVCRawBinReadInt32(psFile);
    psArc->nFNode  = AVCRawBinReadInt32(psFile);
    psArc->nTNode  = AVCRawBinReadInt32(psFile);
    psArc->nLPoly  = AVCRawBinReadInt32(psFile);
    psArc->nRPoly  = AVCRawBinReadInt32(psFile);
    numVertices    = AVCRawBinReadInt32(psFile);

    /* Realloc the vertices array only if it needs to grow...
     * do not realloc to a smaller size.
     * Note that for simplicity reasons, we always store the vertices as
     * double values in memory, even for single precision coverages.
     */
    if (psArc->pasVertices == NULL || numVertices > psArc->numVertices)
        psArc->pasVertices = (AVCVertex*)CPLRealloc(psArc->pasVertices,
                                                numVertices*sizeof(AVCVertex));

    psArc->numVertices = numVertices;

    if (nPrecision == AVC_SINGLE_PREC)
    {
        for(i=0; i<numVertices; i++)
        {
            psArc->pasVertices[i].x = AVCRawBinReadFloat(psFile);
            psArc->pasVertices[i].y = AVCRawBinReadFloat(psFile);
        }
    }
    else
    {
        for(i=0; i<numVertices; i++)
        {
            psArc->pasVertices[i].x = AVCRawBinReadDouble(psFile);
            psArc->pasVertices[i].y = AVCRawBinReadDouble(psFile);
        }

    }

    return 0;
}

/**********************************************************************
 *                          AVCBinReadNextArc()
 *
 * Read the next Arc structure from the file.
 *
 * Returns a pointer to a static AVCArc structure whose contents will be
 * valid only until the next call or NULL if an error happened or if EOF
 * was reached.  
 **********************************************************************/
AVCArc *AVCBinReadNextArc(AVCBinFile *psFile)
{
    if (psFile->eFileType != AVCFileARC ||
        AVCRawBinEOF(psFile->psRawBinFile) ||
        _AVCBinReadNextArc(psFile->psRawBinFile, psFile->cur.psArc,
                           psFile->nPrecision) !=0)
    {
        return NULL;
    }

    return psFile->cur.psArc;
}


/*=====================================================================
 *                              PAL
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadNextPal()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadNextPal() instead)
 *
 * Read the next PAL (Polygon Arc List) structure from the file.
 *
 * The contents of the psPal structure is assumed to be valid, and the
 * psPal->paVertices buffer may be reallocated or free()'d if it is not
 * NULL.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextPal(AVCRawBinFile *psFile, AVCPal *psPal, 
                              int nPrecision)
{
    int i, numArcs;

    psPal->nPolyId = AVCRawBinReadInt32(psFile);
                     AVCRawBinReadInt32(psFile);  /* Skip Record size field */

    if (AVCRawBinEOF(psFile))
        return -1;

    if (nPrecision == AVC_SINGLE_PREC)
    {
        psPal->sMin.x  = AVCRawBinReadFloat(psFile);
        psPal->sMin.y  = AVCRawBinReadFloat(psFile);
        psPal->sMax.x  = AVCRawBinReadFloat(psFile);
        psPal->sMax.y  = AVCRawBinReadFloat(psFile);
    }
    else
    {
        psPal->sMin.x  = AVCRawBinReadDouble(psFile);
        psPal->sMin.y  = AVCRawBinReadDouble(psFile);
        psPal->sMax.x  = AVCRawBinReadDouble(psFile);
        psPal->sMax.y  = AVCRawBinReadDouble(psFile);
    }

    numArcs            = AVCRawBinReadInt32(psFile);

    /* Realloc the arc list array only if it needs to grow...
     * do not realloc to a smaller size.
     */
    if (psPal->pasArcs == NULL || numArcs > psPal->numArcs)
        psPal->pasArcs = (AVCPalArc*)CPLRealloc(psPal->pasArcs,
                                                 numArcs*sizeof(AVCPalArc));

    psPal->numArcs = numArcs;

    for(i=0; i<numArcs; i++)
    {
        psPal->pasArcs[i].nArcId = AVCRawBinReadInt32(psFile);
        psPal->pasArcs[i].nFNode = AVCRawBinReadInt32(psFile);
        psPal->pasArcs[i].nAdjPoly = AVCRawBinReadInt32(psFile);
    }

    return 0;
}

/**********************************************************************
 *                          AVCBinReadNextPal()
 *
 * Read the next PAL structure from the file.
 *
 * Returns a pointer to a static AVCPal structure whose contents will be
 * valid only until the next call or NULL if an error happened or if EOF
 * was reached.  
 **********************************************************************/
AVCPal *AVCBinReadNextPal(AVCBinFile *psFile)
{
    if ((psFile->eFileType!=AVCFilePAL && psFile->eFileType!=AVCFileRPL) ||
        AVCRawBinEOF(psFile->psRawBinFile) ||
        _AVCBinReadNextPal(psFile->psRawBinFile, psFile->cur.psPal,
                           psFile->nPrecision) !=0)
    {
        return NULL;
    }

    return psFile->cur.psPal;
}


/*=====================================================================
 *                              CNT
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadNextCnt()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadNextCnt() instead)
 *
 * Read the next CNT (Polygon Centroid) structure from the file.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextCnt(AVCRawBinFile *psFile, AVCCnt *psCnt, 
                              int nPrecision)
{
    int i, numLabels;

    psCnt->nPolyId = AVCRawBinReadInt32(psFile);
                     AVCRawBinReadInt32(psFile); /* Skip Record size field */

    if (AVCRawBinEOF(psFile))
        return -1;

    if (nPrecision == AVC_SINGLE_PREC)
    {
        psCnt->sCoord.x  = AVCRawBinReadFloat(psFile);
        psCnt->sCoord.y  = AVCRawBinReadFloat(psFile);
    }
    else
    {
        psCnt->sCoord.x  = AVCRawBinReadDouble(psFile);
        psCnt->sCoord.y  = AVCRawBinReadDouble(psFile);
    }

    numLabels      = AVCRawBinReadInt32(psFile);

    /* Realloc the LabelIds array only if it needs to grow...
     * do not realloc to a smaller size.
     */
    if (psCnt->panLabelIds == NULL || numLabels > psCnt->numLabels)
        psCnt->panLabelIds = (GInt32 *)CPLRealloc(psCnt->panLabelIds,
                                                  numLabels*sizeof(GInt32));

    psCnt->numLabels = numLabels;

    for(i=0; i<numLabels; i++)
    {
        psCnt->panLabelIds[i] = AVCRawBinReadInt32(psFile);
    }

    return 0;
}

/**********************************************************************
 *                          AVCBinReadNextCnt()
 *
 * Read the next CNT structure from the file.
 *
 * Returns a pointer to a static AVCCnt structure whose contents will be
 * valid only until the next call or NULL if an error happened or if EOF
 * was reached.  
 **********************************************************************/
AVCCnt *AVCBinReadNextCnt(AVCBinFile *psFile)
{
    if (psFile->eFileType != AVCFileCNT ||
        AVCRawBinEOF(psFile->psRawBinFile) ||
        _AVCBinReadNextCnt(psFile->psRawBinFile, psFile->cur.psCnt,
                           psFile->nPrecision) !=0)
    {
        return NULL;
    }

    return psFile->cur.psCnt;
}


/*=====================================================================
 *                              LAB
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadNextLab()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadNextLab() instead)
 *
 * Read the next LAB (Centroid Label) structure from the file.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextLab(AVCRawBinFile *psFile, AVCLab *psLab, 
                              int nPrecision)
{

    psLab->nValue  = AVCRawBinReadInt32(psFile);
    psLab->nPolyId = AVCRawBinReadInt32(psFile);

    if (AVCRawBinEOF(psFile))
        return -1;

    if (nPrecision == AVC_SINGLE_PREC)
    {
        psLab->sCoord1.x  = AVCRawBinReadFloat(psFile);
        psLab->sCoord1.y  = AVCRawBinReadFloat(psFile);
        psLab->sCoord2.x  = AVCRawBinReadFloat(psFile);
        psLab->sCoord2.y  = AVCRawBinReadFloat(psFile);
        psLab->sCoord3.x  = AVCRawBinReadFloat(psFile);
        psLab->sCoord3.y  = AVCRawBinReadFloat(psFile);
    }
    else
    {
        psLab->sCoord1.x  = AVCRawBinReadDouble(psFile);
        psLab->sCoord1.y  = AVCRawBinReadDouble(psFile);
        psLab->sCoord2.x  = AVCRawBinReadDouble(psFile);
        psLab->sCoord2.y  = AVCRawBinReadDouble(psFile);
        psLab->sCoord3.x  = AVCRawBinReadDouble(psFile);
        psLab->sCoord3.y  = AVCRawBinReadDouble(psFile);
    }

    return 0;
}

/**********************************************************************
 *                          AVCBinReadNextLab()
 *
 * Read the next LAB structure from the file.
 *
 * Returns a pointer to a static AVCLab structure whose contents will be
 * valid only until the next call or NULL if an error happened or if EOF
 * was reached.  
 **********************************************************************/
AVCLab *AVCBinReadNextLab(AVCBinFile *psFile)
{
    if (psFile->eFileType != AVCFileLAB ||
        AVCRawBinEOF(psFile->psRawBinFile) ||
        _AVCBinReadNextLab(psFile->psRawBinFile, psFile->cur.psLab,
                           psFile->nPrecision) !=0)
    {
        return NULL;
    }

    return psFile->cur.psLab;
}

/*=====================================================================
 *                              TOL
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadNextTol()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadNextTol() instead)
 *
 * Read the next TOL (tolerance) structure from the file.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextTol(AVCRawBinFile *psFile, AVCTol *psTol, 
                       int nPrecision)
{

    psTol->nIndex  = AVCRawBinReadInt32(psFile);
    psTol->nFlag   = AVCRawBinReadInt32(psFile);

    if (AVCRawBinEOF(psFile))
        return -1;

    if (nPrecision == AVC_SINGLE_PREC)
    {
        psTol->dValue  = AVCRawBinReadFloat(psFile);
    }
    else
    {
        psTol->dValue  = AVCRawBinReadDouble(psFile);
    }

    return 0;
}

/**********************************************************************
 *                          AVCBinReadNextTol()
 *
 * Read the next TOL structure from the file.
 *
 * Returns a pointer to a static AVCTol structure whose contents will be
 * valid only until the next call or NULL if an error happened or if EOF
 * was reached.  
 **********************************************************************/
AVCTol *AVCBinReadNextTol(AVCBinFile *psFile)
{
    if (psFile->eFileType != AVCFileTOL ||
        AVCRawBinEOF(psFile->psRawBinFile) ||
        _AVCBinReadNextTol(psFile->psRawBinFile, psFile->cur.psTol,
                           psFile->nPrecision) !=0)
    {
        return NULL;
    }

    return psFile->cur.psTol;
}

/*=====================================================================
 *                              PRJ
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadOpenPrj()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadOpen() with type AVCFilePRJ instead)
 *
 * Open a PRJ file.  
 *
 * This call will actually read the whole PRJ file in memory since PRJ
 * files are small text files.
 **********************************************************************/
AVCBinFile *_AVCBinReadOpenPrj(const char *pszPath, const char *pszName)
{
    AVCBinFile   *psFile;
    char         *pszFname, **papszPrj;

    /*-----------------------------------------------------------------
     * Load the PRJ file contents into a stringlist.
     *----------------------------------------------------------------*/
    pszFname = (char*)CPLMalloc((strlen(pszPath)+strlen(pszName)+1)*
                                sizeof(char));
    sprintf(pszFname, "%s%s", pszPath, pszName);

    papszPrj = CSLLoad(pszFname);

    CPLFree(pszFname);

    if (papszPrj == NULL)
    {
        /* Failed to open file... just return NULL since an error message
         * has already been issued by CSLLoad()
         */
        return NULL;
    }

    /*-----------------------------------------------------------------
     * Alloc and init the AVCBinFile handle.
     *----------------------------------------------------------------*/
    psFile = (AVCBinFile*)CPLCalloc(1, sizeof(AVCBinFile));

    psFile->eFileType = AVCFilePRJ;
    psFile->psRawBinFile = NULL;
    psFile->cur.papszPrj = papszPrj;
    psFile->pszFilename = NULL;


    return psFile;
}

/**********************************************************************
 *                          AVCBinReadPrj()
 *
 * Return the contents of the previously opened PRJ (projection) file.
 *
 * PRJ files are simple text files with variable length lines, so we
 * don't use the AVCRawBin*() functions for this case.
 *
 * Returns a reference to a static stringlist with the whole file 
 * contents, or NULL in case of error.
 *
 * The returned stringlist should NOT be freed by the caller.
 **********************************************************************/
char **AVCBinReadNextPrj(AVCBinFile *psFile)
{
    /*-----------------------------------------------------------------
     * The file should have already been loaded by AVCBinFileOpen(),
     * so there is not much to do here!
     *----------------------------------------------------------------*/
    return psFile->cur.papszPrj;
}

/*=====================================================================
 *                              TXT/TX6/TX7
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadNextTxt()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadNextTxt() instead)
 *
 * Read the next TXT/TX6/TX7 (Annotation) structure from the file.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextTxt(AVCRawBinFile *psFile, AVCTxt *psTxt, 
                              int nPrecision)
{
    int i, numVerticesBefore, numVertices, numCharsToRead;

    numVerticesBefore = ABS(psTxt->numVerticesLine) + 
                        ABS(psTxt->numVerticesArrow);

    psTxt->nTxtId  = AVCRawBinReadInt32(psFile);
    if (AVCRawBinEOF(psFile))
        return -1;

                     AVCRawBinReadInt32(psFile); /* Skip Record size field */
    psTxt->nUserId = AVCRawBinReadInt32(psFile);
    psTxt->nLevel  = AVCRawBinReadInt32(psFile);

    psTxt->f_1e2    = AVCRawBinReadFloat(psFile);
    psTxt->nSymbol  = AVCRawBinReadInt32(psFile);
    psTxt->numVerticesLine  = AVCRawBinReadInt32(psFile);
    psTxt->n28      = AVCRawBinReadInt32(psFile);
    psTxt->numChars = AVCRawBinReadInt32(psFile);
    psTxt->numVerticesArrow = AVCRawBinReadInt32(psFile);

    for(i=0; i<20; i++)
    {
        psTxt->anJust1[i] = AVCRawBinReadInt16(psFile);
    }
    for(i=0; i<20; i++)
    {
        psTxt->anJust2[i] = AVCRawBinReadInt16(psFile);
    }

    if (nPrecision == AVC_SINGLE_PREC)
    {
        psTxt->dHeight = AVCRawBinReadFloat(psFile);
        psTxt->dV2     = AVCRawBinReadFloat(psFile);
        psTxt->dV3     = AVCRawBinReadFloat(psFile);
    }
    else
    {
        psTxt->dHeight = AVCRawBinReadDouble(psFile);
        psTxt->dV2     = AVCRawBinReadDouble(psFile);
        psTxt->dV3     = AVCRawBinReadDouble(psFile);
    }

    numCharsToRead = ((int)(psTxt->numChars + 3)/4)*4;
    if (psTxt->pszText == NULL ||
        ((int)(strlen(psTxt->pszText)+3)/4)*4 < numCharsToRead )
    {
        psTxt->pszText = (char*)CPLRealloc(psTxt->pszText,
                                           (numCharsToRead+1)*sizeof(char));
    }

    AVCRawBinReadBytes(psFile, numCharsToRead, (GByte *)(psTxt->pszText) );
    psTxt->pszText[psTxt->numChars] = '\0';

    /* Realloc the vertices array only if it needs to grow...
     * do not realloc to a smaller size.
     */
    numVertices = ABS(psTxt->numVerticesLine) + ABS(psTxt->numVerticesArrow);

    if (psTxt->pasVertices == NULL || numVertices > numVerticesBefore)
        psTxt->pasVertices = (AVCVertex*)CPLRealloc(psTxt->pasVertices,
                                              numVertices*sizeof(AVCVertex));

    if (nPrecision == AVC_SINGLE_PREC)
    {
        for(i=0; i<numVertices; i++)
        {
            psTxt->pasVertices[i].x = AVCRawBinReadFloat(psFile);
            psTxt->pasVertices[i].y = AVCRawBinReadFloat(psFile);
        }
    }
    else
    {
        for(i=0; i<numVertices; i++)
        {
            psTxt->pasVertices[i].x = AVCRawBinReadDouble(psFile);
            psTxt->pasVertices[i].y = AVCRawBinReadDouble(psFile);
        }
    }

    AVCRawBinFSeek(psFile, 8, SEEK_CUR);     /* Skip 8 bytes */

    return 0;
}

/**********************************************************************
 *                          AVCBinReadNextTxt()
 *
 * Read the next TXT/TX6/TX7 structure from the file.
 *
 * Returns a pointer to a static AVCTxt structure whose contents will be
 * valid only until the next call or NULL if an error happened or if EOF
 * was reached.  
 **********************************************************************/
AVCTxt *AVCBinReadNextTxt(AVCBinFile *psFile)
{
    if ((psFile->eFileType != AVCFileTXT && psFile->eFileType != AVCFileTX6) ||
        AVCRawBinEOF(psFile->psRawBinFile) ||
        _AVCBinReadNextTxt(psFile->psRawBinFile, psFile->cur.psTxt,
                           psFile->nPrecision) !=0)
    {
        return NULL;
    }

    return psFile->cur.psTxt;
}


/*=====================================================================
 *                              RXP
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadNextRxp()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadNextRxp() instead)
 *
 * Read the next RXP (Region something...) structure from the file.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextRxp(AVCRawBinFile *psFile, AVCRxp *psRxp, 
                       int nPrecision)
{

    psRxp->n1  = AVCRawBinReadInt32(psFile);
    if (AVCRawBinEOF(psFile))
        return -1;
    psRxp->n2  = AVCRawBinReadInt32(psFile);

    return 0;
}

/**********************************************************************
 *                          AVCBinReadNextRxp()
 *
 * Read the next RXP structure from the file.
 *
 * Returns a pointer to a static AVCRxp structure whose contents will be
 * valid only until the next call or NULL if an error happened or if EOF
 * was reached.  
 **********************************************************************/
AVCRxp *AVCBinReadNextRxp(AVCBinFile *psFile)
{
    if (psFile->eFileType != AVCFileRXP ||
        AVCRawBinEOF(psFile->psRawBinFile) ||
        _AVCBinReadNextRxp(psFile->psRawBinFile, psFile->cur.psRxp,
                           psFile->nPrecision) !=0)
    {
        return NULL;
    }

    return psFile->cur.psRxp;
}

/*=====================================================================
 *                              TABLEs
 *====================================================================*/

/**********************************************************************
 *                          _AVCBinReadNextArcDir()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadOpen() with type AVCFileTABLE instead)
 *
 * Read the next record from an arc.dir file.
 *
 * Note that arc.dir files have no header... they start with the
 * first record immediately.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextArcDir(AVCRawBinFile *psFile, AVCTableDef *psArcDir)
{
    /* Arc/Info Table name 
     */
    AVCRawBinReadBytes(psFile, 32, (GByte *)(psArcDir->szTableName) );
    psArcDir->szTableName[32] = '\0';

    if (AVCRawBinEOF(psFile))
        return -1;

    /* "ARC####" basename for .DAT and .NIT files
     */
    AVCRawBinReadBytes(psFile, 8, (GByte *)(psArcDir->szInfoFile) );
    psArcDir->szInfoFile[7] = '\0';

    psArcDir->numFields = AVCRawBinReadInt16(psFile);
    psArcDir->nRecSize  = AVCRawBinReadInt16(psFile);

    AVCRawBinFSeek(psFile, 20, SEEK_CUR);     /* Skip 20 bytes */
    
    psArcDir->numRecords = AVCRawBinReadInt32(psFile);

    AVCRawBinFSeek(psFile, 10, SEEK_CUR);     /* Skip 10 bytes */
    
    AVCRawBinReadBytes(psFile, 2, (GByte *) (psArcDir->szExternal) );
    psArcDir->szExternal[2] = '\0';

    AVCRawBinFSeek(psFile, 300, SEEK_CUR);  /* Skip the remaining 300 bytes */

    return 0;
}

/**********************************************************************
 *                          _AVCBinReadNextNit()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadOpen() with type AVCFileTABLE instead)
 *
 * Read the next record from an arc####.nit file.
 *
 * Note that arc####.nit files have no header... they start with the
 * first record immediately.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextArcNit(AVCRawBinFile *psFile, AVCFieldInfo *psField)
{
    AVCRawBinReadBytes(psFile, 16, (GByte *)(psField->szName) );
    psField->szName[16] = '\0';

    if (AVCRawBinEOF(psFile))
        return -1;

    psField->nSize     = AVCRawBinReadInt16(psFile);
    psField->v2        = AVCRawBinReadInt16(psFile);  /* Always -1 ? */
    psField->nOffset   = AVCRawBinReadInt16(psFile);
    psField->v4        = AVCRawBinReadInt16(psFile);  /* Always 4 ?  */
    psField->v5        = AVCRawBinReadInt16(psFile);  /* Always -1 ? */
    psField->nFmtWidth = AVCRawBinReadInt16(psFile);
    psField->nFmtPrec  = AVCRawBinReadInt16(psFile);
    psField->nType1    = AVCRawBinReadInt16(psFile);
    psField->nType2    = AVCRawBinReadInt16(psFile);  /* Always 0 ? */
    psField->v10       = AVCRawBinReadInt16(psFile);  /* Always -1 ? */
    psField->v11       = AVCRawBinReadInt16(psFile);  /* Always -1 ? */
    psField->v12       = AVCRawBinReadInt16(psFile);  /* Always -1 ? */
    psField->v13       = AVCRawBinReadInt16(psFile);  /* Always -1 ? */

    AVCRawBinReadBytes(psFile, 16, (GByte *) (psField->szAltName) );   /* Always Blank ? */
    psField->szAltName[16] = '\0';

    AVCRawBinFSeek(psFile, 56, SEEK_CUR);             /* Skip 56 bytes */
    
    psField->nIndex    = AVCRawBinReadInt16(psFile);

    AVCRawBinFSeek(psFile, 28, SEEK_CUR);  /* Skip the remaining 28 bytes */

    return 0;
}

/**********************************************************************
 *                          AVCBinReadListTables()
 *
 * Scan the arc.dir file and return stringlist with one entry for the
 * Arc/Info name of each table that belongs to the specified coverage.
 * Pass pszCoverName = NULL to get the list of all tables.
 *
 * ppapszArcDatFiles if not NULL will be set to point to a stringlist
 * with the corresponding "ARC????" info file basenames corresponding
 * to each table found.
 *
 * Note that arc.dir files have no header... they start with the
 * first record immediately.
 *
 * Returns a stringlist that should be deallocated by the caller
 * with CSLDestroy(), or NULL on error.
 **********************************************************************/
char **AVCBinReadListTables(const char *pszInfoPath, const char *pszCoverName,
                            char ***ppapszArcDatFiles)
{
    char              **papszList = NULL;
    char               *pszFname;
    char                szNameToFind[33] = "";
    int                 nLen;
    AVCRawBinFile      *hFile;
    AVCTableDef         sEntry;

    if (ppapszArcDatFiles)
        *ppapszArcDatFiles = NULL;

    /*----------------------------------------------------------------- 
     * All tables that belong to a given coverage have their name starting
     * with the coverage name (in uppercase letters), followed by a 3 
     * letters extension.
     *----------------------------------------------------------------*/
    if (pszCoverName != NULL)
        sprintf(szNameToFind, "%-.28s.", pszCoverName);
    nLen = strlen(szNameToFind);

    /*----------------------------------------------------------------- 
     * Open the arc.dir and add all entries that match the criteria
     * to our list.
     *----------------------------------------------------------------*/
    pszFname = (char*)CPLMalloc((strlen(pszInfoPath)+9)*sizeof(char));
    sprintf(pszFname, "%sarc.dir", pszInfoPath);

    hFile = AVCRawBinOpen(pszFname, "r");

    if (hFile)
    {
        while (!AVCRawBinEOF(hFile) &&
               _AVCBinReadNextArcDir(hFile, &sEntry) == 0)
        {
            if (/* sEntry.numRecords > 0 && (DO NOT skip empty tables) */
                (pszCoverName == NULL ||
                 EQUALN(szNameToFind, sEntry.szTableName, nLen)) )
            {
                papszList = CSLAddString(papszList, sEntry.szTableName);

                if (ppapszArcDatFiles)
                    *ppapszArcDatFiles = CSLAddString(*ppapszArcDatFiles,
                                                      sEntry.szInfoFile);
            }
        }
        AVCRawBinClose(hFile);

    }

    CPLFree(pszFname);

    return papszList;
}

/**********************************************************************
 *                         _AVCBinReadOpenTable()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadOpen() with type AVCFileTABLE instead)
 *
 * Open a INFO table, read the header file (.NIT), and finally open
 * the associated data file to be ready to read records from it.
 *
 * Returns a valid AVCBinFile handle, or NULL if the file could
 * not be opened.
 *
 * _AVCBinReadCloseTable() will eventually have to be called to release the 
 * resources used by the AVCBinFile structure.
 **********************************************************************/
AVCBinFile *_AVCBinReadOpenTable(const char *pszInfoPath,
                                 const char *pszTableName)
{
    AVCBinFile    *psFile;
    AVCRawBinFile *hFile;
    AVCTableDef    sTableDef;
    AVCFieldInfo  *pasFieldDef;
    char          *pszFname;
    GBool          bFound;
    int            i;

    /* Alloc a buffer big enough for the longest possible filename...
     */
    pszFname = (char*)CPLMalloc((strlen(pszInfoPath)+81)*sizeof(char));

    /*-----------------------------------------------------------------
     * Fetch info about this table from the "arc.dir"
     *----------------------------------------------------------------*/
    sprintf(pszFname, "%sarc.dir", pszInfoPath);
    hFile = AVCRawBinOpen(pszFname, "r");
    bFound = FALSE;

    if (hFile)
    {
        while(!bFound && _AVCBinReadNextArcDir(hFile, &sTableDef) == 0)
        {
            if (EQUALN(sTableDef.szTableName, pszTableName, 
                       strlen(pszTableName)))
            {
                bFound = TRUE;
            }
        }
        AVCRawBinClose(hFile);
    }

    /* Hummm... quite likely that this table does not exist!
     */
    if (!bFound)
    {
        CPLFree(pszFname);
        CPLError(CE_Failure, CPLE_OpenFailed,
                 "Failed to open table %s", pszTableName);
        return NULL;
    }

    /*-----------------------------------------------------------------
     * Establish the location of the data file... depends on the 
     * szExternal[] field.
     *----------------------------------------------------------------*/
    if (EQUAL(sTableDef.szExternal, "XX"))
    {
        /*-------------------------------------------------------------
         * The data file is located outside of the INFO directory.
         * Read the path to the data file from the arc####.dat file
         *------------------------------------------------------------*/
        sprintf(pszFname, "%sarc%s.dat", pszInfoPath, sTableDef.szInfoFile+3);
    
        hFile = AVCRawBinOpen(pszFname, "r");

        if (hFile)
        {
            /* Read the relative file path, and remove trailing spaces.
             */
            AVCRawBinReadBytes(hFile, 80, (GByte *) (sTableDef.szDataFile) );
            sTableDef.szDataFile[80] = '\0';

            for(i = strlen(sTableDef.szDataFile)-1;
                isspace(sTableDef.szDataFile[i]);
                i--)
            {
                sTableDef.szDataFile[i] = '\0';
            }

            AVCRawBinClose(hFile);
        }
        else
        {
            CPLFree(pszFname);
            CPLError(CE_Failure, CPLE_OpenFailed,
                     "Failed to open file %s", pszFname);
            return NULL;
        }
         
    }
    else
    {
        /*-------------------------------------------------------------
         * The data file IS the arc####.dat file
         *------------------------------------------------------------*/
        sprintf(sTableDef.szDataFile, "arc%s.dat", sTableDef.szInfoFile+3);
    }

    /*-----------------------------------------------------------------
     * Read the table field definitions from the "arc####.nit" file.
     *----------------------------------------------------------------*/
    sprintf(pszFname, "%sarc%s.nit", pszInfoPath, sTableDef.szInfoFile+3);

    hFile = AVCRawBinOpen(pszFname, "r");

    if (hFile)
    {
        int iField;

        pasFieldDef = (AVCFieldInfo*)CPLCalloc(sTableDef.numFields,
                                               sizeof(AVCFieldInfo));

        /*-------------------------------------------------------------
         * There must be at least sTableDef.numFields valid entries
         * in the .NIT file...
         *
         * Note that we ignore any deleted field entries (entries with
         * index=-1)... I don't see any use for these deleted fields...
         * and I don't understand why Arc/Info includes them in their
         * E00 table headers...
         *------------------------------------------------------------*/
        for(i=0, iField=0; iField<sTableDef.numFields; i++)
        {
            if (_AVCBinReadNextArcNit(hFile, &(pasFieldDef[iField])) != 0)
            {
                /* Problems.... is the NIT file corrupt???
                 */
                AVCRawBinClose(hFile);
                CPLFree(pszFname);
                CPLFree(pasFieldDef);
                CPLError(CE_Failure, CPLE_FileIO,
                         "Failed reading table field info for table %s "
                         "File may be corrupt?",  pszTableName);
                return NULL;
            }

            /*---------------------------------------------------------
             * Check if the field has been deleted (nIndex == -1).
             * We just ignore deleted fields
             *--------------------------------------------------------*/
            if (pasFieldDef[iField].nIndex > 0)
                iField++;
        }

        AVCRawBinClose(hFile);
    }
    else
    {
        CPLFree(pszFname);
        CPLError(CE_Failure, CPLE_OpenFailed,
                 "Failed to open file %s", pszFname);
        return NULL;
    }


    /*-----------------------------------------------------------------
     * Open the data file... ready to read records from it.
     * If the header says that table has 0 records, then we don't
     * try to open the file... but we don't consider that as an error.
     *----------------------------------------------------------------*/
    if (sTableDef.numRecords > 0)
    {
        VSIStatBuf      sStatBuf;

        sprintf(pszFname, "%s%s", pszInfoPath, sTableDef.szDataFile);
        hFile = AVCRawBinOpen(pszFname, "r");

        /* OOPS... data file does not exist!
         */
        if (hFile == NULL)
        {
            CPLFree(pszFname);
            CPLError(CE_Failure, CPLE_OpenFailed,
                     "Failed to open file %s", pszFname);
            return NULL;
        }

        /*-------------------------------------------------------------
         * In some cases, the number of records field for a table in the 
         * arc.dir does not correspond to the real number of records
         * in the data file.  In this kind of situation, the number of
         * records returned by Arc/Info in an E00 file will be based
         * on the real data file size, and not on the value from the arc.dir.
         *
         * Fetch the data file size, and correct the number of record
         * field in the table header if necessary.
         *------------------------------------------------------------*/
        if ( VSIStat(pszFname, &sStatBuf) != -1 &&
             sTableDef.nRecSize > 0 &&
             sStatBuf.st_size/sTableDef.nRecSize != sTableDef.numRecords)
        {
            sTableDef.numRecords = sStatBuf.st_size/sTableDef.nRecSize;
        }

    }
    else
    {
        hFile = NULL;
    }

    /*-----------------------------------------------------------------
     * Alloc. and init. the AVCBinFile structure.
     *----------------------------------------------------------------*/
    psFile = (AVCBinFile*)CPLCalloc(1, sizeof(AVCBinFile));

    psFile->psRawBinFile = hFile;
    psFile->eFileType = AVCFileTABLE;
    psFile->pszFilename = pszFname;

    psFile->hdr.psTableDef = (AVCTableDef*)CPLMalloc(sizeof(AVCTableDef));
    *(psFile->hdr.psTableDef) = sTableDef;

    psFile->hdr.psTableDef->pasFieldDef = pasFieldDef;

    /* We can't really tell the precision from a Table header...
     * just set an arbitrary value... it probably won't be used anyways!
     */
    psFile->nPrecision = AVC_SINGLE_PREC;

    /*-----------------------------------------------------------------
     * Allocate temp. structures to use to read records from the file
     * And allocate buffers for those fields that are stored as strings.
     *----------------------------------------------------------------*/
    psFile->cur.pasFields = (AVCField*)CPLCalloc(sTableDef.numFields,
                                                 sizeof(AVCField));

    for(i=0; i<sTableDef.numFields; i++)
    {
        if (pasFieldDef[i].nType1*10 == AVC_FT_DATE ||
            pasFieldDef[i].nType1*10 == AVC_FT_CHAR ||
            pasFieldDef[i].nType1*10 == AVC_FT_FIXINT ||
            pasFieldDef[i].nType1*10 == AVC_FT_FIXNUM )
        {
            psFile->cur.pasFields[i].pszStr = 
                (char*)CPLCalloc(pasFieldDef[i].nSize+1, sizeof(char));
        }
    }

    return psFile;
}


/**********************************************************************
 *                         _AVCBinReadNextTableRec()
 *
 * (This function is for internal library use... external calls should
 * go to AVCBinReadNextTableRec() instead)
 *
 * Reads the next record from an attribute table and fills the 
 * pasFields[] array.
 *
 * Note that it is assumed that the pasFields[] array has been properly
 * initialized, re the allocation of buffers for fields strored as
 * strings.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int _AVCBinReadNextTableRec(AVCRawBinFile *psFile, int nFields,
                            AVCFieldInfo *pasDef, AVCField *pasFields,
                            int nRecordSize)
{
    int i, nType, nBytesRead=0;

    if (psFile == NULL)
        return -1;

    for(i=0; i<nFields; i++)
    {
        if (AVCRawBinEOF(psFile))
            return -1;

        nType = pasDef[i].nType1*10;

        if (nType ==  AVC_FT_DATE || nType == AVC_FT_CHAR ||
            nType == AVC_FT_FIXINT || nType == AVC_FT_FIXNUM)
        {
            /*---------------------------------------------------------
             * Values stored as strings
             *--------------------------------------------------------*/
            AVCRawBinReadBytes(psFile, pasDef[i].nSize, (GByte *) (pasFields[i].pszStr) );
            pasFields[i].pszStr[pasDef[i].nSize] = '\0';
        }
        else if (nType == AVC_FT_BININT && pasDef[i].nSize == 4)
        {
            /*---------------------------------------------------------
             * 32 bit binary integers
             *--------------------------------------------------------*/
            pasFields[i].nInt32 = AVCRawBinReadInt32(psFile);
        }
        else if (nType == AVC_FT_BININT && pasDef[i].nSize == 2)
        {
            /*---------------------------------------------------------
             * 16 bit binary integers
             *--------------------------------------------------------*/
            pasFields[i].nInt16 = AVCRawBinReadInt16(psFile);
        }
        else if (nType == AVC_FT_BINFLOAT && pasDef[i].nSize == 4)
        {
            /*---------------------------------------------------------
             * Single precision floats
             *--------------------------------------------------------*/
            pasFields[i].fFloat = AVCRawBinReadFloat(psFile);
        }
        else if (nType == AVC_FT_BINFLOAT && pasDef[i].nSize == 8)
        {
            /*---------------------------------------------------------
             * Double precision floats
             *--------------------------------------------------------*/
            pasFields[i].dDouble = AVCRawBinReadDouble(psFile);
        }
        else
        {
            /*---------------------------------------------------------
             * Hummm... unsupported field type...
             *--------------------------------------------------------*/
            CPLError(CE_Failure, CPLE_NotSupported,
                     "Unsupported field type: (type=%d, size=%d)",
                     nType, pasDef[i].nSize);
            return -1;
        }

        nBytesRead += pasDef[i].nSize;
    }

    /*-----------------------------------------------------------------
     * Record size is rounded to a multiple of 2 bytes.
     * Check the number of bytes read, and move the read pointer if
     * necessary.
     *----------------------------------------------------------------*/
    if (nBytesRead < nRecordSize)
        AVCRawBinFSeek(psFile, SEEK_CUR, nRecordSize - nBytesRead);

    return 0;
}

/**********************************************************************
 *                          AVCBinReadNextTableRec()
 *
 * Reads the next record from an attribute table.
 *
 * Returns a pointer to an array of static AVCField structure whose 
 * contents will be valid only until the next call,
 * or NULL if an error happened or if EOF was reached.  
 **********************************************************************/
AVCField *AVCBinReadNextTableRec(AVCBinFile *psFile)
{
    if (psFile->eFileType != AVCFileTABLE ||
        psFile->hdr.psTableDef->numRecords == 0 ||
        AVCRawBinEOF(psFile->psRawBinFile) ||
        _AVCBinReadNextTableRec(psFile->psRawBinFile, 
                                psFile->hdr.psTableDef->numFields,
                                psFile->hdr.psTableDef->pasFieldDef,
                                psFile->cur.pasFields,
                                psFile->hdr.psTableDef->nRecSize) !=0)
    {
        return NULL;
    }

    return psFile->cur.pasFields;
}


