%{
// yacc_f_001
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "mdx.h"

int yyerror(const char *);
extern int yylex();
extern int yyparse();

extern int eucparser_scan_string(const char *s);
extern void eucparser_cleanup();

extern char *yytext;

Stack YC_STC = { 0 };

// yacc_f_002
%}

/* key words */
%token CREATE		/* create */
%token DIMENSIONS	/* dimensions */
%token LEVELS		/* levels */
%token MEMBERS		/* members */
%token BUILD		/* build */
%token CUBE			/* cube */
%token MEASURES		/* measures */
%token INSERT		/* insert */
%token WITH			/* with */
%token SELECT		/* select */
%token FROM			/* from */
%token ON			/* on */
%token WHERE		/* where */
%token MEMBER		/* member */
%token AS			/* as */
%token AND			/* and */
%token OR			/* or */

%token COLUMNS		/* columns */
%token ROWS			/* rows */
%token PAGES		/* pages */
%token CHAPTERS		/* chapters */
%token SECTIONS		/* sections */

%token NIL			/* null */

/* set functions key words */
%token SET			/* set */
%token CHILDREN		/* children */
%token CROSS_JOIN	/* crossjoin */
%token FILTER		/* filter */
%token LATERAL_MEMBERS	/* lateralMembers */
%token ORDER		/* order */
%token ASC			/* ASC */
%token DESC			/* DESC */
%token BASC			/* BASC */
%token BDESC		/* BDESC */
%token TOP_COUNT	/* topCount */
%token EXCEPT		/* except */
%token ALL			/* ALL */
%token YTD			/* Ytd */
%token DESCENDANTS
%token SELF
%token AFTER
%token BEFORE
%token BEFORE_AND_AFTER
%token SELF_AND_AFTER
%token SELF_AND_BEFORE
%token SELF_BEFORE_AFTER
%token LEAVES
%token TAIL
%token BOTTOM_PERCENT
%token TOP_PERCENT
%token UNION
%token INTERSECT

/* member functions key words */
%token PARENT			/* parent */
%token CURRENT_MEMBER	/* currentmember */
%token PREV_MEMBER		/* prevmember */
%token PARALLEL_PERIOD	/* parallelPeriod */

/* expression functions key words */
%token SUM				/* sum */
%token COUNT			/* count */
%token EXCLUDEEMPTY		/* EXCLUDEEMPTY */
%token INCLUDEEMPTY		/* INCLUDEEMPTY */
%token LOOK_UP_CUBE 	/* lookUpCube */
%token IIF				/* iif */
%token COALESCE_EMPTY	/* coalesceEmpty */

/* punctuations */
%token COMMA				/* , */
%token DOT					/* . */
%token COLON				/* : */
%token SEMICOLON			/* ; */

%token ROUND_BRACKET_L		/* ( */
%token ROUND_BRACKET_R		/* ) */
%token BRACE_L				/* { */
%token BRACE_R				/* } */

%token PLUS					/* + */
%token MINUS				/* - */
%token MULTIPLIED			/* * */
%token DIVIDED				/* / */
%token LESS					/* <  (Less Than)				 */
%token LESS_EQ				/* <= (Less Than or Equal To)	 */
%token NOT_EQ				/* <> (Not Equal To)			 */
%token EQ					/* =  (Equal To)				 */
%token GREA					/* >  (Greater Than)			 */
%token GREA_EQ				/* >= (Greater Than or Equal To) */

%token FLAG_EXP  /* @@EXP */

%token VAR
%token BLOCK

%token STRING

%token DECIMAL

%%

