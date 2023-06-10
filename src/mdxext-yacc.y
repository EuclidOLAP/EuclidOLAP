%{
// yacc_f_001
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "mdx.h"
#include "mdm-astmrfn-interpreter.h"
#include "mdm-astlogifn-interpreter.h"

int yyerror(const char *);
extern int yylex();
extern int yyparse();

extern int eucparser_scan_string(const char *s);
extern void eucparser_cleanup();

extern char *yytext;

Stack AST_STACK = { 0 };

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
%token CLOSING_PERIOD	/* ClosingPeriod */
%token OPENING_PERIOD	/* OpeningPeriod */
%token FIRST_CHILD		/* FirstChild */
%token LAST_CHILD		/* LastChild */
%token FIRST_SIBLING	/* FirstSibling */
%token LAST_SIBLING		/* LastSibling */
%token LAG				/* Lag */
%token LEAD				/* Lead */

/* expression functions key words */
%token SUM				/* sum */
%token COUNT			/* count */
%token EXCLUDEEMPTY		/* EXCLUDEEMPTY */
%token INCLUDEEMPTY		/* INCLUDEEMPTY */
%token LOOK_UP_CUBE 	/* lookUpCube */
%token IIF				/* iif */
%token COALESCE_EMPTY	/* coalesceEmpty */

/* Logical Functions */
%token IS_EMPTY			/* IsEmpty */

/* punctuations */
%token COMMA				/* , */
%token DOT					/* . */
%token COLON				/* : */
%token SEMICOLON			/* ; */
%token AMPERSAND			/* & */
%token AT_SIGN				/* @ */

%token ROUND_BRACKET_L		/* ( */
%token ROUND_BRACKET_R		/* ) */
%token OPENING_BRACKET		/* [ */
%token CLOSING_BRACKET		/* ] */
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

%token FLAG_EXP				/* @@EXP */

%token VAR
%token BLOCK

%token STRING

%token DECIMAL

%%

statement:
	create_dimensions SEMICOLON {
		stack_push(&AST_STACK, IDS_STRLS_CRTDIMS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  | create_levels SEMICOLON {
		stack_push(&AST_STACK, IDS_ARRLS_DIMS_LVS_INFO);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
 	}
  |	create_members SEMICOLON {
		stack_push(&AST_STACK, IDS_STRLS_CRTMBRS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  |	build_cube SEMICOLON {
		stack_push(&AST_STACK, IDS_OBJLS_BIUCUBE);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  | insert_cube_measures SEMICOLON {
	  	stack_push(&AST_STACK, IDS_CXOBJ_ISRTCUBEMEARS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
	}
  | multi_dim_query SEMICOLON {
		stack_push(&AST_STACK, IDS_MULTI_DIM_SELECT_DEF);

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
		stack_pop(&AST_STACK, (void **) &cube_def);
		ArrayList *ax_def_ls;
		stack_pop(&AST_STACK, (void **) &ax_def_ls);

		SelectDef *select_def = SelectDef_new(THREAD_MAM, NULL);
		select_def->cube_def = cube_def;
		select_def->ax_def_ls = ax_def_ls;

		FormulaContext *fc;
		stack_pop(&AST_STACK, (void **) &fc);
		select_def->member_formulas = fc->member_formulas;
		select_def->set_formulas = fc->set_formulas;
		stack_push(&AST_STACK, select_def);
	}
  |	SELECT axes_statement FROM cube__statement {
		CubeDef *cube_def;
		stack_pop(&AST_STACK, (void **) &cube_def);
		ArrayList *ax_def_ls;
		stack_pop(&AST_STACK, (void **) &ax_def_ls);

		SelectDef *select_def = SelectDef_new(THREAD_MAM, NULL);
		select_def->cube_def = cube_def;
		select_def->ax_def_ls = ax_def_ls;

		stack_push(&AST_STACK, select_def);
	}
  | multi_dim_query WHERE tuple_statement {
		TupleDef *where_tuple_def;
		stack_pop(&AST_STACK, (void **) &where_tuple_def);
		SelectDef *select_def;
		stack_pop(&AST_STACK, (void **) &select_def);
		select_def->where_tuple_def = where_tuple_def;
		stack_push(&AST_STACK, select_def);
	}
;

with_section:
	WITH {
		FormulaContext *fc = FormulaContext_creat();
		stack_push(&AST_STACK, fc);
	}
  | with_section member_formula_statement {
		MemberFormula *mf;
		stack_pop(&AST_STACK, (void **) &mf);
		FormulaContext *fc;
		stack_pop(&AST_STACK, (void **) &fc);
		als_add(fc->member_formulas, mf);
		stack_push(&AST_STACK, fc);
	}
  | with_section set_formula_statement {
		SetFormula *sf;
		stack_pop(&AST_STACK, (void **) &sf);
		FormulaContext *fc;
		stack_pop(&AST_STACK, (void **) &fc);
		als_add(fc->set_formulas, sf);
		stack_push(&AST_STACK, fc);
	}
;

set_formula_statement:
	SET var_or_block AS set_statement {
		SetFormula *sf = SetFormula_creat();
		stack_pop(&AST_STACK, (void **) &(sf->set_def));
		stack_pop(&AST_STACK, (void **) &(sf->var_block));
		stack_push(&AST_STACK, sf);
	}
;

member_formula_statement:
	MEMBER var_block_chain AS expression {
		MemberFormula *mf = MemberFormula_creat();
		stack_pop(&AST_STACK, (void **) &(mf->exp));
		stack_pop(&AST_STACK, (void **) &(mf->path));
		stack_push(&AST_STACK, mf);
	}
;

expression:
	term {
		Term *t = NULL;
		stack_pop(&AST_STACK, (void **) &t);
		Expression *e = Expression_creat();
		Expression_plus_term(e, t);
		stack_push(&AST_STACK, e);
	}
  | expression PLUS term {
		Term *t = NULL;
		stack_pop(&AST_STACK, (void **) &t);
		Expression *e = NULL;
		stack_pop(&AST_STACK, (void **) &e);
		Expression_plus_term(e, t);
		stack_push(&AST_STACK, e);
	}
  | expression MINUS term {
		Term *t = NULL;
		stack_pop(&AST_STACK, (void **) &t);
		Expression *e = NULL;
		stack_pop(&AST_STACK, (void **) &e);
		Expression_minus_term(e, t);
		stack_push(&AST_STACK, e);
	}
;

term:
	factory {
		Factory *f = NULL;
		stack_pop(&AST_STACK, (void **) &f);
		Term *t = Term_creat();
		Term_mul_factory(t, f);
		stack_push(&AST_STACK, t);
	}
  | term MULTIPLIED factory {
		Factory *f = NULL;
		stack_pop(&AST_STACK, (void **) &f);
		Term *t = Term_creat();
		stack_pop(&AST_STACK, (void **) &t);
		Term_mul_factory(t, f);
		stack_push(&AST_STACK, t);
	}
  | term DIVIDED factory {
		Factory *f = NULL;
		stack_pop(&AST_STACK, (void **) &f);
		Term *t = Term_creat();
		stack_pop(&AST_STACK, (void **) &t);
		Term_div_factory(t, f);
		stack_push(&AST_STACK, t);
	}
;

factory:
	DECIMAL {
		Factory *f = Factory_creat();
		f->t_cons = FACTORY_DEF__DECIMAL;
		f->decimal = strtod(yytext, NULL);
		stack_push(&AST_STACK, f);
	}
  |
	mdm_entity_universal_path {
		MDMEntityUniversalPath *universal_path = NULL;
		stack_pop(&AST_STACK, (void **)&universal_path);

		ArrayList *up_ls = als_new(8, "<MDMEntityUniversalPath *>", THREAD_MAM, NULL);
		als_add(up_ls, universal_path);

		TupleDef *t_def = ids_tupledef_new(TUPLE_DEF__UPATH_LS);
		t_def->universal_path_ls = up_ls;

		Factory *factory = Factory_creat();
		factory->t_cons = FACTORY_DEF__TUP_DEF;
		factory->tuple_def = t_def;

		stack_push(&AST_STACK, factory);
	}
  |
	tuple_2__ {
		TupleDef *t_def = NULL;
		stack_pop(&AST_STACK, (void **) &t_def);

		Factory *f = Factory_creat();
		f->t_cons = FACTORY_DEF__TUP_DEF;
		f->tuple_def = t_def;

		stack_push(&AST_STACK, f);
	}
  | ROUND_BRACKET_L expression ROUND_BRACKET_R {
		Factory *factory = Factory_creat();
		factory->t_cons = FACTORY_DEF__EXPRESSION;
		stack_pop(&AST_STACK, (void **) &(factory->exp));
		stack_push(&AST_STACK, factory);
	}
  | expression_function {
		Factory *factory = Factory_creat();
		factory->t_cons = FACTORY_DEF__EXP_FN;
		stack_pop(&AST_STACK, (void **) &(factory->exp));
		stack_push(&AST_STACK, factory);
	}
  /* | mdm_entity_universal_path {

	} */
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
		ArrayList *exp_ls = NULL;
		stack_pop(&AST_STACK, (void **) &exp_ls);
		stack_push(&AST_STACK, ExpFnCoalesceEmpty_creat(exp_ls));
	}
;

expression_list:
	expression {
		Expression *exp = NULL;
		stack_pop(&AST_STACK, (void **) &exp);
		ArrayList *exp_ls = als_new(8, "Expression *", THREAD_MAM, NULL);
		als_add(exp_ls, exp);
		stack_push(&AST_STACK, exp_ls);
	}
  | expression_list COMMA expression {
		Expression *exp = NULL;
		stack_pop(&AST_STACK, (void **) &exp);
		ArrayList *exp_ls = NULL;
		stack_pop(&AST_STACK, (void **) &exp_ls);
		als_add(exp_ls, exp);
		stack_push(&AST_STACK, exp_ls);
	}
;

exp_fn__iif:
	IIF ROUND_BRACKET_L boolean_expression COMMA expression COMMA expression ROUND_BRACKET_R {
		Expression *exp2 = NULL;
		stack_pop(&AST_STACK, (void **) &exp2);
		Expression *exp1 = NULL;
		stack_pop(&AST_STACK, (void **) &exp1);
		BooleanExpression *bool_exp = NULL;
		stack_pop(&AST_STACK, (void **) &bool_exp);
		stack_push(&AST_STACK, ExpFnIif_creat(bool_exp, exp1, exp2));
	}
;

exp_fn__look_up_cube:
	LOOK_UP_CUBE ROUND_BRACKET_L str COMMA str ROUND_BRACKET_R {
		char *exp_str = NULL;
		stack_pop(&AST_STACK, (void **) &exp_str);
		char *cube_name = NULL;
		stack_pop(&AST_STACK, (void **) &cube_name);
		stack_push(&AST_STACK, ExpFnLookUpCube_creat(cube_name, exp_str, NULL));
	}
  | LOOK_UP_CUBE ROUND_BRACKET_L str COMMA expression ROUND_BRACKET_R {
		Expression *exp = NULL;
		stack_pop(&AST_STACK, (void **) &exp);
		char *cube_name = NULL;
		stack_pop(&AST_STACK, (void **) &cube_name);
		stack_push(&AST_STACK, ExpFnLookUpCube_creat(cube_name, NULL, exp));
	}
  | LOOK_UP_CUBE ROUND_BRACKET_L var_or_block COMMA str ROUND_BRACKET_R {
		char *exp_str = NULL;
		stack_pop(&AST_STACK, (void **) &exp_str);
		char *cube_name = NULL;
		stack_pop(&AST_STACK, (void **) &cube_name);
		stack_push(&AST_STACK, ExpFnLookUpCube_creat(cube_name, exp_str, NULL));
	}
  | LOOK_UP_CUBE ROUND_BRACKET_L var_or_block COMMA expression ROUND_BRACKET_R {
		Expression *exp = NULL;
		stack_pop(&AST_STACK, (void **) &exp);
		char *cube_name = NULL;
		stack_pop(&AST_STACK, (void **) &cube_name);
		stack_push(&AST_STACK, ExpFnLookUpCube_creat(cube_name, NULL, exp));
	}
;

exp_fn_count:
	COUNT ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		SetDef *set_def = NULL;
		stack_pop(&AST_STACK, (void **) &set_def);
		ExpFnCount *count = ExpFnCount_creat();
		ExpFnCount_set_setDef(count, set_def);
		stack_push(&AST_STACK, count);
	}
  | COUNT ROUND_BRACKET_L set_statement COMMA EXCLUDEEMPTY ROUND_BRACKET_R {
		SetDef *set_def = NULL;
		stack_pop(&AST_STACK, (void **) &set_def);
		ExpFnCount *count = ExpFnCount_creat();
		ExpFnCount_set_setDef(count, set_def);
		ExpFnCount_excludeEmpty(count);
		stack_push(&AST_STACK, count);
	}
  | COUNT ROUND_BRACKET_L set_statement COMMA INCLUDEEMPTY ROUND_BRACKET_R {
		SetDef *set_def = NULL;
		stack_pop(&AST_STACK, (void **) &set_def);
		ExpFnCount *count = ExpFnCount_creat();
		ExpFnCount_set_setDef(count, set_def);
		stack_push(&AST_STACK, count);
	}
;

exp_fn_sum:
	SUM ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ExpFnSum *sum = ExpFnSum_creat(NULL, NULL);
		stack_pop(&AST_STACK, (void **) &(sum->set_def));
		stack_push(&AST_STACK, sum);
	}
  | SUM ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ExpFnSum *sum = ExpFnSum_creat(NULL, NULL);
		stack_pop(&AST_STACK, (void **) &(sum->exp));
		stack_pop(&AST_STACK, (void **) &(sum->set_def));
		stack_push(&AST_STACK, sum);
	}
;

cube__statement:
	var_or_block {
		char *cube_name = NULL;
		stack_pop(&AST_STACK, (void **) &cube_name);
		CubeDef *cube_def = ids_cubedef_new(cube_name);
		stack_push(&AST_STACK, cube_def);
	}
;

axes_statement:
	axis_statement {
		AxisDef *ax_def = NULL;
		stack_pop(&AST_STACK, (void **) &ax_def);
		ArrayList *ax_def_ls = als_new(32, "AxisDef *", THREAD_MAM, NULL);
		als_add(ax_def_ls, ax_def);
		stack_push(&AST_STACK, ax_def_ls);
	}
  | axes_statement COMMA axis_statement {
		AxisDef *ax_def = NULL;
		stack_pop(&AST_STACK, (void **) &ax_def);
		ArrayList *ax_def_ls;
		stack_pop(&AST_STACK, (void **) &ax_def_ls);
		als_add(ax_def_ls, ax_def);
		stack_push(&AST_STACK, ax_def_ls);
	}
;

axis_statement:
	set_statement ON axis_num {
		void *ax_num = NULL;
		stack_pop(&AST_STACK, &ax_num);
		SetDef *set_def = NULL;
		stack_pop(&AST_STACK, (void **) &set_def);
		AxisDef *axis_def = ids_axisdef_new(set_def, (unsigned short)(*(long *)&ax_num));
		stack_push(&AST_STACK, axis_def);
	}
;

axis_num:
	DECIMAL {
		stack_push(&AST_STACK, (void *)(0x00UL | atoi(yytext)));
	}
  | COLUMNS {
		stack_push(&AST_STACK, (void *)0x00UL);
	}
  | ROWS {
		stack_push(&AST_STACK, (void *)0x01UL);
	}
  | PAGES {
		stack_push(&AST_STACK, (void *)0x02UL);
	}
  | CHAPTERS {
		stack_push(&AST_STACK, (void *)0x03UL);
	}
  | SECTIONS {
		stack_push(&AST_STACK, (void *)0x04UL);
	}
;

set_statement:
	BRACE_L tuples_statement BRACE_R {
		ArrayList *t_def_ls = NULL;
		stack_pop(&AST_STACK, (void **) &t_def_ls);
		SetDef *set_def = ids_setdef_new(SET_DEF__TUP_DEF_LS);
		ids_setdef__set_tuple_def_ls(set_def, t_def_ls);
		stack_push(&AST_STACK, set_def);
	}
  |
	tuple_statement {
		TupleDef *t_def = NULL;
		stack_pop(&AST_STACK, (void **) &t_def);

		SetDef *set_def = ids_setdef_new(SET_DEF__TUPLE_STATEMENT);
		set_def->tuple_def = t_def;

		stack_push(&AST_STACK, set_def);
	}
  /* | set_function_template {
		SetDef *set_def = ids_setdef_new(SET_DEF__SET_FUNCTION);
		stack_pop(&AST_STACK, (void **) &(set_def->set_fn));
		stack_push(&AST_STACK, set_def);
	} */
  /* | var_or_block {
		SetDef *set_def = ids_setdef_new(SET_DEF__VAR_OR_BLOCK);
		stack_pop(&AST_STACK, (void **) &(set_def->var_block));
		stack_push(&AST_STACK, set_def);
	} */
  |
	mdm_entity_universal_path {
		MDMEntityUniversalPath *md_up = NULL;
		stack_pop(&AST_STACK, (void **) &md_up);
		SetDef *set_def = ids_setdef_new(SET_DEF__MDE_UNI_PATH);
		set_def->up = md_up;
		stack_push(&AST_STACK, set_def);
	}
;

set_function_template:
	CHILDREN ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def = NULL;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		SetFnChildren *fn = SetFnChildren_creat(mbr_def);
		stack_push(&AST_STACK, fn);
	}
  | MEMBERS ROUND_BRACKET_L dimension_statement ROUND_BRACKET_R {
		SetFnMembers *fn_ms = SetFnMembers_creat();
		stack_pop(&AST_STACK, (void **) &(fn_ms->dr_def));
		stack_push(&AST_STACK, fn_ms);
	}
  | MEMBERS ROUND_BRACKET_L dimension_statement COMMA var_or_block ROUND_BRACKET_R {
		SetFnMembers *fn_ms = SetFnMembers_creat();
		char *option = NULL;
		stack_pop(&AST_STACK, (void **) &option);
		strcpy(fn_ms->option, option);
		stack_pop(&AST_STACK, (void **) &(fn_ms->dr_def));
		stack_push(&AST_STACK, fn_ms);
	}
  | MEMBERS ROUND_BRACKET_L dimension_statement COMMA ALL ROUND_BRACKET_R {
		SetFnMembers *fn_ms = SetFnMembers_creat();
		strcpy(fn_ms->option, "ALL");
		stack_pop(&AST_STACK, (void **) &(fn_ms->dr_def));
		stack_push(&AST_STACK, fn_ms);
	}
  | CROSS_JOIN ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		SetFnCrossJoin *cross_join = SetFnCrossJoin_creat();
		ArrayList *set_def_ls = NULL;
		stack_pop(&AST_STACK, (void **) &set_def_ls);
		int i, len = als_size(set_def_ls);
		for (i=0;i<len;i++) {
			SetFnCrossJoin_add_set(cross_join, als_get(set_def_ls, i));
		}
		stack_push(&AST_STACK, cross_join);
	}
  | FILTER ROUND_BRACKET_L set_statement COMMA boolean_expression ROUND_BRACKET_R {
		BooleanExpression *bool_exp = NULL;
		SetDef *set_def = NULL;
		stack_pop(&AST_STACK, (void **) &bool_exp);
		stack_pop(&AST_STACK, (void **) &set_def);
		SetFnFilter *filter = SetFnFilter_creat(set_def, bool_exp);
		stack_push(&AST_STACK, filter);
	}
  | LATERAL_MEMBERS ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def = NULL;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		SetFnLateralMembers *lateral_ms = SetFnLateralMembers_creat(mbr_def);
		stack_push(&AST_STACK, lateral_ms);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		Expression *exp = NULL;
		stack_pop(&AST_STACK, (void **) &exp);
		SetDef *set = NULL;
		stack_pop(&AST_STACK, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_ASC);
		stack_push(&AST_STACK, order);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA ASC ROUND_BRACKET_R {
		Expression *exp = NULL;
		stack_pop(&AST_STACK, (void **) &exp);
		SetDef *set = NULL;
		stack_pop(&AST_STACK, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_ASC);
		stack_push(&AST_STACK, order);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA DESC ROUND_BRACKET_R {
		Expression *exp = NULL;
		stack_pop(&AST_STACK, (void **) &exp);
		SetDef *set = NULL;
		stack_pop(&AST_STACK, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_DESC);
		stack_push(&AST_STACK, order);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA BASC ROUND_BRACKET_R {
		Expression *exp = NULL;
		stack_pop(&AST_STACK, (void **) &exp);
		SetDef *set = NULL;
		stack_pop(&AST_STACK, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_BASC);
		stack_push(&AST_STACK, order);
	}
  | ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA BDESC ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&AST_STACK, (void **) &exp);
		SetDef *set;
		stack_pop(&AST_STACK, (void **) &set);
		SetFnOrder *order = SetFnOrder_creat(set, exp, SET_FN__ORDER_BDESC);
		stack_push(&AST_STACK, order);
	}
  | TOP_COUNT ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		Expression *count_exp;
		stack_pop(&AST_STACK, (void **) &count_exp);
		SetDef *set;
		stack_pop(&AST_STACK, (void **) &set);
		SetFnTopCount *top_count = SetFnTopCount_creat(set, count_exp, NULL);
		stack_push(&AST_STACK, top_count);
	}
  | TOP_COUNT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		Expression *num_exp;
		stack_pop(&AST_STACK, (void **) &num_exp);
		Expression *count_exp;
		stack_pop(&AST_STACK, (void **) &count_exp);
		SetDef *set;
		stack_pop(&AST_STACK, (void **) &set);
		SetFnTopCount *top_count = SetFnTopCount_creat(set, count_exp, num_exp);
		stack_push(&AST_STACK, top_count);
	}
  | EXCEPT ROUND_BRACKET_L set_statement COMMA set_statement ROUND_BRACKET_R {
		SetDef *set_2;
		stack_pop(&AST_STACK, (void **) &set_2);
		SetDef *set_1;
		stack_pop(&AST_STACK, (void **) &set_1);
		SetFnExcept *except = SetFnExcept_creat(set_1, set_2, -1);
		stack_push(&AST_STACK, except);
	}
  | EXCEPT ROUND_BRACKET_L set_statement COMMA set_statement COMMA ALL ROUND_BRACKET_R {
		SetDef *set_2;
		stack_pop(&AST_STACK, (void **) &set_2);
		SetDef *set_1;
		stack_pop(&AST_STACK, (void **) &set_1);
		SetFnExcept *except = SetFnExcept_creat(set_1, set_2, SET_FN__EXCEPT_ALL);
		stack_push(&AST_STACK, except);
	}
  | YTD ROUND_BRACKET_L ROUND_BRACKET_R {
		stack_push(&AST_STACK, SetFnYTD_creat(NULL));
	}
  | YTD ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		stack_push(&AST_STACK, SetFnYTD_creat(mbr_def));
	}
  | DESCENDANTS ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		stack_push(&AST_STACK, SetFnDescendants_creat(mbr_def, NULL, NULL, SET_FN__DESCENDANTS_OPT_SELF));
	}
  | DESCENDANTS ROUND_BRACKET_L member_statement COMMA level_role_statement ROUND_BRACKET_R {
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		stack_push(&AST_STACK, SetFnDescendants_creat(mbr_def, lvr_def, NULL, SET_FN__DESCENDANTS_OPT_SELF));
	}
  | DESCENDANTS ROUND_BRACKET_L member_statement COMMA level_role_statement COMMA desc_flag ROUND_BRACKET_R {
		void *vf;
		stack_pop(&AST_STACK, (void **) &vf);
		long *lflag = (long *)&vf;
		char flag = *lflag;
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		stack_push(&AST_STACK, SetFnDescendants_creat(mbr_def, lvr_def, NULL, flag));
	}
  |
	DESCENDANTS ROUND_BRACKET_L member_statement COMMA DECIMAL {
		Factory *f = Factory_creat();
		f->t_cons = FACTORY_DEF__DECIMAL;
		f->decimal = strtod(yytext, NULL);

		Term *t = Term_creat();
		Term_mul_factory(t, f);

		Expression *distance = Expression_creat();
		Expression_plus_term(distance, t);

		MemberDef *mbr_def = NULL;
		stack_pop(&AST_STACK, (void **)&mbr_def);
		stack_push(&AST_STACK, SetFnDescendants_creat(mbr_def, NULL, distance, SET_FN__DESCENDANTS_OPT_SELF));
	} ROUND_BRACKET_R
  |
	DESCENDANTS ROUND_BRACKET_L member_statement COMMA DECIMAL {
		Factory *f = Factory_creat();
		f->t_cons = FACTORY_DEF__DECIMAL;
		f->decimal = strtod(yytext, NULL);

		Term *t = Term_creat();
		Term_mul_factory(t, f);

		Expression *distance = Expression_creat();
		Expression_plus_term(distance, t);

		stack_push(&AST_STACK, distance);
	} COMMA desc_flag ROUND_BRACKET_R {
		void *vf;
		stack_pop(&AST_STACK, (void **) &vf);
		long *lflag = (long *)&vf;
		char flag = *lflag;

		Expression *distance = NULL;
		stack_pop(&AST_STACK, (void **) &distance);

		MemberDef *mbr_def = NULL;
		stack_pop(&AST_STACK, (void **) &mbr_def);

		stack_push(&AST_STACK, SetFnDescendants_creat(mbr_def, NULL, distance, flag));
	}
  | TAIL ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		SetDef *set;
		stack_pop(&AST_STACK, (void **) &set);
		stack_push(&AST_STACK, SetFnTail_creat(set, NULL));
	}
  | TAIL ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		Expression *count;
		stack_pop(&AST_STACK, (void **) &count);
		SetDef *set;
		stack_pop(&AST_STACK, (void **) &set);
		stack_push(&AST_STACK, SetFnTail_creat(set, count));
	}
  | BOTTOM_PERCENT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&AST_STACK, (void **) &exp);
		Expression *percentage;
		stack_pop(&AST_STACK, (void **) &percentage);
		SetDef *set;
		stack_pop(&AST_STACK, (void **) &set);
		stack_push(&AST_STACK, SetFnBottomOrTopPercent_creat(SET_FN__BOTTOM_PERCENT, set, percentage, exp));
	}
  | TOP_PERCENT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&AST_STACK, (void **) &exp);
		Expression *percentage;
		stack_pop(&AST_STACK, (void **) &percentage);
		SetDef *set;
		stack_pop(&AST_STACK, (void **) &set);
		stack_push(&AST_STACK, SetFnBottomOrTopPercent_creat(SET_FN__TOP_PERCENT, set, percentage, exp));
	}
  | UNION ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		ArrayList *set_def_ls;
		stack_pop(&AST_STACK, (void **) &set_def_ls);
		stack_push(&AST_STACK, SetFnUnion_creat(set_def_ls, 0));
	}
  | UNION ROUND_BRACKET_L set_list COMMA ALL ROUND_BRACKET_R {
		ArrayList *set_def_ls;
		stack_pop(&AST_STACK, (void **) &set_def_ls);
		stack_push(&AST_STACK, SetFnUnion_creat(set_def_ls, SET_FN__UNION_ALL));
	}
  | INTERSECT ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		ArrayList *set_def_ls;
		stack_pop(&AST_STACK, (void **) &set_def_ls);
		stack_push(&AST_STACK, SetFnIntersect_creat(set_def_ls, 0));
	}
  | INTERSECT ROUND_BRACKET_L set_list COMMA ALL ROUND_BRACKET_R {
		ArrayList *set_def_ls;
		stack_pop(&AST_STACK, (void **) &set_def_ls);
		stack_push(&AST_STACK, SetFnIntersect_creat(set_def_ls, SET_FN__INTERSECT_ALL));
	}
