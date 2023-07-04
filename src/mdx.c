// #include <arpa/inet.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <string.h>

// #include "net.h"
// #include "cfg.h"
// #include "utils.h"
// #include "command.h"
#include <pthread.h>

#include "log.h"
#include "mdx.h"
#include "obj-type-def.h"

extern Stack AST_STACK;

extern void do_parse_mdx(char *mdx);

// for synchronize the function parse_mdx
static pthread_mutex_t sync_parse_mtx;

void mdx_init() {
    pthread_mutex_init(&sync_parse_mtx, NULL);
}

void parse_mdx(char *mdx, Stack *stk) {

    pthread_mutex_lock(&sync_parse_mtx);

    do_parse_mdx(mdx);

	memcpy(stk, &AST_STACK, sizeof(Stack));
	stack_reset(&AST_STACK);

	pthread_mutex_unlock(&sync_parse_mtx);
}

MemberDef *MemberDef_creat(ids_ct t_cons)
{
    MemberDef *mdef = mam_alloc(sizeof(MemberDef), OBJ_TYPE__MemberDef, NULL, 0);
    mdef->t_cons = t_cons;
    return mdef;
}

LevelRoleDef *LevelRoleDef_creat(ArrayList *lr_path)
{
    LevelRoleDef *lvr_def = mam_alloc(sizeof(LevelRoleDef), OBJ_TYPE__LevelRoleDef, NULL, 0);
    lvr_def->lr_path = lr_path;
    return lvr_def;
}

MembersDef *ids_mbrsdef_new(ids_ct t_cons)
{
    MembersDef *def = mam_alloc(sizeof(MembersDef), OBJ_TYPE__MembersDef, NULL, 0);
    def->t_cons = t_cons;
    def->mbr_def_ls = als_new(32, "MemberDef *", THREAD_MAM, NULL);
    return def;
}

void ids_mbrsdef__add_mbr_def(MembersDef *ms, MemberDef *m)
{
    als_add(ms->mbr_def_ls, m);
}

TupleDef *ids_tupledef_new(ids_ct t_cons)
{
    TupleDef *def = mam_alloc(sizeof(TupleDef), OBJ_TYPE__TupleDef, NULL, 0);
    def->t_cons = t_cons;
    return def;
}

void ids_tupledef___set_mbrs_def(TupleDef *t, MembersDef *ms)
{
    t->ms_def = ms;
}

SetDef *ids_setdef_new(ids_ct t_cons)
{
    SetDef *def = mam_alloc(sizeof(SetDef), OBJ_TYPE__SetDef, NULL, 0);
    def->t_cons = t_cons;
    return def;
}

void ids_setdef__set_tuple_def_ls(SetDef *sd, ArrayList *ls)
{
    sd->tuple_def_ls = ls;
}

AxisDef *ids_axisdef_new(SetDef *set_def, unsigned short posi)
{
    AxisDef *def = mam_alloc(sizeof(AxisDef), OBJ_TYPE__AxisDef, NULL, 0);
    def->posi = posi;
    def->set_def = set_def;
    return def;
}

CubeDef *ids_cubedef_new(char *name)
{
    CubeDef *def = mam_alloc(sizeof(CubeDef), OBJ_TYPE__CubeDef, NULL, 0);
    def->name = name;
    return def;
}

SelectDef *SelectDef_new(enum_oms strat, MemAllocMng *mam)
{

    if (strat == DIRECT)
        return obj_alloc(sizeof(SelectDef), OBJ_TYPE__SelectDef);

    if (strat == USED_MAM)
    {
        log_print("[ error ] exit! exception in SelectDef_new(..)\n");
        exit(1);
    }

    if (strat == THREAD_MAM)
        mam = MemAllocMng_current_thread_mam();

    if (mam)
        return mam_alloc(sizeof(SelectDef), OBJ_TYPE__SelectDef, mam, 0);

    log_print("[ error ] exit! exception in SelectDef_new(..)\n");
    exit(1);
}

Factory *Factory_creat()
{
    return mam_alloc(sizeof(Factory), OBJ_TYPE__Factory, NULL, 0);
}

Term *Term_creat()
{
    Term *t = mam_alloc(sizeof(Term), OBJ_TYPE__Term, NULL, 0);
    t->mul_factories = als_new(32, "Factory *", THREAD_MAM, NULL);
    t->div_factories = als_new(32, "Factory *", THREAD_MAM, NULL);
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
    Expression *e = mam_alloc(sizeof(Expression), OBJ_TYPE__Expression, NULL, 0);
    e->plus_terms = als_new(32, "Term *", THREAD_MAM, NULL);
    e->minus_terms = als_new(32, "Term *", THREAD_MAM, NULL);
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
    return mam_alloc(sizeof(MemberFormula), OBJ_TYPE__MemberFormula, NULL, 0);
}

