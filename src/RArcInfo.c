#include<stdio.h>
#include<dirent.h>
#include"RArcInfo.h"
#include"avc.h"

#include<R.h>
#include<Rinternals.h>
#include<Rdefines.h>

int _AVCBinReadNextArcDir(AVCRawBinFile *psInfo, AVCTableDef *psArcDir);
AVCBinFile *_AVCBinReadOpenTable(const char *pszInfoPath, const char *pszTableName);


/*
It returns the table names and something more:
- Arc file
- Number of fields
- Register Size
- Number of registers
- External/Internal Table Identifier
*/
SEXP get_table_names(SEXP directory)
{
	SEXP *table, aux;
	AVCRawBinFile *arcfile;
	AVCTableDef tabledefaux;
	char arcdir[PATH], *dirname;
	int i,n, **idata;

	dirname= (char *) CHAR(STRING_ELT(directory,0));/*FIXME*/
	strcpy(arcdir,dirname);

	complete_path(arcdir,"arc.dir", 0);

	if(!(arcfile=AVCRawBinOpen(arcdir,"r")))
	{
		error("Error opening arc.dir");
	}

	n=0;
	while(!AVCRawBinEOF(arcfile))
	{
		if(!_AVCBinReadNextArcDir(arcfile, &tabledefaux))
			n++;
	}

	AVCRawBinFSeek(arcfile, 0,SEEK_SET);

	table=calloc(6, sizeof(SEXP));

	PROTECT(table[0]=NEW_STRING(n));
	PROTECT(table[1]=NEW_STRING(n));

	idata=calloc(4, sizeof(char *));
	PROTECT(table[2]=NEW_INTEGER(n));
	idata[0]=INTEGER(table[2]);
	PROTECT(table[3]=NEW_INTEGER(n));
	idata[1]=INTEGER(table[3]);
	PROTECT(table[4]=NEW_INTEGER(n));
	idata[2]=INTEGER(table[4]);
	PROTECT(table[5]=NEW_LOGICAL(n));
	idata[3]=LOGICAL(table[5]);


	i=0;
	while(!AVCRawBinEOF(arcfile))
	{
		if(_AVCBinReadNextArcDir(arcfile, &tabledefaux))
			break;


		SET_STRING_ELT(table[0],i,COPY_TO_USER_STRING(tabledefaux.szTableName));
		SET_STRING_ELT(table[1],i,COPY_TO_USER_STRING(tabledefaux.szInfoFile));

		idata[0][i]=tabledefaux.numFields;
		idata[1][i]=tabledefaux.nRecSize;
		idata[2][i]=tabledefaux.numRecords;
		if(!strcmp(tabledefaux.szExternal,"XX"))
			idata[3][i]=1;
		else
			idata[3][i]=0;

		i++;
	}

	PROTECT(aux=NEW_LIST(6));

	for(i=0;i<6;i++)
		SET_VECTOR_ELT(aux,i,table[i]);

	UNPROTECT(7);

	free(table);
	free(idata);

	return aux;
}


/*It returns the fields of the table (name and type)*/
SEXP get_table_fields(SEXP info_dir, SEXP table_name)
{
	int i, *idata;
	char infodir[PATH];
	SEXP *table, aux;
	AVCBinFile *tablefile;
	AVCTableDef *tabledef;
	AVCFieldInfo *fields;

	strcpy(infodir,CHAR(STRING_ELT(info_dir,0)));
	complete_path(infodir,"",1);

tablefile=_AVCBinReadOpenTable( infodir, CHAR(STRING_ELT(table_name,0)) );

	if(!tablefile)
		error("The path to the info directory is invalid or the table doesn't exist");
	
	tabledef=(tablefile->hdr).psTableDef;
	fields=tabledef->pasFieldDef;


	table=calloc(2, sizeof(SEXP));

	PROTECT(table[0]=NEW_STRING(tabledef->numFields));
	PROTECT(table[1]=NEW_INTEGER(tabledef->numFields));
	idata=INTEGER(table[1]);
	for(i=0;i<tabledef->numFields;i++)
	{
		SET_STRING_ELT(table[0],i,COPY_TO_USER_STRING(fields[i].szName));

		idata[i]=fields[i].nType1;
	}

	PROTECT(aux=NEW_LIST(2));
	SET_VECTOR_ELT(aux,0,table[0]);
	SET_VECTOR_ELT(aux,1,table[1]);

	UNPROTECT(3);

	free(table);

	return aux;
}