;

desc_flag:
	SELF {
		long flag = SET_FN__DESCENDANTS_OPT_SELF;
		stack_push(&AST_STACK, *((void **)&flag));
	}
  | AFTER {
		long flag = SET_FN__DESCENDANTS_OPT_AFTER;
		stack_push(&AST_STACK, *((void **)&flag));
	}
  | BEFORE {
		long flag = SET_FN__DESCENDANTS_OPT_BEFORE;
		stack_push(&AST_STACK, *((void **)&flag));
	}
  | BEFORE_AND_AFTER {
		long flag = SET_FN__DESCENDANTS_OPT_BEFORE_AND_AFTER;
		stack_push(&AST_STACK, *((void **)&flag));
	}
  | SELF_AND_AFTER {
		long flag = SET_FN__DESCENDANTS_OPT_SELF_AND_AFTER;
		stack_push(&AST_STACK, *((void **)&flag));
	}
  | SELF_AND_BEFORE {
		long flag = SET_FN__DESCENDANTS_OPT_SELF_AND_BEFORE;
		stack_push(&AST_STACK, *((void **)&flag));
	}
  | SELF_BEFORE_AFTER {
		long flag = SET_FN__DESCENDANTS_OPT_SELF_BEFORE_AFTER;
		stack_push(&AST_STACK, *((void **)&flag));
	}
  | LEAVES {
		long flag = SET_FN__DESCENDANTS_OPT_LEAVES;
		stack_push(&AST_STACK, *((void **)&flag));
	}
