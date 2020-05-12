/**********************************************************************
 * $Id: avc_misc.c,v 1.1.1.1 2001/06/27 20:10:53 vir Exp $
 *
 * Name:     avc_misc.c
 * Project:  Arc/Info vector coverage (AVC)  BIN<->E00 conversion library
 * Language: ANSI C
 * Purpose:  Misc. functions used by several parts of the library
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
 * $Log: avc_misc.c,v $
 * Revision 1.1.1.1  2001/06/27 20:10:53  vir
 * Initial release (0.1) under the cvs tree at Sourceforge.
 *
 * Revision 1.1.1.1  2001/06/27 20:04:10  vir
 * Initial release (0.1) under the CVS tree.
 *
 * Revision 1.2  1999/08/23 18:24:27  daniel
 * Fixed support for attribute fields of type 40
 *
 * Revision 1.1  1999/05/11 02:34:46  daniel
 * Initial revision
 *
 **********************************************************************/

#include "avc.h"


/**********************************************************************
 *                          AVCE00ComputeRecSize()
 *
 * Computes the number of chars required to generate a E00 attribute
 * table record.
 *
 * Returns -1 on error, i.e. if it encounters an unsupported field type.
 **********************************************************************/
int _AVCE00ComputeRecSize(int numFields, AVCFieldInfo *pasDef)
{
    int i, nType, nBufSize=0;

    /*------------------------------------------------------------- 
     * Add up the nbr of chars used by each field
     *------------------------------------------------------------*/
    for(i=0; i < numFields; i++)
    {
        nType = pasDef[i].nType1*10;
        if (nType ==  AVC_FT_DATE || nType == AVC_FT_CHAR ||
            nType == AVC_FT_FIXINT )
        {
            nBufSize += pasDef[i].nSize;
        }
        else if (nType == AVC_FT_BININT && pasDef[i].nSize == 4)
            nBufSize += 11;
        else if (nType == AVC_FT_BININT && pasDef[i].nSize == 2)
            nBufSize += 6;
        else if ((nType == AVC_FT_BINFLOAT && pasDef[i].nSize == 4) ||
                  nType == AVC_FT_FIXNUM )
            nBufSize += 14;
        else if (nType == AVC_FT_BINFLOAT && pasDef[i].nSize == 8)
            nBufSize += 24;
        else
        {
            /*-----------------------------------------------------
             * Hummm... unsupported field type...
             *----------------------------------------------------*/
            CPLError(CE_Failure, CPLE_NotSupported,
                     "_AVCE00ComputeRecSize(): Unsupported field type: "
                     "(type=%d, size=%d)",
                     nType, pasDef[i].nSize);
            return -1;
        }
    }

    return nBufSize;
}



/**********************************************************************
 *                          _AVCDestroyTableFields()
 *
 * Release all memory associated with an array of AVCField structures.
 **********************************************************************/
void _AVCDestroyTableFields(AVCTableDef *psTableDef, AVCField *pasFields)
{
    int     i, nFieldType;

    if (pasFields)
    {
        for(i=0; i<psTableDef->numFields; i++)
        {
            nFieldType = psTableDef->pasFieldDef[i].nType1*10;
            if (nFieldType == AVC_FT_DATE   ||
                nFieldType == AVC_FT_CHAR   ||
                nFieldType == AVC_FT_FIXINT ||
                nFieldType == AVC_FT_FIXNUM)
            {
                CPLFree(pasFields[i].pszStr);
            }
        }
        CPLFree(pasFields);
    }

}

/**********************************************************************
 *                          _AVCDestroyTableDef()
 *
 * Release all memory associated with a AVCTableDef structure.
 *
 **********************************************************************/
void _AVCDestroyTableDef(AVCTableDef *psTableDef)
{
    if (psTableDef)
    {
        CPLFree(psTableDef->pasFieldDef);
        CPLFree(psTableDef);
    }
}


/**********************************************************************
 *                          _AVCDupTableDef()
 *
 * Create a new copy of a AVCTableDef structure.
 **********************************************************************/
AVCTableDef *_AVCDupTableDef(AVCTableDef *psSrcDef)
{
    AVCTableDef *psNewDef;

    if (psSrcDef == NULL)
        return NULL;

    psNewDef = (AVCTableDef*)CPLMalloc(1*sizeof(AVCTableDef));

    memcpy(psNewDef, psSrcDef, sizeof(AVCTableDef));

    psNewDef->pasFieldDef = (AVCFieldInfo*)CPLMalloc(psSrcDef->numFields*
                                                     sizeof(AVCFieldInfo));

    memcpy(psNewDef->pasFieldDef, psSrcDef->pasFieldDef, 
           psSrcDef->numFields*sizeof(AVCFieldInfo));

   return psNewDef;
}
