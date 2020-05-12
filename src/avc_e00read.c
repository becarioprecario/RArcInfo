/**********************************************************************
 * $Id: avc_e00read.c,v 1.1.1.1 2001/06/27 20:10:53 vir Exp $
 *
 * Name:     avc_e00read.c
 * Project:  Arc/Info vector coverage (AVC)  BIN->E00 conversion library
 * Language: ANSI C
 * Purpose:  Functions to open a binary coverage and read it as if it
 *           was an ASCII E00 file.  This file is the main entry point
 *           for the library.
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
 * $Log: avc_e00read.c,v $
 * Revision 1.1.1.1  2001/06/27 20:10:53  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:09  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.6  1999/08/26 17:22:18  daniel
 * Use VSIFopen() instead of fopen() directly
 *
 * Revision 1.5  1999/08/23 18:21:41  daniel
 * New syntax for AVCBinReadListTables()
 *
 * Revision 1.4  1999/05/11 02:10:01  daniel
 * Free psInfo struct inside AVCE00ReadClose()
 *
 * Revision 1.3  1999/04/06 19:43:26  daniel
 * Added E00 coverage path in EXP 0  header line
 *
 * Revision 1.2  1999/02/25 04:19:01  daniel
 * Added TXT, TX6/TX7, RXP and RPL support + other minor changes
 *
 * Revision 1.1  1999/01/29 16:28:52  daniel
 * Initial revision
 *
 **********************************************************************/

#include <ctype.h>      /* toupper() */

#ifdef WIN32
#  include <direct.h>   /* getcwd() */
#endif

#include "avc.h"

static GBool _AVCFileExists(const char *pszPath, const char *pszName);
static int _AVCE00ReadBuildSqueleton(AVCE00ReadPtr psInfo);


/**********************************************************************
 *                          AVCE00ReadOpen()
 *
 * Open a Arc/Info coverage to read it as if it was an E00 file.
 *
 * You can either pass the name of the coverage directory, or the path
 * to one of the files in the coverage directory.  The name of the
 * coverage MUST be included in pszCoverPath... this means that
 * passing "." is invalid.
 * The following are all valid values for pszCoverPath:
 *               /home/data/country
 *               /home/data/country/
 *               /home/data/country/arc.adf
 * (Of course you should replace the '/' with '\\' on DOS systems!)
 *
 * Returns a new AVCE00ReadPtr handle or NULL if the coverage could 
 * not be opened or if it does not appear to be a valid Arc/Info coverage.
 *
 * The handle will eventually have to be released with AVCE00ReadClose().
 **********************************************************************/
