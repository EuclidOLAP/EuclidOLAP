#ifndef EUCLID__MDX_H
#define EUCLID__MDX_H 1

#include "utils.h"

void mdx_init();

void parse_mdx(char *mdx, Stack *stk);

// IDS - Intermediate Data Structure
#define IDS_STRLS_CRTMBRS ((void *)0x01)
#define IDS_OBJLS_BIUCUBE ((void *)0x02)
#define IDS_CXOBJ_ISRTCUBEMEARS ((void *)0x03)
#define IDS_MULTI_DIM_SELECT_DEF ((void *)0x04)
#define IDS_ARRLS_DIMS_LVS_INFO ((void *)0x05)
#define IDS_STRLS_CRTDIMS ((void *)0x06)

typedef struct general_chain_expression {
    ArrayList *chain;
    type_obj final_form;
} GeneralChainExpression;

typedef struct __vector_measures__
{
    ArrayList *ls_vector;
    ArrayList *ls_mears_vals;
} IDSVectorMears;

typedef unsigned char ids_ct;


typedef struct Level_Role_Def_
{
    ArrayList *lr_path;
} LevelRoleDef;
LevelRoleDef *LevelRoleDef_creat(ArrayList *lr_path);


#define MEMBER_DEF__MBR_ABS_PATH 1
#define MEMBER_DEF__MBR_FUNCTION 2

typedef struct member_definition
{
    ids_ct t_cons;
    ArrayList *mbr_abs_path;
    void *member_fn;
} MemberDef;

MemberDef *ids_mbrdef_new__mbr_abs_path(ArrayList *);
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

typedef struct tuple_definition
{
    ids_ct t_cons;
    MembersDef *ms_def;
} TupleDef;

TupleDef *ids_tupledef_new(ids_ct);

void ids_tupledef___set_mbrs_def(TupleDef *, MembersDef *);

#define SET_DEF__TUP_DEF_LS 1
#define SET_DEF__SET_FUNCTION 2
#define SET_DEF__VAR_OR_BLOCK 3