;

boolean_expression:
	boolean_term {
		BooleanTerm *bt;
		stack_pop(&AST_STACK, (void **) &bt);
		BooleanExpression *bool_exp = BooleanExpression_creat();
		BooleanExpression_addTerm(bool_exp, bt);
		stack_push(&AST_STACK, bool_exp);
	}
  | boolean_expression OR boolean_term {
		BooleanTerm *bt;
		stack_pop(&AST_STACK, (void **) &bt);
		BooleanExpression *bool_exp;
		stack_pop(&AST_STACK, (void **) &bool_exp);
		BooleanExpression_addTerm(bool_exp, bt);
		stack_push(&AST_STACK, bool_exp);
	}
;

boolean_term:
	boolean_factory {
		BooleanFactory *bf;
		stack_pop(&AST_STACK, (void **) &bf);
		BooleanTerm *bt = BooleanTerm_creat();
		BooleanTerm_addFactory(bt, bf);
		stack_push(&AST_STACK, bt);
	}
  | boolean_term AND boolean_factory {
		BooleanFactory *bf;
		stack_pop(&AST_STACK, (void **) &bf);
		BooleanTerm *bt;
		stack_pop(&AST_STACK, (void **) &bt);
		BooleanTerm_addFactory(bt, bf);
		stack_push(&AST_STACK, bt);
	}