AVCE00ReadPtr  AVCE00ReadOpen(const char *pszCoverPath)
{
    AVCE00ReadPtr   psInfo;
    int             i, nLen, nCoverPrecision;
    VSIStatBuf      sStatBuf;
    
    CPLErrorReset();

    /*-----------------------------------------------------------------
     * pszCoverPath must be either a valid directory name or a valid
     * file name.
     *----------------------------------------------------------------*/
    if (pszCoverPath == NULL || strlen(pszCoverPath) == 0 ||
        VSIStat(pszCoverPath, &sStatBuf) == -1)
    {
        CPLError(CE_Failure, CPLE_OpenFailed, 
                 "Invalid coverage path: %s.", 
                 pszCoverPath?pszCoverPath:"(NULL)");
        return NULL;
    }

    /*-----------------------------------------------------------------
     * Alloc the AVCE00ReadPtr handle
     *----------------------------------------------------------------*/
    psInfo = (AVCE00ReadPtr)CPLCalloc(1, sizeof(struct AVCE00ReadInfo_t));

    /*-----------------------------------------------------------------
     * 2 possibilities about the value passed in pszCoverPath:
     * - It can be the directory name of the coverage
     * - or it can be the path to one of the files in the coverage
     *
     * If the name passed in pszCoverPath is not a directory, then we
     * need to strip the last part of the filename to keep only the
     * path, terminated by a '/' (or a '\\').
     *----------------------------------------------------------------*/
    if (VSI_ISDIR(sStatBuf.st_mode))
    {
        /*-------------------------------------------------------------
         * OK, we have a valid directory name... make sure it is 
         * terminated with a '/' (or '\\')
         *------------------------------------------------------------*/
        nLen = strlen(pszCoverPath);

        if (pszCoverPath[nLen-1] == '/' || pszCoverPath[nLen-1] == '\\')
            psInfo->pszCoverPath = CPLStrdup(pszCoverPath);
        else
        {
#ifdef WIN32
            psInfo->pszCoverPath = CPLStrdup(CPLSPrintf("%s\\",pszCoverPath));
#else
            psInfo->pszCoverPath = CPLStrdup(CPLSPrintf("%s/",pszCoverPath));
#endif
        }
    }
    else
    {
        /*-------------------------------------------------------------
         * We are dealing with a filename.
         * Extract the coverage path component and store it.
         * The coverage path will remain terminated by a '/' or '\\' char.
         *------------------------------------------------------------*/
        psInfo->pszCoverPath = CPLStrdup(pszCoverPath);

        for( i = strlen(psInfo->pszCoverPath)-1; 
             i > 0 && psInfo->pszCoverPath[i] != '/' &&
                 psInfo->pszCoverPath[i] != '\\';
             i-- ) {}

        psInfo->pszCoverPath[i+1] = '\0';
    }


    /*-----------------------------------------------------------------
     * Extract the coverage name from the coverage path.  Note that
     * for this the coverage path must be in the form:
     * "dir1/dir2/dir3/covername/" ... if it is not the case, then
     * we would have to use getcwd() to find the current directory name...
     * but for now we'll just produce an error if this happens.
     *----------------------------------------------------------------*/
    nLen = 0;
    for( i = strlen(psInfo->pszCoverPath)-1; 
	 i > 0 && psInfo->pszCoverPath[i-1] != '/' &&
	          psInfo->pszCoverPath[i-1] != '\\'&&
	          psInfo->pszCoverPath[i-1] != ':';
	 i-- ) 
    {
        nLen++;
    }

    if (nLen > 0)
    {
        psInfo->pszCoverName = CPLStrdup(psInfo->pszCoverPath+i);
        psInfo->pszCoverName[nLen] = '\0';
    }
    else
    {
        CPLError(CE_Failure, CPLE_OpenFailed, 
                 "Invalid coverage path (%s): "
                 "coverage name must be included in path.", pszCoverPath);

        CPLFree(psInfo->pszCoverPath);
        CPLFree(psInfo);
        return NULL;
    }

    /*-----------------------------------------------------------------
     * Lazy way to build the INFO path: simply add "../info/"...
     * this could probably be improved!
     *----------------------------------------------------------------*/
    psInfo->pszInfoPath = (char*)CPLMalloc((strlen(psInfo->pszCoverPath)+9)*
                                           sizeof(char));
#ifdef WIN32
#  define AVC_INFOPATH "..\\info\\"
#else
#  define AVC_INFOPATH "../info/"
#endif
    sprintf(psInfo->pszInfoPath, "%s%s", psInfo->pszCoverPath, AVC_INFOPATH);

    /*-----------------------------------------------------------------
     * Check that the info directory exists and contains the "arc.dir"
     *----------------------------------------------------------------*/
    if (! _AVCFileExists(psInfo->pszInfoPath, "arc.dir") )
    {
        CPLError(CE_Failure, CPLE_OpenFailed, 
             "Invalid coverage (%s): 'info' directory not found or invalid.", 
                                              pszCoverPath);
        CPLFree(psInfo->pszCoverPath);
        CPLFree(psInfo->pszInfoPath);
        CPLFree(psInfo);
        return NULL;
    }

    /*-----------------------------------------------------------------
     * Build the E00 file squeleton and be ready to return a E00 header...
     * We'll also read the coverage precision by the same way.
     *----------------------------------------------------------------*/
    nCoverPrecision = _AVCE00ReadBuildSqueleton(psInfo);

    psInfo->iCurSection = 0;
    psInfo->iCurStep = AVC_GEN_NOTSTARTED;
    psInfo->bReadAllSections = TRUE;

    /*-----------------------------------------------------------------
     * Init the E00 generator.
     *----------------------------------------------------------------*/
    psInfo->hGenInfo = AVCE00GenInfoAlloc(nCoverPrecision);


    /*-----------------------------------------------------------------
     * If an error happened during the open call, cleanup and return NULL.
     *----------------------------------------------------------------*/
    if (CPLGetLastErrorNo() != 0)
    {
        AVCE00ReadClose(psInfo);
        psInfo = NULL;
    }

    return psInfo;
}

/**********************************************************************
 *                          AVCE00ReadClose()
 *
 * Close a coverage and release all memory used by the AVCE00ReadPtr
 * handle.
 **********************************************************************/
void AVCE00ReadClose(AVCE00ReadPtr psInfo)
{
    CPLErrorReset();

    if (psInfo == NULL)
        return;

    CPLFree(psInfo->pszCoverPath);
    CPLFree(psInfo->pszInfoPath);

    if (psInfo->hFile)
        AVCBinReadClose(psInfo->hFile);

    if (psInfo->hGenInfo)
        AVCE00GenInfoFree(psInfo->hGenInfo);

    if (psInfo->pasSections)
    {
        int i;
        for(i=0; i<psInfo->numSections; i++)
        {
            CPLFree(psInfo->pasSections[i].pszName);
        }
        CPLFree(psInfo->pasSections);
    }

    CPLFree(psInfo);
}


/**********************************************************************
 *                          _AVCFileExists()
 *
 * Returns TRUE if a file with the specified name exists in the
 * specified directory.
 *
 * For now I simply try to fopen() the file ... would it be more
 * efficient to use stat() ???
 **********************************************************************/
static GBool _AVCFileExists(const char *pszPath, const char *pszName)
{
    char        *pszBuf;
    GBool       bFileExists = FALSE;
    FILE        *fp;

    pszBuf = (char*)CPLMalloc((strlen(pszPath)+strlen(pszName)+1)*
                              sizeof(char));
    sprintf(pszBuf, "%s%s", pszPath, pszName);

    if ((fp = VSIFOpen(pszBuf, "rb")) != NULL)
    {
        bFileExists = TRUE;
        VSIFClose(fp);
    }

    CPLFree(pszBuf);

    return bFileExists;
}

/**********************************************************************
 *                          _AVCIncreaseSectionsArray()
 *
 * Add a number of structures to the Sections array and return the
 * index of the first one that was added.  Note that the address of the
 * original array (*pasArray) is quite likely to change!
 *
 * The value of *pnumItems will be updated to reflect the new array size.
 **********************************************************************/