statement:
	create_dimensions SEMICOLON {
		stack_push(&YC_STC, IDS_STRLS_CRTDIMS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  | create_levels SEMICOLON {
		stack_push(&YC_STC, IDS_ARRLS_DIMS_LVS_INFO);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
 	}
  |	create_members SEMICOLON {
		stack_push(&YC_STC, IDS_STRLS_CRTMBRS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  |	build_cube SEMICOLON {
		stack_push(&YC_STC, IDS_OBJLS_BIUCUBE);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  | insert_cube_measures SEMICOLON {
	  	stack_push(&YC_STC, IDS_CXOBJ_ISRTCUBEMEARS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  | multi_dim_query SEMICOLON {
		stack_push(&YC_STC, IDS_MULTI_DIM_SELECT_DEF);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  | FLAG_EXP expression {
		// do nothing

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
;

multi_dim_query:
	with_section SELECT axes_statement FROM cube__statement {
		CubeDef *cube_def;
		stack_pop(&YC_STC, (void **) &cube_def);
		ArrayList *ax_def_ls;
		stack_pop(&YC_STC, (void **) &ax_def_ls);

		SelectDef *select_def = SelectDef_new(THREAD_MAM, NULL);
		select_def->cube_def = cube_def;
		select_def->ax_def_ls = ax_def_ls;

		FormulaContext *fc;
		stack_pop(&YC_STC, (void **) &fc);
		select_def->member_formulas = fc->member_formulas;
		select_def->set_formulas = fc->set_formulas;
		stack_push(&YC_STC, select_def);
	}
  |	SELECT axes_statement FROM cube__statement {
		CubeDef *cube_def;
		stack_pop(&YC_STC, (void **) &cube_def);
		ArrayList *ax_def_ls;
		stack_pop(&YC_STC, (void **) &ax_def_ls);

		SelectDef *select_def = SelectDef_new(THREAD_MAM, NULL);
		select_def->cube_def = cube_def;
		select_def->ax_def_ls = ax_def_ls;

		stack_push(&YC_STC, select_def);
	}
  | multi_dim_query WHERE tuple_statement {
		TupleDef *where_tuple_def;
		stack_pop(&YC_STC, (void **) &where_tuple_def);
		SelectDef *select_def;
		stack_pop(&YC_STC, (void **) &select_def);
		select_def->where_tuple_def = where_tuple_def;
		stack_push(&YC_STC, select_def);
	}
;

with_section:
	WITH {
		FormulaContext *fc = FormulaContext_creat();
		stack_push(&YC_STC, fc);
	}
  | with_section member_formula_statement {
		MemberFormula *mf;
		stack_pop(&YC_STC, (void **) &mf);
		FormulaContext *fc;
		stack_pop(&YC_STC, (void **) &fc);
		als_add(fc->member_formulas, mf);
		stack_push(&YC_STC, fc);
	}
  | with_section set_formula_statement {
		SetFormula *sf;
		stack_pop(&YC_STC, (void **) &sf);
		FormulaContext *fc;
		stack_pop(&YC_STC, (void **) &fc);
		als_add(fc->set_formulas, sf);
		stack_push(&YC_STC, fc);
	}
;

set_formula_statement:
	SET var_or_block AS set_statement {
		SetFormula *sf = SetFormula_creat();
		stack_pop(&YC_STC, (void **) &(sf->set_def));
		stack_pop(&YC_STC, (void **) &(sf->var_block));
		stack_push(&YC_STC, sf);
	}
;

member_formula_statement:
	MEMBER var_block_chain AS expression {
		MemberFormula *mf = MemberFormula_creat();
		stack_pop(&YC_STC, (void **) &(mf->exp));
		stack_pop(&YC_STC, (void **) &(mf->path));
		stack_push(&YC_STC, mf);
	}
;

expression:
	term {
		Term *t;
		stack_pop(&YC_STC, (void **) &t);
		Expression *e = Expression_creat();
		Expression_plus_term(e, t);
		stack_push(&YC_STC, e);
	}
  | expression PLUS term {
		Term *t;
		stack_pop(&YC_STC, (void **) &t);
		Expression *e;
		stack_pop(&YC_STC, (void **) &e);
		Expression_plus_term(e, t);
		stack_push(&YC_STC, e);
	}
  | expression MINUS term {
		Term *t;
		stack_pop(&YC_STC, (void **) &t);
		Expression *e;
		stack_pop(&YC_STC, (void **) &e);
		Expression_minus_term(e, t);
		stack_push(&YC_STC, e);
	}
;

term:
	factory {
		Factory *f;
		stack_pop(&YC_STC, (void **) &f);
		Term *t = Term_creat();
		Term_mul_factory(t, f);
		stack_push(&YC_STC, t);
	}
  | term MULTIPLIED factory {
		Factory *f;
		stack_pop(&YC_STC, (void **) &f);
		Term *t = Term_creat();
		stack_pop(&YC_STC, (void **) &t);
		Term_mul_factory(t, f);
		stack_push(&YC_STC, t);
	}
  | term DIVIDED factory {
		Factory *f;
		stack_pop(&YC_STC, (void **) &f);
		Term *t = Term_creat();
		stack_pop(&YC_STC, (void **) &t);
		Term_div_factory(t, f);
		stack_push(&YC_STC, t);
	}
;

factory:
	DECIMAL {
		Factory *f = Factory_creat();
		f->t_cons = FACTORY_DEF__DECIMAL;
		f->decimal = strtod(yytext, NULL);
		stack_push(&YC_STC, f);
	}
  |	tuple_statement {
		TupleDef *t_def;
		stack_pop(&YC_STC, (void **) &t_def);

		Factory *f = Factory_creat();
		f->t_cons = FACTORY_DEF__TUP_DEF;
		f->tuple_def = t_def;

		stack_push(&YC_STC, f);
	}
  | ROUND_BRACKET_L expression ROUND_BRACKET_R {
		Factory *factory = Factory_creat();
		factory->t_cons = FACTORY_DEF__EXPRESSION;
		stack_pop(&YC_STC, (void **) &(factory->exp));
		stack_push(&YC_STC, factory);
	}
  | expression_function {
		Factory *factory = Factory_creat();
		factory->t_cons = FACTORY_DEF__EXP_FN;
		stack_pop(&YC_STC, (void **) &(factory->exp));
		stack_push(&YC_STC, factory);
	}
;

expression_function:
	exp_fn_sum {
		// do nothing
	}
  | exp_fn_count {
		// do nothing
	}
  | exp_fn__look_up_cube {
		// do nothing
	}
  | exp_fn__iif {
		// do nothing
	}
  | exp_fn__coalesce_empty {
		// do nothing
	}
;

exp_fn__coalesce_empty:
	COALESCE_EMPTY ROUND_BRACKET_L expression_list ROUND_BRACKET_R {
		ArrayList *exp_ls;
		stack_pop(&YC_STC, (void **) &exp_ls);
		stack_push(&YC_STC, ExpFnCoalesceEmpty_creat(exp_ls));
	}
;

expression_list:
	expression {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		ArrayList *exp_ls = als_new(8, "Expression *", THREAD_MAM, NULL);
		als_add(exp_ls, exp);
		stack_push(&YC_STC, exp_ls);
	}
  | expression_list COMMA expression {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		ArrayList *exp_ls;
		stack_pop(&YC_STC, (void **) &exp_ls);
		als_add(exp_ls, exp);
		stack_push(&YC_STC, exp_ls);
	}
;

exp_fn__iif:
	IIF ROUND_BRACKET_L boolean_expression COMMA expression COMMA expression ROUND_BRACKET_R {
		Expression *exp2;
		stack_pop(&YC_STC, (void **) &exp2);
		Expression *exp1;
		stack_pop(&YC_STC, (void **) &exp1);
		BooleanExpression *bool_exp;
		stack_pop(&YC_STC, (void **) &bool_exp);
		stack_push(&YC_STC, ExpFnIif_creat(bool_exp, exp1, exp2));
	}
;

exp_fn__look_up_cube:
	LOOK_UP_CUBE ROUND_BRACKET_L str COMMA str ROUND_BRACKET_R {
		char *exp_str;
		stack_pop(&YC_STC, (void **) &exp_str);
		char *cube_name;
		stack_pop(&YC_STC, (void **) &cube_name);
		stack_push(&YC_STC, ExpFnLookUpCube_creat(cube_name, exp_str, NULL));
	}
  | LOOK_UP_CUBE ROUND_BRACKET_L str COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		char *cube_name;
		stack_pop(&YC_STC, (void **) &cube_name);
		stack_push(&YC_STC, ExpFnLookUpCube_creat(cube_name, NULL, exp));
	}
  | LOOK_UP_CUBE ROUND_BRACKET_L var_or_block COMMA str ROUND_BRACKET_R {
		char *exp_str;
		stack_pop(&YC_STC, (void **) &exp_str);
		char *cube_name;
		stack_pop(&YC_STC, (void **) &cube_name);
		stack_push(&YC_STC, ExpFnLookUpCube_creat(cube_name, exp_str, NULL));
	}
  | LOOK_UP_CUBE ROUND_BRACKET_L var_or_block COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		char *cube_name;
		stack_pop(&YC_STC, (void **) &cube_name);
		stack_push(&YC_STC, ExpFnLookUpCube_creat(cube_name, NULL, exp));
	}
;

exp_fn_count:
	COUNT ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		SetDef *set_def;
		stack_pop(&YC_STC, (void **) &set_def);
		ExpFnCount *count = ExpFnCount_creat();
		ExpFnCount_set_setDef(count, set_def);
		stack_push(&YC_STC, count);
	}
  | COUNT ROUND_BRACKET_L set_statement COMMA EXCLUDEEMPTY ROUND_BRACKET_R {
		SetDef *set_def;
		stack_pop(&YC_STC, (void **) &set_def);
		ExpFnCount *count = ExpFnCount_creat();
		ExpFnCount_set_setDef(count, set_def);
		ExpFnCount_excludeEmpty(count);
		stack_push(&YC_STC, count);
	}
  | COUNT ROUND_BRACKET_L set_statement COMMA INCLUDEEMPTY ROUND_BRACKET_R {
		SetDef *set_def;
		stack_pop(&YC_STC, (void **) &set_def);
		ExpFnCount *count = ExpFnCount_creat();
		ExpFnCount_set_setDef(count, set_def);
		stack_push(&YC_STC, count);
	}
;

exp_fn_sum:
	SUM ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ExpFnSum *sum = ExpFnSum_creat(NULL, NULL);
		stack_pop(&YC_STC, (void **) &(sum->set_def));
		stack_push(&YC_STC, sum);
	}
  | SUM ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ExpFnSum *sum = ExpFnSum_creat(NULL, NULL);
		stack_pop(&YC_STC, (void **) &(sum->exp));
		stack_pop(&YC_STC, (void **) &(sum->set_def));
		stack_push(&YC_STC, sum);
	}
;

cube__statement:
	var_or_block {
		char *cube_name;
		stack_pop(&YC_STC, (void **) &cube_name);
		CubeDef *cube_def = ids_cubedef_new(cube_name);
		stack_push(&YC_STC, cube_def);
	}
;

axes_statement:
	axis_statement {
		AxisDef *ax_def;
		stack_pop(&YC_STC, (void **) &ax_def);
		ArrayList *ax_def_ls = als_new(32, "AxisDef *", THREAD_MAM, NULL);
		als_add(ax_def_ls, ax_def);
		stack_push(&YC_STC, ax_def_ls);
	}
  | axes_statement COMMA axis_statement {
		AxisDef *ax_def;
		stack_pop(&YC_STC, (void **) &ax_def);
		ArrayList *ax_def_ls;
		stack_pop(&YC_STC, (void **) &ax_def_ls);
		als_add(ax_def_ls, ax_def);
		stack_push(&YC_STC, ax_def_ls);
	}
;

axis_statement:
	set_statement ON axis_num {
		void *ax_num;
		stack_pop(&YC_STC, &ax_num);
		SetDef *set_def;
		stack_pop(&YC_STC, (void **) &set_def);
		AxisDef *axis_def = ids_axisdef_new(set_def, (unsigned short)(*(long *)&ax_num));
		stack_push(&YC_STC, axis_def);
	}
;

axis_num:
	DECIMAL {
		stack_push(&YC_STC, (void *)(0x00UL | atoi(yytext)));
	}
  | COLUMNS {
		stack_push(&YC_STC, (void *)0x00UL);
	}
  | ROWS {
		stack_push(&YC_STC, (void *)0x01UL);
	}
  | PAGES {
		stack_push(&YC_STC, (void *)0x02UL);
	}
  | CHAPTERS {
		stack_push(&YC_STC, (void *)0x03UL);
	}
  | SECTIONS {
		stack_push(&YC_STC, (void *)0x04UL);
	}
;

set_statement:
	BRACE_L tuples_statement BRACE_R {
		ArrayList *t_def_ls;
		stack_pop(&YC_STC, (void **) &t_def_ls);
		SetDef *set_def = ids_setdef_new(SET_DEF__TUP_DEF_LS);
		ids_setdef__set_tuple_def_ls(set_def, t_def_ls);
		stack_push(&YC_STC, set_def);
	}
  | set_function {
		SetDef *set_def = ids_setdef_new(SET_DEF__SET_FUNCTION);
		stack_pop(&YC_STC, (void **) &(set_def->set_fn));
		stack_push(&YC_STC, set_def);
	}
  | var_or_block {
		SetDef *set_def = ids_setdef_new(SET_DEF__VAR_OR_BLOCK);
		stack_pop(&YC_STC, (void **) &(set_def->var_block));
		stack_push(&YC_STC, set_def);
	}
;

set_function:
	CHILDREN ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		SetFnChildren *fn = SetFnChildren_creat(mbr_def);
		stack_push(&YC_STC, fn);
	}
  | MEMBERS ROUND_BRACKET_L dimension_statement ROUND_BRACKET_R {
		SetFnMembers *fn_ms = SetFnMembers_creat();
		stack_pop(&YC_STC, (void **) &(fn_ms->dr_def));
		stack_push(&YC_STC, fn_ms);
	}
  | MEMBERS ROUND_BRACKET_L dimension_statement COMMA var_or_block ROUND_BRACKET_R {
		SetFnMembers *fn_ms = SetFnMembers_creat();
		char *option;
		stack_pop(&YC_STC, (void **) &option);
		strcpy(fn_ms->option, option);
		stack_pop(&YC_STC, (void **) &(fn_ms->dr_def));
		stack_push(&YC_STC, fn_ms);
	}
  | MEMBERS ROUND_BRACKET_L dimension_statement COMMA ALL ROUND_BRACKET_R {
		SetFnMembers *fn_ms = SetFnMembers_creat();
		strcpy(fn_ms->option, "ALL");
		stack_pop(&YC_STC, (void **) &(fn_ms->dr_def));
		stack_push(&YC_STC, fn_ms);
	}
  | CROSS_JOIN ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		SetFnCrossJoin *cross_join = SetFnCrossJoin_creat();
		ArrayList *set_def_ls;
		stack_pop(&YC_STC, (void **) &set_def_ls);
		int i, len = als_size(set_def_ls);
		for (i=0;i<len;i++) {
			SetFnCrossJoin_add_set(cross_join, als_get(set_def_ls, i));
		}
		stack_push(&YC_STC, cross_join);
	}
  | FILTER ROUND_BRACKET_L set_statement COMMA boolean_expression ROUND_BRACKET_R {
		BooleanExpression *bool_exp;
		SetDef *set_def;
		stack_pop(&YC_STC, (void **) &bool_exp);
		stack_pop(&YC_STC, (void **) &set_def);
		SetFnFilter *filter = SetFnFilter_creat(set_def, bool_exp);
		stack_push(&YC_STC, filter);
	}
  | LATERAL_MEMBERS ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		SetFnLateralMembers *lateral_ms = SetFnLateralMembers_creat(mbr_def);
		stack_push(&YC_STC, lateral_ms);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_ASC);
		stack_push(&YC_STC, order);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA ASC ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_ASC);
		stack_push(&YC_STC, order);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA DESC ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_DESC);
		stack_push(&YC_STC, order);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA BASC ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_BASC);
		stack_push(&YC_STC, order);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA BDESC ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_BDESC);
		stack_push(&YC_STC, order);
	}
  | TOP_COUNT ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		Expression *count_exp;
		stack_pop(&YC_STC, (void **) &count_exp);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		SetFnTopCount *top_count = SetFnTopCount_creat(set, count_exp, NULL);
		stack_push(&YC_STC, top_count);
	}
  | TOP_COUNT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		Expression *num_exp;
		stack_pop(&YC_STC, (void **) &num_exp);
		Expression *count_exp;
		stack_pop(&YC_STC, (void **) &count_exp);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		SetFnTopCount *top_count = SetFnTopCount_creat(set, count_exp, num_exp);
		stack_push(&YC_STC, top_count);
	}
  | EXCEPT ROUND_BRACKET_L set_statement COMMA set_statement ROUND_BRACKET_R {
		SetDef *set_2;
		stack_pop(&YC_STC, (void **) &set_2);
		SetDef *set_1;
		stack_pop(&YC_STC, (void **) &set_1);
		SetFnExcept *except = SetFnExcept_creat(set_1, set_2, -1);
		stack_push(&YC_STC, except);
	}
  | EXCEPT ROUND_BRACKET_L set_statement COMMA set_statement COMMA ALL ROUND_BRACKET_R {
		SetDef *set_2;
		stack_pop(&YC_STC, (void **) &set_2);
		SetDef *set_1;
		stack_pop(&YC_STC, (void **) &set_1);
		SetFnExcept *except = SetFnExcept_creat(set_1, set_2, SET_FN__EXCEPT_ALL);
		stack_push(&YC_STC, except);
	}
  | YTD ROUND_BRACKET_L ROUND_BRACKET_R {
		stack_push(&YC_STC, SetFnYTD_creat(NULL));
	}
  | YTD ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		stack_push(&YC_STC, SetFnYTD_creat(mbr_def));
	}
  | DESCENDANTS ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		stack_push(&YC_STC, SetFnDescendants_creat(mbr_def, NULL, NULL, SET_FN__DESCENDANTS_OPT_SELF));
	}
  | DESCENDANTS ROUND_BRACKET_L member_statement COMMA level_role_statement ROUND_BRACKET_R {
		LevelRoleDef *lvr_def;
		stack_pop(&YC_STC, (void **) &lvr_def);
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		stack_push(&YC_STC, SetFnDescendants_creat(mbr_def, lvr_def, NULL, SET_FN__DESCENDANTS_OPT_SELF));
	}
  | DESCENDANTS ROUND_BRACKET_L member_statement COMMA level_role_statement COMMA desc_flag ROUND_BRACKET_R {
		void *vf;
		stack_pop(&YC_STC, (void **) &vf);
		long *lflag = (long *)&vf;
		char flag = *lflag;
		LevelRoleDef *lvr_def;
		stack_pop(&YC_STC, (void **) &lvr_def);
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		stack_push(&YC_STC, SetFnDescendants_creat(mbr_def, lvr_def, NULL, flag));
	}
  | DESCENDANTS ROUND_BRACKET_L member_statement COMMA expression ROUND_BRACKET_R {
		Expression *distance;
		stack_pop(&YC_STC, (void **) &distance);
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		stack_push(&YC_STC, SetFnDescendants_creat(mbr_def, NULL, distance, SET_FN__DESCENDANTS_OPT_SELF));
	}
  | DESCENDANTS ROUND_BRACKET_L member_statement COMMA expression COMMA desc_flag ROUND_BRACKET_R {
		void *vf;
		stack_pop(&YC_STC, (void **) &vf);
		long *lflag = (long *)&vf;
		char flag = *lflag;
		Expression *distance;
		stack_pop(&YC_STC, (void **) &distance);
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		stack_push(&YC_STC, SetFnDescendants_creat(mbr_def, NULL, distance, flag));
	}
  | TAIL ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		stack_push(&YC_STC, SetFnTail_creat(set, NULL));
	}
  | TAIL ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		Expression *count;
		stack_pop(&YC_STC, (void **) &count);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		stack_push(&YC_STC, SetFnTail_creat(set, count));
	}
  | BOTTOM_PERCENT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		Expression *percentage;
		stack_pop(&YC_STC, (void **) &percentage);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		stack_push(&YC_STC, SetFnBottomOrTopPercent_creat(SET_FN__BOTTOM_PERCENT, set, percentage, exp));
	}
  | TOP_PERCENT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		Expression *percentage;
		stack_pop(&YC_STC, (void **) &percentage);
		SetDef *set;
		stack_pop(&YC_STC, (void **) &set);
		stack_push(&YC_STC, SetFnBottomOrTopPercent_creat(SET_FN__TOP_PERCENT, set, percentage, exp));
	}
  | UNION ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		ArrayList *set_def_ls;
		stack_pop(&YC_STC, (void **) &set_def_ls);
		stack_push(&YC_STC, SetFnUnion_creat(set_def_ls, 0));
	}
  | UNION ROUND_BRACKET_L set_list COMMA ALL ROUND_BRACKET_R {
		ArrayList *set_def_ls;
		stack_pop(&YC_STC, (void **) &set_def_ls);
		stack_push(&YC_STC, SetFnUnion_creat(set_def_ls, SET_FN__UNION_ALL));
	}
  | INTERSECT ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		ArrayList *set_def_ls;
		stack_pop(&YC_STC, (void **) &set_def_ls);
		stack_push(&YC_STC, SetFnIntersect_creat(set_def_ls, 0));
	}
  | INTERSECT ROUND_BRACKET_L set_list COMMA ALL ROUND_BRACKET_R {
		ArrayList *set_def_ls;
		stack_pop(&YC_STC, (void **) &set_def_ls);
		stack_push(&YC_STC, SetFnIntersect_creat(set_def_ls, SET_FN__INTERSECT_ALL));
	}