/*It imports the data from an arc file*/
SEXP get_arc_data(SEXP directory, SEXP coverage, SEXP filename) 
{
	int i,j,n, **ptable;
	double *x,*y;
	char pathtofile[PATH];
	AVCArc *reg;
	AVCBinFile *file;
	SEXP *table, points, aux;


	strcpy(pathtofile, CHAR(STRING_ELT(directory,0)));

	complete_path(pathtofile, (char *)CHAR(STRING_ELT(coverage,0)), 1);

	if(!(file=AVCBinReadOpen(pathtofile,CHAR(STRING_ELT(filename,0)), AVCFileARC)))
		error("Error opening file");

	n=0;

	while(AVCBinReadNextArc(file)){n++;}

	Rprintf("Number of ARCS:%d\n",n);


	table=calloc(7,sizeof(SEXP));
	ptable=(int **)calloc(7, sizeof(int *));
	for(i=0;i<7;i++)
	{
		PROTECT(table[i]=NEW_INTEGER(n));
		ptable[i]=(int *)INTEGER(table[i]);
	}


	PROTECT(points=NEW_LIST(n));

	if(AVCBinReadRewind(file))
		error("Rewind");

	for(i=0;i<n;i++)
	{
		
		if(!(reg=(AVCArc*)AVCBinReadNextArc(file)))
			error("Error while reading register");


		ptable[0][i]=reg->nArcId;

		ptable[1][i]=reg->nUserId;

		ptable[2][i]=reg->nFNode;

		ptable[3][i]=reg->nTNode;

		ptable[4][i]=reg->nLPoly;

		ptable[5][i]=reg->nRPoly;

		ptable[6][i]=reg->numVertices;

		SET_VECTOR_ELT(points,i,NEW_LIST(2));

		aux=VECTOR_ELT(points,i);

		SET_VECTOR_ELT(aux,0,NEW_NUMERIC(reg->numVertices));
		SET_VECTOR_ELT(aux,1,NEW_NUMERIC(reg->numVertices));

		x=REAL(VECTOR_ELT(aux,0));
		y=REAL(VECTOR_ELT(aux,1));

		for(j=0;j<reg->numVertices;j++)
		{
			x[j]=reg->pasVertices[j].x;
			y[j]=reg->pasVertices[j].y;
		}

	}

	PROTECT(aux=NEW_LIST(8));

	for(i=0;i<7;i++)
	{
		SET_VECTOR_ELT(aux,i,table[i]);
	}

	SET_VECTOR_ELT(aux,7,points);

	UNPROTECT(9);

	free(table);
	return aux;
}

SEXP get_bnd_data(SEXP info_dir, SEXP tablename)
{
	int i;
	double *d;
	AVCBinFile *tablefile;
	AVCField *datafield;
	SEXP data;

if(!(tablefile=_AVCBinReadOpenTable( CHAR(STRING_ELT(info_dir,0)), CHAR(STRING_ELT(tablename,0)) )) )
	error("Couldn't open table");

	tablefile->eFileType=AVCFileTABLE;

	if(AVCBinReadRewind(tablefile))	
		error("Couldn't open table");

	if(tablefile->hdr.psTableDef->numFields!=4 && tablefile->hdr.psTableDef->numRecords!=1)
		error("The file is not in BND format");

	datafield=AVCBinReadNextTableRec(tablefile);

	PROTECT(data=NEW_NUMERIC(4));

	d=REAL(data);

	for(i=0;i<4;i++)
	{
		if(datafield[i].fFloat!=0)/*Single precision*/
			d[i]=datafield[i].fFloat;
		else/*Default and DOUBLE precicion*/
			d[i]=datafield[i].dDouble;
	}

	UNPROTECT(1);

	return data;
}