void MemberFormula_print(MemberFormula *mf)
{
    log_print("{\"type\": \"MemberFormula\",\"display\":\"");
    int i, len = als_size(mf->path);
    for (i = 0; i < len; i++)
    {
        if (i < len - 1)
            log_print("[%s].", als_get(mf->path, i));
        else
            log_print("[%s]", als_get(mf->path, i));
    }
    log_print("\"}");
}

SetFormula *SetFormula_creat()
{
    return mam_alloc(sizeof(SetFormula), OBJ_TYPE__SetFormula, NULL, 0);
}

FormulaContext *FormulaContext_creat()
{
    FormulaContext *fc = mam_alloc(sizeof(FormulaContext), OBJ_TYPE__FormulaContext, NULL, 0);
    fc->member_formulas = als_new(32, "MemberFormula *", THREAD_MAM, NULL);
    fc->set_formulas = als_new(32, "SetFormula *", THREAD_MAM, NULL);
    return fc;
}

MDContext *MDContext_creat()
{
    return mam_alloc(sizeof(MDContext), OBJ_TYPE__MDContext, NULL, 0);
}

// ASTMemberFunc_Parent *ASTMemberFunc_Parent_creat(MemberDef *child_def)
// {
//     ASTMemberFunc_Parent *fn = mam_alloc(sizeof(ASTMemberFunc_Parent), OBJ_TYPE__ASTMemberFunc_Parent, NULL, 0);
//     fn->ast_member = child_def;
//     return fn;
// }

ExpFnSum *ExpFnSum_creat(SetDef *_set, Expression *_exp)
{
    ExpFnSum *sum = mam_alloc(sizeof(ExpFnSum), OBJ_TYPE__ExpFnSum, NULL, 0);
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
    ExpFnCount *count = mam_alloc(sizeof(ExpFnCount), OBJ_TYPE__ExpFnCount, NULL, 0);
    count->include_empty = 1; // default value - include empty
    return count;
}

ExpFnLookUpCube *ExpFnLookUpCube_creat(char *cube_name, char *exp_str, Expression *exp)
{
    ExpFnLookUpCube *luc = mam_alloc(sizeof(ExpFnLookUpCube), OBJ_TYPE__ExpFnLookUpCube, NULL, 0);
    luc->cube_name = cube_name;
    luc->exp_str = exp_str;
    luc->exp = exp;
    return luc;
}

ExpFnIif *ExpFnIif_creat(BooleanExpression *bool_exp, Expression *exp1, Expression *exp2)
{
    ExpFnIif *iif = mam_alloc(sizeof(ExpFnIif), OBJ_TYPE__ExpFnIif, NULL, 0);
    iif->bool_exp = bool_exp;
    iif->exp1 = exp1;
    iif->exp2 = exp2;
    return iif;
}

ExpFnCoalesceEmpty *ExpFnCoalesceEmpty_creat(ArrayList *exp_ls)
{
    ExpFnCoalesceEmpty *ce = mam_alloc(sizeof(ExpFnCoalesceEmpty), OBJ_TYPE__ExpFnCoalesceEmpty, NULL, 0);
    ce->exp_ls = exp_ls;
    return ce;
}

DimRoleDef *DimRoleDef_creat()
{
    return mam_alloc(sizeof(DimRoleDef), OBJ_TYPE__DimRoleDef, NULL, 0);
}

BooleanFactory *BooleanFactory_creat(Expression *le, char ops, Expression *re)
{
    BooleanFactory *fac = mam_alloc(sizeof(BooleanFactory), OBJ_TYPE__BooleanFactory, NULL, 0);
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
    BooleanTerm *term = mam_alloc(sizeof(BooleanTerm), OBJ_TYPE__BooleanTerm, NULL, 0);
    term->factories = als_new(16, "BooleanFactory *", THREAD_MAM, NULL);
    return term;
}

void BooleanTerm_addFactory(BooleanTerm *term, BooleanFactory *fac)
{
    als_add(term->factories, fac);
}

BooleanExpression *BooleanExpression_creat()
{
    BooleanExpression *exp = mam_alloc(sizeof(BooleanExpression), OBJ_TYPE__BooleanExpression, NULL, 0);
    exp->terms = als_new(16, "BooleanTerm *", THREAD_MAM, NULL);
    return exp;
}

