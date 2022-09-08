//#include <arpa/inet.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

//#include "net.h"
//#include "cfg.h"
//#include "utils.h"
//#include "command.h"

#include "mdx.h"
#include "obj-type-def.h"

MemberDef *ids_mbrdef_new__mbr_abs_path(ArrayList *mbr_abs_path)
{
    MemberDef *def = (MemberDef *)__objAlloc__(sizeof(MemberDef), OBJ_TYPE__MemberDef);
    def->t_cons = MEMBER_DEF__MBR_ABS_PATH;
    def->mbr_abs_path = mbr_abs_path;
    return def;
}

MemberDef *MemberDef_creat(ids_ct t_cons)
{
    MemberDef *mdef = __objAlloc__(sizeof(MemberDef), OBJ_TYPE__MemberDef);
    mdef->t_cons = t_cons;
    return mdef;
}

LevelRoleDef *LevelRoleDef_creat(ArrayList *lr_path) {
    LevelRoleDef *lvr_def = __objAlloc__(sizeof(LevelRoleDef), OBJ_TYPE__LevelRoleDef);
    lvr_def->lr_path = lr_path;
    return lvr_def;
}

MembersDef *ids_mbrsdef_new(ids_ct t_cons)
{
    MembersDef *def = (MembersDef *)__objAlloc__(sizeof(MembersDef), OBJ_TYPE__MembersDef);
    def->t_cons = t_cons;
    def->mbr_def_ls = als_create(32, "MemberDef *");
    return def;
}

void ids_mbrsdef__add_mbr_def(MembersDef *ms, MemberDef *m)
{
    als_add(ms->mbr_def_ls, m);
}

TupleDef *ids_tupledef_new(ids_ct t_cons)
{
    TupleDef *def = (TupleDef *)__objAlloc__(sizeof(TupleDef), OBJ_TYPE__TupleDef);
    def->t_cons = t_cons;
    return def;
}

void ids_tupledef___set_mbrs_def(TupleDef *t, MembersDef *ms)
{
    t->ms_def = ms;
}

SetDef *ids_setdef_new(ids_ct t_cons)
{
    SetDef *def = (SetDef *)__objAlloc__(sizeof(SetDef), OBJ_TYPE__SetDef);
    def->t_cons = t_cons;
    return def;
}

void ids_setdef__set_tuple_def_ls(SetDef *sd, ArrayList *ls)
{
    sd->tuple_def_ls = ls;
}

AxisDef *ids_axisdef_new(SetDef *set_def, unsigned short posi)
{
    AxisDef *def = (AxisDef *)__objAlloc__(sizeof(AxisDef), OBJ_TYPE__AxisDef);
    def->posi = posi;
    def->set_def = set_def;
    return def;
}

CubeDef *ids_cubedef_new(char *name)
{
    CubeDef *def = (CubeDef *)__objAlloc__(sizeof(CubeDef), OBJ_TYPE__CubeDef);
    def->name = name;
    return def;
}

SelectDef *ids_selectdef_new(CubeDef *cube_def, ArrayList *ax_def_ls)
{
    SelectDef *def = (SelectDef *)__objAlloc__(sizeof(SelectDef), OBJ_TYPE__SelectDef);
    def->cube_def = cube_def;
    def->ax_def_ls = ax_def_ls;
    return def;
}

Factory *Factory_creat()
{
    return (Factory *)__objAlloc__(sizeof(Factory), OBJ_TYPE__Factory);
}

Term *Term_creat()
{
    Term *t = (Term *)__objAlloc__(sizeof(Term), OBJ_TYPE__Term);
    t->mul_factories = als_create(32, "Factory *");
    t->div_factories = als_create(32, "Factory *");
    return t;
}

void Term_mul_factory(Term *t, Factory *f)
{
    als_add(t->mul_factories, f);
}

void Term_div_factory(Term *t, Factory *f)
{
    als_add(t->div_factories, f);
}

Expression *Expression_creat()
{
    Expression *e = (Expression *)__objAlloc__(sizeof(Expression), OBJ_TYPE__Expression);
    e->plus_terms = als_create(32, "Term *");
    e->minus_terms = als_create(32, "Term *");
    return e;
}

void Expression_plus_term(Expression *e, Term *t)
{
    als_add(e->plus_terms, t);
}

void Expression_minus_term(Expression *e, Term *t)
{
    als_add(e->minus_terms, t);
}

MemberFormula *MemberFormula_creat()
{
    return (MemberFormula *)__objAlloc__(sizeof(MemberFormula), OBJ_TYPE__MemberFormula);
}

void MemberFormula_print(MemberFormula *mf)
{
    printf("{\"type\": \"MemberFormula\",\"display\":\"");
    int i, len = als_size(mf->path);
    for (i = 0; i < len; i++)
    {
        if (i < len - 1)
            printf("[%s].", als_get(mf->path, i));
        else
            printf("[%s]", als_get(mf->path, i));
    }
    printf("\"}");
}