SEXP get_pal_data(SEXP directory, SEXP coverage, SEXP filename) 
{
	int i,j,n;
	int **idata;
	char pathtofile[PATH];
	void **ptable;
	AVCPal *reg;
	AVCBinFile *file;
	SEXP *table, points, aux;


	strcpy(pathtofile, CHAR(STRING_ELT(directory,0)));


	complete_path(pathtofile, (char *) CHAR(STRING_ELT(coverage,0)), 1);


	if(!(file=AVCBinReadOpen(pathtofile,CHAR(STRING_ELT(filename,0)), AVCFilePAL)))
		error("Error opening file");

	n=0;

	while(AVCBinReadNextPal(file)){n++;}

	Rprintf("Number of POLYGONS:%d\n",n);

	idata=calloc(3,sizeof(int *));

        table=calloc(6,sizeof(SEXP));
        ptable=(void **)calloc(6, sizeof(void *));

        PROTECT(table[0]=NEW_INTEGER(n));  /*Polygon ID*/
        ptable[0]=(int *)INTEGER(table[0]);
        PROTECT(table[1]=NEW_NUMERIC(n));  /*Min X. coordinate*/
        ptable[1]=(double *)REAL(table[1]);
        PROTECT(table[2]=NEW_NUMERIC(n));  /*Min Y. coordinate*/
        ptable[2]=(double *)REAL(table[2]);
        PROTECT(table[3]=NEW_NUMERIC(n));  /*Max X. coordinate*/
        ptable[3]=(double *)REAL(table[3]);
        PROTECT(table[4]=NEW_NUMERIC(n));  /*Max Y. coordinate*/
        ptable[4]=(double *)REAL(table[4]);
        PROTECT(table[5]=NEW_INTEGER(n));  /*Number of arcs*/
        ptable[5]=(int *)INTEGER(table[5]);
 
 
        PROTECT(points=NEW_LIST(n));  


	if(AVCBinReadRewind(file))
		error("Rewind");

	for(i=0;i<n;i++)
	{
		
		if(!(reg=(AVCPal*)AVCBinReadNextPal(file)))
			error("Error while reading register");


		((int *)ptable[0])[i]=reg->nPolyId;

		((double *)ptable[1])[i]=reg->sMin.x;
		((double *)ptable[2])[i]=reg->sMin.y;

		((double *)ptable[3])[i]=reg->sMax.x;
		((double *)ptable[4])[i]=reg->sMax.y;

		((int *)ptable[5])[i]=reg->numArcs;


		SET_VECTOR_ELT(points,i,NEW_LIST(3));
		aux=VECTOR_ELT(points,i);

		SET_VECTOR_ELT(aux,0,NEW_INTEGER(reg->numArcs));
		idata[0]=INTEGER(VECTOR_ELT(aux,0));
		SET_VECTOR_ELT(aux,1,NEW_INTEGER(reg->numArcs));
		idata[1]=INTEGER(VECTOR_ELT(aux,1));
		SET_VECTOR_ELT(aux,2,NEW_INTEGER(reg->numArcs));
		idata[2]=INTEGER(VECTOR_ELT(aux,2));

		for(j=0;j<reg->numArcs;j++)
		{
			idata[0][j]=reg->pasArcs[j].nArcId;
			idata[1][j]=reg->pasArcs[j].nFNode;
			idata[2][j]=reg->pasArcs[j].nAdjPoly;
		}

	}


        PROTECT(aux=NEW_LIST(7));
 
        for(i=0;i<6;i++)
        {
                SET_VECTOR_ELT(aux,i,table[i]);
        }
 
        SET_VECTOR_ELT(aux,6,points);
 
        UNPROTECT(8);  


	free(ptable);
	free(idata);

	return aux;
}



