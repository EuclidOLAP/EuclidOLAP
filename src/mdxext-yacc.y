%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "mdx.h"
#include "mdm-astmrfn-interpreter.h"
#include "mdm-astlogifn-interpreter.h"
#include "mdm-ast-str-fn.h"
#include "mdm-ast-set-func.h"

int yyerror(const char *);

extern int yylex();
extern int yyparse();
extern int eucparser_scan_string(const char *s);
extern void eucparser_cleanup();

extern char *yytext;

Stack AST_STACK = { 0 };
%}

%token EOF_			/* <<EOF>> */

/* key words */
%token CREATE		/* create */
%token DIMENSIONS	/* dimensions */
%token HIERARCHY	/* hierarchy */
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

%token MAKE			/* make */

%token COLUMNS		/* columns */
%token ROWS			/* rows */
%token PAGES		/* pages */
%token CHAPTERS		/* chapters */
%token SECTIONS		/* sections */

%token NIL			/* null */

/* set functions key words */
%token SET				/* set */
%token CHILDREN			/* children */
%token CROSS_JOIN		/* crossjoin */
%token FILTER			/* filter */
%token LATERAL_MEMBERS	/* lateralMembers */
%token ORDER			/* order */
%token TOP_COUNT		/* topCount */
%token EXCEPT			/* except */
%token ALL				/* ALL */
%token YTD				/* Ytd */
%token DESCENDANTS
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

/* Numeric Functions key words */
%token SUM				/* sum */
%token COUNT			/* count */
%token EXCLUDEEMPTY		/* EXCLUDEEMPTY */
%token INCLUDEEMPTY		/* INCLUDEEMPTY */
%token LOOK_UP_CUBE 	/* lookUpCube */
%token IIF				/* iif */
%token COALESCE_EMPTY	/* coalesceEmpty */

/* Logical Functions */
%token IS_EMPTY			/* IsEmpty */

/* String Functions */
%token NAME				/* Name */

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

%token EQUIVALENT_TO		/* -> */

%token FLAG_EXP				/* @@EXP */

%token VAR
%token BLOCK

%token STRING

%token DECIMAL

%%