static int _AVCIncreaseSectionsArray(AVCE00Section **pasArray, int *pnumItems,
                                    int numToAdd)
{
    int i;

    *pasArray = (AVCE00Section*)CPLRealloc(*pasArray, 
                                           (*pnumItems+numToAdd)*
                                                    sizeof(AVCE00Section));

    for(i=0; i<numToAdd; i++)
    {
        (*pasArray)[*pnumItems+i].eType = AVCFileUnknown;
        (*pasArray)[*pnumItems+i].pszName = NULL;
    }

    i = *pnumItems;
    (*pnumItems) += numToAdd;

    return i;
}

/**********************************************************************
 *                         _AVCE00ReadFindCoverPrecision()
 *
 * This functions tries to find the coverage precision in one of the
 * coverage file headers that contain this information.
 * (i.e. ARC, PAL, CNT and LAB) ... if none of htese files can be
 * found then single precision is used by default... we could issue
 * a warning, but there is probably no need for one...
 **********************************************************************/
static int _AVCE00ReadFindCoverPrecision(AVCE00ReadPtr psInfo)
{
    int         i, eType, nPrec = AVC_SINGLE_PREC;
    AVCBinFile  *psFile;

    /*-----------------------------------------------------------------
     * Scan the pasSections array to locate a file that would contain
     * info about the coverage precision.
     *----------------------------------------------------------------*/
    for(i=0; i<psInfo->numSections; i++)
    {
        eType = psInfo->pasSections[i].eType;
        if (eType == AVCFileARC || eType == AVCFilePAL || 
            eType == AVCFileRPL ||
            eType == AVCFileCNT || eType == AVCFileLAB)
        {
            /* Found one!  Read its header ...
             */
            psFile = AVCBinReadOpen(psInfo->pszCoverPath, 
                                    psInfo->pasSections[i].pszName, eType);
            if (psFile)
            {
                nPrec = psFile->nPrecision;
                AVCBinReadClose(psFile);        
                break; /* Terminate the loop! */
            }
        }
    }

    return nPrec;
}


/**********************************************************************
 *                         _AVCE00ReadAddJabberwockySection()
 *
 * Add to the squeleton a section that contains subsections 
 * for all the files with a given extension.
 **********************************************************************/
static void _AVCE00ReadAddJabberwockySection(AVCE00ReadPtr psInfo,
                                             AVCFileType   eFileType,
                                             const char   *pszSectionName,
                                             char          cPrecisionCode,
                                             const char   *pszFileExtension,
                                             char        **papszCoverDir )
{
    int         iSect, iDirEntry, nLen, nExtLen;
    GBool       bFoundFiles = FALSE;

    nExtLen = strlen(pszFileExtension);

    /*-----------------------------------------------------------------
     * Scan the directory for files with a ".txt" extension.
     *----------------------------------------------------------------*/

    for (iDirEntry=0; papszCoverDir && papszCoverDir[iDirEntry]; iDirEntry++)
    {
        nLen = strlen(papszCoverDir[iDirEntry]);

        if (nLen > nExtLen && EQUAL(papszCoverDir[iDirEntry] + nLen-nExtLen, 
                                    pszFileExtension) )
        {

            if (bFoundFiles == FALSE)
            {
                /* Insert a "TX6 #" header before the first TX6 file
                 */
                iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                                  &(psInfo->numSections), 1);
                psInfo->pasSections[iSect].eType = AVCFileUnknown;

                psInfo->pasSections[iSect].pszName = 
                            CPLStrdup(CPLSPrintf("%s  %c", pszSectionName,
                                                 cPrecisionCode));

                bFoundFiles = TRUE;
            }

            /* Add this file to the squeleton 
             */
            iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                              &(psInfo->numSections), 1);

            psInfo->pasSections[iSect].eType = eFileType;
            psInfo->pasSections[iSect].pszName = 
                                   CPLStrdup(papszCoverDir[iDirEntry]);
        }
    }

    if (bFoundFiles)
    {
        /* Add a line to close the TX6 section.
         */
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                          &(psInfo->numSections), 1);
        psInfo->pasSections[iSect].eType = AVCFileUnknown;
        psInfo->pasSections[iSect].pszName = CPLStrdup("JABBERWOCKY");
    }

}

/**********************************************************************
 *                         _AVCE00ReadBuildSqueleton()
 *
 * Build the squeleton of the E00 file corresponding to the specified
 * coverage and set the appropriate fields in the AVCE00ReadPtr struct.
 *
 * Note that the order of the sections in the squeleton is important
 * since some software may rely on this ordering when they read E00 files.
 *
 * The function returns the coverage precision that it will read from one
 * of the file headers.  
 **********************************************************************/