SEXP get_lab_data(SEXP directory, SEXP coverage, SEXP filename) 
{
	int i,n;
	void **pdata;
	char pathtofile[PATH];
	AVCLab *reg;
	AVCBinFile *file;
	SEXP *table,aux;


	strcpy(pathtofile, CHAR(STRING_ELT(directory,0)));

	complete_path(pathtofile, (char *) CHAR(STRING_ELT(coverage,0)),1);/*FIXME*/

	if(!(file=AVCBinReadOpen(pathtofile,CHAR(STRING_ELT(filename,0)), AVCFileLAB)))
		error("Error opening file");

	n=0;

	while(AVCBinReadNextLab(file)){n++;}

	Rprintf("Number of LABELS:%d\n",n);


	table=calloc(8, sizeof(SEXP));
	pdata=calloc(8, sizeof(void *));

	PROTECT(table[0]=NEW_INTEGER(n));
	pdata[0]=INTEGER(table[0]);
	PROTECT(table[1]=NEW_INTEGER(n));
	pdata[1]=INTEGER(table[1]);

	for(i=2;i<8;i++)
	{
		PROTECT(table[i]=NEW_NUMERIC(n));
		pdata[i]=REAL(table[i]);

	}

	if(AVCBinReadRewind(file))
		error("Rewind");

	for(i=0;i<n;i++)
	{
		if(!(reg=(AVCLab*)AVCBinReadNextLab(file)))
			error("Error while reading register");


		((int *)pdata[0])[i]=reg->nValue;
		((int *)pdata[1])[i]=reg->nPolyId;

		((double*)pdata[2])[i]=reg->sCoord1.x;
		((double*)pdata[3])[i]=reg->sCoord1.y;
		((double*)pdata[4])[i]=reg->sCoord2.x;
		((double*)pdata[5])[i]=reg->sCoord2.y;
		((double*)pdata[6])[i]=reg->sCoord3.x;
		((double*)pdata[7])[i]=reg->sCoord3.y;
		
	}


	PROTECT(aux=NEW_LIST(8));

	for(i=0;i<8;i++)
	{
		SET_VECTOR_ELT(aux,i,table[i]);
	}

	UNPROTECT(9);

	free(table);
	free(pdata);

	return aux;
}


SEXP get_cnt_data(SEXP directory, SEXP coverage, SEXP filename) 
{
	int i,j,n, *ilabel;
	void **pdata;
	char pathtofile[PATH];
	AVCCnt *reg;
	AVCBinFile *file;
	SEXP *table, label, aux;


	strcpy(pathtofile, CHAR(STRING_ELT(directory,0)));
	complete_path(pathtofile, (char *) CHAR(STRING_ELT(coverage,0)),1);/*FIXME*/

	if(!(file=AVCBinReadOpen(pathtofile,CHAR(STRING_ELT(filename,0)), AVCFileCNT)))
		error("Error opening file");

	n=0;

	while(AVCBinReadNextCnt(file)){n++;}

	Rprintf("Number of CENTROIDS:%d\n",n);

	table=calloc(4, sizeof(SEXP));
	pdata=calloc(4,sizeof(void *));

	PROTECT(table[0]=NEW_INTEGER(n));
	pdata[0]=INTEGER(table[0]);

	PROTECT(table[1]=NEW_NUMERIC(n));
	pdata[1]=REAL(table[1]);

	PROTECT(table[2]=NEW_NUMERIC(n));
	pdata[2]=REAL(table[2]);

	PROTECT(table[3]=NEW_INTEGER(n));
	pdata[3]=INTEGER(table[3]);

	PROTECT(label=NEW_LIST(n));

	if(AVCBinReadRewind(file))
		error("Rewind");

	for(i=0;i<n;i++)
	{
		
		if(!(reg=(AVCCnt*)AVCBinReadNextCnt(file)))
			error("Error while reading register");

		((int *)pdata[0])[i]=reg->nPolyId;

		((double *)pdata[1])[i]=reg->sCoord.x;
		((double *)pdata[2])[i]=reg->sCoord.y;

		((int *)pdata[3])[i]=reg->numLabels;

		SET_VECTOR_ELT(label,i,NEW_INTEGER(reg->numLabels));
		ilabel=INTEGER(VECTOR_ELT(label,i));
		if(reg->numLabels >0)
		{
			for(j=0;j<reg->numLabels;j++)
			{
/*				printf("%d\n", reg->panLabelIds[j]);*/
				ilabel[j]=reg->panLabelIds[j];
			}
		}

	}


	PROTECT(aux=NEW_LIST(5));

	for(i=0;i<4;i++)
		SET_VECTOR_ELT(aux, i, table[i]);

	SET_VECTOR_ELT(aux, 4, label);

	UNPROTECT(6);

	free(table);
	free(pdata);

	return aux;
}