;

boolean_factory:
	expression LESS expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&AST_STACK, (void **) &righ_exp);
		stack_pop(&AST_STACK, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__LESS, righ_exp);
		stack_push(&AST_STACK, bf);
	}
  | expression LESS_EQ expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&AST_STACK, (void **) &righ_exp);
		stack_pop(&AST_STACK, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__LESS_EQ, righ_exp);
		stack_push(&AST_STACK, bf);
	}
  | expression EQ expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&AST_STACK, (void **) &righ_exp);
		stack_pop(&AST_STACK, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__EQ, righ_exp);
		stack_push(&AST_STACK, bf);
	}
  | expression NOT_EQ expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&AST_STACK, (void **) &righ_exp);
		stack_pop(&AST_STACK, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__NOT_EQ, righ_exp);
		stack_push(&AST_STACK, bf);
	}
  | expression GREA expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&AST_STACK, (void **) &righ_exp);
		stack_pop(&AST_STACK, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__GREA, righ_exp);
		stack_push(&AST_STACK, bf);
	}
  | expression GREA_EQ expression {
		Expression *left_exp, *righ_exp;
		stack_pop(&AST_STACK, (void **) &righ_exp);
		stack_pop(&AST_STACK, (void **) &left_exp);
		BooleanFactory *bf = BooleanFactory_creat(left_exp, BOOL_FAC_OPS__GREA_EQ, righ_exp);
		stack_push(&AST_STACK, bf);
	}
  | ROUND_BRACKET_L boolean_expression ROUND_BRACKET_R {
		BooleanExpression *bool_exp = NULL;
		stack_pop(&AST_STACK, (void **) &bool_exp);
		BooleanFactory *bf = BooleanFactory_creat(NULL, 0, NULL);
		BooleanFactory_setBoolExp(bf, bool_exp);
		stack_push(&AST_STACK, bf);
	}
  | boolean_function {
		void *ast_bool_fn = NULL;
		stack_pop(&AST_STACK, (void **)&ast_bool_fn);

		BooleanFactory *bool_fac = mam_alloc(sizeof(BooleanFactory), OBJ_TYPE__BooleanFactory, NULL, 0);
		bool_fac->ast_boolean_func = ast_bool_fn;

		stack_push(&AST_STACK, bool_fac);
	}
;

set_list:
	set_statement {
		ArrayList *set_def_ls = als_new(16, "SetDef *", THREAD_MAM, NULL);
		SetDef *sd;
		stack_pop(&AST_STACK, (void **) &sd);
		als_add(set_def_ls, sd);
		stack_push(&AST_STACK, set_def_ls);
	}
  | set_list COMMA set_statement {
		SetDef *sd;
		stack_pop(&AST_STACK, (void **) &sd);
		ArrayList *set_def_ls;
		stack_pop(&AST_STACK, (void **) &set_def_ls);
		als_add(set_def_ls, sd);
		stack_push(&AST_STACK, set_def_ls);
	}
;

dimension_statement:
	var_or_block {
		DimRoleDef *dr_def = DimRoleDef_creat();
		stack_pop(&AST_STACK, (void **) &(dr_def->name));
		stack_push(&AST_STACK, dr_def);
	}
;

tuples_statement:
	general_chain {
		GeneralChainExpression *gce;
		stack_pop(&AST_STACK, (void **) &gce);
		gce->final_form = OBJ_TYPE__MddTuple;
		ArrayList *t_def_ls = als_new(32, "TupleDef * | GeneralChainExpression *", THREAD_MAM, NULL);
		als_add(t_def_ls, gce);
		stack_push(&AST_STACK, t_def_ls);
	}
  | tuple_statement {
		TupleDef *t_def;
		stack_pop(&AST_STACK, (void **) &t_def);
		ArrayList *t_def_ls = als_new(32, "TupleDef * | GeneralChainExpression *", THREAD_MAM, NULL);
		als_add(t_def_ls, t_def);
		stack_push(&AST_STACK, t_def_ls);
	}
  | tuples_statement COMMA tuple_statement {
		TupleDef *t_def;
		stack_pop(&AST_STACK, (void **) &t_def);
		ArrayList *t_def_ls;
		stack_pop(&AST_STACK, (void **) &t_def_ls);
		als_add(t_def_ls, t_def);
		stack_push(&AST_STACK, t_def_ls);
	}
  | tuples_statement COMMA general_chain {
		GeneralChainExpression *gce;
		stack_pop(&AST_STACK, (void **) &gce);
		gce->final_form = OBJ_TYPE__MddTuple;
		ArrayList *t_def_ls;
		stack_pop(&AST_STACK, (void **) &t_def_ls);
		als_add(t_def_ls, gce);
		stack_push(&AST_STACK, t_def_ls);
	}
;

tuple_2__:
	ROUND_BRACKET_L up_list_2__ ROUND_BRACKET_R {
		ArrayList *up_ls = NULL;
		stack_pop(&AST_STACK, (void **) &up_ls);

		TupleDef *t_def = ids_tupledef_new(TUPLE_DEF__UPATH_LS);
		t_def->universal_path_ls = up_ls;

		stack_push(&AST_STACK, t_def);
	}
;

tuple_statement:
	/* ROUND_BRACKET_L mbrs_statement ROUND_BRACKET_R {
		MembersDef *ms_def;
		stack_pop(&AST_STACK, (void **) &ms_def);
		TupleDef *t_def = ids_tupledef_new(TUPLE_DEF__MBRS_DEF);
		ids_tupledef___set_mbrs_def(t_def, ms_def);
		stack_push(&AST_STACK, t_def);
	}
  | */
	ROUND_BRACKET_L up_list ROUND_BRACKET_R {
		ArrayList *up_ls = NULL;
		stack_pop(&AST_STACK, (void **) &up_ls);

		TupleDef *t_def = ids_tupledef_new(TUPLE_DEF__UPATH_LS);
		t_def->universal_path_ls = up_ls;

		stack_push(&AST_STACK, t_def);
	}
  /* |
	mdm_entity_universal_path {
		MDMEntityUniversalPath *universal_path = NULL;
		stack_pop(&AST_STACK, (void **)&universal_path);

		ArrayList *up_ls = als_new(1, "<MDMEntityUniversalPath *>", THREAD_MAM, NULL);
		als_add(up_ls, universal_path);

		TupleDef *tuple_def = ids_tupledef_new(TUPLE_DEF__UPATH_LS);
		tuple_def->universal_path_ls = up_ls;

		stack_push(&AST_STACK, tuple_def);
	} */
;

/* mbrs_statement:
	member_statement {
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		MembersDef *ms_def = ids_mbrsdef_new(MBRS_DEF__MBR_DEF_LS);
		ids_mbrsdef__add_mbr_def(ms_def, mbr_def);
		stack_push(&AST_STACK, ms_def);
	}
  | mbrs_statement COMMA member_statement {
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		MembersDef *ms_def;
		stack_pop(&AST_STACK, (void **) &ms_def);
		ids_mbrsdef__add_mbr_def(ms_def, mbr_def);
		stack_push(&AST_STACK, ms_def);
  }
; */