static int _AVCE00ReadBuildSqueleton(AVCE00ReadPtr psInfo)
{
    int         iSect, iTable, numTables;
    char      **papszTables, **papszCoverDir, szCWD[75]="", *pcTmp;
    char       *pszEXPPath=NULL;
    int         nCoverPrecision;
    char        cPrecisionCode = '2';
    

    papszCoverDir = CPLReadDir(psInfo->pszCoverPath);

    psInfo->numSections = 0;
    psInfo->pasSections = NULL;

    /*-----------------------------------------------------------------
     * Build the absolute coverage path to include in the EXP  0 line
     * This line usually contains the full path of the E00 file that
     * is being created, but since the lib does not write the output
     * file directly, there is no simple way to get that value.  Instead,
     * we will use the absolute coverage path to which we add a .E00
     * extension.
     * We need also make sure cover path is all in uppercase.
     *----------------------------------------------------------------*/
#ifdef WIN32
    if (psInfo->pszCoverPath[0] != '\\' &&
        !(isalpha(psInfo->pszCoverPath[0]) && psInfo->pszCoverPath[1] == ':'))
#else
    if (psInfo->pszCoverPath[0] != '/')
#endif
    {
        int nLen;
        if (getcwd(szCWD, 74) == NULL)
            szCWD[0] = '\0';    /* Failed: buffer may be too small */

        nLen = strlen(szCWD);

#ifdef WIN32
        if (nLen > 0 && szCWD[nLen -1] != '\\')
            strcat(szCWD, "\\");
#else
        if (nLen > 0 && szCWD[nLen -1] != '/')
            strcat(szCWD, "/");
#endif
    }

    pszEXPPath = CPLStrdup(CPLSPrintf("EXP  0 %s%-.*s.E00", szCWD,
                                      strlen(psInfo->pszCoverPath)-1,
                                      psInfo->pszCoverPath));
    pcTmp = pszEXPPath;
    for( ; *pcTmp != '\0'; pcTmp++)
        *pcTmp = toupper(*pcTmp);

    /*-----------------------------------------------------------------
     * EXP Header
     *----------------------------------------------------------------*/
    iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                  &(psInfo->numSections), 1);
    psInfo->pasSections[iSect].eType = AVCFileUnknown;
    psInfo->pasSections[iSect].pszName = pszEXPPath;

    /*-----------------------------------------------------------------
     * ARC section (arc.adf)
     *----------------------------------------------------------------*/
    if (_AVCFileExists(psInfo->pszCoverPath, "arc.adf"))
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), 1);

        psInfo->pasSections[iSect].eType = AVCFileARC;
        psInfo->pasSections[iSect].pszName = CPLStrdup("arc.adf");
    }

    /*-----------------------------------------------------------------
     * CNT section (cnt.adf)
     *----------------------------------------------------------------*/
    if (_AVCFileExists(psInfo->pszCoverPath, "cnt.adf"))
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), 1);

        psInfo->pasSections[iSect].eType = AVCFileCNT;
        psInfo->pasSections[iSect].pszName = CPLStrdup("cnt.adf");
    }

    /*-----------------------------------------------------------------
     * LAB section (lab.adf)
     *----------------------------------------------------------------*/
    if (_AVCFileExists(psInfo->pszCoverPath, "lab.adf"))
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), 1);

        psInfo->pasSections[iSect].eType = AVCFileLAB;
        psInfo->pasSections[iSect].pszName = CPLStrdup("lab.adf");
    }

    /*-----------------------------------------------------------------
     * PAL section (pal.adf)
     *----------------------------------------------------------------*/
    if (_AVCFileExists(psInfo->pszCoverPath, "pal.adf"))
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), 1);

        psInfo->pasSections[iSect].eType = AVCFilePAL;
        psInfo->pasSections[iSect].pszName = CPLStrdup("pal.adf");
    }

    /*-----------------------------------------------------------------
     * Read the coverage precision... we get this info from one of
     * the files above, and we'll need it for some sections headers below.
     *----------------------------------------------------------------*/
    nCoverPrecision = _AVCE00ReadFindCoverPrecision(psInfo);
    if (nCoverPrecision == AVC_DOUBLE_PREC)
        cPrecisionCode = '3';
    else
        cPrecisionCode = '2';

    /*-----------------------------------------------------------------
     * Read the coverage precision... we get this info from one of
     * the files above, and we'll need it for some sections headers below.
     *----------------------------------------------------------------*/
    nCoverPrecision = _AVCE00ReadFindCoverPrecision(psInfo);
    if (nCoverPrecision == AVC_DOUBLE_PREC)
        cPrecisionCode = '3';
    else
        cPrecisionCode = '2';

    /*-----------------------------------------------------------------
     * TOL section (tol.adf for single precision, par.adf for double)
     *----------------------------------------------------------------*/
    if (_AVCFileExists(psInfo->pszCoverPath, "tol.adf"))
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), 1);

        psInfo->pasSections[iSect].eType = AVCFileTOL;
        psInfo->pasSections[iSect].pszName = CPLStrdup("tol.adf");
    }

    if (_AVCFileExists(psInfo->pszCoverPath, "par.adf"))
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), 1);

        psInfo->pasSections[iSect].eType = AVCFileTOL;
        psInfo->pasSections[iSect].pszName = CPLStrdup("par.adf");
    }

    /*-----------------------------------------------------------------
     * TXT section (txt.adf)
     *----------------------------------------------------------------*/
    if (_AVCFileExists(psInfo->pszCoverPath, "txt.adf"))
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), 1);

        psInfo->pasSections[iSect].eType = AVCFileTXT;
        psInfo->pasSections[iSect].pszName = CPLStrdup("txt.adf");
    }

    /*-----------------------------------------------------------------
     * TX6 section (*.txt)
     * Scan the directory for files with a ".txt" extension.
     *----------------------------------------------------------------*/
    _AVCE00ReadAddJabberwockySection(psInfo, AVCFileTX6, "TX6", 
                                     cPrecisionCode, ".txt", papszCoverDir);

    /*-----------------------------------------------------------------
     * SIN  2/3 and EOX lines ... ???
     *----------------------------------------------------------------*/
    iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                  &(psInfo->numSections), 2);
    psInfo->pasSections[iSect].eType = AVCFileUnknown;
    psInfo->pasSections[iSect].pszName = CPLStrdup("SIN  X");
    psInfo->pasSections[iSect].pszName[5] = cPrecisionCode;
    iSect++;
    psInfo->pasSections[iSect].eType = AVCFileUnknown;
    psInfo->pasSections[iSect].pszName = CPLStrdup("EOX");
    iSect++;

    /*-----------------------------------------------------------------
     * LOG section (log.adf) (ends with EOL)
     *----------------------------------------------------------------*/

    /*-----------------------------------------------------------------
     * PRJ section (prj.adf) (ends with EOP)
     *----------------------------------------------------------------*/
    if (_AVCFileExists(psInfo->pszCoverPath, "prj.adf"))
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), 1);

        psInfo->pasSections[iSect].eType = AVCFilePRJ;
        psInfo->pasSections[iSect].pszName = CPLStrdup("prj.adf");
    }

    /*-----------------------------------------------------------------
     * RXP section (*.rxp)
     * Scan the directory for files with a ".rxp" extension.
     *----------------------------------------------------------------*/
    _AVCE00ReadAddJabberwockySection(psInfo, AVCFileRXP, "RXP", 
                                     cPrecisionCode, ".rxp", papszCoverDir);


    /*-----------------------------------------------------------------
     * RPL section (*.pal)
     * Scan the directory for files with a ".rpl" extension.
     *----------------------------------------------------------------*/
    _AVCE00ReadAddJabberwockySection(psInfo, AVCFileRPL, "RPL", 
                                     cPrecisionCode, ".pal", papszCoverDir);

    /*-----------------------------------------------------------------
     * IFO section (tables)
     *----------------------------------------------------------------*/
    papszTables = AVCBinReadListTables(psInfo->pszInfoPath, 
                                       psInfo->pszCoverName,
                                       NULL);

    if ((numTables = CSLCount(papszTables)) > 0)
    {
        iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                      &(psInfo->numSections), numTables+2);

        psInfo->pasSections[iSect].eType = AVCFileUnknown;
        psInfo->pasSections[iSect].pszName = CPLStrdup("IFO  X");
        psInfo->pasSections[iSect].pszName[5] = cPrecisionCode;
        iSect++;

        for(iTable=0; iTable<numTables; iTable++)
        {
            psInfo->pasSections[iSect].eType = AVCFileTABLE;
            psInfo->pasSections[iSect].pszName=CPLStrdup(papszTables[iTable]);
            iSect++;
        }

        psInfo->pasSections[iSect].eType = AVCFileUnknown;
        psInfo->pasSections[iSect].pszName = CPLStrdup("EOI");
        iSect++;

        
        CSLDestroy(papszTables);
    }

    /*-----------------------------------------------------------------
     * File ends with EOS
     *----------------------------------------------------------------*/
    iSect = _AVCIncreaseSectionsArray(&(psInfo->pasSections), 
                                  &(psInfo->numSections), 1);
    psInfo->pasSections[iSect].eType = AVCFileUnknown;
    psInfo->pasSections[iSect].pszName = CPLStrdup("EOS");


    CSLDestroy(papszCoverDir);

    return nCoverPrecision;
}