typedef struct set_definition
{
    ids_ct t_cons;
    ArrayList *tuple_def_ls;
    void *set_fn;
    char *var_block;
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
typedef struct factory_definition
{
    ids_ct t_cons;
    TupleDef *tuple_def;
    double decimal;
    void *exp;
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

typedef struct term_expression
{
    ArrayList *plus_terms;
    ArrayList *minus_terms;
} Expression;
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

typedef struct set_fn_children
{
    MemberDef *m_def;
} SetFnChildren;
SetFnChildren *SetFnChildren_creat(MemberDef *m_def);

typedef struct mbr_fn_parent
{
    MemberDef *child_def;
} MemberFnParent;
MemberFnParent *MemberFnParent_creat(MemberDef *child_def);

typedef struct exp_fn_sum
{
    SetDef *set_def;
    Expression *exp;
} ExpFnSum;
ExpFnSum *ExpFnSum_creat(SetDef *, Expression *);

typedef struct Exp_Fn_Count
{
    char include_empty; // 0 - EXCLUDEEMPTY, 1(def) - INCLUDEEMPTY
    SetDef *set_def;
} ExpFnCount;
ExpFnCount *ExpFnCount_creat();
void ExpFnCount_set_setDef(ExpFnCount *, SetDef *);
void ExpFnCount_excludeEmpty(ExpFnCount *);


typedef struct ExpFn_LookUpCube
{
    char *cube_name;
    char *exp_str;
    Expression *exp;
} ExpFnLookUpCube;

ExpFnLookUpCube *ExpFnLookUpCube_creat(char *cube_name, char *exp_str, Expression *exp);


typedef struct set_fn_members
{
    DimRoleDef *dr_def;
    char option[128];
} SetFnMembers;
SetFnMembers *SetFnMembers_creat();

typedef struct Set_Fn_CrossJoin
{
    ArrayList *set_def_ls;
} SetFnCrossJoin;
SetFnCrossJoin *SetFnCrossJoin_creat();
void SetFnCrossJoin_add_set(SetFnCrossJoin *, SetDef *);

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

typedef struct Set_Fn_Filter_
{
    SetDef *set_def;
    BooleanExpression *boolExp;
} SetFnFilter;
SetFnFilter *SetFnFilter_creat(SetDef *, BooleanExpression *);

typedef struct Member_Fn_Current_Member_
{
    DimRoleDef *dr_def;
} MemberFnCurrentMember;
MemberFnCurrentMember *MemberFnCurrentMember_creat();

typedef struct Member_Fn_Prev_Member_
{
    MemberDef *curr_mr;
} MemberFnPrevMember;
MemberFnPrevMember *MemberFnPrevMember_creat(MemberDef *);


typedef struct Member_Role_Fn_Parallel_Period_
{
    LevelRoleDef *lvr_def;
    Expression *index;
    MemberDef *mr_def;
} MemberRoleFnParallelPeriod;
MemberRoleFnParallelPeriod *MemberRoleFnParallelPeriod_creat(LevelRoleDef *, Expression *, MemberDef *);


typedef struct Member_Role_Fn_Closing_Period_
{
    LevelRoleDef *lvr_def;
    MemberDef *mr_def;
} MemberRoleFnClosingPeriod;
MemberRoleFnClosingPeriod *MemberRoleFnClosingPeriod_creat(LevelRoleDef *lvr, MemberDef *mr);


typedef struct Member_Role_Fn_Opening_Period_
{
    LevelRoleDef *lvr_def;
    MemberDef *mr_def;
} MemberRoleFnOpeningPeriod;
MemberRoleFnOpeningPeriod *MemberRoleFnOpeningPeriod_creat(LevelRoleDef *lvr, MemberDef *mr);


typedef struct Member_Role_Fn_FirstChild_
{
    MemberDef *mr_def;
} MemberRoleFnFirstChild;
MemberRoleFnFirstChild *MemberRoleFnFirstChild_creat(MemberDef *mr);

typedef struct Member_Role_Fn_LastChild_
{
    MemberDef *mr_def;
} MemberRoleFnLastChild;
MemberRoleFnLastChild *MemberRoleFnLastChild_creat(MemberDef *mr);


typedef struct Member_Role_Fn_FirstSibling_
{
    MemberDef *mr_def;
} MemberRoleFnFirstSibling;
MemberRoleFnFirstSibling *MemberRoleFnFirstSibling_creat(MemberDef *mr);

typedef struct Member_Role_Fn_LastSibling_
{
    MemberDef *mr_def;
} MemberRoleFnLastSibling;
MemberRoleFnLastSibling *MemberRoleFnLastSibling_creat(MemberDef *mr);


typedef struct Member_Role_Fn_Lag_
{
    MemberDef *mr_def;
    long index;
} MemberRoleFnLag;
MemberRoleFnLag *MemberRoleFnLag_creat(MemberDef *_mr_def, long _index);


typedef struct SetFn_LateralMembers
{
    MemberDef *mr_def;
} SetFnLateralMembers;
SetFnLateralMembers *SetFnLateralMembers_creat(MemberDef *);


#define SET_FN__ORDER_ASC   1
#define SET_FN__ORDER_DESC  2
#define SET_FN__ORDER_BASC  3
#define SET_FN__ORDER_BDESC 4

typedef struct SetFn_Order
{
    SetDef *set;
    Expression *exp;
    char option;
} SetFnOrder;

SetFnOrder *SetFnOrder_creat(SetDef *, Expression *, char);

typedef struct SetFn_TopCount
{
    SetDef *set;
    Expression *count_exp;
    Expression *num_exp;
} SetFnTopCount;

SetFnTopCount *SetFnTopCount_creat(SetDef *set, Expression *count_exp, Expression *num_exp);

#define SET_FN__EXCEPT_ALL  1

typedef struct SetFn_Except
{
    SetDef *set_1;
    SetDef *set_2;
    char option;
} SetFnExcept;

SetFnExcept *SetFnExcept_creat(SetDef *set_1, SetDef *set_2, char option);


typedef struct SetFn_YTD
{
    MemberDef *mbr_def;
} SetFnYTD;

SetFnYTD *SetFnYTD_creat(MemberDef *);

#define SET_FN__DESCENDANTS_OPT_SELF                1
#define SET_FN__DESCENDANTS_OPT_AFTER               2
#define SET_FN__DESCENDANTS_OPT_BEFORE              3
#define SET_FN__DESCENDANTS_OPT_BEFORE_AND_AFTER    4
#define SET_FN__DESCENDANTS_OPT_SELF_AND_AFTER      5
#define SET_FN__DESCENDANTS_OPT_SELF_AND_BEFORE     6
#define SET_FN__DESCENDANTS_OPT_SELF_BEFORE_AFTER   7
#define SET_FN__DESCENDANTS_OPT_LEAVES              8

typedef struct SetFn_Descendants
{
    MemberDef *mbr_def;
    LevelRoleDef *lvr_def;
    Expression *distance;
    char flag;
} SetFnDescendants;

SetFnDescendants *SetFnDescendants_creat(MemberDef *, LevelRoleDef *, Expression *, char);

typedef struct SetFn_Tail
{
    SetDef *set;
    Expression *count;
} SetFnTail;

SetFnTail *SetFnTail_creat(SetDef *, Expression *);

#define SET_FN__BOTTOM_PERCENT  1
#define SET_FN__TOP_PERCENT     2

typedef struct SetFn_Bottom_Or_Top_Percent
{
    SetDef *set;
    Expression *percentage;
    Expression *exp;
    char type;
} SetFnBottomOrTopPercent;

SetFnBottomOrTopPercent *SetFnBottomOrTopPercent_creat(char type, SetDef *set, Expression *percentage, Expression *exp);

#define SET_FN__UNION_ALL  1

typedef struct SetFn_Union
{
    ArrayList *set_def_ls;
    char option;
} SetFnUnion;

SetFnUnion *SetFnUnion_creat(ArrayList *, char);

#define SET_FN__INTERSECT_ALL  1

typedef struct SetFn_Intersect
{
    ArrayList *set_def_ls;
    char option;
} SetFnIntersect;

SetFnIntersect *SetFnIntersect_creat(ArrayList *set_def_ls, char option);

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