SetFormula *SetFormula_creat()
{
    return __objAlloc__(sizeof(SetFormula), OBJ_TYPE__SET_FORMULA);
}

FormulaContext *FormulaContext_creat()
{
    FormulaContext *fc = (FormulaContext *)__objAlloc__(sizeof(FormulaContext), OBJ_TYPE__FormulaContext);
    fc->member_formulas = als_create(32, "MemberFormula *");
    fc->set_formulas = als_create(32, "SetFormula *");
    return fc;
}

MDContext *MDContext_creat()
{
    return __objAlloc__(sizeof(MDContext), OBJ_TYPE__MD_CONTEXT);
}

SetFnChildren *SetFnChildren_creat(MemberDef *m_def)
{
    SetFnChildren *fn = __objAlloc__(sizeof(SetFnChildren), OBJ_TYPE__SET_FN_CHILDREN);
    fn->m_def = m_def;
    return fn;
}

MemberFnParent *MemberFnParent_creat(MemberDef *child_def)
{
    MemberFnParent *fn = __objAlloc__(sizeof(MemberFnParent), OBJ_TYPE__MemberFnParent);
    fn->child_def = child_def;
    return fn;
}

ExpFnSum *ExpFnSum_creat(SetDef *_set, Expression *_exp)
{
    ExpFnSum *sum = __objAlloc__(sizeof(ExpFnSum), OBJ_TYPE__ExpFnSum);
    sum->set_def = _set;
    sum->exp = _exp;
    return sum;
}
void ExpFnCount_set_setDef(ExpFnCount *count, SetDef *set_def)
{
    count->set_def = set_def;
}
void ExpFnCount_excludeEmpty(ExpFnCount *count)
{
    count->include_empty = 0;
}

ExpFnCount *ExpFnCount_creat()
{
    ExpFnCount *count = __objAlloc__(sizeof(ExpFnCount), OBJ_TYPE__ExpFnCount);
    count->include_empty = 1; // default value - include empty
    return count;
}

ExpFnLookUpCube *ExpFnLookUpCube_creat(char *cube_name, char *exp_str, Expression *exp) {
    ExpFnLookUpCube *luc = __objAlloc__(sizeof(ExpFnLookUpCube), OBJ_TYPE__ExpFnLookUpCube);
    luc->cube_name = cube_name;
    luc->exp_str = exp_str;
    luc->exp = exp;
    return luc;
}

ExpFnIif *ExpFnIif_creat(BooleanExpression *bool_exp, Expression *exp1, Expression *exp2) {
    ExpFnIif *iif = __objAlloc__(sizeof(ExpFnIif), OBJ_TYPE__ExpFnIif);
    iif->bool_exp = bool_exp;
    iif->exp1 = exp1;
    iif->exp2 = exp2;
    return iif;
}

ExpFnCoalesceEmpty *ExpFnCoalesceEmpty_creat(ArrayList *exp_ls) {
    ExpFnCoalesceEmpty *ce = __objAlloc__(sizeof(ExpFnCoalesceEmpty), OBJ_TYPE__ExpFnCoalesceEmpty);
    ce->exp_ls = exp_ls;
    return ce;
}

DimRoleDef *DimRoleDef_creat()
{
    return __objAlloc__(sizeof(DimRoleDef), OBJ_TYPE__DimRoleDef);
}

SetFnMembers *SetFnMembers_creat()
{
    return __objAlloc__(sizeof(SetFnMembers), OBJ_TYPE__SetFnMembers);
}

SetFnCrossJoin *SetFnCrossJoin_creat()
{
    SetFnCrossJoin *fn = __objAlloc__(sizeof(SetFnCrossJoin), OBJ_TYPE__SetFnCrossJoin);
    fn->set_def_ls = als_create(8, "SetDef *");
    return fn;
}

void SetFnCrossJoin_add_set(SetFnCrossJoin *fn, SetDef *set_def)
{
    als_add(fn->set_def_ls, set_def);
}

BooleanFactory *BooleanFactory_creat(Expression *le, char ops, Expression *re)
{
    BooleanFactory *fac = (BooleanFactory *)__objAlloc__(sizeof(BooleanFactory), OBJ_TYPE__BooleanFactory);
    fac->left__exp = le;
    fac->op = ops;
    fac->right_exp = re;
    return fac;
}

void BooleanFactory_setBoolExp(BooleanFactory *bf, void *boolean_expression)
{
    bf->boolean_expression = boolean_expression;
}

BooleanTerm *BooleanTerm_creat()
{
    BooleanTerm *term = (BooleanTerm *)__objAlloc__(sizeof(BooleanTerm), OBJ_TYPE__BooleanTerm);
    term->factories = als_create(16, "BooleanFactory *");
    return term;
}

void BooleanTerm_addFactory(BooleanTerm *term, BooleanFactory *fac)
{
    als_add(term->factories, fac);
}

BooleanExpression *BooleanExpression_creat()
{
    BooleanExpression *exp = (BooleanExpression *)__objAlloc__(sizeof(BooleanExpression), OBJ_TYPE__BooleanExpression);
    exp->terms = als_create(16, "BooleanTerm *");
    return exp;
}

