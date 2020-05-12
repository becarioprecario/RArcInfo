/**********************************************************************
 * $Id: avcexport.c,v 1.1.1.1 2001/06/27 20:10:56 vir Exp $
 *
 * Name:     avcexport.c
 * Project:  Arc/Info Vector coverage (AVC) BIN->E00 conversion library
 * Language: ANSI C
 * Purpose:  Convert an Arc/Info binary coverage to E00.
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
 * $Log: avcexport.c,v $
 * Revision 1.1.1.1  2001/06/27 20:10:56  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:13  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.5  1999/12/05 05:28:59  daniel
 * Updated usage info with build date
 *
 * Revision 1.4  1999/05/11 02:11:35  daniel
 * Renamed from avcconv.c to avcexport.c
 *
 * Revision 1.3  1999/02/25 03:34:48  daniel
 * Fixed the Revsion field in the "usage" output
 *
 * Revision 1.2  1999/02/25 03:32:25  daniel
 * Added support for output_file arg.
 *
 **********************************************************************/

#include "avc.h"


static void ConvertCover(const char *pszFname, FILE *fpOut);

/**********************************************************************
 *                          main()
 *
 * This program converts an Arc/Info vector coverage from the native
 * binary format to E00.
 **********************************************************************/
int main(int argc, char *argv[])
{
    const char  *pszCoverPath, *pszOutFile;
    FILE        *fpOut;

/*---------------------------------------------------------------------
 *      Read program arguments.
 *--------------------------------------------------------------------*/
    if (argc<3)
    {
        printf("\n");
        printf("AVCEXPORT - (Build date: %s %s)\n", __DATE__, __TIME__);
        printf("     Convert an Arc/Info vector coverage from binary to E00.\n");
        printf("     Copyright (c) 1999, Daniel Morissette (danmo@videotron.ca)\n");
        printf("     AVCE00 web page:  http://pages.infinit.net/danmo/e00/\n");
        printf("\n");
        printf("Usage: avcexport <coverage_path> <output_file>\n");
        printf("\n");
        return 1;
    }

    pszCoverPath = argv[1];
    pszOutFile   = argv[2];

/*---------------------------------------------------------------------
 *     Open output file... passing "-" will send output to stdout
 *-------------------------------------------------------------------*/
    if (strcmp(pszOutFile, "-") == 0)
    {
        fpOut = stdout;
    }
    else
    {
        fpOut = fopen(pszOutFile, "wt");

        if (fpOut == NULL)
        {
            perror(CPLSPrintf("avcexport: Cannot create %s", pszOutFile));
            return 1;
        }
    }

/*---------------------------------------------------------------------
 *     Convert the whole coverage to E00
 *-------------------------------------------------------------------*/
    ConvertCover(pszCoverPath, fpOut);

/*---------------------------------------------------------------------
 *     Close output file and exit.
 *-------------------------------------------------------------------*/
    if (strcmp(pszOutFile, "-") != 0)
    {
        fclose(fpOut);
    }

    return 0;
}


/**********************************************************************
 *                          ConvertCover()
 *
 * Convert a complete coverage to E00.
 **********************************************************************/
static void ConvertCover(const char *pszFname, FILE *fpOut)
{
    AVCE00ReadPtr hReadInfo;
    const char *pszLine;

    hReadInfo = AVCE00ReadOpen(pszFname);

    if (hReadInfo)
    {
        while ((pszLine = AVCE00ReadNextLine(hReadInfo)) != NULL)
        {
            fprintf(fpOut, "%s\n", pszLine);
        }

        AVCE00ReadClose(hReadInfo);
    }
}