SEXP get_tol_data(SEXP directory, SEXP coverage, SEXP filename) 
{
	int i,n;
	void **pdata;
	char pathtofile[PATH];
	AVCTol *reg;
	AVCBinFile *file;
	SEXP *table, aux;


	strcpy(pathtofile, CHAR(STRING_ELT(directory,0)));
	complete_path(pathtofile, (char *) CHAR(STRING_ELT(coverage,0)), 1);/*FIXME*/

	if(!(file=AVCBinReadOpen(pathtofile,CHAR(STRING_ELT(filename,0)), AVCFileTOL)))
		error("Error opening file");

	n=0;

	while(AVCBinReadNextTol(file)){n++;}

	Rprintf("Number of TOLERANCES:%d\n", n);
	
	table=calloc(3, sizeof(SEXP));
	pdata=calloc(3, sizeof(void *));

	PROTECT(table[0]=NEW_INTEGER(n));
	pdata[0]=INTEGER(table[0]);
	PROTECT(table[1]=NEW_INTEGER(n));
	pdata[1]=INTEGER(table[1]);
	PROTECT(table[2]=NEW_NUMERIC(n));
	pdata[2]=REAL(table[2]);

	if(AVCBinReadRewind(file))
		error("Rewind");

	for(i=0;i<n;i++)
	{
		if(!(reg=(AVCTol*)AVCBinReadNextTol(file)))
			error("Error while reading register");

		((int *)pdata[0])[i]=reg->nIndex;

		((int *)pdata[1])[i]=reg->nFlag;

		((double *)pdata[2])[i]=reg->dValue;
	}

	PROTECT(aux=NEW_LIST(3));
	for(i=0;i<3;i++)
		SET_VECTOR_ELT(aux, i, table[i]);

	UNPROTECT(4);

	free(table);
	free(pdata);

	return aux;
}