/**********************************************************************
 *                         _AVCE00ReadNextTableLine()
 *
 * Return the next line of the E00 representation of a info table.
 *
 * This function is used by AVCE00ReadNextLine() to generate table
 * output... it should never be called directly.
 **********************************************************************/
static const char *_AVCE00ReadNextTableLine(AVCE00ReadPtr psInfo)
{
    const char *pszLine = NULL;
    AVCE00Section *psSect;

    psSect = &(psInfo->pasSections[psInfo->iCurSection]);

    CPLAssert(psSect->eType == AVCFileTABLE);

    if (psInfo->iCurStep == AVC_GEN_NOTSTARTED)
    {
        /*---------------------------------------------------------
         * Open table and start returning header
         *--------------------------------------------------------*/
        psInfo->hFile = AVCBinReadOpen(psInfo->pszInfoPath, 
                                       psSect->pszName, psSect->eType);

        /* For some reason the file could not be opened... abort now.
         * An error message should have already been produced by 
         * AVCBinReadOpen()
         */
        if (psInfo->hFile == NULL)
            return NULL;

        psInfo->iCurStep = AVC_GEN_TABLEHEADER;

        pszLine = AVCE00GenTableHdr(psInfo->hGenInfo,
                                    psInfo->hFile->hdr.psTableDef,
                                    FALSE);
    }
        
    if (pszLine == NULL &&
        psInfo->iCurStep == AVC_GEN_TABLEHEADER)
    {
        /*---------------------------------------------------------
         * Continue table header
         *--------------------------------------------------------*/
        pszLine = AVCE00GenTableHdr(psInfo->hGenInfo,
                                    psInfo->hFile->hdr.psTableDef,
                                    TRUE);

        if (pszLine == NULL)
        {
            /* Finished with table header... time to proceed with the
             * table data.
             * Reset the AVCE00GenInfo struct. so that it returns NULL,
             * which will force reading of the first record from the 
             * file on the next call to AVCE00ReadNextLine()
             */
            AVCE00GenReset(psInfo->hGenInfo);
            psInfo->iCurStep = AVC_GEN_TABLEDATA;
        }

    }

    if (pszLine == NULL &&
        psInfo->iCurStep == AVC_GEN_TABLEDATA)
    {
        /*---------------------------------------------------------
         * Continue with records of data
         *--------------------------------------------------------*/

        pszLine = AVCE00GenTableRec(psInfo->hGenInfo, 
                                    psInfo->hFile->hdr.psTableDef->numFields,
                                    psInfo->hFile->hdr.psTableDef->pasFieldDef,
                                    psInfo->hFile->cur.pasFields,
                                    TRUE);

        if (pszLine == NULL)
        {
            /* Current record is finished generating... we need to read 
             * a new one from the file.
             */
            if (AVCBinReadNextObject(psInfo->hFile) != NULL)
            {
                pszLine = AVCE00GenTableRec(psInfo->hGenInfo, 
                                    psInfo->hFile->hdr.psTableDef->numFields,
                                    psInfo->hFile->hdr.psTableDef->pasFieldDef,
                                    psInfo->hFile->cur.pasFields,
                                    FALSE);
            }            

        }
    }

    if (pszLine == NULL)
    {
        /*---------------------------------------------------------
         * No more lines to output for this table ... Close it.
         *--------------------------------------------------------*/
        AVCBinReadClose(psInfo->hFile);
        psInfo->hFile = NULL;

        /*---------------------------------------------------------
         * And now proceed to the next section...
         * OK, I don't really like recursivity either... but it was
         * the simplest way to do this, and anyways we should never
         * have more than one level of recursivity.
         *--------------------------------------------------------*/
        if (psInfo->bReadAllSections)
            psInfo->iCurSection++;
        else
            psInfo->iCurSection = psInfo->numSections;
        psInfo->iCurStep = AVC_GEN_NOTSTARTED;

        pszLine = AVCE00ReadNextLine(psInfo);
    }

    /*-----------------------------------------------------------------
     * Check for errors... if any error happened, tehn return NULL
     *----------------------------------------------------------------*/
    if (CPLGetLastErrorNo() != 0)
    {
        pszLine = NULL;
    }

    return pszLine;
}


