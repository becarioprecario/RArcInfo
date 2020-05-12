/**********************************************************************
 * $Id: avcdelete.c,v 1.1.1.1 2001/06/27 20:10:56 vir Exp $
 *
 * Name:     avcdelete.c
 * Project:  Arc/Info Vector coverage (AVC) BIN<->E00 conversion library
 * Language: ANSI C
 * Purpose:  Delete an Arc/Info binary coverage.
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
 * $Log: avcdelete.c,v $
 * Revision 1.1.1.1  2001/06/27 20:10:56  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:13  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.2  1999/12/05 05:28:49  daniel
 * Updated usage info with build date
 *
 * Revision 1.1  1999/08/23 17:15:48  daniel
 * *** empty log message ***
 *
 **********************************************************************/

#include "avc.h"

/**********************************************************************
 *                          main()
 *
 * This program properly deletes an Arc/Info vector coverage and 
 * associated INFO tables.
 **********************************************************************/
int main(int argc, char *argv[])
{
    const char  *pszCoverPath;

/*---------------------------------------------------------------------
 *      Read program arguments.
 *--------------------------------------------------------------------*/
    if (argc != 2)
    {
        printf("\n");
        printf("AVCDELETE - (Build date: %s %s)\n", __DATE__, __TIME__);
        printf("     Delete an Arc/Info vector coverage and associated INFO files\n");
        printf("     Copyright (c) 1999, Daniel Morissette (danmo@videotron.ca)\n");
        printf("     AVCE00 web page:  http://pages.infinit.net/danmo/e00/\n");
        printf("\n");
        printf("Usage: avcdelete <coverage_path>\n");
        printf("\n");
        return 1;
    }

    pszCoverPath = argv[1];

/*---------------------------------------------------------------------
 *     Delete requested coverage
 *--------------------------------------------------------------------*/

    return AVCE00DeleteCoverage(pszCoverPath);
}