;

desc_flag:
	SELF {
		long flag = SET_FN__DESCENDANTS_OPT_SELF;
		stack_push(&YC_STC, *((void **)&flag));
	}
  | AFTER {
		long flag = SET_FN__DESCENDANTS_OPT_AFTER;
		stack_push(&YC_STC, *((void **)&flag));
	}
  | BEFORE {
		long flag = SET_FN__DESCENDANTS_OPT_BEFORE;
		stack_push(&YC_STC, *((void **)&flag));
	}
  | BEFORE_AND_AFTER {
		long flag = SET_FN__DESCENDANTS_OPT_BEFORE_AND_AFTER;
		stack_push(&YC_STC, *((void **)&flag));
	}
  | SELF_AND_AFTER {
		long flag = SET_FN__DESCENDANTS_OPT_SELF_AND_AFTER;
		stack_push(&YC_STC, *((void **)&flag));
	}
  | SELF_AND_BEFORE {
		long flag = SET_FN__DESCENDANTS_OPT_SELF_AND_BEFORE;
		stack_push(&YC_STC, *((void **)&flag));
	}
  | SELF_BEFORE_AFTER {
		long flag = SET_FN__DESCENDANTS_OPT_SELF_BEFORE_AFTER;
		stack_push(&YC_STC, *((void **)&flag));
	}
  | LEAVES {
		long flag = SET_FN__DESCENDANTS_OPT_LEAVES;
		stack_push(&YC_STC, *((void **)&flag));
	}
