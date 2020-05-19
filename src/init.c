#include <R.h>
#include <Rinternals.h>
#include "RArcInfo.h"

#include <R_ext/Rdynload.h>

static R_CallMethodDef CallEntries[] = {
//    {"get_names_of_coverages", (DL_FUNC) &get_names_of_coverages, 1},
    {"get_table_names", (DL_FUNC) &get_table_names, 1},
    {"get_table_fields", (DL_FUNC) &get_table_fields, 2},
    {"get_arc_data", (DL_FUNC) &get_arc_data, 3},
    {"get_bnd_data", (DL_FUNC) &get_bnd_data, 2},
    {"get_pal_data", (DL_FUNC) &get_pal_data, 3},
    {"get_lab_data", (DL_FUNC) &get_lab_data, 3},
    {"get_cnt_data", (DL_FUNC) &get_cnt_data, 3},
    {"get_tol_data", (DL_FUNC) &get_tol_data, 3},
    {"get_table_data", (DL_FUNC) &get_table_data, 2},
    {"get_txt_data", (DL_FUNC) &get_txt_data, 3},
    {"e00toavc", (DL_FUNC) &e00toavc, 2},
    {"avctoe00", (DL_FUNC) &avctoe00, 2},
    {NULL, NULL, 0}
};


void 
#ifdef HAVE_VISIBILITY_ATTRIBUTE
__attribute__ ((visibility ("default")))
#endif
R_init_sp(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);

}

