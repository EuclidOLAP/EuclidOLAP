#ifndef EUCLID__MDX_H
#define EUCLID__MDX_H 1

#include "utils.h"
#include "mdx-ast-struct.h"

void mdx_init();

void parse_mdx(char *mdx, Stack *stk);

// IDS - Intermediate Data Structure
#define IDS_STRLS_CRTMBRS ((void *)0x01)
#define IDS_OBJLS_BIUCUBE ((void *)0x02)
#define IDS_CXOBJ_ISRTCUBEMEARS ((void *)0x03)
#define IDS_MULTI_DIM_SELECT_DEF ((void *)0x04)
#define IDS_ARRLS_DIMS_LVS_INFO ((void *)0x05)
#define IDS_STRLS_CRTDIMS ((void *)0x06)
#define IDS_CREATE_HIERARCHY ((void *)0x07)
#define IDS_MAKE_EQUIVALENT ((void *)0x08)

typedef struct general_chain_expression {
    ArrayList *chain;
    type_obj final_form;
} GeneralChainExpression;

typedef struct __vector_measures__
{
    ArrayList *ls_vector;
    ArrayList *ls_mears_vals;
} IDSVectorMears;




typedef struct Level_Role_Def_
{
    ArrayList *lr_path;
} LevelRoleDef;
LevelRoleDef *LevelRoleDef_creat(ArrayList *lr_path);




// MemberDef *ids_mbrdef_new__mbr_abs_path(ArrayList *);
MemberDef *MemberDef_creat(ids_ct t_cons);

#define MBRS_DEF__MBR_DEF_LS 1

typedef struct members_definition
{
    ids_ct t_cons;
    ArrayList *mbr_def_ls;
} MembersDef;

MembersDef *ids_mbrsdef_new(ids_ct);

void ids_mbrsdef__add_mbr_def(MembersDef *, MemberDef *);

typedef struct dim_role_def
{
    char *name;
} DimRoleDef;
DimRoleDef *DimRoleDef_creat();

#define TUPLE_DEF__MBRS_DEF 1
#define TUPLE_DEF__UPATH_LS 2

typedef struct tuple_definition
{
    ids_ct t_cons;
    MembersDef *ms_def;

    // ArrayList<MDMEntityUniversalPath>
    ArrayList *universal_path_ls;
} TupleDef;

TupleDef *ids_tupledef_new(ids_ct);

void ids_tupledef___set_mbrs_def(TupleDef *, MembersDef *);

#define SET_DEF__TUP_DEF_LS 1
#define SET_DEF__SET_FUNCTION 2
#define SET_DEF__VAR_OR_BLOCK 3
#define SET_DEF__MDE_UNI_PATH 4
#define SET_DEF__TUPLE_STATEMENT 5

typedef struct set_definition
{
    ids_ct t_cons;
    ArrayList *tuple_def_ls;
    void *set_fn;
    char *var_block;
    MDMEntityUniversalPath *up;
    TupleDef *tuple_def;
} SetDef;

SetDef *ids_setdef_new(ids_ct);

void ids_setdef__set_tuple_def_ls(SetDef *, ArrayList *);

typedef struct axis_definition
{
    SetDef *set_def;
    unsigned short posi;
} AxisDef;

AxisDef *ids_axisdef_new(SetDef *, unsigned short);

typedef struct cube_definition
{
    char *name;
} CubeDef;

CubeDef *ids_cubedef_new(char *name);

typedef struct select_definition
{
    ArrayList *member_formulas;
    ArrayList *set_formulas;
    CubeDef *cube_def;
    ArrayList *ax_def_ls;
    TupleDef *where_tuple_def;
} SelectDef;

/**
 * @param strat Memory allocation strategy.
 * @param mam When the strat parameter is SPEC_MAM, mam cannot be NULL.
 */
SelectDef *SelectDef_new(enum_oms strat, MemAllocMng *mam);

typedef struct _MD_context_
{
    SelectDef *select_def;
} MDContext;
MDContext *MDContext_creat();

#define FACTORY_DEF__TUP_DEF 1
#define FACTORY_DEF__DECIMAL 2
#define FACTORY_DEF__EXPRESSION 3
#define FACTORY_DEF__EXP_FN 4
#define FACTORY_DEF__STREXP 5
#define FACTORY_DEF__STR_LITERAL 6
#define FACTORY_DEF__EU_PATH 7
typedef struct factory_definition
{
    ids_ct t_cons;
    TupleDef *tuple_def;
    double decimal;
    void *exp;
    ASTStrExp *strexp;
    char *str_literal;
    MDMEntityUniversalPath *up;
} Factory;
Factory *Factory_creat();

typedef struct term_definition
{
    ArrayList *mul_factories;
    ArrayList *div_factories;
} Term;
Term *Term_creat();
void Term_mul_factory(Term *t, Factory *f);
void Term_div_factory(Term *t, Factory *f);


Expression *Expression_creat();
void Expression_plus_term(Expression *e, Term *t);
void Expression_minus_term(Expression *e, Term *t);

typedef struct member_formula
{
    ArrayList *path;
    Expression *exp;
} MemberFormula;
MemberFormula *MemberFormula_creat();
void MemberFormula_print(MemberFormula *mf);

typedef struct set_formula
{
    char *var_block;
    SetDef *set_def;
} SetFormula;
SetFormula *SetFormula_creat();

typedef struct formula_context
{
    ArrayList *member_formulas;
    ArrayList *set_formulas;
} FormulaContext;
FormulaContext *FormulaContext_creat();

typedef struct ExpFn_LookUpCube
{
    char *cube_name;
    char *exp_str;
    Expression *exp;
} ExpFnLookUpCube;

ExpFnLookUpCube *ExpFnLookUpCube_creat(char *cube_name, char *exp_str, Expression *exp);


#define BOOL_FAC_OPS__LESS 1
#define BOOL_FAC_OPS__LESS_EQ 2
#define BOOL_FAC_OPS__EQ 3
#define BOOL_FAC_OPS__NOT_EQ 4
#define BOOL_FAC_OPS__GREA 5
#define BOOL_FAC_OPS__GREA_EQ 6
typedef struct Boolean_Factory_
{
    char op;
    Expression *left__exp;
    Expression *right_exp;
    void *boolean_expression;

    void *ast_boolean_func;
} BooleanFactory;
BooleanFactory *BooleanFactory_creat(Expression *, char, Expression *);
void BooleanFactory_setBoolExp(BooleanFactory *bf, void *boolean_expression);

typedef struct Boolean_Term_
{
    ArrayList *factories;
} BooleanTerm;
BooleanTerm *BooleanTerm_creat();
void BooleanTerm_addFactory(BooleanTerm *, BooleanFactory *);

typedef struct Boolean_Expression_
{
    ArrayList *terms;
} BooleanExpression;
BooleanExpression *BooleanExpression_creat();
void BooleanExpression_addTerm(BooleanExpression *, BooleanTerm *);

typedef struct ExpFn_Iif
{
    BooleanExpression *bool_exp;
    Expression *exp1;
    Expression *exp2;
} ExpFnIif;

ExpFnIif *ExpFnIif_creat(BooleanExpression *bool_exp, Expression *exp1, Expression *exp2);

typedef struct ExpFn_CoalesceEmpty
{
    ArrayList *exp_ls;
} ExpFnCoalesceEmpty;

ExpFnCoalesceEmpty *ExpFnCoalesceEmpty_creat(ArrayList *exp_ls);

#endif