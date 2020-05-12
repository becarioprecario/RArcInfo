/**********************************************************************
 * $Id: avcimport.c,v 1.1.1.1 2001/06/27 20:10:56 vir Exp $
 *
 * Name:     avcimport.c
 * Project:  Arc/Info Vector coverage (AVC) BIN<->E00 conversion library
 * Language: ANSI C
 * Purpose:  Convert an E00 file to an Arc/Info binary coverage.
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
 * $Log: avcimport.c,v $
 * Revision 1.1.1.1  2001/06/27 20:10:56  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:13  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.3  1999/12/05 05:42:14  daniel
 * Updated usage info with build date
 *
 * Revision 1.2  1999/05/17 16:25:02  daniel
 * Use AVC_DEFAULT_PREC
 *
 * Revision 1.1  1999/05/11 02:11:49  daniel
 * Initial revision
 *
 **********************************************************************/

#include "avc.h"


static void ConvertCover(FILE *fpIn, const char *pszCoverName );

/**********************************************************************
 *                          main()
 *
 * This program converts an Arc/Info vector coverage from the native
 * binary format to E00.
 **********************************************************************/
int main(int argc, char *argv[])
{
    const char  *pszCoverPath, *pszInFile;
    FILE        *fpIn;

/*---------------------------------------------------------------------
 *      Read program arguments.
 *--------------------------------------------------------------------*/
    if (argc<3)
    {
        printf("\n");
        printf("AVCIMPORT - (Build date: %s %s)\n", __DATE__, __TIME__);
        printf("     Convert an Arc/Info vector coverage from E00 to binary.\n");
        printf("     Copyright (c) 1999, Daniel Morissette (danmo@videotron.ca)\n");
        printf("     AVCE00 web page:  http://pages.infinit.net/danmo/e00/\n");
        printf("\n");
        printf("Usage: avcimport <input_file> <coverage_path>\n");
        printf("\n");
        return 1;
    }

    pszInFile   = argv[1];
    pszCoverPath = argv[2];

/*---------------------------------------------------------------------
 *     Open output file... passing "-" will take input from stdin
 *-------------------------------------------------------------------*/
    if (strcmp(pszInFile, "-") == 0)
    {
        fpIn = stdin;
    }
    else
    {
        fpIn = fopen(pszInFile, "rt");

        if (fpIn == NULL)
        {
            perror(CPLSPrintf("avcimport: Cannot open %s", pszInFile));
            return 1;
        }
    }

/*---------------------------------------------------------------------
 *     Convert the whole E00 file to a binary coverage 
 *-------------------------------------------------------------------*/
    ConvertCover(fpIn, pszCoverPath);

/*---------------------------------------------------------------------
 *     Close input file and exit.
 *-------------------------------------------------------------------*/
    if (strcmp(pszInFile, "-") != 0)
    {
        fclose(fpIn);
    }

    return 0;
}


/**********************************************************************
 *                          ConvertCover()
 *
 * Create a binary coverage from an E00 file.
 *
 * It would be possible to have an option for the precision... coming soon!
 **********************************************************************/
static void ConvertCover(FILE *fpIn, const char *pszCoverName)
{
    AVCE00WritePtr hWriteInfo;
    const char *pszLine;

    hWriteInfo = AVCE00WriteOpen(pszCoverName, AVC_DEFAULT_PREC);

    if (hWriteInfo)
    {
        while (CPLGetLastErrorNo() == 0 &&
               (pszLine = CPLReadLine(fpIn) ) != NULL )
        {
            AVCE00WriteNextLine(hWriteInfo, pszLine);
        }

        AVCE00WriteClose(hWriteInfo);
    }
}