euclidolap_mdx:
	create_dimensions SEMICOLON EOF_ {
		stack_push(&AST_STACK, IDS_STRLS_CRTDIMS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	create_dimensions EOF_ {
		stack_push(&AST_STACK, IDS_STRLS_CRTDIMS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	create_hierarchy SEMICOLON EOF_ {
		stack_push(&AST_STACK, IDS_CREATE_HIERARCHY);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	create_hierarchy EOF_ {
		stack_push(&AST_STACK, IDS_CREATE_HIERARCHY);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	create_levels SEMICOLON EOF_ {
		stack_push(&AST_STACK, IDS_ARRLS_DIMS_LVS_INFO);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
 	}
  |
	create_levels EOF_ {
		stack_push(&AST_STACK, IDS_ARRLS_DIMS_LVS_INFO);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
 	}
  |
	create_members SEMICOLON EOF_ {
		stack_push(&AST_STACK, IDS_STRLS_CRTMBRS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	create_members EOF_ {
		stack_push(&AST_STACK, IDS_STRLS_CRTMBRS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	build_cube SEMICOLON EOF_ {
		stack_push(&AST_STACK, IDS_OBJLS_BIUCUBE);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	build_cube EOF_ {
		stack_push(&AST_STACK, IDS_OBJLS_BIUCUBE);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	make_equivalent SEMICOLON EOF_ {
		stack_push(&AST_STACK, IDS_MAKE_EQUIVALENT);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	make_equivalent EOF_ {
		stack_push(&AST_STACK, IDS_MAKE_EQUIVALENT);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	insert_cube_measures SEMICOLON EOF_ {
	  	stack_push(&AST_STACK, IDS_CXOBJ_ISRTCUBEMEARS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	insert_cube_measures EOF_ {
	  	stack_push(&AST_STACK, IDS_CXOBJ_ISRTCUBEMEARS);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0001;
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags & 0xFFFD;
	}
  |
	multi_dim_query SEMICOLON EOF_ {
		stack_push(&AST_STACK, IDS_MULTI_DIM_SELECT_DEF);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0003;
	}
  |
	multi_dim_query EOF_ {
		stack_push(&AST_STACK, IDS_MULTI_DIM_SELECT_DEF);

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0003;
	}
  |
	FLAG_EXP expression EOF_ {
		// do nothing

		// Set the MDX parsing done flag to 1 to indicate that the parsing process is complete.
		MemAllocMng *cur_thrd_mam = MemAllocMng_current_thread_mam();
		cur_thrd_mam->bin_flags = cur_thrd_mam->bin_flags | 0x0003;
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

string_function:
	string_func_name {}
;

string_func_name:
	NAME ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);
		ASTStrFunc_Name *func = mam_alloc(sizeof(ASTStrFunc_Name), OBJ_TYPE__ASTStrFunc_Name, NULL, 0);
		func->up = up;
		func->head.interpret = strfn_name_interpreter;
		stack_push(&AST_STACK, func);
	}
  |
	NAME ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTStrFunc_Name *func = mam_alloc(sizeof(ASTStrFunc_Name), OBJ_TYPE__ASTStrFunc_Name, NULL, 0);
		func->head.interpret = strfn_name_interpreter;
		stack_push(&AST_STACK, func);
	}
  |
	NAME {
		ASTStrFunc_Name *func = mam_alloc(sizeof(ASTStrFunc_Name), OBJ_TYPE__ASTStrFunc_Name, NULL, 0);
		func->head.interpret = strfn_name_interpreter;
		stack_push(&AST_STACK, func);
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

		Factory *factory = Factory_creat();
		factory->t_cons = FACTORY_DEF__EU_PATH;
		factory->up = universal_path;

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
  |
	STRING {
		Factory *fac = Factory_creat();
		fac->t_cons = FACTORY_DEF__STR_LITERAL;
		fac->str_literal = mam_alloc(strlen(yytext) - 1, OBJ_TYPE__STRING, NULL, 0);
		memcpy(fac->str_literal, yytext + 1, strlen(yytext) - 2);
		stack_push(&AST_STACK, fac);
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
	LOOK_UP_CUBE ROUND_BRACKET_L vbs_token COMMA expression ROUND_BRACKET_R {
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
  |
	mdm_entity_universal_path {
		MDMEntityUniversalPath *md_up = NULL;
		stack_pop(&AST_STACK, (void **) &md_up);
		SetDef *set_def = ids_setdef_new(SET_DEF__MDE_UNI_PATH);
		set_def->up = md_up;
		stack_push(&AST_STACK, set_def);
	}
;

set_function:
	set_func_children {}
  |
	set_func_members {}
  |
	set_func_crossjoin {}
  |
	set_func_filter {}
  |
	set_func_lateralmembers {}
  |
	set_func_order {}
  |
	set_func_topcount {}
  |
	set_func_except {}
  |
	set_func_ytd {}
  |
	set_func_descendants {}
  |
	set_func_tail {}
  |
	set_func_bottompercent {}
  |
	set_func_toppercent {}
  |
	set_func_union {}
  |
	set_func_intersect {}
;

set_func_children:
	CHILDREN ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		MDMEntityUniversalPath *eup = NULL;
		stack_pop(&AST_STACK, (void **)&eup);
		ASTSetFunc_Children *mr_func = mam_alloc(sizeof(ASTSetFunc_Children), OBJ_TYPE__ASTSetFunc_Children, NULL, 0);
		mr_func->head.interpret = interpret_children;
		mr_func->mrole_sep = eup;
		stack_push(&AST_STACK, mr_func);
	}
  |
	CHILDREN ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTSetFunc_Children *mr_func = mam_alloc(sizeof(ASTSetFunc_Children), OBJ_TYPE__ASTSetFunc_Children, NULL, 0);
		mr_func->head.interpret = interpret_children;
		stack_push(&AST_STACK, mr_func);
	}
  |
	CHILDREN {
		ASTSetFunc_Children *mr_func = mam_alloc(sizeof(ASTSetFunc_Children), OBJ_TYPE__ASTSetFunc_Children, NULL, 0);
		mr_func->head.interpret = interpret_children;
		stack_push(&AST_STACK, mr_func);
	}
;

set_func_members:
	MEMBERS ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		MDMEntityUniversalPath *eup = NULL;
		stack_pop(&AST_STACK, (void **)&eup);

		ASTSetFunc_Members *func = mam_alloc(sizeof(ASTSetFunc_Members), OBJ_TYPE__ASTSetFunc_Members, NULL, 0);
		func->head.interpret = interpret_members;
		func->eup = eup;

		stack_push(&AST_STACK, func);
	}
  |
	MEMBERS ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTSetFunc_Members *func = mam_alloc(sizeof(ASTSetFunc_Members), OBJ_TYPE__ASTSetFunc_Members, NULL, 0);
		func->head.interpret = interpret_members;
		stack_push(&AST_STACK, func);
	}
  |
	MEMBERS {
		ASTSetFunc_Members *func = mam_alloc(sizeof(ASTSetFunc_Members), OBJ_TYPE__ASTSetFunc_Members, NULL, 0);
		func->head.interpret = interpret_members;
		stack_push(&AST_STACK, func);
	}
;

set_func_crossjoin:
	CROSS_JOIN ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		ASTSetFunc_CrossJoin *func = mam_alloc(sizeof(ASTSetFunc_CrossJoin), OBJ_TYPE__ASTSetFunc_CrossJoin, NULL, 0);
		func->head.interpret = interpret_crossjoin;
		stack_pop(&AST_STACK, (void **)&(func->setdefs));
		stack_push(&AST_STACK, func);
	}
;

set_func_filter:
	FILTER ROUND_BRACKET_L set_statement COMMA boolean_expression ROUND_BRACKET_R {
		ASTSetFunc_Filter *func = mam_alloc(sizeof(ASTSetFunc_Filter), OBJ_TYPE__ASTSetFunc_Filter, NULL, 0);
		func->head.interpret = interpret_filter;
		stack_pop(&AST_STACK, (void **)&(func->boolExp));
		stack_pop(&AST_STACK, (void **)&(func->set_def));
		stack_push(&AST_STACK, func);
	}
;

set_func_lateralmembers:
	LATERAL_MEMBERS ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_LateralMembers *func = mam_alloc(sizeof(ASTSetFunc_LateralMembers), OBJ_TYPE__ASTSetFunc_LateralMembers, NULL, 0);
		func->head.interpret = interpret_lateralmembers;
		stack_pop(&AST_STACK, (void **)&(func->mrole_up));
		stack_push(&AST_STACK, func);
	}
;

set_func_order:
	ORDER ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_Order *func = mam_alloc(sizeof(ASTSetFunc_Order), OBJ_TYPE__ASTSetFunc_Order, NULL, 0);
		func->head.interpret = interpret_order;

		stack_pop(&AST_STACK, (void **)&(func->expsep));
		stack_pop(&AST_STACK, (void **)&(func->setsep));

		func->option = ASC;

		stack_push(&AST_STACK, func);
	}
  |
	ORDER ROUND_BRACKET_L set_statement COMMA expression COMMA VAR {
		ASTSetFunc_Order *func = mam_alloc(sizeof(ASTSetFunc_Order), OBJ_TYPE__ASTSetFunc_Order, NULL, 0);
		func->head.interpret = interpret_order;

		stack_pop(&AST_STACK, (void **)&(func->expsep));
		stack_pop(&AST_STACK, (void **)&(func->setsep));

		if (!strcasecmp(yytext, "DESC")) {
			func->option = DESC;
		} else if (!strcasecmp(yytext, "BASC")) {
			func->option = BASC;
		} else if (!strcasecmp(yytext, "BDESC")) {
			func->option = BDESC;
		} else {
			// default ASC
			func->option = ASC;
		}

		stack_push(&AST_STACK, func);
	} ROUND_BRACKET_R
;

set_func_topcount:
	TOP_COUNT ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_TopCount *func = mam_alloc(sizeof(ASTSetFunc_TopCount), OBJ_TYPE__ASTSetFunc_TopCount, NULL, 0);
		func->head.interpret = interpret_topcount;

		stack_pop(&AST_STACK, (void **)&(func->count_exp));
		stack_pop(&AST_STACK, (void **)&(func->set_def));

		stack_push(&AST_STACK, func);
	}
  |
	TOP_COUNT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_TopCount *func = mam_alloc(sizeof(ASTSetFunc_TopCount), OBJ_TYPE__ASTSetFunc_TopCount, NULL, 0);
		func->head.interpret = interpret_topcount;

		stack_pop(&AST_STACK, (void **)&(func->num_exp));
		stack_pop(&AST_STACK, (void **)&(func->count_exp));
		stack_pop(&AST_STACK, (void **)&(func->set_def));

		stack_push(&AST_STACK, func);
	}
;

set_func_except:
	EXCEPT ROUND_BRACKET_L set_statement COMMA set_statement ROUND_BRACKET_R {
		ASTSetFunc_Except *func = mam_alloc(sizeof(ASTSetFunc_Except), OBJ_TYPE__ASTSetFunc_Except, NULL, 0);
		func->head.interpret = interpret_except;

		stack_pop(&AST_STACK, (void **)&(func->setdef_2));
		stack_pop(&AST_STACK, (void **)&(func->setdef_1));

		stack_push(&AST_STACK, func);
	}
  | EXCEPT ROUND_BRACKET_L set_statement COMMA set_statement COMMA ALL ROUND_BRACKET_R {
		ASTSetFunc_Except *func = mam_alloc(sizeof(ASTSetFunc_Except), OBJ_TYPE__ASTSetFunc_Except, NULL, 0);
		func->head.interpret = interpret_except;

		stack_pop(&AST_STACK, (void **)&(func->setdef_2));
		stack_pop(&AST_STACK, (void **)&(func->setdef_1));
		func->all = 1;

		stack_push(&AST_STACK, func);
	}
;

set_func_ytd:
	YTD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTSetFunc_YTD *func = mam_alloc(sizeof(ASTSetFunc_YTD), OBJ_TYPE__ASTSetFunc_YTD, NULL, 0);
		func->head.interpret = interpret_ytd;
		stack_push(&AST_STACK, func);
	}
  | YTD ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_YTD *func = mam_alloc(sizeof(ASTSetFunc_YTD), OBJ_TYPE__ASTSetFunc_YTD, NULL, 0);
		func->head.interpret = interpret_ytd;
		stack_pop(&AST_STACK, (void **)&(func->mrole_def));
		stack_push(&AST_STACK, func);
	}
;

set_func_descendants:
	DESCENDANTS ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_Descendants *func = mam_alloc(sizeof(ASTSetFunc_Descendants), OBJ_TYPE__ASTSetFunc_Descendants, NULL, 0);
		func->head.interpret = interpret_descendants;
		stack_pop(&AST_STACK, (void **)&(func->mrole_def));
		func->opt = SELF;
		stack_push(&AST_STACK, func);
	}
  |
	DESCENDANTS ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_Descendants *func = mam_alloc(sizeof(ASTSetFunc_Descendants), OBJ_TYPE__ASTSetFunc_Descendants, NULL, 0);
		func->head.interpret = interpret_descendants;
		stack_pop(&AST_STACK, (void **)&(func->lvrole_def));
		stack_pop(&AST_STACK, (void **)&(func->mrole_def));
		func->opt = SELF;
		stack_push(&AST_STACK, func);
	}
  |
	DESCENDANTS ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path COMMA VAR {
		ASTSetFunc_Descendants *func = mam_alloc(sizeof(ASTSetFunc_Descendants), OBJ_TYPE__ASTSetFunc_Descendants, NULL, 0);
		func->head.interpret = interpret_descendants;
		stack_pop(&AST_STACK, (void **)&(func->lvrole_def));
		stack_pop(&AST_STACK, (void **)&(func->mrole_def));

		if (!strcasecmp(yytext, "SELF")) {
			func->opt = SELF;
		} else if (!strcasecmp(yytext, "AFTER")) {
			func->opt = AFTER;
		} else if (!strcasecmp(yytext, "BEFORE")) {
			func->opt = BEFORE;
		} else if (!strcasecmp(yytext, "BEFORE_AND_AFTER")) {
			func->opt = BEFORE_AND_AFTER;
		} else if (!strcasecmp(yytext, "SELF_AND_AFTER")) {
			func->opt = SELF_AND_AFTER;
		} else if (!strcasecmp(yytext, "SELF_AND_BEFORE")) {
			func->opt = SELF_AND_BEFORE;
		} else if (!strcasecmp(yytext, "SELF_BEFORE_AFTER")) {
			func->opt = SELF_BEFORE_AFTER;
		} else if (!strcasecmp(yytext, "LEAVES")) {
			func->opt = LEAVES;
		} else {
			func->opt = SELF;
		}

		stack_push(&AST_STACK, func);
	} ROUND_BRACKET_R
  |
	DESCENDANTS ROUND_BRACKET_L mdm_entity_universal_path COMMA DECIMAL {
		Factory *f = Factory_creat();
		f->t_cons = FACTORY_DEF__DECIMAL;
		f->decimal = strtod(yytext, NULL);

		Term *t = Term_creat();
		Term_mul_factory(t, f);

		Expression *distance = Expression_creat();
		Expression_plus_term(distance, t);

		ASTSetFunc_Descendants *func = mam_alloc(sizeof(ASTSetFunc_Descendants), OBJ_TYPE__ASTSetFunc_Descendants, NULL, 0);
		func->head.interpret = interpret_descendants;
		stack_pop(&AST_STACK, (void **)&(func->mrole_def));

		func->disexp = distance;
		func->opt = SELF;
		stack_push(&AST_STACK, func);
	} ROUND_BRACKET_R
  |
	DESCENDANTS ROUND_BRACKET_L mdm_entity_universal_path COMMA DECIMAL {
		Factory *f = Factory_creat();
		f->t_cons = FACTORY_DEF__DECIMAL;
		f->decimal = strtod(yytext, NULL);

		Term *t = Term_creat();
		Term_mul_factory(t, f);

		Expression *distance = Expression_creat();
		Expression_plus_term(distance, t);

		stack_push(&AST_STACK, distance);
	} COMMA VAR {
		ASTSetFunc_Descendants *func = mam_alloc(sizeof(ASTSetFunc_Descendants), OBJ_TYPE__ASTSetFunc_Descendants, NULL, 0);
		func->head.interpret = interpret_descendants;

		if (!strcasecmp(yytext, "SELF")) {
			func->opt = SELF;
		} else if (!strcasecmp(yytext, "AFTER")) {
			func->opt = AFTER;
		} else if (!strcasecmp(yytext, "BEFORE")) {
			func->opt = BEFORE;
		} else if (!strcasecmp(yytext, "BEFORE_AND_AFTER")) {
			func->opt = BEFORE_AND_AFTER;
		} else if (!strcasecmp(yytext, "SELF_AND_AFTER")) {
			func->opt = SELF_AND_AFTER;
		} else if (!strcasecmp(yytext, "SELF_AND_BEFORE")) {
			func->opt = SELF_AND_BEFORE;
		} else if (!strcasecmp(yytext, "SELF_BEFORE_AFTER")) {
			func->opt = SELF_BEFORE_AFTER;
		} else if (!strcasecmp(yytext, "LEAVES")) {
			func->opt = LEAVES;
		} else {
			func->opt = SELF;
		}

		stack_pop(&AST_STACK, (void **)&(func->disexp));
		stack_pop(&AST_STACK, (void **)&(func->mrole_def));

		stack_push(&AST_STACK, func);
	} ROUND_BRACKET_R
;


set_func_tail:
	TAIL ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTSetFunc_Tail *func = mam_alloc(sizeof(ASTSetFunc_Tail), OBJ_TYPE__ASTSetFunc_Tail, NULL, 0);
		func->head.interpret = interpret_tail;
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  | TAIL ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_Tail *func = mam_alloc(sizeof(ASTSetFunc_Tail), OBJ_TYPE__ASTSetFunc_Tail, NULL, 0);
		func->head.interpret = interpret_tail;
		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

set_func_bottompercent:
	BOTTOM_PERCENT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_BottomOrTopPercent *func = mam_alloc(sizeof(ASTSetFunc_BottomOrTopPercent), OBJ_TYPE__ASTSetFunc_BottomOrTopPercent, NULL, 0);
		func->head.interpret = interpret_bottomortoppercent;
		stack_pop(&AST_STACK, (void **)&(func->exp));
		stack_pop(&AST_STACK, (void **)&(func->percentage));
		stack_pop(&AST_STACK, (void **)&(func->set));
		func->option = BOTTOM_PER;
		stack_push(&AST_STACK, func);
	}
;

set_func_toppercent:
	TOP_PERCENT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_BottomOrTopPercent *func = mam_alloc(sizeof(ASTSetFunc_BottomOrTopPercent), OBJ_TYPE__ASTSetFunc_BottomOrTopPercent, NULL, 0);
		func->head.interpret = interpret_bottomortoppercent;
		stack_pop(&AST_STACK, (void **)&(func->exp));
		stack_pop(&AST_STACK, (void **)&(func->percentage));
		stack_pop(&AST_STACK, (void **)&(func->set));
		func->option = TOP_PER;
		stack_push(&AST_STACK, func);
	}
;

set_func_union:
	UNION ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		ASTSetFunc_Union *func = mam_alloc(sizeof(ASTSetFunc_Union), OBJ_TYPE__ASTSetFunc_Union, NULL, 0);
		func->head.interpret = interpret_union;
		stack_pop(&AST_STACK, (void **)&(func->set_def_ls));
		stack_push(&AST_STACK, func);
	}
  | UNION ROUND_BRACKET_L set_list COMMA ALL ROUND_BRACKET_R {
		ASTSetFunc_Union *func = mam_alloc(sizeof(ASTSetFunc_Union), OBJ_TYPE__ASTSetFunc_Union, NULL, 0);
		func->head.interpret = interpret_union;
		stack_pop(&AST_STACK, (void **)&(func->set_def_ls));
		func->all_opt = 1;
		stack_push(&AST_STACK, func);
	}
;

set_func_intersect:
	INTERSECT ROUND_BRACKET_L set_list ROUND_BRACKET_R {
		ASTSetFunc_Intersect *func = mam_alloc(sizeof(ASTSetFunc_Intersect), OBJ_TYPE__ASTSetFunc_Intersect, NULL, 0);
		func->head.interpret = interpret_intersect;
		stack_pop(&AST_STACK, (void **)&(func->set_def_ls));
		stack_push(&AST_STACK, func);
	}
  | INTERSECT ROUND_BRACKET_L set_list COMMA ALL ROUND_BRACKET_R {
		ASTSetFunc_Intersect *func = mam_alloc(sizeof(ASTSetFunc_Intersect), OBJ_TYPE__ASTSetFunc_Intersect, NULL, 0);
		func->head.interpret = interpret_intersect;
		stack_pop(&AST_STACK, (void **)&(func->set_def_ls));
		func->all_opt = 1;
		stack_push(&AST_STACK, func);
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

tuples_statement:
	mdm_entity_universal_path {
		MDMEntityUniversalPath *eup = NULL;
		stack_pop(&AST_STACK, (void **) &eup);

		ArrayList *t_def_ls = als_new(32, "TupleDef * | MDMEntityUniversalPath *", THREAD_MAM, NULL);
		als_add(t_def_ls, eup);
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
  | tuples_statement COMMA mdm_entity_universal_path {
		MDMEntityUniversalPath *eup = NULL;
		stack_pop(&AST_STACK, (void **) &eup);
		ArrayList *t_def_ls;
		stack_pop(&AST_STACK, (void **) &t_def_ls);
		als_add(t_def_ls, eup);
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
	ROUND_BRACKET_L up_list ROUND_BRACKET_R {
		ArrayList *up_ls = NULL;
		stack_pop(&AST_STACK, (void **) &up_ls);

		TupleDef *t_def = ids_tupledef_new(TUPLE_DEF__UPATH_LS);
		t_def->universal_path_ls = up_ls;

		stack_push(&AST_STACK, t_def);
	}
;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

member_func_parent:
	PARENT ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_Parent *func = mam_alloc(sizeof(ASTMemberFn_Parent), OBJ_TYPE__ASTMemberFn_Parent, NULL, 0);
		func->head.interpret = interpret_parent;
		stack_pop(&AST_STACK, (void **) &(func->mr_up));
		stack_push(&AST_STACK, func);
	}
  |
	PARENT ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_Parent *func = mam_alloc(sizeof(ASTMemberFn_Parent), OBJ_TYPE__ASTMemberFn_Parent, NULL, 0);
		func->head.interpret = interpret_parent;
		stack_push(&AST_STACK, func);
	}
  |
	PARENT {
		ASTMemberFn_Parent *func = mam_alloc(sizeof(ASTMemberFn_Parent), OBJ_TYPE__ASTMemberFn_Parent, NULL, 0);
		func->head.interpret = interpret_parent;
		stack_push(&AST_STACK, func);
	}
;

member_func_current_member:
	CURRENT_MEMBER ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_CurrentMember *func = mam_alloc(sizeof(ASTMemberFn_CurrentMember), OBJ_TYPE__ASTMemberFn_CurrentMember, NULL, 0);
		func->head.interpret = interpret_currentmember;
		stack_pop(&AST_STACK, (void **) &(func->dr_up));
		stack_push(&AST_STACK, func);
	}
  |
	CURRENT_MEMBER ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_CurrentMember *func = mam_alloc(sizeof(ASTMemberFn_CurrentMember), OBJ_TYPE__ASTMemberFn_CurrentMember, NULL, 0);
		func->head.interpret = interpret_currentmember;
		stack_push(&AST_STACK, func);
	}
  |
	CURRENT_MEMBER {
		ASTMemberFn_CurrentMember *func = mam_alloc(sizeof(ASTMemberFn_CurrentMember), OBJ_TYPE__ASTMemberFn_CurrentMember, NULL, 0);
		func->head.interpret = interpret_currentmember;
		stack_push(&AST_STACK, func);
	}