/**********************************************************************
 *                          AVCE00ReadNextLine()
 *
 * Returns the next line of the E00 representation of the coverage
 * or NULL when there are no more lines to generate, or if an error happened.
 * The returned line is a null-terminated string, and it does not
 * include a newline character.
 *
 * Call CPLGetLastErrorNo() after calling AVCE00ReadNextLine() to 
 * make sure that the line was generated succesfully.
 *
 * Note that AVCE00ReadNextLine() returns a reference to an
 * internal buffer whose contents will
 * be valid only until the next call to this function.  The caller should
 * not attempt to free() the returned pointer.
 **********************************************************************/
const char *AVCE00ReadNextLine(AVCE00ReadPtr psInfo)
{
    const char *pszLine = NULL;
    AVCE00Section *psSect;

    CPLErrorReset();

    /*-----------------------------------------------------------------
     * Check if we have finished generating E00 output
     *----------------------------------------------------------------*/
    if (psInfo->iCurSection >= psInfo->numSections)
        return NULL;

    psSect = &(psInfo->pasSections[psInfo->iCurSection]);

    /*-----------------------------------------------------------------
     * For simplicity, the generation of table output is in a separate
     * function.
     *----------------------------------------------------------------*/
    if (psSect->eType == AVCFileTABLE)
    {
        return _AVCE00ReadNextTableLine(psInfo);
    }

    if (psSect->eType == AVCFileUnknown)
    {
    /*-----------------------------------------------------------------
     * Section not attached to any file, used to hold header lines
     * or section separators, etc... just return the line directly and
     * move pointer to the next section.
     *----------------------------------------------------------------*/
        pszLine = psSect->pszName;
        if (psInfo->bReadAllSections)
            psInfo->iCurSection++;
        else
            psInfo->iCurSection = psInfo->numSections;
        psInfo->iCurStep = AVC_GEN_NOTSTARTED;
    }
    /*=================================================================
     *              ARC, PAL, CNT, LAB, TOL and TXT
     *================================================================*/
    else if (psInfo->iCurStep == AVC_GEN_NOTSTARTED &&
             (psSect->eType == AVCFileARC ||
              psSect->eType == AVCFilePAL ||
              psSect->eType == AVCFileRPL ||
              psSect->eType == AVCFileCNT ||
              psSect->eType == AVCFileLAB ||
              psSect->eType == AVCFileTOL ||
              psSect->eType == AVCFileTXT ||
              psSect->eType == AVCFileTX6 ||
              psSect->eType == AVCFileRXP   ) )
    {
    /*-----------------------------------------------------------------
     * Start processing of an ARC, PAL, CNT, LAB or TOL section:
     *   Open the file, get ready to read the first object from the 
     *   file, and return the header line.
     *  If the file fails to open then we will return NULL.
     *----------------------------------------------------------------*/
        psInfo->hFile = AVCBinReadOpen(psInfo->pszCoverPath, 
                                       psSect->pszName, psSect->eType);

        /*-------------------------------------------------------------
         * For some reason the file could not be opened... abort now.
         * An error message should have already been produced by 
         * AVCBinReadOpen()
         *------------------------------------------------------------*/
        if (psInfo->hFile == NULL)
            return NULL;

        pszLine = AVCE00GenStartSection(psInfo->hGenInfo, 
                                        psSect->eType, psSect->pszName);

        /*-------------------------------------------------------------
         * Reset the AVCE00GenInfo struct. so that it returns NULL,
         * which will force reading of the first object from the 
         * file on the next call to AVCE00ReadNextLine()
         *------------------------------------------------------------*/
        AVCE00GenReset(psInfo->hGenInfo);
        psInfo->iCurStep = AVC_GEN_DATA;
    }
    else if (psInfo->iCurStep == AVC_GEN_DATA &&
             (psSect->eType == AVCFileARC ||
              psSect->eType == AVCFilePAL ||
              psSect->eType == AVCFileRPL ||
              psSect->eType == AVCFileCNT ||
              psSect->eType == AVCFileLAB ||
              psSect->eType == AVCFileTOL ||
              psSect->eType == AVCFileTXT ||
              psSect->eType == AVCFileTX6 ||
              psSect->eType == AVCFileRXP    ) )
    {
    /*-----------------------------------------------------------------
     * Return the next line of an ARC/PAL/CNT/TOL/TXT object... 
     * if necessary, read the next object from the binary file.
     *----------------------------------------------------------------*/
        pszLine = AVCE00GenObject(psInfo->hGenInfo, 
                                  psSect->eType,
                  (psSect->eType==AVCFileARC?(void*)(psInfo->hFile->cur.psArc):
                   psSect->eType==AVCFilePAL?(void*)(psInfo->hFile->cur.psPal):
                   psSect->eType==AVCFileRPL?(void*)(psInfo->hFile->cur.psPal):
                   psSect->eType==AVCFileCNT?(void*)(psInfo->hFile->cur.psCnt):
                   psSect->eType==AVCFileLAB?(void*)(psInfo->hFile->cur.psLab):
                   psSect->eType==AVCFileTOL?(void*)(psInfo->hFile->cur.psTol):
                   psSect->eType==AVCFileTXT?(void*)(psInfo->hFile->cur.psTxt):
                   psSect->eType==AVCFileTX6?(void*)(psInfo->hFile->cur.psTxt):
                   psSect->eType==AVCFileRXP?(void*)(psInfo->hFile->cur.psRxp):
                   NULL),
                                  TRUE);
        if (pszLine == NULL)
        {
            /*---------------------------------------------------------
             * Current object is finished generating... we need to read 
             * a new one from the file.
             *--------------------------------------------------------*/
            if (AVCBinReadNextObject(psInfo->hFile) != NULL)
            {
                pszLine = AVCE00GenObject(psInfo->hGenInfo, 
                                          psSect->eType,
                  (psSect->eType==AVCFileARC?(void*)(psInfo->hFile->cur.psArc):
                   psSect->eType==AVCFilePAL?(void*)(psInfo->hFile->cur.psPal):
                   psSect->eType==AVCFileRPL?(void*)(psInfo->hFile->cur.psPal):
                   psSect->eType==AVCFileCNT?(void*)(psInfo->hFile->cur.psCnt):
                   psSect->eType==AVCFileLAB?(void*)(psInfo->hFile->cur.psLab):
                   psSect->eType==AVCFileTOL?(void*)(psInfo->hFile->cur.psTol):
                   psSect->eType==AVCFileTXT?(void*)(psInfo->hFile->cur.psTxt):
                   psSect->eType==AVCFileTX6?(void*)(psInfo->hFile->cur.psTxt):
                   psSect->eType==AVCFileRXP?(void*)(psInfo->hFile->cur.psRxp):
                   NULL),
                                          FALSE);
            }            
        }
        if (pszLine == NULL)
        {
            /*---------------------------------------------------------
             * Still NULL ??? This means we finished reading this file...
             * Start returning the "end of section" line(s)...
             *--------------------------------------------------------*/
            AVCBinReadClose(psInfo->hFile);
            psInfo->hFile = NULL;
            psInfo->iCurStep = AVC_GEN_ENDSECTION;
            pszLine = AVCE00GenEndSection(psInfo->hGenInfo, psSect->eType,
                                          FALSE);
        }
    }
    /*=================================================================
     *                          PRJ
     *================================================================*/
    else if (psInfo->iCurStep == AVC_GEN_NOTSTARTED &&
              psSect->eType == AVCFilePRJ   )
    {
        /*-------------------------------------------------------------
         * Start processing of PRJ section... return first header line.
         *------------------------------------------------------------*/
        pszLine = AVCE00GenStartSection(psInfo->hGenInfo, 
                                        psSect->eType, psSect->pszName);

        psInfo->hFile = NULL;
        psInfo->iCurStep = AVC_GEN_DATA;
    }
    else if (psInfo->iCurStep == AVC_GEN_DATA &&
             psSect->eType == AVCFilePRJ  )
    {
        /*-------------------------------------------------------------
         * Return the next line of a PRJ section
         *------------------------------------------------------------*/
        if (psInfo->hFile == NULL)
        {
            /*---------------------------------------------------------
             * File has not been read yet...
             * Read the PRJ file, and return the first PRJ line.
             *--------------------------------------------------------*/
            psInfo->hFile = AVCBinReadOpen(psInfo->pszCoverPath, 
                                           psSect->pszName, psSect->eType);

            /* For some reason the file could not be opened... abort now.
             * An error message should have already been produced by 
             * AVCBinReadOpen()
             */
            if (psInfo->hFile == NULL)
                return NULL;

            pszLine = AVCE00GenPrj(psInfo->hGenInfo, 
                                   psInfo->hFile->cur.papszPrj, FALSE);
        }
        else
        {
            /*---------------------------------------------------------
             * Generate the next line of output.
             *--------------------------------------------------------*/
            pszLine = AVCE00GenPrj(psInfo->hGenInfo, 
                                   psInfo->hFile->cur.papszPrj, TRUE);
        }

        if (pszLine == NULL)
        {
            /*---------------------------------------------------------
             * Still NULL ??? This means we finished generating this PRJ 
             * section...
             * Start returning the "end of section" line(s)...
             *--------------------------------------------------------*/
            AVCBinReadClose(psInfo->hFile);
            psInfo->hFile = NULL;
            psInfo->iCurStep = AVC_GEN_ENDSECTION;
            pszLine = AVCE00GenEndSection(psInfo->hGenInfo, psSect->eType,
                                          FALSE);
        }
    }
    else if (psInfo->iCurStep != AVC_GEN_ENDSECTION)
    {
        /* We should never get here! */
        CPLAssert(FALSE);
    }


    /*=================================================================
     *                End of section, for all files
     *================================================================*/

    /*-----------------------------------------------------------------
     * Finished processing of an ARC, PAL, CNT, LAB, TOL, PRJ file ...
     * continue returning the "end of section" line(s), and move the pointer
     * to the next section once we're done.
     *----------------------------------------------------------------*/
    if (psInfo->iCurStep == AVC_GEN_ENDSECTION && pszLine == NULL)
    {
        pszLine = AVCE00GenEndSection(psInfo->hGenInfo, psSect->eType, TRUE);

        if (pszLine == NULL)
        {
            /*---------------------------------------------------------
             * Finished returning the last lines of the section...
             * proceed to the next section...
             * OK, I don't really like recursivity either... but it was
             * the simplest way to do this, and anyways we should never
             * have more than one level of recursivity.
             *--------------------------------------------------------*/
            if (psInfo->bReadAllSections)
                psInfo->iCurSection++;
            else
                psInfo->iCurSection = psInfo->numSections;
            psInfo->iCurStep = AVC_GEN_NOTSTARTED;

            pszLine = AVCE00ReadNextLine(psInfo);
        }
    }

    return pszLine;
}