;

boolean_expression:
	boolean_term {
		BooleanTerm *bt;
		stack_pop(&YC_STC, (void **) &bt);
		BooleanExpression *bool_exp = BooleanExpression_creat();
		BooleanExpression_addTerm(bool_exp, bt);
		stack_push(&YC_STC, bool_exp);
	}
  | boolean_expression OR boolean_term {
		BooleanTerm *bt;
		stack_pop(&YC_STC, (void **) &bt);
		BooleanExpression *bool_exp;
		stack_pop(&YC_STC, (void **) &bool_exp);
		BooleanExpression_addTerm(bool_exp, bt);
		stack_push(&YC_STC, bool_exp);
	}
;

boolean_term:
	boolean_factory {
		BooleanFactory *bf;
		stack_pop(&YC_STC, (void **) &bf);
		BooleanTerm *bt = BooleanTerm_creat();
		BooleanTerm_addFactory(bt, bf);
		stack_push(&YC_STC, bt);
	}
  | boolean_term AND boolean_factory {
		BooleanFactory *bf;
		stack_pop(&YC_STC, (void **) &bf);
		BooleanTerm *bt;
		stack_pop(&YC_STC, (void **) &bt);
		BooleanTerm_addFactory(bt, bf);
		stack_push(&YC_STC, bt);
	}