;

member_func_prev_member:
	PREV_MEMBER ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_PrevMember *func = mam_alloc(sizeof(ASTMemberFn_PrevMember), OBJ_TYPE__ASTMemberFn_PrevMember, NULL, 0);
		func->head.interpret = interpret_prevmember;
		stack_pop(&AST_STACK, (void **) &(func->mr_up));
		stack_push(&AST_STACK, func);
	}
  |
	PREV_MEMBER ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_PrevMember *func = mam_alloc(sizeof(ASTMemberFn_PrevMember), OBJ_TYPE__ASTMemberFn_PrevMember, NULL, 0);
		func->head.interpret = interpret_prevmember;
		stack_push(&AST_STACK, func);
	}
  |
	PREV_MEMBER {
		ASTMemberFn_PrevMember *func = mam_alloc(sizeof(ASTMemberFn_PrevMember), OBJ_TYPE__ASTMemberFn_PrevMember, NULL, 0);
		func->head.interpret = interpret_prevmember;
		stack_push(&AST_STACK, func);
	}
;

member_func_first_child:
	FIRST_CHILD ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_FirstChild *func = mam_alloc(sizeof(ASTMemberFn_FirstChild), OBJ_TYPE__ASTMemberFn_FirstChild, NULL, 0);
		func->head.interpret = interpret_firstchild;
		stack_pop(&AST_STACK, (void **) &(func->mr_up));
		stack_push(&AST_STACK, func);
	}
  |
	FIRST_CHILD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_FirstChild *func = mam_alloc(sizeof(ASTMemberFn_FirstChild), OBJ_TYPE__ASTMemberFn_FirstChild, NULL, 0);
		func->head.interpret = interpret_firstchild;
		stack_push(&AST_STACK, func);
	}
  |
	FIRST_CHILD {
		ASTMemberFn_FirstChild *func = mam_alloc(sizeof(ASTMemberFn_FirstChild), OBJ_TYPE__ASTMemberFn_FirstChild, NULL, 0);
		func->head.interpret = interpret_firstchild;
		stack_push(&AST_STACK, func);
	}