void BooleanExpression_addTerm(BooleanExpression *exp, BooleanTerm *term)
{
    als_add(exp->terms, term);
}

// ASTMemberFunc_CurrentMember *ASTMemberFunc_CurrentMember_creat()
// {
//     return mam_alloc(sizeof(ASTMemberFunc_CurrentMember), OBJ_TYPE__ASTMemberFunc_CurrentMember, NULL, 0);
// }

// ASTMemberFunc_PrevMember *ASTMemberFunc_PrevMember_creat(MemberDef *m_def)
// {
//     ASTMemberFunc_PrevMember *fn = mam_alloc(sizeof(ASTMemberFunc_PrevMember), OBJ_TYPE__ASTMemberFunc_PrevMember, NULL, 0);
//     fn->curr_mr = m_def;
//     return fn;
// }

ASTMemberFunc_ParallelPeriod *ASTMemberFunc_ParallelPeriod_creat(LevelRoleDef *lvDef, Expression *idx, MemberDef *mDef)
{
    ASTMemberFunc_ParallelPeriod *pp = mam_alloc(sizeof(ASTMemberFunc_ParallelPeriod), OBJ_TYPE__ASTMemberFunc_ParallelPeriod, NULL, 0);
    pp->lvr_def = lvDef;
    pp->index = idx;
    pp->mr_def = mDef;
    return pp;
}


ASTMemberFunc_ClosingPeriod *ASTMemberFunc_ClosingPeriod_creat(LevelRoleDef *lvr, MemberDef *mr) {
    ASTMemberFunc_ClosingPeriod *cp = mam_alloc(sizeof(ASTMemberFunc_ClosingPeriod), OBJ_TYPE__ASTMemberFunc_ClosingPeriod, NULL, 0);
    cp->lvr_def = lvr;
    cp->mr_def = mr;
    return cp;
}

ASTMemberFunc_OpeningPeriod *ASTMemberFunc_OpeningPeriod_creat(LevelRoleDef *lvr, MemberDef *mr) {
    ASTMemberFunc_OpeningPeriod *op = mam_alloc(sizeof(ASTMemberFunc_OpeningPeriod), OBJ_TYPE__ASTMemberFunc_OpeningPeriod, NULL, 0);
    op->lvr_def = lvr;
    op->mr_def = mr;
    return op;
}


// ASTMemberFunc_FirstChild *ASTMemberFunc_FirstChild_creat(MemberDef *mr) {
//     ASTMemberFunc_FirstChild *mr_fn = mam_alloc(sizeof(ASTMemberFunc_FirstChild), OBJ_TYPE__ASTMemberFunc_FirstChild, NULL, 0);
//     mr_fn->mr_def = mr;
//     return mr_fn;
// }

ASTMemberFunc_LastChild *ASTMemberFunc_LastChild_creat(MemberDef *mr) {
    ASTMemberFunc_LastChild *mr_fn = mam_alloc(sizeof(ASTMemberFunc_LastChild), OBJ_TYPE__ASTMemberFunc_LastChild, NULL, 0);
    mr_fn->mr_def = mr;
    return mr_fn;
}


ASTMemberFunc_FirstSibling *ASTMemberFunc_FirstSibling_creat(MemberDef *mr) {
    ASTMemberFunc_FirstSibling *mr_fn = mam_alloc(sizeof(ASTMemberFunc_FirstSibling), OBJ_TYPE__ASTMemberFunc_FirstSibling, NULL, 0);
    mr_fn->mr_def = mr;
    return mr_fn;
}

ASTMemberFunc_LastSibling *ASTMemberFunc_LastSibling_creat(MemberDef *mr) {
    ASTMemberFunc_LastSibling *mr_fn = mam_alloc(sizeof(ASTMemberFunc_LastSibling), OBJ_TYPE__ASTMemberFunc_LastSibling, NULL, 0);
    mr_fn->mr_def = mr;
    return mr_fn;
}


ASTMemberFunc_Lag *ASTMemberFunc_Lag_creat(MemberDef *_mr_def, long _index) {
    ASTMemberFunc_Lag *mr_fn = mam_alloc(sizeof(ASTMemberFunc_Lag), OBJ_TYPE__ASTMemberFunc_Lag, NULL, 0);
    mr_fn->mr_def = _mr_def;
    mr_fn->index = _index;
    return mr_fn;
}