;

boolean_factory:
	expression LESS expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&YC_STC, (void **) &righ_exp);
		stack_pop(&YC_STC, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__LESS, righ_exp);
		stack_push(&YC_STC, bf);
	}
  | expression LESS_EQ expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&YC_STC, (void **) &righ_exp);
		stack_pop(&YC_STC, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__LESS_EQ, righ_exp);
		stack_push(&YC_STC, bf);
	}
  | expression EQ expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&YC_STC, (void **) &righ_exp);
		stack_pop(&YC_STC, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__EQ, righ_exp);
		stack_push(&YC_STC, bf);
	}
  | expression NOT_EQ expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&YC_STC, (void **) &righ_exp);
		stack_pop(&YC_STC, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__NOT_EQ, righ_exp);
		stack_push(&YC_STC, bf);
	}
  | expression GREA expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&YC_STC, (void **) &righ_exp);
		stack_pop(&YC_STC, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__GREA, righ_exp);
		stack_push(&YC_STC, bf);
	}
  | expression GREA_EQ expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&YC_STC, (void **) &righ_exp);
		stack_pop(&YC_STC, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__GREA_EQ, righ_exp);
		stack_push(&YC_STC, bf);
	}
  | ROUND_BRACKET_L boolean_expression ROUND_BRACKET_R {
		BooleanExpression *bool_exp;
		stack_pop(&YC_STC, (void **) &bool_exp);
		BooleanFactory *bf = BooleanFactory_creat(NULL, 0, NULL);
		BooleanFactory_setBoolExp(bf, bool_exp);
		stack_push(&YC_STC, bf);
	}
;