/**********************************************************************
 *                         AVCE00ReadSectionsList()
 *
 * Returns an array of AVCE00Section structures that describe the
 * squeleton of the whole coverage.  The value of *numSect will be
 * set to the number of sections in the array.
 *
 * You can scan the returned array, and use AVCE00ReadGotoSection() to move
 * the read pointer directly to the beginning of a given section
 * of the file.
 *
 * Sections of type AVCFileUnknown correspond to lines in the
 * E00 output that are not directly linked to any coverage file, like 
 * the "EXP 0" line, the "IFO X", "SIN X", etc.
 *
 * THE RETURNED ARRAY IS AN INTERNAL STRUCTURE AND SHOULD NOT BE
 * MODIFIED OR FREED BY THE CALLER... its contents will be valid
 * for as long as the coverage will remain open.
 **********************************************************************/
AVCE00Section *AVCE00ReadSectionsList(AVCE00ReadPtr psInfo, int *numSect)
{
    CPLErrorReset();

    *numSect = psInfo->numSections;
    return psInfo->pasSections;
}

/**********************************************************************
 *                         AVCE00ReadGotoSection()
 *
 * Move the read pointer to the E00 section (coverage file) described in 
 * the psSect structure.  Call AVCE00ReadListSections() to get the list of
 * sections for the current coverage.
 *
 * if bContinue=TRUE, then reading will automatically continue with the
 * next sections of the file once the requested section is finished.
 * Otherwise, if bContinue=FALSE then reading will stop at the end
 * of this section (i.e. AVCE00ReadNextLine() will return NULL when 
 * it reaches the end of this section)
 *
 * Sections of type AVCFileUnknown returned by AVCE00ReadListSections()
 * correspond to lines in the E00 output that are not directly linked
 * to any coverage file, like the "EXP 0" line, the "IFO X", "SIN X", etc.
 * You can jump to these sections or any other one without problems.
 *
 * This function returns 0 on success or -1 on error.
 **********************************************************************/