void BooleanExpression_addTerm(BooleanExpression *exp, BooleanTerm *term)
{
    als_add(exp->terms, term);
}

SetFnFilter *SetFnFilter_creat(SetDef *setDef, BooleanExpression *boolExp)
{
    SetFnFilter *filter = (SetFnFilter *)__objAlloc__(sizeof(SetFnFilter), OBJ_TYPE__SetFnFilter);
    filter->set_def = setDef;
    filter->boolExp = boolExp;
    return filter;
}

MemberFnCurrentMember *MemberFnCurrentMember_creat() {
    return __objAlloc__(sizeof(MemberFnCurrentMember), OBJ_TYPE__MemberFnCurrentMember);
}

MemberFnPrevMember *MemberFnPrevMember_creat(MemberDef *m_def) {
    MemberFnPrevMember *fn = (MemberFnPrevMember *)__objAlloc__(sizeof(MemberFnPrevMember), OBJ_TYPE__MemberFnPrevMember);
    fn->curr_mr=m_def;
    return fn;
}

MemberRoleFnParallelPeriod *MemberRoleFnParallelPeriod_creat(LevelRoleDef *lvDef, Expression *idx, MemberDef *mDef) {
    MemberRoleFnParallelPeriod *pp = __objAlloc__(sizeof(MemberRoleFnParallelPeriod), OBJ_TYPE__MemberRoleFnParallelPeriod);
    pp->lvr_def = lvDef;
    pp->index = idx;
    pp->mr_def = mDef;
    return pp;
}

SetFnLateralMembers *SetFnLateralMembers_creat(MemberDef *mdef) {
    SetFnLateralMembers *latmbr = __objAlloc__(sizeof(SetFnLateralMembers), OBJ_TYPE__SetFnLateralMembers);
    latmbr->mr_def = mdef;
    return latmbr;
}

SetFnOrder *SetFnOrder_creat(SetDef *set, Expression *exp, char opt) {
    SetFnOrder *order = __objAlloc__(sizeof(SetFnOrder), OBJ_TYPE__SetFnOrder);
    order->set = set;
    order->exp = exp;
    order->option = opt;
    return order;
}

SetFnTopCount *SetFnTopCount_creat(SetDef *set, Expression *count_exp, Expression *num_exp) {
    SetFnTopCount *tc = __objAlloc__(sizeof(SetFnTopCount), OBJ_TYPE__SetFnTopCount);
    tc->set = set;
    tc->count_exp = count_exp;
    tc->num_exp = num_exp;
    return tc;
}

SetFnExcept *SetFnExcept_creat(SetDef *set_1, SetDef *set_2, char option) {
    SetFnExcept *except = __objAlloc__(sizeof(SetFnExcept), OBJ_TYPE__SetFnExcept);
    except->set_1 = set_1;
    except->set_2 = set_2;
    except->option = option;
    return except;
}

SetFnYTD *SetFnYTD_creat(MemberDef *mdef) {
    SetFnYTD *ytd = __objAlloc__(sizeof(SetFnYTD), OBJ_TYPE__SetFnYTD);
    ytd->mbr_def=mdef;
    return ytd;
}

SetFnDescendants *SetFnDescendants_creat(MemberDef *mbr_def, LevelRoleDef *lvr_def, Expression *distance, char flag) {
    SetFnDescendants *desc = __objAlloc__(sizeof(SetFnDescendants), OBJ_TYPE__SetFnDescendants);
    desc->mbr_def = mbr_def;
    desc->lvr_def = lvr_def;
    desc->distance = distance;
    desc->flag = flag;
    return desc;
}

SetFnTail *SetFnTail_creat(SetDef *set, Expression *count) {
    SetFnTail *tail = __objAlloc__(sizeof(SetFnTail), OBJ_TYPE__SetFnTail);
    tail->set = set;
    tail->count = count;
    return tail;
}

SetFnBottomOrTopPercent *SetFnBottomOrTopPercent_creat(char type, SetDef *set, Expression *percentage, Expression *exp) {
    SetFnBottomOrTopPercent *percent = __objAlloc__(sizeof(SetFnBottomOrTopPercent), OBJ_TYPE__SetFnBottomOrTopPercent);
    percent->type = type;
    percent->set = set;
    percent->percentage = percentage;
    percent->exp = exp;
    return percent;
}

SetFnUnion *SetFnUnion_creat(ArrayList *setDefs, char opt) {
    SetFnUnion *union_ = __objAlloc__(sizeof(SetFnUnion), OBJ_TYPE__SetFnUnion);
    union_->set_def_ls=setDefs;
    union_->option=opt;
    return union_;
}

SetFnIntersect *SetFnIntersect_creat(ArrayList *set_def_ls, char option) {
    SetFnIntersect *intersect = __objAlloc__(sizeof(SetFnIntersect), OBJ_TYPE__SetFnIntersect);
    intersect->set_def_ls=set_def_ls;
    intersect->option=option;
    return intersect;
}