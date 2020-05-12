#ifndef RINFO_H
#define RINFO_H

#include<Rversion.h>
#include<Rinternals.h>
#include<Rdefines.h>
#include"cpl_port.h" /*Needed to set endianness.*/

#ifdef WIN32 
#define SLASH 92 /* '\' */
#else
#define SLASH 47 /* '/' */
#endif

#define PATH 257 /*The length of an array containing a path*/


SEXP get_names_of_coverages(SEXP directory);
SEXP get_table_names(SEXP directory);
SEXP get_table_fields(SEXP info_dir, SEXP table_name);
SEXP get_arc_data(SEXP directory, SEXP coverage, SEXP filename) ;
SEXP get_bnd_data(SEXP info_dir, SEXP tablename);
SEXP get_pal_data(SEXP directory, SEXP coverage, SEXP filename);
SEXP get_lab_data(SEXP directory, SEXP coverage, SEXP filename);
SEXP get_cnt_data(SEXP directory, SEXP coverage, SEXP filename);
SEXP get_tol_data(SEXP directory, SEXP coverage, SEXP filename);
SEXP get_table_data(SEXP infodir, SEXP tablename);
SEXP get_txt_data(SEXP directory, SEXP coverage, SEXP filename);

void complete_path(char *path1, char *path2, int dir);

static void ConvertCovere00toavc(FILE *fpIn, const char *pszCoverName);
SEXP e00toavc (SEXP e00file, SEXP avcdir);

static void ConvertCoveravctoe00(const char *pszFname, FILE *fpOut);
SEXP avctoe00 (SEXP avcdir, SEXP e00file);

#endif