int AVCE00ReadGotoSection(AVCE00ReadPtr psInfo, AVCE00Section *psSect,
                          GBool bContinue)
{
    int     iSect;
    GBool   bFound = FALSE;

    CPLErrorReset();

    /*-----------------------------------------------------------------
     * Locate the requested section in the array.
     *----------------------------------------------------------------*/
    for(iSect=0; iSect<psInfo->numSections; iSect++)
    {
        if (psInfo->pasSections[iSect].eType == psSect->eType &&
            EQUAL(psInfo->pasSections[iSect].pszName, psSect->pszName))
        {
            bFound = TRUE;
            break;
        }
    }

    /*-----------------------------------------------------------------
     * Not found ... generate an error...
     *----------------------------------------------------------------*/
    if (!bFound)
    {
        CPLError(CE_Failure, CPLE_IllegalArg, 
                 "Requested E00 section does not exist!");
        return -1;
    }

    /*-----------------------------------------------------------------
     * Found it ... close current section and get ready to read 
     * the new one.
     *----------------------------------------------------------------*/
    if (psInfo->hFile)
    {
        AVCBinReadClose(psInfo->hFile);
        psInfo->hFile = NULL;
    }

    psInfo->bReadAllSections = bContinue;
    psInfo->iCurSection = iSect;
    psInfo->iCurStep = AVC_GEN_NOTSTARTED;

    return 0;
}

/**********************************************************************
 *                         AVCE00ReadRewind()
 *
 * Rewinds the AVCE00ReadPtr just like the stdio rewind() 
 * function would do if you were reading an ASCII E00 file.
 *
 * Returns 0 on success or -1 on error.
 **********************************************************************/
int  AVCE00ReadRewind(AVCE00ReadPtr psInfo)
{
    CPLErrorReset();

    return AVCE00ReadGotoSection(psInfo, &(psInfo->pasSections[0]), TRUE);
}