member_statement:
	var_block_chain {
		ArrayList *mbr_abs_path;
		stack_pop(&AST_STACK, (void **) &mbr_abs_path);
		MemberDef *mbr_def = ids_mbrdef_new__mbr_abs_path(mbr_abs_path);
		stack_push(&AST_STACK, mbr_def);
	}
  | PARENT ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		ASTMemberFunc_Parent *fn = ASTMemberFunc_Parent_creat(NULL);
		stack_pop(&AST_STACK, (void **) &(fn->ast_member));
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | CURRENT_MEMBER ROUND_BRACKET_L dimension_statement ROUND_BRACKET_R {
		ASTMemberFunc_CurrentMember *cm = ASTMemberFunc_CurrentMember_creat();
		stack_pop(&AST_STACK, (void **) &(cm->dr_def));
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = cm;
		stack_push(&AST_STACK, mbr_def);
	}
  | PREV_MEMBER ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *curr_mr;
		stack_pop(&AST_STACK, (void **) &curr_mr);
		ASTMemberFunc_PrevMember *fn = ASTMemberFunc_PrevMember_creat(curr_mr);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | FIRST_CHILD ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_FirstChild *mr_fn = ASTMemberFunc_FirstChild_creat(member_role_def);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | LAST_CHILD ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_LastChild *mr_fn = ASTMemberFunc_LastChild_creat(member_role_def);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | FIRST_SIBLING ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		/* FirstSibling */
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_FirstSibling *mr_fn = ASTMemberFunc_FirstSibling_creat(member_role_def);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | LAST_SIBLING ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		/* LastSibling */
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_LastSibling *mr_fn = ASTMemberFunc_LastSibling_creat(member_role_def);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | LAG ROUND_BRACKET_L member_statement COMMA decimal_value ROUND_BRACKET_R {
		void *ptol = NULL;
		stack_pop(&AST_STACK, (void **) &ptol);
		long index = (long) ptol;
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_Lag *mr_fn = ASTMemberFunc_Lag_creat(member_role_def, index);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | LEAD ROUND_BRACKET_L member_statement COMMA decimal_value ROUND_BRACKET_R {
		void *ptol = NULL;
		stack_pop(&AST_STACK, (void **) &ptol);
		long index = (long) ptol;
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_Lag *mr_fn = ASTMemberFunc_Lag_creat(member_role_def, 0 - index);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | member_role_fn_parallel_period {
		ASTMemberFunc_ParallelPeriod *pp;
		stack_pop(&AST_STACK, (void **) &pp);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = pp;
		stack_push(&AST_STACK, mbr_def);
	}
  | member_role_fn_closing_period {
		ASTMemberFunc_ClosingPeriod *closing_period;
		stack_pop(&AST_STACK, (void **) &closing_period);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = closing_period;
		stack_push(&AST_STACK, mbr_def);
	}
  | member_role_fn_opening_period {
		ASTMemberFunc_OpeningPeriod *opening_period;
		stack_pop(&AST_STACK, (void **) &opening_period);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = opening_period;
		stack_push(&AST_STACK, mbr_def);
	}
;


member_function_template:
	PARENT ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		ASTMemberFunc_Parent *fn = ASTMemberFunc_Parent_creat(NULL);
		stack_pop(&AST_STACK, (void **) &(fn->ast_member));
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | CURRENT_MEMBER ROUND_BRACKET_L dimension_statement ROUND_BRACKET_R {
		ASTMemberFunc_CurrentMember *cm = ASTMemberFunc_CurrentMember_creat();
		stack_pop(&AST_STACK, (void **) &(cm->dr_def));
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = cm;
		stack_push(&AST_STACK, mbr_def);
	}
  | PREV_MEMBER ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *curr_mr;
		stack_pop(&AST_STACK, (void **) &curr_mr);
		ASTMemberFunc_PrevMember *fn = ASTMemberFunc_PrevMember_creat(curr_mr);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | FIRST_CHILD ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_FirstChild *mr_fn = ASTMemberFunc_FirstChild_creat(member_role_def);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | LAST_CHILD ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_LastChild *mr_fn = ASTMemberFunc_LastChild_creat(member_role_def);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | FIRST_SIBLING ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		/* FirstSibling */
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_FirstSibling *mr_fn = ASTMemberFunc_FirstSibling_creat(member_role_def);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | LAST_SIBLING ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
		/* LastSibling */
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_LastSibling *mr_fn = ASTMemberFunc_LastSibling_creat(member_role_def);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | LAG ROUND_BRACKET_L member_statement COMMA decimal_value ROUND_BRACKET_R {
		void *ptol = NULL;
		stack_pop(&AST_STACK, (void **) &ptol);
		long index = (long) ptol;
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_Lag *mr_fn = ASTMemberFunc_Lag_creat(member_role_def, index);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  | LEAD ROUND_BRACKET_L member_statement COMMA decimal_value ROUND_BRACKET_R {
		void *ptol = NULL;
		stack_pop(&AST_STACK, (void **) &ptol);
		long index = (long) ptol;
		MemberDef *member_role_def;
		stack_pop(&AST_STACK, (void **) &member_role_def);
		ASTMemberFunc_Lag *mr_fn = ASTMemberFunc_Lag_creat(member_role_def, 0 - index);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = mr_fn;
		stack_push(&AST_STACK, mbr_def);
	}
  |
	member_role_fn_parallel_period {
		ASTMemberFunc_ParallelPeriod *pp;
		stack_pop(&AST_STACK, (void **) &pp);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = pp;
		stack_push(&AST_STACK, mbr_def);
	}
  |
	member_role_fn_closing_period {
		ASTMemberFunc_ClosingPeriod *closing_period;
		stack_pop(&AST_STACK, (void **) &closing_period);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = closing_period;
		stack_push(&AST_STACK, mbr_def);
	}
  |
	member_role_fn_opening_period {
		ASTMemberFunc_OpeningPeriod *opening_period;
		stack_pop(&AST_STACK, (void **) &opening_period);
		MemberDef *mbr_def = MemberDef_creat(MEMBER_DEF__MBR_FUNCTION);
		mbr_def->member_fn = opening_period;
		stack_push(&AST_STACK, mbr_def);
	}
;


member_role_fn_closing_period:
	CLOSING_PERIOD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFunc_ClosingPeriod *mr_fn = ASTMemberFunc_ClosingPeriod_creat(NULL, NULL);
		stack_push(&AST_STACK, mr_fn);
	}
  |
	CLOSING_PERIOD ROUND_BRACKET_L level_role_statement ROUND_BRACKET_R {
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		ASTMemberFunc_ClosingPeriod *mr_fn = ASTMemberFunc_ClosingPeriod_creat(lvr_def, NULL);
		stack_push(&AST_STACK, mr_fn);

	}
  |
	CLOSING_PERIOD ROUND_BRACKET_L level_role_statement COMMA member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		ASTMemberFunc_ClosingPeriod *mr_fn = ASTMemberFunc_ClosingPeriod_creat(lvr_def, mbr_def);
		stack_push(&AST_STACK, mr_fn);

	}
;

member_role_fn_opening_period:
	OPENING_PERIOD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFunc_OpeningPeriod *mr_fn = ASTMemberFunc_OpeningPeriod_creat(NULL, NULL);
		stack_push(&AST_STACK, mr_fn);
	}
  |
	OPENING_PERIOD ROUND_BRACKET_L level_role_statement ROUND_BRACKET_R {
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		ASTMemberFunc_OpeningPeriod *mr_fn = ASTMemberFunc_OpeningPeriod_creat(lvr_def, NULL);
		stack_push(&AST_STACK, mr_fn);

	}
  |
	OPENING_PERIOD ROUND_BRACKET_L level_role_statement COMMA member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		ASTMemberFunc_OpeningPeriod *mr_fn = ASTMemberFunc_OpeningPeriod_creat(lvr_def, mbr_def);
		stack_push(&AST_STACK, mr_fn);
		
	}
;