set_list:
	set_statement {
		ArrayList *set_def_ls = als_new(16, "SetDef *", THREAD_MAM, NULL);
		SetDef *sd;
		stack_pop(&YC_STC, (void **) &sd);
		als_add(set_def_ls, sd);
		stack_push(&YC_STC, set_def_ls);
	}
  | set_list COMMA set_statement {
		SetDef *sd;
		stack_pop(&YC_STC, (void **) &sd);
		ArrayList *set_def_ls;
		stack_pop(&YC_STC, (void **) &set_def_ls);
		als_add(set_def_ls, sd);
		stack_push(&YC_STC, set_def_ls);
	}
;

dimension_statement:
	var_or_block {
		DimRoleDef *dr_def = DimRoleDef_creat();
		stack_pop(&YC_STC, (void **) &(dr_def->name));
		stack_push(&YC_STC, dr_def);
	}
;

tuples_statement:
	general_chain {
		GeneralChainExpression *gce;
		stack_pop(&YC_STC, (void **) &gce);
		gce->final_form = OBJ_TYPE__MddTuple;
		ArrayList *t_def_ls = als_new(32, "TupleDef * | GeneralChainExpression *", THREAD_MAM, NULL);
		als_add(t_def_ls, gce);
		stack_push(&YC_STC, t_def_ls);
	}
  | tuple_statement {
		TupleDef *t_def;
		stack_pop(&YC_STC, (void **) &t_def);
		ArrayList *t_def_ls = als_new(32, "TupleDef * | GeneralChainExpression *", THREAD_MAM, NULL);
		als_add(t_def_ls, t_def);
		stack_push(&YC_STC, t_def_ls);
	}
  | tuples_statement COMMA tuple_statement {
		TupleDef *t_def;
		stack_pop(&YC_STC, (void **) &t_def);
		ArrayList *t_def_ls;
		stack_pop(&YC_STC, (void **) &t_def_ls);
		als_add(t_def_ls, t_def);
		stack_push(&YC_STC, t_def_ls);
	}
  | tuples_statement COMMA general_chain {
		GeneralChainExpression *gce;
		stack_pop(&YC_STC, (void **) &gce);
		gce->final_form = OBJ_TYPE__MddTuple;
		ArrayList *t_def_ls;
		stack_pop(&YC_STC, (void **) &t_def_ls);
		als_add(t_def_ls, gce);
		stack_push(&YC_STC, t_def_ls);
	}
;

tuple_statement:
	ROUND_BRACKET_L mbrs_statement ROUND_BRACKET_R {
		MembersDef *ms_def;
		stack_pop(&YC_STC, (void **) &ms_def);
		TupleDef *t_def = ids_tupledef_new(TUPLE_DEF__MBRS_DEF);
		ids_tupledef___set_mbrs_def(t_def, ms_def);
		stack_push(&YC_STC, t_def);
	}
;

mbrs_statement:
	member_statement {
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		MembersDef *ms_def = ids_mbrsdef_new(MBRS_DEF__MBR_DEF_LS);
		ids_mbrsdef__add_mbr_def(ms_def, mbr_def);
		stack_push(&YC_STC, ms_def);
	}
  | mbrs_statement COMMA member_statement {
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		MembersDef *ms_def;
		stack_pop(&YC_STC, (void **) &ms_def);
		ids_mbrsdef__add_mbr_def(ms_def, mbr_def);
		stack_push(&YC_STC, ms_def);
  }
;

member_statement:
	var_block_chain {
		ArrayList *mbr_abs_path;
		stack_pop(&YC_STC, (void **) &mbr_abs_path);
		MemberDef *mbr_def = ids_mbrdef_new__mbr_abs_path(mbr_abs_path);
		stack_push(&YC_STC, mbr_def);
	}
  | PARENT ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberFnParent *fn = MemberFnParent_creat(NULL);
		stack_pop(&YC_STC, (void **) &(fn->child_def));
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = fn;
		stack_push(&YC_STC, mbr_def);
	}
  | CURRENT_MEMBER ROUND_BRACKET_L dimension_statement ROUND_BRACKET_R {
		MemberFnCurrentMember *cm = MemberFnCurrentMember_creat();
		stack_pop(&YC_STC, (void **) &(cm->dr_def));
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = cm;
		stack_push(&YC_STC, mbr_def);
	}
  | PREV_MEMBER ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *curr_mr;
		stack_pop(&YC_STC, (void **) &curr_mr);
		MemberFnPrevMember *fn = MemberFnPrevMember_creat(curr_mr);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = fn;
		stack_push(&YC_STC, mbr_def);
	}
  | member_role_fn_parallel_period {
		MemberRoleFnParallelPeriod *pp;
		stack_pop(&YC_STC, (void **) &pp);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = pp;
		stack_push(&YC_STC, mbr_def);
	}
;

member_role_fn_parallel_period:
	PARALLEL_PERIOD ROUND_BRACKET_L ROUND_BRACKET_R {
		MemberRoleFnParallelPeriod *pp = MemberRoleFnParallelPeriod_creat(NULL, NULL, NULL);
		stack_push(&YC_STC, pp);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L level_role_statement ROUND_BRACKET_R {
		LevelRoleDef *lvr_def;
		stack_pop(&YC_STC, (void **) &lvr_def);
		MemberRoleFnParallelPeriod *pp = MemberRoleFnParallelPeriod_creat(lvr_def, NULL, NULL);
		stack_push(&YC_STC, pp);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L level_role_statement COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		LevelRoleDef *lvr_def;
		stack_pop(&YC_STC, (void **) &lvr_def);
		MemberRoleFnParallelPeriod *pp = MemberRoleFnParallelPeriod_creat(lvr_def, exp, NULL);
		stack_push(&YC_STC, pp);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L level_role_statement COMMA expression COMMA member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&YC_STC, (void **) &mbr_def);
		Expression *exp;
		stack_pop(&YC_STC, (void **) &exp);
		LevelRoleDef *lvr_def;
		stack_pop(&YC_STC, (void **) &lvr_def);
		MemberRoleFnParallelPeriod *pp = MemberRoleFnParallelPeriod_creat(lvr_def, exp, mbr_def);
		stack_push(&YC_STC, pp);
	}
;