SEXP get_txt_data(SEXP directory, SEXP coverage, SEXP filename) 
{
	int i,j,n;
	int **idata;
	double *x, *y;
	char pathtofile[PATH];
	AVCTxt *reg;
	AVCBinFile *file;
	SEXP *table, points,aux;


	strcpy(pathtofile, CHAR(STRING_ELT(directory,0)));
	complete_path(pathtofile, (char *) CHAR(STRING_ELT(coverage,0)), 1);/*FIXME*/

	if(!(file=AVCBinReadOpen(pathtofile,CHAR(STRING_ELT(filename,0)), AVCFileTXT)))
		error("Error opening file");

	n=0;

	while(AVCBinReadNextTxt(file)){n++;}

	Rprintf("Number of TxT ANNOTATIONS:%d\n",n);


	table=calloc(6, sizeof(SEXP));
	idata=calloc(5, sizeof(int *));


	PROTECT(table[0]=NEW_INTEGER(n));/*nTxtId*/
	idata[0]=INTEGER(table[0]);
	PROTECT(table[1]=NEW_INTEGER(n));/*nUserId*/
	idata[1]=INTEGER(table[1]);
	PROTECT(table[2]=NEW_INTEGER(n));/*nLevel*/
	idata[2]=INTEGER(table[2]);
	PROTECT(table[3]=NEW_INTEGER(n));/*numVerticesLine*/
	idata[3]=INTEGER(table[3]);
	PROTECT(table[4]=NEW_INTEGER(n));/*numVerticesArrow*/
	idata[4]=INTEGER(table[4]);

	PROTECT(table[5]=NEW_STRING(n));/*Character strings*/


	PROTECT(points=NEW_LIST(n));

	if(AVCBinReadRewind(file))
		error("Rewind");

	for(i=0;i<n;i++)
	{
		if(!(reg=(AVCTxt*)AVCBinReadNextTxt(file)))
			error("Error while reading register");

		((int *)idata[0])[i]=reg->nTxtId;
		((int *)idata[1])[i]=reg->nUserId;
		((int *)idata[2])[i]=reg->nLevel;
		((int *)idata[3])[i]=reg->numVerticesLine;
		((int *)idata[4])[i]=reg->numVerticesArrow;

		SET_STRING_ELT(table[5],i, COPY_TO_USER_STRING(reg->pszText));

		SET_VECTOR_ELT(points, i, NEW_LIST(2));
		aux=VECTOR_ELT(points, i);

/*This can be improved storing only the right numnber of vertices*/
		SET_VECTOR_ELT(aux, 0, NEW_NUMERIC(4));
		x=REAL(VECTOR_ELT(aux,0));
		SET_VECTOR_ELT(aux, 1, NEW_NUMERIC(4));
		y=REAL(VECTOR_ELT(aux,1));

		for(j=0;j<4;j++)
		{
			x[j]=reg->pasVertices[j].x;
			y[j]=reg->pasVertices[j].y;
		}

	}

	PROTECT(aux=NEW_LIST(7));

	for(i=0;i<6;i++)
		SET_VECTOR_ELT(aux, i, table[i]);

	SET_VECTOR_ELT(aux, i, points);

	UNPROTECT(8);

	free(table);
	free(idata);

	return aux;
}

SEXP get_table_data(SEXP infodir, SEXP tablename) 
{
	int i,j,n;
	char pathtoinfodir[PATH];
	void **pdata;
	AVCTableDef *tabledef;
	AVCField *reg;
	AVCBinFile *file;
	SEXP *table,aux;

	strcpy(pathtoinfodir, CHAR(STRING_ELT(infodir,0)));
	complete_path(pathtoinfodir, "", 1);

	if(!(file=AVCBinReadOpen(pathtoinfodir,CHAR(STRING_ELT(tablename,0)),AVCFileTABLE)))
	{
		error("Couldn't open table file\n");
	}

	n=0;

	while(AVCBinReadNextTableRec(file)){n++;}

	AVCBinReadRewind(file);


	tabledef=(file->hdr).psTableDef;

	table=calloc(tabledef->numFields, sizeof(SEXP));
	pdata=calloc(tabledef->numFields, sizeof(void *));


	for(i=0;i<tabledef->numFields;i++)
        {
/*printf("%d %d %d\n",i,j,tabledef->pasFieldDef[j].nType1);*/
		switch(tabledef->pasFieldDef[i].nType1)
		{
			case 1:
			case 2: PROTECT(table[i]=NEW_STRING(n));break;

			case 3: PROTECT(table[i]=NEW_INTEGER(n));
				pdata[i]=(int *)INTEGER(table[i]);break;

			case 4: PROTECT(table[i]=NEW_NUMERIC(n));
				pdata[i]=(double *)REAL(table[i]);break;
                                
			case 5: PROTECT(table[i]=NEW_INTEGER(n));
				pdata[i]=(int *)INTEGER(table[i]);break;

			case 6: PROTECT(table[i]=NEW_NUMERIC(n));
				pdata[i]=(double *)REAL(table[i]);break;
		}
	}		


	for(i=0;i<n;i++)
	{
		reg=AVCBinReadNextTableRec(file);

		for(j=0;j<tabledef->numFields;j++)
		{
/*			printf("%d %d %d\n",i,j,tabledef->pasFieldDef[j].nType1);*/
			switch(tabledef->pasFieldDef[j].nType1)
			{
				case 1: 
				case 2:
	SET_STRING_ELT(table[j],i, COPY_TO_USER_STRING(reg[j].pszStr)); 
				break;

				case 3:
				((int *)pdata[j])[i]=atoi(reg[j].pszStr);
				break;

				case 4:
				((double *)pdata[j])[i]=atof(reg[j].pszStr);
				break;

				case 5:
				if(reg[j].nInt16!=0)/*Single precision*/
					((int *)pdata[j])[i]=reg[j].nInt16;
				else/*Default and double precision*/
					((int *)pdata[j])[i]=reg[j].nInt32;
				break;
				
				case 6:
				if(reg[j].fFloat!=0)/*Single precision*/
					((double *)pdata[j])[i]=reg[j].fFloat;
				else/*Default and double precision*/
					((double *)pdata[j])[i]=reg[j].dDouble;
				break;
			}
		}
	}

	PROTECT(aux=NEW_LIST(tabledef->numFields));

	for(i=0;i<tabledef->numFields;i++)
		SET_VECTOR_ELT(aux, i, table[i]);

	UNPROTECT(1+tabledef->numFields);

	free(table);
	free(pdata);

	return aux;
}