;

member_function:
	member_func_parent {}
  |
	member_func_current_member {}
  |
	member_func_prev_member {}
  |
	member_func_first_child {}
  /*
	member_func_last_child {}
  |
	member_func_first_sibling {}
  |
	member_func_last_sibling {}
  |
	member_func_lag {}
  |
	member_func_lead {}
  |
	member_func_parallel_period {}
  |
	member_func_closing_period {}
  |
	member_func_opening_period {} */
;

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

member_function_template:

	
  LAST_CHILD ROUND_BRACKET_L member_statement ROUND_BRACKET_R {
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

member_function_template_suffix:
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

// ????????????????????????????????????????????????????????????????????????????????????????????????????
// ????????????????????????????????????????????????????????????????????????????????????????????????????
// ????????????????????????????????????????????????????????????????????????????????????????????????????

member_statement:
	mdm_entity_universal_path {
		MDMEntityUniversalPath *eup_ = NULL;
		stack_pop(&AST_STACK, (void **) &eup_);

		MemberDef *mbr_def = mam_alloc(sizeof(MemberDef), OBJ_TYPE__MemberDef, NULL, 0);
		mbr_def->t_cons = MEMBER_DEF__UNIVERSALPATH;
		mbr_def->eup = eup_;

		stack_push(&AST_STACK, mbr_def);
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
		IDSVectorMears *ids_vm;
		stack_pop(&AST_STACK, (void **) &ids_vm);

		ArrayList *ls_vms = als_new(128, "{ insert_cube_measures ::= }, { IDSVectorMears * }", THREAD_MAM, NULL);
		als_add(ls_vms, ids_vm);

		stack_push(&AST_STACK, NULL); // NULL is 0

		stack_push(&AST_STACK, ls_vms);
	}
  |
	INSERT var_or_block EQUIVALENT_TO DECIMAL {
		unsigned long worker_id = atoi(yytext);
		stack_push(&AST_STACK, *((void **)&worker_id));
	} vector_measures {
		IDSVectorMears *ids_vm;
		stack_pop(&AST_STACK, (void **) &ids_vm);

		ArrayList *ls_vms = als_new(128, "{ insert_cube_measures ::= }, { IDSVectorMears * }", THREAD_MAM, NULL);
		als_add(ls_vms, ids_vm);

		stack_push(&AST_STACK, ls_vms);
	}
  |
	insert_cube_measures COMMA vector_measures {
		IDSVectorMears *ids_vm;
		stack_pop(&AST_STACK, (void **) &ids_vm);
		ArrayList *ls_vms;
		stack_pop(&AST_STACK, (void **) &ls_vms);
		als_add(ls_vms, ids_vm);
		stack_push(&AST_STACK, ls_vms);
	}
;

vector_measures:
	ROUND_BRACKET_L vector__ MEASURES measures_values ROUND_BRACKET_R {
		ArrayList *ls_vector, *ls_mears_vals;
		stack_pop(&AST_STACK, (void **) &ls_mears_vals);
		stack_pop(&AST_STACK, (void **) &ls_vector);
		IDSVectorMears *ids_vm = mam_alloc(sizeof(IDSVectorMears), OBJ_TYPE__IDSVectorMears, NULL, 0);
		ids_vm->ls_vector = ls_vector;
		ids_vm->ls_mears_vals = ls_mears_vals;
		stack_push(&AST_STACK, ids_vm);
	}
;

vector__:
	mdm_entity_universal_path {
		MDMEntityUniversalPath *eup = NULL;
		stack_pop(&AST_STACK, (void **) &eup);
		ArrayList *list = als_new(16, "<MDMEntityUniversalPath *>", THREAD_MAM, NULL);
		als_add(list, eup);
		stack_push(&AST_STACK, list);
	}
  | vector__ COMMA mdm_entity_universal_path {
		MDMEntityUniversalPath *eup = NULL;
		stack_pop(&AST_STACK, (void **) &eup);
		ArrayList *list = NULL;
		stack_pop(&AST_STACK, (void **) &list);
		als_add(list, eup);
		stack_push(&AST_STACK, list);
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

create_hierarchy:
	CREATE HIERARCHY var_or_block DOT var_or_block {
		// no need to do anything.
	}
;

create_dimensions:
	CREATE DIMENSIONS var_or_block {
		char *dimension_name = NULL;
		stack_pop(&AST_STACK, (void **) &dimension_name);

		ArrayList *hie_name_ls = als_new(8, "<char *> hierarchy names", THREAD_MAM, NULL);
		als_add(hie_name_ls, NULL);
		stack_push(&AST_STACK, hie_name_ls);

		ArrayList *dim_name_ls = als_new(8, "<char *> dimension names", THREAD_MAM, NULL);
		als_add(dim_name_ls, dimension_name);
		stack_push(&AST_STACK, dim_name_ls);
	}
  |
	CREATE DIMENSIONS var_or_block DOT var_or_block {
		char *hierarchy_name = NULL;
		stack_pop(&AST_STACK, (void **) &hierarchy_name);

		char *dimension_name = NULL;
		stack_pop(&AST_STACK, (void **) &dimension_name);

		ArrayList *hie_name_ls = als_new(8, "<char *> hierarchy names", THREAD_MAM, NULL);
		als_add(hie_name_ls, hierarchy_name);
		stack_push(&AST_STACK, hie_name_ls);

		ArrayList *dim_name_ls = als_new(8, "<char *> dimension names", THREAD_MAM, NULL);
		als_add(dim_name_ls, dimension_name);
		stack_push(&AST_STACK, dim_name_ls);
	}
  |
	create_dimensions var_or_block {
		char *dimension_name = NULL;
		stack_pop(&AST_STACK, (void **) &dimension_name);

		ArrayList *dim_name_ls = NULL;
		stack_pop(&AST_STACK, (void **) &dim_name_ls);
		als_add(dim_name_ls, dimension_name);

		ArrayList *hie_name_ls = NULL;
		stack_pop(&AST_STACK, (void **) &hie_name_ls);
		als_add(hie_name_ls, NULL);

		stack_push(&AST_STACK, hie_name_ls);
		stack_push(&AST_STACK, dim_name_ls);
	}
  |
	create_dimensions var_or_block DOT var_or_block {
		char *hierarchy_name = NULL;
		stack_pop(&AST_STACK, (void **) &hierarchy_name);

		char *dimension_name = NULL;
		stack_pop(&AST_STACK, (void **) &dimension_name);

		ArrayList *dim_name_ls = NULL;
		stack_pop(&AST_STACK, (void **) &dim_name_ls);
		als_add(dim_name_ls, dimension_name);

		ArrayList *hie_name_ls = NULL;
		stack_pop(&AST_STACK, (void **) &hie_name_ls);
		als_add(hie_name_ls, hierarchy_name);

		stack_push(&AST_STACK, hie_name_ls);
		stack_push(&AST_STACK, dim_name_ls);
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
		ArrayList_set(dim_lvs, 1, NULL);
		stack_push(&AST_STACK, dim_lvs);
	}
  |
	var_or_block DOT var_or_block ROUND_BRACKET_L levels_list ROUND_BRACKET_R {
		ArrayList *dim_lvs;
		stack_pop(&AST_STACK, (void **) &dim_lvs);

		char *hierarchy_name;
		stack_pop(&AST_STACK, (void **) &hierarchy_name);

		char *dim_name;
		stack_pop(&AST_STACK, (void **) &dim_name);

		ArrayList_set(dim_lvs, 0, dim_name);
		ArrayList_set(dim_lvs, 1, hierarchy_name);

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
	CREATE MEMBERS mdm_entity_universal_path {
		MDMEntityUniversalPath *eup = NULL;
		stack_pop(&AST_STACK, (void **) &eup);

		ArrayList *list = als_new(128, "<MDMEntityUniversalPath *>", THREAD_MAM, NULL);
		als_add(list, eup);
		
		stack_push(&AST_STACK, list);
	}
  |
	create_members COMMA mdm_entity_universal_path {
		MDMEntityUniversalPath *eup = NULL;
		stack_pop(&AST_STACK, (void **) &eup);

		ArrayList *list = NULL;
		stack_pop(&AST_STACK, (void **) &list);
		als_add(list, eup);
		
		stack_push(&AST_STACK, list);
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

make_equivalent:
	MAKE mdm_entity_universal_path EQUIVALENT_TO mdm_entity_universal_path {
		MDMEntityUniversalPath *dest = NULL;
		MDMEntityUniversalPath *src = NULL;
		stack_pop(&AST_STACK, (void **) &dest);
		stack_pop(&AST_STACK, (void **) &src);
		ArrayList *list = als_new(64, "<MDMEntityUniversalPath *>", THREAD_MAM, NULL);
		als_add(list, src);
		als_add(list, dest);
		stack_push(&AST_STACK, list);
	}
  |
	make_equivalent COMMA mdm_entity_universal_path EQUIVALENT_TO mdm_entity_universal_path {
		
		MDMEntityUniversalPath *dest = NULL;
		MDMEntityUniversalPath *src = NULL;
		stack_pop(&AST_STACK, (void **) &dest);
		stack_pop(&AST_STACK, (void **) &src);
		ArrayList *list = NULL;
		stack_pop(&AST_STACK, (void **) &list);
		als_add(list, src);
		als_add(list, dest);
		stack_push(&AST_STACK, list);

	}
;

dims_and_roles:
	var_or_block {
		char *dim_name = NULL;
		stack_pop(&AST_STACK, (void **) &dim_name);
		ArrayList *dr_ls = als_new(64, "yacc dims_and_roles ::= var_or_block var_or_block", THREAD_MAM, NULL);
		als_add(dr_ls, dim_name);
		als_add(dr_ls, dim_name);
		stack_push(&AST_STACK, dr_ls);
	}
  |
	var_or_block COLON var_or_block {
		char *dim_name, *role_name;
		stack_pop(&AST_STACK, (void **) &role_name);
		stack_pop(&AST_STACK, (void **) &dim_name);
		ArrayList *dr_ls = als_new(64, "yacc dims_and_roles ::= var_or_block var_or_block", THREAD_MAM, NULL);
		als_add(dr_ls, dim_name);
		als_add(dr_ls, role_name);
		stack_push(&AST_STACK, dr_ls);
	}
  |
	dims_and_roles var_or_block {
		char *dim_name = NULL;
		stack_pop(&AST_STACK, (void **) &dim_name);
		ArrayList *dr_ls = NULL;
		stack_pop(&AST_STACK, (void **) &dr_ls);
		als_add(dr_ls, dim_name);
		als_add(dr_ls, dim_name);
		stack_push(&AST_STACK, dr_ls);
	}
  |	dims_and_roles var_or_block COLON var_or_block {
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



dimension_statement:
	var_or_block {
		DimRoleDef *dr_def = DimRoleDef_creat();
		stack_pop(&AST_STACK, (void **) &(dr_def->name));
		stack_push(&AST_STACK, dr_def);
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
	member_function {
		void *member_func = NULL;
		stack_pop(&AST_STACK, (void **) &member_func);

		MDMEntityUniversalPath *up = mam_alloc(sizeof(MDMEntityUniversalPath), OBJ_TYPE__MDMEntityUniversalPath, NULL, 0);
		up->list = als_new(8, NULL, THREAD_MAM, NULL);

		als_add(up->list, member_func);
		stack_push(&AST_STACK, up);
	}
  |
	set_function {
		void *set_func_tpl = NULL;
		stack_pop(&AST_STACK, (void **) &set_func_tpl);

		MDMEntityUniversalPath *up = mam_alloc(sizeof(MDMEntityUniversalPath), OBJ_TYPE__MDMEntityUniversalPath, NULL, 0);
		up->list = als_new(8, NULL, THREAD_MAM, NULL);

		als_add(up->list, set_func_tpl);
		stack_push(&AST_STACK, up);
	}
  |
	string_function {
		void *strfunc = NULL;
		stack_pop(&AST_STACK, (void **) &strfunc);

		MDMEntityUniversalPath *up = mam_alloc(sizeof(MDMEntityUniversalPath), OBJ_TYPE__MDMEntityUniversalPath, NULL, 0);
		up->list = als_new(8, NULL, THREAD_MAM, NULL);

		als_add(up->list, strfunc);
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
	mdm_entity_universal_path DOT member_function {
		void *member_func = NULL;
		stack_pop(&AST_STACK, (void **) &member_func);
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		als_add(up->list, member_func);
		stack_push(&AST_STACK, up);
	}
  |
	mdm_entity_universal_path DOT set_function {
		void *suf_setfn_tpl = NULL;
		stack_pop(&AST_STACK, (void **) &suf_setfn_tpl);
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		als_add(up->list, suf_setfn_tpl);
		stack_push(&AST_STACK, up);
	}
  |
	mdm_entity_universal_path DOT string_function {
		void *strfunc = NULL;
		stack_pop(&AST_STACK, (void **) &strfunc);
		MDMEntityUniversalPath *up = NULL;
		stack_pop(&AST_STACK, (void **) &up);

		als_add(up->list, strfunc);
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
	var_or_block {
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

str_token:
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

/* chain_ring:
	var_or_block {
		// do nothing
	}
; */

vbs_token:
	var_or_block {}
  |
	str_token {}
;

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