level_role_statement:
	var_block_chain {
		ArrayList *lr_path;
		stack_pop(&YC_STC, (void **) &lr_path);
		LevelRoleDef *lvr_def = LevelRoleDef_creat(lr_path);
		stack_push(&YC_STC, lvr_def);
	}
;

insert_cube_measures:
	INSERT var_or_block vector_measures {
		ArrayList *ls_vms = als_new(128, "{ insert_cube_measures ::= }, { IDSVectorMears * }", THREAD_MAM, NULL);
		IDSVectorMears *ids_vm;
		stack_pop(&YC_STC, (void **) &ids_vm);
		als_add(ls_vms, ids_vm);
		stack_push(&YC_STC, ls_vms);
	}
  | insert_cube_measures COMMA vector_measures {
		IDSVectorMears *ids_vm;
		stack_pop(&YC_STC, (void **) &ids_vm);
		ArrayList *ls_vms;
		stack_pop(&YC_STC, (void **) &ls_vms);
		als_add(ls_vms, ids_vm);
		stack_push(&YC_STC, ls_vms);
	}
;

vector_measures:
	ROUND_BRACKET_L vector MEASURES measures_values ROUND_BRACKET_R {
		ArrayList *ls_vector, *ls_mears_vals;
		stack_pop(&YC_STC, (void **) &ls_mears_vals);
		stack_pop(&YC_STC, (void **) &ls_vector);
		IDSVectorMears *ids_vm = mam_alloc(sizeof(IDSVectorMears), OBJ_TYPE__IDSVectorMears, NULL, 0);
		ids_vm->ls_vector = ls_vector;
		ids_vm->ls_mears_vals = ls_mears_vals;
		stack_push(&YC_STC, ids_vm);
	}
;

vector:
	mdm_entity_path {
		ArrayList *ls_vector = als_new(16, "{ vector ::= }, { ArrayList * }", THREAD_MAM, NULL);
		ArrayList *ls_mep;
		stack_pop(&YC_STC, (void **) &ls_mep);
		als_add(ls_vector, ls_mep);
		stack_push(&YC_STC, ls_vector);
	}
  | vector COMMA mdm_entity_path {
		ArrayList *ls_mep, *ls_vector;
		stack_pop(&YC_STC, (void **) &ls_mep);
		stack_pop(&YC_STC, (void **) &ls_vector);
		als_add(ls_vector, ls_mep);
		stack_push(&YC_STC, ls_vector);
	}
;

measures_values:
	NIL {
		// ArrayList *mear_vals_ls = als_new(1, "This is an empty list.", THREAD_MAM, NULL);
		stack_push(&YC_STC, als_new(1, "This is an empty list.", THREAD_MAM, NULL));
	}
  | measures_values_existing {
		// need do nothing
	}
;

measures_values_existing:
	var_or_block DECIMAL {
		char *mmbr_name;
		stack_pop(&YC_STC, (void **) &mmbr_name);
		double *val = mam_alloc(sizeof(double), OBJ_TYPE__RAW_BYTES, NULL, 0);
		*val = atof(yytext);
		ArrayList *mear_vals_ls = als_new(16, "{ yacc measures_values ::= }, { 0,2,4 ... char * }, { 1,3,5 ... double * }", THREAD_MAM, NULL);
		als_add(mear_vals_ls, mmbr_name);
		als_add(mear_vals_ls, val);
		stack_push(&YC_STC, mear_vals_ls);
	}
  | measures_values var_or_block DECIMAL {
		char *mmbr_name;
		stack_pop(&YC_STC, (void **) &mmbr_name);
		ArrayList *mear_vals_ls;
		stack_pop(&YC_STC, (void **) &mear_vals_ls);
		double *val = mam_alloc(sizeof(double), OBJ_TYPE__RAW_BYTES, NULL, 0);
		*val = atof(yytext);
		als_add(mear_vals_ls, mmbr_name);
		als_add(mear_vals_ls, val);
		stack_push(&YC_STC, mear_vals_ls);
	}
;

mdm_entity_path:
	var_or_block {
		ArrayList *path_ls = als_new(12, "yacc mdm_entity_path ::= , type of elements is char *", THREAD_MAM, NULL);
		char *str;
		stack_pop(&YC_STC, (void **) &str);
		als_add(path_ls, str);
		stack_push(&YC_STC, path_ls);
	}
  | mdm_entity_path DOT var_or_block {
		char *str;
		stack_pop(&YC_STC, (void **) &str);
		ArrayList *path_ls;
		stack_pop(&YC_STC, (void **) &path_ls);
		als_add(path_ls, str);
		stack_push(&YC_STC, path_ls);
	}
;

create_dimensions:
	CREATE DIMENSIONS vars {
		// no need to do anything.
	}
;

create_levels:
	CREATE LEVELS dim_levels {
		ArrayList *dim_lvs;
		stack_pop(&YC_STC, (void **) &dim_lvs);
		ArrayList *dim_lv_map_ls = als_new(32, "ArrayList *", THREAD_MAM, NULL);
		als_add(dim_lv_map_ls, dim_lvs);
		stack_push(&YC_STC, dim_lv_map_ls);
	}
  | create_levels COMMA dim_levels {
		ArrayList *dim_lvs;
		stack_pop(&YC_STC, (void **) &dim_lvs);
		ArrayList *dim_lv_map_ls;
		stack_pop(&YC_STC, (void **) &dim_lv_map_ls);
		als_add(dim_lv_map_ls, dim_lvs);
		stack_push(&YC_STC, dim_lv_map_ls);
	}
;

dim_levels:
	var_or_block ROUND_BRACKET_L levels_list ROUND_BRACKET_R {
		ArrayList *dim_lvs;
		stack_pop(&YC_STC, (void **) &dim_lvs);
		char *dim_name;
		stack_pop(&YC_STC, (void **) &dim_name);
		ArrayList_set(dim_lvs, 0, dim_name);
		stack_push(&YC_STC, dim_lvs);
	}
;