member_role_fn_parallel_period:
	PARALLEL_PERIOD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFunc_ParallelPeriod *pp = ASTMemberFunc_ParallelPeriod_creat(NULL, NULL, NULL);
		stack_push(&AST_STACK, pp);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L level_role_statement ROUND_BRACKET_R {
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		ASTMemberFunc_ParallelPeriod *pp = ASTMemberFunc_ParallelPeriod_creat(lvr_def, NULL, NULL);
		stack_push(&AST_STACK, pp);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L level_role_statement COMMA expression ROUND_BRACKET_R {
		Expression *exp;
		stack_pop(&AST_STACK, (void **) &exp);
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		ASTMemberFunc_ParallelPeriod *pp = ASTMemberFunc_ParallelPeriod_creat(lvr_def, exp, NULL);
		stack_push(&AST_STACK, pp);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L level_role_statement COMMA expression COMMA member_statement ROUND_BRACKET_R {
		MemberDef *mbr_def;
		stack_pop(&AST_STACK, (void **) &mbr_def);
		Expression *exp;
		stack_pop(&AST_STACK, (void **) &exp);
		LevelRoleDef *lvr_def;
		stack_pop(&AST_STACK, (void **) &lvr_def);
		ASTMemberFunc_ParallelPeriod *pp = ASTMemberFunc_ParallelPeriod_creat(lvr_def, exp, mbr_def);
		stack_push(&AST_STACK, pp);
	}
;

level_role_statement:
	var_block_chain {
		ArrayList *lr_path;
		stack_pop(&AST_STACK, (void **) &lr_path);
		LevelRoleDef *lvr_def = LevelRoleDef_creat(lr_path);
		stack_push(&AST_STACK, lvr_def);
	}
;

insert_cube_measures:
	INSERT var_or_block vector_measures {
		ArrayList *ls_vms = als_new(128, "{ insert_cube_measures ::= }, { IDSVectorMears * }", THREAD_MAM, NULL);
		IDSVectorMears *ids_vm;
		stack_pop(&AST_STACK, (void **) &ids_vm);
		als_add(ls_vms, ids_vm);
		stack_push(&AST_STACK, ls_vms);
	}
  | insert_cube_measures COMMA vector_measures {
		IDSVectorMears *ids_vm;
		stack_pop(&AST_STACK, (void **) &ids_vm);
		ArrayList *ls_vms;
		stack_pop(&AST_STACK, (void **) &ls_vms);
		als_add(ls_vms, ids_vm);
		stack_push(&AST_STACK, ls_vms);
	}
;

vector_measures:
	ROUND_BRACKET_L vector MEASURES measures_values ROUND_BRACKET_R {
		ArrayList *ls_vector, *ls_mears_vals;
		stack_pop(&AST_STACK, (void **) &ls_mears_vals);
		stack_pop(&AST_STACK, (void **) &ls_vector);
		IDSVectorMears *ids_vm = mam_alloc(sizeof(IDSVectorMears), OBJ_TYPE__IDSVectorMears, NULL, 0);
		ids_vm->ls_vector = ls_vector;
		ids_vm->ls_mears_vals = ls_mears_vals;
		stack_push(&AST_STACK, ids_vm);
	}
;

vector:
	mdm_entity_path {
		ArrayList *ls_vector = als_new(16, "{ vector ::= }, { ArrayList * }", THREAD_MAM, NULL);
		ArrayList *ls_mep;
		stack_pop(&AST_STACK, (void **) &ls_mep);
		als_add(ls_vector, ls_mep);
		stack_push(&AST_STACK, ls_vector);
	}
  | vector COMMA mdm_entity_path {
		ArrayList *ls_mep, *ls_vector;
		stack_pop(&AST_STACK, (void **) &ls_mep);
		stack_pop(&AST_STACK, (void **) &ls_vector);
		als_add(ls_vector, ls_mep);
		stack_push(&AST_STACK, ls_vector);
	}
;

measures_values:
	NIL {
		// ArrayList *mear_vals_ls = als_new(1, "This is an empty list.", THREAD_MAM, NULL);
		stack_push(&AST_STACK, als_new(1, "This is an empty list.", THREAD_MAM, NULL));
	}
  | measures_values_existing {
		// need do nothing
	}
;

measures_values_existing:
	var_or_block DECIMAL {
		char *mmbr_name;
		stack_pop(&AST_STACK, (void **) &mmbr_name);
		double *val = mam_alloc(sizeof(double), OBJ_TYPE__RAW_BYTES, NULL, 0);
		*val = atof(yytext);
		ArrayList *mear_vals_ls = als_new(16, "{ yacc measures_values ::= }, { 0,2,4 ... char * }, { 1,3,5 ... double * }", THREAD_MAM, NULL);
		als_add(mear_vals_ls, mmbr_name);
		als_add(mear_vals_ls, val);
		stack_push(&AST_STACK, mear_vals_ls);
	}
  | measures_values var_or_block DECIMAL {
		char *mmbr_name;
		stack_pop(&AST_STACK, (void **) &mmbr_name);
		ArrayList *mear_vals_ls;
		stack_pop(&AST_STACK, (void **) &mear_vals_ls);
		double *val = mam_alloc(sizeof(double), OBJ_TYPE__RAW_BYTES, NULL, 0);
		*val = atof(yytext);
		als_add(mear_vals_ls, mmbr_name);
		als_add(mear_vals_ls, val);
		stack_push(&AST_STACK, mear_vals_ls);
	}
;

mdm_entity_path:
	var_or_block {
		ArrayList *path_ls = als_new(12, "yacc mdm_entity_path ::= , type of elements is char *", THREAD_MAM, NULL);
		char *str;
		stack_pop(&AST_STACK, (void **) &str);
		als_add(path_ls, str);
		stack_push(&AST_STACK, path_ls);
	}
  | mdm_entity_path DOT var_or_block {
		char *str;
		stack_pop(&AST_STACK, (void **) &str);
		ArrayList *path_ls;
		stack_pop(&AST_STACK, (void **) &path_ls);
		als_add(path_ls, str);
		stack_push(&AST_STACK, path_ls);
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
		stack_pop(&AST_STACK, (void **) &dim_lvs);
		ArrayList *dim_lv_map_ls = als_new(32, "ArrayList *", THREAD_MAM, NULL);
		als_add(dim_lv_map_ls, dim_lvs);
		stack_push(&AST_STACK, dim_lv_map_ls);
	}
  | create_levels COMMA dim_levels {
		ArrayList *dim_lvs;
		stack_pop(&AST_STACK, (void **) &dim_lvs);
		ArrayList *dim_lv_map_ls;
		stack_pop(&AST_STACK, (void **) &dim_lv_map_ls);
		als_add(dim_lv_map_ls, dim_lvs);
		stack_push(&AST_STACK, dim_lv_map_ls);
	}
;

dim_levels:
	var_or_block ROUND_BRACKET_L levels_list ROUND_BRACKET_R {
		ArrayList *dim_lvs;
		stack_pop(&AST_STACK, (void **) &dim_lvs);
		char *dim_name;
		stack_pop(&AST_STACK, (void **) &dim_name);
		ArrayList_set(dim_lvs, 0, dim_name);
		stack_push(&AST_STACK, dim_lvs);
	}
;

levels_list:
	decimal_value COLON var_or_block {
		char *lv_name;
		stack_pop(&AST_STACK, (void **) &lv_name);
		void *lv_trans;
		stack_pop(&AST_STACK, (void **) &lv_trans);
		ArrayList *lv_ls = als_new(64, "[ (char *dim_name), (long level), (char *level_name), (long level), (char *level_name) ... ]", THREAD_MAM, NULL);
		als_add(lv_ls, NULL);
		als_add(lv_ls, lv_trans);
		als_add(lv_ls, lv_name);
		stack_push(&AST_STACK, lv_ls);
	}
  | levels_list COMMA decimal_value COLON var_or_block {
		char *lv_name;
		stack_pop(&AST_STACK, (void **) &lv_name);
		void *lv_trans;
		stack_pop(&AST_STACK, (void **) &lv_trans);
		ArrayList *lv_ls;
		stack_pop(&AST_STACK, (void **) &lv_ls);
		als_add(lv_ls, lv_trans);
		als_add(lv_ls, lv_name);
		stack_push(&AST_STACK, lv_ls);
	}
;

decimal_value:
	DECIMAL {
		long level = atoi(yytext);
		stack_push(&AST_STACK, *((void **)&level));
	}
  | MINUS DECIMAL {
		long level = 0 - atoi(yytext);
		stack_push(&AST_STACK, *((void **)&level));
	}
;

create_members:
	CREATE MEMBERS var_block_chain {
		ArrayList *mbr_path_ls;
		stack_pop(&AST_STACK, (void **) &mbr_path_ls);
		ArrayList *mbrs_ls = als_new(128, "ele type: ArrayList *, yacc create_members", THREAD_MAM, NULL);
		als_add(mbrs_ls, mbr_path_ls);
		stack_push(&AST_STACK, mbrs_ls);
	}
  |	create_members COMMA var_block_chain {
		ArrayList *mbr_path_ls;
		stack_pop(&AST_STACK, (void **) &mbr_path_ls);
		ArrayList *mbrs_ls;
		stack_pop(&AST_STACK, (void **) &mbrs_ls);
		als_add(mbrs_ls, mbr_path_ls);
		stack_push(&AST_STACK, mbrs_ls);
	}
;

var_block_chain:
	var_or_block {
		char *str;
		stack_pop(&AST_STACK, (void **) &str);
		ArrayList *als = als_new(16, "ele type: char *, yacc var_block_chain", THREAD_MAM, NULL);
		als_add(als, str);
		stack_push(&AST_STACK, als);
	}
  |	var_block_chain DOT var_or_block {
		char *str;
		stack_pop(&AST_STACK, (void **) &str);
		ArrayList *als;
		stack_pop(&AST_STACK, (void **) &als);
		als_add(als, str);
		stack_push(&AST_STACK, als);
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
		stack_pop(&AST_STACK, (void **) &role_name);
		stack_pop(&AST_STACK, (void **) &dim_name);
		ArrayList *dr_ls = als_new(64, "yacc dims_and_roles ::= var_or_block var_or_block", THREAD_MAM, NULL);
		als_add(dr_ls, dim_name);
		als_add(dr_ls, role_name);
		stack_push(&AST_STACK, dr_ls);
	}
  |	dims_and_roles var_or_block var_or_block {
		char *dim_name, *role_name;
		stack_pop(&AST_STACK, (void **) &role_name);
		stack_pop(&AST_STACK, (void **) &dim_name);
		ArrayList *dr_ls;
		stack_pop(&AST_STACK, (void **) &dr_ls);
		als_add(dr_ls, dim_name);
		als_add(dr_ls, role_name);
		stack_push(&AST_STACK, dr_ls);
	}
;

boolean_function:
	logical_func_isEmpty {
		// don't need do anything at here.
	}
;

logical_func_isEmpty:
	IS_EMPTY ROUND_BRACKET_L expression ROUND_BRACKET_R {
		Expression *_exp = NULL;
		stack_pop(&AST_STACK, (void **)&_exp);

		ASTLogicalFunc_IsEmpty *is_empty = mam_alloc(sizeof(ASTLogicalFunc_IsEmpty), OBJ_TYPE__ASTLogicalFunc_IsEmpty, NULL, 0);
		is_empty->head.interpret = interpret_ast_is_empty;

		is_empty->exp = _exp;

		stack_push(&AST_STACK, is_empty);
	}
;

mrfn_Parent_suftpl:
	PARENT {
		// MemberRoleFuncParent *mr_func = mam_alloc(sizeof(MemberRoleFuncParent), OBJ_TYPE__MemberRoleFuncParent, NULL, 0);
		// mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		ASTMemberFunc_Parent *mr_func = mam_alloc(sizeof(ASTMemberFunc_Parent), OBJ_TYPE__ASTMemberFunc_Parent, NULL, 0);
		mr_func->head.interpret = interpret_ast_mrf_parent;
		stack_push(&AST_STACK, mr_func);
	}
  |
	PARENT ROUND_BRACKET_L ROUND_BRACKET_R {
		// MemberRoleFuncParent *mr_func = mam_alloc(sizeof(MemberRoleFuncParent), OBJ_TYPE__MemberRoleFuncParent, NULL, 0);
		// mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		ASTMemberFunc_Parent *mr_func = mam_alloc(sizeof(ASTMemberFunc_Parent), OBJ_TYPE__ASTMemberFunc_Parent, NULL, 0);
		mr_func->head.interpret = interpret_ast_mrf_parent;
		stack_push(&AST_STACK, mr_func);
	}
  /* |
	PARENT ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		// mdm_entity_universal_path is expected to represent a hierarchy(hierarchy role)
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		MemberRoleFuncParent *mr_func = mam_alloc(sizeof(MemberRoleFuncParent), OBJ_TYPE__MemberRoleFuncParent, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		mr_func->hierarchy = up;
		stack_push(&AST_STACK, mr_func);
	} */
;

mrfn_CurrentMember_suftpl:
	CURRENT_MEMBER {
		MemberRoleFuncCurrentMember *mr_func = mam_alloc(sizeof(MemberRoleFuncCurrentMember), OBJ_TYPE__MemberRoleFuncCurrentMember, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);
	}
  |
	CURRENT_MEMBER ROUND_BRACKET_L ROUND_BRACKET_R {
		MemberRoleFuncCurrentMember *mr_func = mam_alloc(sizeof(MemberRoleFuncCurrentMember), OBJ_TYPE__MemberRoleFuncCurrentMember, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);
	}
;

mrfn_PrevMember_suftpl:
	PREV_MEMBER {
		MemberRoleFuncPrevMember *mr_func = mam_alloc(sizeof(MemberRoleFuncPrevMember), OBJ_TYPE__MemberRoleFuncPrevMember, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);
	}
  |
	PREV_MEMBER ROUND_BRACKET_L ROUND_BRACKET_R {
		MemberRoleFuncPrevMember *mr_func = mam_alloc(sizeof(MemberRoleFuncPrevMember), OBJ_TYPE__MemberRoleFuncPrevMember, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);
	}
;

mrfn_FirstChild_suftpl:
	FIRST_CHILD {
		MemberRoleFuncFirstChild *mr_func = mam_alloc(sizeof(MemberRoleFuncFirstChild), OBJ_TYPE__MemberRoleFuncFirstChild, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);
	}
  |
	FIRST_CHILD ROUND_BRACKET_L ROUND_BRACKET_R {
		MemberRoleFuncFirstChild *mr_func = mam_alloc(sizeof(MemberRoleFuncFirstChild), OBJ_TYPE__MemberRoleFuncFirstChild, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);
	}
;

mrfn_LastChild_suftpl:
	LAST_CHILD {
		MemberRoleFuncLastChild *mr_func = mam_alloc(sizeof(MemberRoleFuncLastChild), OBJ_TYPE__MemberRoleFuncLastChild, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);
	}
  |
	LAST_CHILD ROUND_BRACKET_L ROUND_BRACKET_R {
		MemberRoleFuncLastChild *mr_func = mam_alloc(sizeof(MemberRoleFuncLastChild), OBJ_TYPE__MemberRoleFuncLastChild, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);
	}
;

mrfn_FirstSibling_suftpl:
	FIRST_SIBLING {
		MemberRoleFuncFirstSibling *mr_func = mam_alloc(sizeof(MemberRoleFuncFirstSibling), OBJ_TYPE__MemberRoleFuncFirstSibling, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);

	}
  |
	FIRST_SIBLING ROUND_BRACKET_L ROUND_BRACKET_R {
		MemberRoleFuncFirstSibling *mr_func = mam_alloc(sizeof(MemberRoleFuncFirstSibling), OBJ_TYPE__MemberRoleFuncFirstSibling, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);

	}
  |
	FIRST_SIBLING ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		// mdm_entity_universal_path is expected to represent a hierarchy(hierarchy role)
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		MemberRoleFuncFirstSibling *mr_func = mam_alloc(sizeof(MemberRoleFuncFirstSibling), OBJ_TYPE__MemberRoleFuncFirstSibling, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		mr_func->hierarchy = up;
		stack_push(&AST_STACK, mr_func);
	}
;

mrfn_LastSibling_suftpl:
	LAST_SIBLING {
		MemberRoleFuncLastSibling *mr_func = mam_alloc(sizeof(MemberRoleFuncLastSibling), OBJ_TYPE__MemberRoleFuncLastSibling, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);

	}
  |
	LAST_SIBLING ROUND_BRACKET_L ROUND_BRACKET_R {
		MemberRoleFuncLastSibling *mr_func = mam_alloc(sizeof(MemberRoleFuncLastSibling), OBJ_TYPE__MemberRoleFuncLastSibling, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);

	}
  |
	LAST_SIBLING ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		// mdm_entity_universal_path is expected to represent a hierarchy(hierarchy role)
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		MemberRoleFuncLastSibling *mr_func = mam_alloc(sizeof(MemberRoleFuncLastSibling), OBJ_TYPE__MemberRoleFuncLastSibling, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		mr_func->hierarchy = up;
		stack_push(&AST_STACK, mr_func);
	}
;

mrfn_Lag_suftpl:
	LAG ROUND_BRACKET_L decimal_value ROUND_BRACKET_R {
		void *__vp__ = NULL;
		stack_pop(&AST_STACK, (void **) &__vp__);

		MemberRoleFuncLag *mr_func = mam_alloc(sizeof(MemberRoleFuncLag), OBJ_TYPE__MemberRoleFuncLag, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		mr_func->index = (long) __vp__;
		stack_push(&AST_STACK, mr_func);
	}
;

mrfn_Lead_suftpl:
	LEAD ROUND_BRACKET_L decimal_value ROUND_BRACKET_R {
		void *__vp__ = NULL;
		stack_pop(&AST_STACK, (void **) &__vp__);

		MemberRoleFuncLead *mr_func = mam_alloc(sizeof(MemberRoleFuncLead), OBJ_TYPE__MemberRoleFuncLead, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		mr_func->index = (long) __vp__;
		stack_push(&AST_STACK, mr_func);
	}
;

member_function_template_suffix:
	mrfn_Parent_suftpl {
		// Don't do anything
	}
  |
	mrfn_CurrentMember_suftpl {
		// Don't do anything
	}
  |
	mrfn_PrevMember_suftpl {
		// Don't do anything
	}
  |
	mrfn_FirstChild_suftpl {
		// Don't do anything
	}
  |
	mrfn_LastChild_suftpl {
		// Don't do anything
	}
  |
	mrfn_FirstSibling_suftpl {
		// Don't do anything
	}
  |
	mrfn_LastSibling_suftpl {
		// Don't do anything
	}
  |
	mrfn_Lag_suftpl {
		// Don't do anything
	}
  |
	mrfn_Lead_suftpl {
		// Don't do anything
	}
;

setfn_Children_suftpl:
	CHILDREN {
		SetFuncChildren *mr_func = mam_alloc(sizeof(SetFuncChildren), OBJ_TYPE__SetFuncChildren, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);

	}
  |
	CHILDREN ROUND_BRACKET_L ROUND_BRACKET_R {
		SetFuncChildren *mr_func = mam_alloc(sizeof(SetFuncChildren), OBJ_TYPE__SetFuncChildren, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);

	}
;

setfn_Members_suftpl:
	MEMBERS {
		SetFuncMembers *mr_func = mam_alloc(sizeof(SetFuncMembers), OBJ_TYPE__SetFuncMembers, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);

	}
  |
	MEMBERS ROUND_BRACKET_L ROUND_BRACKET_R {
		SetFuncMembers *mr_func = mam_alloc(sizeof(SetFuncMembers), OBJ_TYPE__SetFuncMembers, NULL, 0);
		mr_func->suf_flag = MDX_FN_SUFFIX_TRUE;
		stack_push(&AST_STACK, mr_func);

	}
;

set_function_template_suffix:
	setfn_Children_suftpl {
		// Don't do anything
	}
  |
	setfn_Members_suftpl {
		// Don't do anything
	}
;

mdm_entity_universal_path:
	mdm_entity_up_segment {
		MdmEntityUpSegment *up_seg = NULL;
		stack_pop(&AST_STACK, (void **) &up_seg);

		MDMEntityUniversalPath *up = mam_alloc(sizeof(MDMEntityUniversalPath), OBJ_TYPE__MDMEntityUniversalPath, NULL, 0);
		up->list = als_new(8, NULL, THREAD_MAM, NULL);

		als_add(up->list, up_seg);
		stack_push(&AST_STACK, up);
	}
  |
	member_function_template {
		MemberDef *m_def = NULL;
		stack_pop(&AST_STACK, (void **) &m_def);

		MDMEntityUniversalPath *up = mam_alloc(sizeof(MDMEntityUniversalPath), OBJ_TYPE__MDMEntityUniversalPath, NULL, 0);
		up->list = als_new(8, NULL, THREAD_MAM, NULL);

		als_add(up->list, m_def);
		stack_push(&AST_STACK, up);

	}
  |
	set_function_template {
		void *set_func_tpl = NULL;
		stack_pop(&AST_STACK, (void **) &set_func_tpl);

		MDMEntityUniversalPath *up = mam_alloc(sizeof(MDMEntityUniversalPath), OBJ_TYPE__MDMEntityUniversalPath, NULL, 0);
		up->list = als_new(8, NULL, THREAD_MAM, NULL);

		als_add(up->list, set_func_tpl);
		stack_push(&AST_STACK, up);
	}
  |
	mdm_entity_universal_path DOT mdm_entity_up_segment {
		MdmEntityUpSegment *up_seg = NULL;
		stack_pop(&AST_STACK, (void **) &up_seg);
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		als_add(up->list, up_seg);
		stack_push(&AST_STACK, up);

	}
  |
	mdm_entity_universal_path DOT member_function_template_suffix {
		MemberDef *suf_mfn_tpl = NULL;
		stack_pop(&AST_STACK, (void **) &suf_mfn_tpl);
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		als_add(up->list, suf_mfn_tpl);
		stack_push(&AST_STACK, up);
	}
  |
	mdm_entity_universal_path DOT set_function_template_suffix {
		void *suf_setfn_tpl = NULL;
		stack_pop(&AST_STACK, (void **) &suf_setfn_tpl);
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		als_add(up->list, suf_setfn_tpl);
		stack_push(&AST_STACK, up);
	}
;

up_list_2__:
	mdm_entity_universal_path COMMA mdm_entity_universal_path {
		MDMEntityUniversalPath *up_1 = NULL;
		stack_pop(&AST_STACK, (void **)&up_1);

		MDMEntityUniversalPath *up_2 = NULL;
		stack_pop(&AST_STACK, (void **)&up_2);
		
		ArrayList *up_ls = als_new(8, "<MDMEntityUniversalPath *>", THREAD_MAM, NULL);
		als_add(up_ls, up_1);
		als_add(up_ls, up_2);

		stack_push(&AST_STACK, up_ls);
	}
  |
	up_list_2__ COMMA mdm_entity_universal_path {
		MDMEntityUniversalPath *universal_path = NULL;
		stack_pop(&AST_STACK, (void **) &universal_path);

		ArrayList *up_ls = NULL;
		stack_pop(&AST_STACK, (void **) &up_ls);

		als_add(up_ls, universal_path);

		stack_push(&AST_STACK, up_ls);
	}
;

up_list:
	mdm_entity_universal_path {
		MDMEntityUniversalPath *universal_path = NULL;
		stack_pop(&AST_STACK, (void **) &universal_path);

		ArrayList *up_ls = als_new(8, "<MDMEntityUniversalPath *>", THREAD_MAM, NULL);
		als_add(up_ls, universal_path);

		stack_push(&AST_STACK, up_ls);
	}
  |
	up_list COMMA mdm_entity_universal_path {
		MDMEntityUniversalPath *universal_path = NULL;
		stack_pop(&AST_STACK, (void **) &universal_path);

		ArrayList *up_ls = NULL;
		stack_pop(&AST_STACK, (void **) &up_ls);

		als_add(up_ls, universal_path);

		stack_push(&AST_STACK, up_ls);
	}
;


mdm_entity_up_segment:
	chain_ring {
		MdmEntityUpSegment *up_seg = mam_alloc(sizeof(MdmEntityUpSegment), OBJ_TYPE__MdmEntityUpSegment, NULL, 0);
		up_seg->type = MEU_SEG_TYPE_TXT;

		char *strp = NULL;
		stack_pop(&AST_STACK, (void **)&strp);

		up_seg->info.seg_str = strp;
		// stack_pop(&AST_STACK, &(up_seg->info.seg_str));

		stack_push(&AST_STACK, up_seg);
	}
  |
	AMPERSAND OPENING_BRACKET DECIMAL {
		MdmEntityUpSegment *up_seg = mam_alloc(sizeof(MdmEntityUpSegment), OBJ_TYPE__MdmEntityUpSegment, NULL, 0);
		up_seg->type = MEU_SEG_TYPE_ID;
		char *__ptr;
		up_seg->info.mde_global_id = strtoul(yytext, &__ptr, 10);
		stack_push(&AST_STACK, up_seg);
	} CLOSING_BRACKET
  |
	AT_SIGN OPENING_BRACKET DECIMAL {
		MdmEntityUpSegment *up_seg = mam_alloc(sizeof(MdmEntityUpSegment), OBJ_TYPE__MdmEntityUpSegment, NULL, 0);
		up_seg->type = MEU_SEG_TYPE_STAMP;
		char *__ptr;
		up_seg->info.mde_timestamp = strtoul(yytext, &__ptr, 10);
		stack_push(&AST_STACK, up_seg);
	} CLOSING_BRACKET
  |
	AMPERSAND DECIMAL {
		MdmEntityUpSegment *up_seg = mam_alloc(sizeof(MdmEntityUpSegment), OBJ_TYPE__MdmEntityUpSegment, NULL, 0);
		up_seg->type = MEU_SEG_TYPE_ID;
		char *__ptr;
		up_seg->info.mde_global_id = strtoul(yytext, &__ptr, 10);
		stack_push(&AST_STACK, up_seg);
	}
  |
	AT_SIGN DECIMAL {
		MdmEntityUpSegment *up_seg = mam_alloc(sizeof(MdmEntityUpSegment), OBJ_TYPE__MdmEntityUpSegment, NULL, 0);
		up_seg->type = MEU_SEG_TYPE_STAMP;
		char *__ptr;
		up_seg->info.mde_timestamp = strtoul(yytext, &__ptr, 10);
		stack_push(&AST_STACK, up_seg);
	}
  |
	AMPERSAND DECIMAL {
		MdmEntityUpSegment *up_seg = mam_alloc(sizeof(MdmEntityUpSegment), OBJ_TYPE__MdmEntityUpSegment, NULL, 0);
		up_seg->type = MEU_SEG_TYPE_ID;
		char *__ptr;
		up_seg->info.mde_global_id = strtoul(yytext, &__ptr, 10);
		stack_push(&AST_STACK, up_seg);
	} BLOCK
  |
	AT_SIGN DECIMAL {
		MdmEntityUpSegment *up_seg = mam_alloc(sizeof(MdmEntityUpSegment), OBJ_TYPE__MdmEntityUpSegment, NULL, 0);
		up_seg->type = MEU_SEG_TYPE_STAMP;
		char *__ptr;
		up_seg->info.mde_timestamp = strtoul(yytext, &__ptr, 10);
		stack_push(&AST_STACK, up_seg);
	} BLOCK
;


str:
	STRING {
		char *str = mam_alloc(strlen(yytext) - 1, OBJ_TYPE__STRING, NULL, 0);
		memcpy(str, yytext + 1, strlen(yytext) - 2);
		stack_push(&AST_STACK, str);
	}
;

vars:
	var_or_block	{
		char *vb_str;
		stack_pop(&AST_STACK, (void **) &vb_str);

		ArrayList *vb_ls = als_new(8, "yacc vars ::=", THREAD_MAM, NULL);

		als_add(vb_ls, vb_str);
		stack_push(&AST_STACK, vb_ls);
	}
  |	vars var_or_block	{
		char *vb_str;
		stack_pop(&AST_STACK, (void **) &vb_str);
		ArrayList *vb_ls;
		stack_pop(&AST_STACK, (void **) &vb_ls);
		als_add(vb_ls, vb_str);
		stack_push(&AST_STACK, vb_ls);
	}
;

var_or_block:
	VAR	{
		char *str = mam_alloc(strlen(yytext) + 1, OBJ_TYPE__STRING, NULL, 0);
		memcpy(str, yytext, strlen(yytext));
		stack_push(&AST_STACK, str);
	}
  |	BLOCK	{
		char *str = mam_alloc(strlen(yytext) - 1, OBJ_TYPE__STRING, NULL, 0);
		memcpy(str, yytext + 1, strlen(yytext) - 2);

		int __length__ = strlen(str);
		int _move_forward = 0;
		for (int i = 0; i < __length__; i++) {
			str[i - _move_forward] = str[i];
			if (str[i] == ']') {
				_move_forward++;
				i++;
			}
		}
		str[__length__ - _move_forward] = 0;

		stack_push(&AST_STACK, str);
	}
;

chain_ring:
	var_or_block {
		// do nothing
	}
  /* | str {
		// do nothing
	} */
;

general_chain:
	chain_ring {
		GeneralChainExpression *gce = mam_alloc(sizeof(GeneralChainExpression), OBJ_TYPE__GeneralChainExpression, NULL, 0);
		gce->chain = als_new(8, "void *", THREAD_MAM, NULL);
		void *ring;
		stack_pop(&AST_STACK, &ring);
		als_add(gce->chain, ring);
		stack_push(&AST_STACK, gce);
	}
  | general_chain DOT chain_ring {
		void *ring;
		stack_pop(&AST_STACK, &ring);
		GeneralChainExpression *gce;
		stack_pop(&AST_STACK, (void **) &gce);
		als_add(gce->chain, ring);
		stack_push(&AST_STACK, gce);
	}

%%
// yacc_f_003

void do_parse_mdx(char *mdx)
{
	eucparser_scan_string(mdx);
	yyparse();
	eucparser_cleanup();
}

int yyerror(const char *s)
{
    return -100;
}
// yacc_f_004