/*
Just returns a string(path1)  joining the two path, adding a slash ('/' or '\') between
path1 and path2
If path2 is a directory dir must be set to non-zero, but, if it is a file
dir must be zero.
*/
void complete_path(char *path1, char *path2, int dir)
{
	int l;

	l=strlen(path1);
        if(path1[l-1]!=SLASH)
        {
		path1[l]=SLASH;
		path1[l+1]='\0';
        }
        strcat(path1,path2);
        l=strlen(path1);
 
        if(dir && path1[l-1]!=SLASH)
	{
                path1[l]=SLASH;
		path1[l+1]='\0';
		l++;
	}

	path1[l]='\0';
}


/*
Code taken from the file avcimport.c
Used to convert an E00 file into an Arc/Info binary coverage.
 */

/**********************************************************************
 *                          ConvertCover()
 *
 * Create a binary coverage from an E00 file.
 *
 * It would be possible to have an option for the precision... coming soon!
 **********************************************************************/
static void ConvertCovere00toavc(FILE *fpIn, const char *pszCoverName)
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


/* This is the R wrapper to the previous function to convert a E00 file to
 * an Arc/Info binary coverage*/

SEXP e00toavc (SEXP e00file, SEXP avcdir)
{
	FILE *fpIn;

	fpIn = fopen( CHAR(STRING_ELT(e00file,0)), "rt");

	if (fpIn == NULL)
	{
		error("Cannot open E00 file\n");
	}

	ConvertCovere00toavc(fpIn, CHAR(STRING_ELT(avcdir,0)));

	fclose(fpIn);

	return R_NilValue;
}


/* COde taken from avcexport.c*/

/**********************************************************************
 *                          ConvertCover()
 *
 * Convert a complete coverage to E00.
 **********************************************************************/
static void ConvertCoveravctoe00(const char *pszFname, FILE *fpOut)
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

/* Code to convert from a binary coverage to an E00 file*/

SEXP avctoe00 (SEXP avcdir, SEXP e00file)
{
	FILE *fpOut;

	fpOut = fopen( CHAR(STRING_ELT(e00file,0)), "wt");

	if (fpOut == NULL)
	{
		error("Cannot create E00 file\n");
	}

	ConvertCoveravctoe00( CHAR(STRING_ELT(avcdir,0)), fpOut);

	fclose(fpOut);

	return R_NilValue;
}