levels_list:
	decimal_value COLON var_or_block {
		char *lv_name;
		stack_pop(&YC_STC, (void **) &lv_name);
		void *lv_trans;
		stack_pop(&YC_STC, (void **) &lv_trans);
		ArrayList *lv_ls = als_new(64, "[ (char *dim_name), (long level), (char *level_name), (long level), (char *level_name) ... ]", THREAD_MAM, NULL);
		als_add(lv_ls, NULL);
		als_add(lv_ls, lv_trans);
		als_add(lv_ls, lv_name);
		stack_push(&YC_STC, lv_ls);
	}
  | levels_list COMMA decimal_value COLON var_or_block {
		char *lv_name;
		stack_pop(&YC_STC, (void **) &lv_name);
		void *lv_trans;
		stack_pop(&YC_STC, (void **) &lv_trans);
		ArrayList *lv_ls;
		stack_pop(&YC_STC, (void **) &lv_ls);
		als_add(lv_ls, lv_trans);
		als_add(lv_ls, lv_name);
		stack_push(&YC_STC, lv_ls);
	}
;

decimal_value:
	DECIMAL {
		long level = atoi(yytext);
		stack_push(&YC_STC, *((void **)&level));
	}
;

create_members:
	CREATE MEMBERS var_block_chain {
		ArrayList *mbr_path_ls;
		stack_pop(&YC_STC, (void **) &mbr_path_ls);
		ArrayList *mbrs_ls = als_new(128, "ele type: ArrayList *, yacc create_members", THREAD_MAM, NULL);
		als_add(mbrs_ls, mbr_path_ls);
		stack_push(&YC_STC, mbrs_ls);
	}
  |	create_members COMMA var_block_chain {
		ArrayList *mbr_path_ls;
		stack_pop(&YC_STC, (void **) &mbr_path_ls);
		ArrayList *mbrs_ls;
		stack_pop(&YC_STC, (void **) &mbrs_ls);
		als_add(mbrs_ls, mbr_path_ls);
		stack_push(&YC_STC, mbrs_ls);
	}
;

var_block_chain:
	var_or_block {
		char *str;
		stack_pop(&YC_STC, (void **) &str);
		ArrayList *als = als_new(16, "ele type: char *, yacc var_block_chain", THREAD_MAM, NULL);
		als_add(als, str);
		stack_push(&YC_STC, als);
	}
  |	var_block_chain DOT var_or_block {
		char *str;
		stack_pop(&YC_STC, (void **) &str);
		ArrayList *als;
		stack_pop(&YC_STC, (void **) &als);
		als_add(als, str);
		stack_push(&YC_STC, als);
	}
;

build_cube:
	BUILD CUBE var_or_block DIMENSIONS dims_and_roles MEASURES vars {
		// do nothing
	}
;

dims_and_roles:
	var_or_block var_or_block {
		char *dim_name, *role_name;
		stack_pop(&YC_STC, (void **) &role_name);
		stack_pop(&YC_STC, (void **) &dim_name);
		ArrayList *dr_ls = als_new(64, "yacc dims_and_roles ::= var_or_block var_or_block", THREAD_MAM, NULL);
		als_add(dr_ls, dim_name);
		als_add(dr_ls, role_name);
		stack_push(&YC_STC, dr_ls);
	}
  |	dims_and_roles var_or_block var_or_block {
		char *dim_name, *role_name;
		stack_pop(&YC_STC, (void **) &role_name);
		stack_pop(&YC_STC, (void **) &dim_name);
		ArrayList *dr_ls;
		stack_pop(&YC_STC, (void **) &dr_ls);
		als_add(dr_ls, dim_name);
		als_add(dr_ls, role_name);
		stack_push(&YC_STC, dr_ls);
	}
;

str:
	STRING {
		char *str = mam_alloc(strlen(yytext) - 1, OBJ_TYPE__STRING, NULL, 0);
		memcpy(str, yytext + 1, strlen(yytext) - 2);
		stack_push(&YC_STC, str);
	}
;

vars:
	var_or_block	{
		char *vb_str;
		stack_pop(&YC_STC, (void **) &vb_str);

		ArrayList *vb_ls = als_new(8, "yacc vars ::=", THREAD_MAM, NULL);

		als_add(vb_ls, vb_str);
		stack_push(&YC_STC, vb_ls);
	}
  |	vars var_or_block	{
		char *vb_str;
		stack_pop(&YC_STC, (void **) &vb_str);
		ArrayList *vb_ls;
		stack_pop(&YC_STC, (void **) &vb_ls);
		als_add(vb_ls, vb_str);
		stack_push(&YC_STC, vb_ls);
	}
;

var_or_block:
	VAR	{
		char *str = mam_alloc(strlen(yytext) + 1, OBJ_TYPE__STRING, NULL, 0);
		memcpy(str, yytext, strlen(yytext));
		stack_push(&YC_STC, str);
	}
  |	BLOCK	{
		char *str = mam_alloc(strlen(yytext) - 1, OBJ_TYPE__STRING, NULL, 0);
		memcpy(str, yytext + 1, strlen(yytext) - 2);
		stack_push(&YC_STC, str);
	}
;

chain_ring:
	var_or_block {
		// do nothing
	}
  | str {
		// do nothing
	}
;

general_chain:
	chain_ring {
		GeneralChainExpression *gce = mam_alloc(sizeof(GeneralChainExpression), OBJ_TYPE__GeneralChainExpression, NULL, 0);
		gce->chain = als_new(8, "void *", THREAD_MAM, NULL);
		void *ring;
		stack_pop(&YC_STC, &ring);
		als_add(gce->chain, ring);
		stack_push(&YC_STC, gce);
	}
  | general_chain DOT chain_ring {
		void *ring;
		stack_pop(&YC_STC, &ring);
		GeneralChainExpression *gce;
		stack_pop(&YC_STC, (void **) &gce);
		als_add(gce->chain, ring);
		stack_push(&YC_STC, gce);
	}

%%
// yacc_f_003

void *parse_mdx(char *mdx)
{
	eucparser_scan_string(mdx);
	yyparse();
	eucparser_cleanup();

	return NULL;
}

int yyerror(const char *s)
{
    return -100;
}
// yacc_f_004
