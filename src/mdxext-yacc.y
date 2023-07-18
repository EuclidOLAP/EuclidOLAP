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
#include "mdm-ast-num-func.h"
#include "mdm-ast-lv-func.h"

int yyerror(const char *);

extern int yylex();
extern int yyparse();
extern int eucparser_scan_string(const char *s);
extern void eucparser_cleanup();

extern char *yytext;

Stack AST_STACK = { 0 };

static void ast_func_append_to_up(void);
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
%token QTD				/* Qtd */
%token DESCENDANTS
%token TAIL
%token BOTTOM_PERCENT
%token TOP_PERCENT
%token UNION
%token INTERSECT
%token DISTINCT			/* Distinct */
%token DRILL_DOWN_LEVEL			/* DrilldownLevel */
%token INCLUDE_CALC_MEMBERS		/* param: Include_Calc_Members */
%token DRILL_DOWN_LEVEL_BOTTOM	/* DrilldownLevelBottom */
%token DRILL_DOWN_LEVEL_TOP		/* DrilldownLevelTop */
%token DRILL_DOWN_MEMBER		/* DrillDownMember */
%token DRILL_DOWN_MEMBER_BOTTOM	/* DrillDownMemberBottom */
%token DRILL_DOWN_MEMBER_TOP	/* DrillDownMemberTop */
%token RECURSIVE		/* param: Recursive */
%token DRILLUP_LEVEL		/* DrillupLevel */
%token DRILLUP_MEMBER		/* DrillupMember */
%token ANCESTORS		/* Ancestors */
%token BOTTOM_COUNT		/* BottomCount */
%token BOTTOM_SUM		/* BottomSum */
%token TOP_SUM		/* TopSum */
%token EXTRACT		/* Extract */
%token PERIODS_TO_DATE		/* PeriodsToDate */
%token GENERATE		/* Generate */
%token HEAD			/* Head */
%token SUB_SET			/* Subset */

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
%token NEXT_MEMBER		/* NextMember */
%token ANCESTOR			/* Ancestor */
%token COUSIN			/* Cousin */
%token DEFAULT_MEMBER	/* DefaultMember */

/* Numeric Functions key words */
%token SUM				/* sum */
%token COUNT			/* count */
%token EXCLUDEEMPTY		/* EXCLUDEEMPTY */
%token INCLUDEEMPTY		/* INCLUDEEMPTY */
%token LOOK_UP_CUBE 	/* lookUpCube */
%token IIF				/* iif */
%token COALESCE_EMPTY	/* coalesceEmpty */
%token AVG				/* Avg */
%token MAX				/* Max */
%token MIN				/* Min */
%token AGGREGATE		/* Aggregate */
%token MEDIAN			/* Median */
%token RANK				/* Rank */
%token ABS				/* abs */
%token CORRELATION		/* Correlation */
%token COVARIANCE		/* Covariance */
%token LINREGINTERCEPT		/* LinRegIntercept */
%token LINREGR2		/* LinRegR2 */
%token LIN_REG_SLOPE		/* LinRegSlope */
%token LIN_REG_VARIANCE		/* LinRegVariance */
%token STDEV				/* Stdev */
%token FN_VAR				/* Var */
%token ORDINAL				/* Ordinal */

%token CASE			/* Case */
%token ELSE			/* Else */
%token END			/* end */
%token WHEN			/* when */
%token THEN			/* then */

/* Logical Functions */
%token IS_EMPTY			/* IsEmpty */
%token IS_ANCESTOR		/* IsAncestor */
%token IS_GENERATION		/* IsGeneration */
%token IS_LEAF		/* IsLeaf */
%token IS_SIBLING		/* IsSibling */

%token NOT			/* Not */

/* Level Functions */
%token LEVEL				/* Level */

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
  |
	case_statement {
		Factory *factory = Factory_creat();
		factory->t_cons = FACTORY_DEF__EXP_FN;
		stack_pop(&AST_STACK, (void **) &(factory->exp));
		stack_push(&AST_STACK, factory);
	}
;

case_statement:
	CASE expression simple_case ELSE expression END {
		Expression *else_exp = NULL;
		ASTNumFunc_CaseStatement *func = NULL;
		Expression *input_exp = NULL;
		stack_pop(&AST_STACK, (void **) &else_exp);
		stack_pop(&AST_STACK, (void **) &func);
		stack_pop(&AST_STACK, (void **) &input_exp);
		func->input_exp = input_exp;
		func->else_result_exp = else_exp;
		stack_push(&AST_STACK, func);
	}
  |
	CASE expression simple_case END {
		ASTNumFunc_CaseStatement *func = NULL;
		Expression *input_exp = NULL;
		stack_pop(&AST_STACK, (void **) &func);
		stack_pop(&AST_STACK, (void **) &input_exp);
		func->input_exp = input_exp;
		stack_push(&AST_STACK, func);
	}
  |
	CASE search_case ELSE expression END {
		Expression *else_exp = NULL;
		ASTNumFunc_CaseStatement *func = NULL;
		stack_pop(&AST_STACK, (void **) &else_exp);
		stack_pop(&AST_STACK, (void **) &func);
		func->else_result_exp = else_exp;
		stack_push(&AST_STACK, func);
	}
  |
	CASE search_case END {}
;

simple_case:
	WHEN expression THEN expression {
		ASTNumFunc_CaseStatement *func = mam_alloc(sizeof(ASTNumFunc_CaseStatement), OBJ_TYPE__ASTNumFunc_CaseStatement, NULL, 0);
		func->head.interpret = interpret_CaseStatement;
		func->when_then_ls = als_new(16, "BooleanExpression|Expression", THREAD_MAM, NULL);
		Expression *whenexp = NULL;
		Expression *thenexp = NULL;
		stack_pop(&AST_STACK, (void **) &thenexp);
		stack_pop(&AST_STACK, (void **) &whenexp);
		als_add(func->when_then_ls, whenexp);
		als_add(func->when_then_ls, thenexp);
		stack_push(&AST_STACK, func);
	}
  |
	simple_case WHEN expression THEN expression {
		Expression *whenexp = NULL;
		Expression *thenexp = NULL;
		stack_pop(&AST_STACK, (void **) &thenexp);
		stack_pop(&AST_STACK, (void **) &whenexp);
		ASTNumFunc_CaseStatement *func = NULL;
		stack_pop(&AST_STACK, (void **) &func);
		als_add(func->when_then_ls, whenexp);
		als_add(func->when_then_ls, thenexp);
		stack_push(&AST_STACK, func);
	}
;

search_case:
	WHEN boolean_expression THEN expression {
		ASTNumFunc_CaseStatement *func = mam_alloc(sizeof(ASTNumFunc_CaseStatement), OBJ_TYPE__ASTNumFunc_CaseStatement, NULL, 0);
		func->head.interpret = interpret_CaseStatement;
		func->when_then_ls = als_new(16, "BooleanExpression|Expression", THREAD_MAM, NULL);
		BooleanExpression *whenexp = NULL;
		Expression *thenexp = NULL;
		stack_pop(&AST_STACK, (void **) &thenexp);
		stack_pop(&AST_STACK, (void **) &whenexp);
		als_add(func->when_then_ls, whenexp);
		als_add(func->when_then_ls, thenexp);
		stack_push(&AST_STACK, func);
	}
  |
	search_case WHEN boolean_expression THEN expression {
		BooleanExpression *whenexp = NULL;
		Expression *thenexp = NULL;
		stack_pop(&AST_STACK, (void **) &thenexp);
		stack_pop(&AST_STACK, (void **) &whenexp);
		ASTNumFunc_CaseStatement *func = NULL;
		stack_pop(&AST_STACK, (void **) &func);
		als_add(func->when_then_ls, whenexp);
		als_add(func->when_then_ls, thenexp);
		stack_push(&AST_STACK, func);
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
  |
	exp_fn__avg {}
  |
	exp_fn__max {}
  |
	exp_fn__min {}
  |
	exp_fn__aggregate {}
  |
	exp_fn__median {}
  |
	exp_fn__rank {}
  |
	exp_fn__abs {}
  |
	exp_fn__correlation {}
  |
	exp_fn__covariance {}
  |
	exp_fn__LinRegIntercept {}
  |
	exp_fn__LinRegR2 {}
  |
	exp_fn__LinRegSlope {}
  |
	exp_fn__LinRegVariance {}
  |
	exp_fn__Stdev {}
  |
	exp_fn__Var {}
  |
	exp_fn__Ordinal {}
;

exp_fn__Ordinal:
	ORDINAL ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTNumFunc_Ordinal *func = mam_alloc(sizeof(ASTNumFunc_Ordinal), OBJ_TYPE__ASTNumFunc_Ordinal, NULL, 0);
		func->head.interpret = interpret_Ordinal;
		stack_pop(&AST_STACK, (void **) &(func->lrdef));
		stack_push(&AST_STACK, func);
	}
  |
	ORDINAL ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTNumFunc_Ordinal *func = mam_alloc(sizeof(ASTNumFunc_Ordinal), OBJ_TYPE__ASTNumFunc_Ordinal, NULL, 0);
		func->head.interpret = interpret_Ordinal;
		stack_push(&AST_STACK, func);
	}
  |
	ORDINAL {
		ASTNumFunc_Ordinal *func = mam_alloc(sizeof(ASTNumFunc_Ordinal), OBJ_TYPE__ASTNumFunc_Ordinal, NULL, 0);
		func->head.interpret = interpret_Ordinal;
		stack_push(&AST_STACK, func);
	}
;

exp_fn__Var:
	FN_VAR ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Var *func = mam_alloc(sizeof(ASTNumFunc_Var), OBJ_TYPE__ASTNumFunc_Var, NULL, 0);
		func->head.interpret = interpret_Var;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	FN_VAR ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_Var *func = mam_alloc(sizeof(ASTNumFunc_Var), OBJ_TYPE__ASTNumFunc_Var, NULL, 0);
		func->head.interpret = interpret_Var;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__Stdev:
	STDEV ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Stdev *func = mam_alloc(sizeof(ASTNumFunc_Stdev), OBJ_TYPE__ASTNumFunc_Stdev, NULL, 0);
		func->head.interpret = interpret_Stdev;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	STDEV ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_Stdev *func = mam_alloc(sizeof(ASTNumFunc_Stdev), OBJ_TYPE__ASTNumFunc_Stdev, NULL, 0);
		func->head.interpret = interpret_Stdev;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__LinRegVariance:
	LIN_REG_VARIANCE ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_LinRegVariance *func = mam_alloc(sizeof(ASTNumFunc_LinRegVariance), OBJ_TYPE__ASTNumFunc_LinRegVariance, NULL, 0);
		func->head.interpret = interpret_LinRegVariance;
		stack_pop(&AST_STACK, (void **) &(func->expdef_x));
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	LIN_REG_VARIANCE ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_LinRegVariance *func = mam_alloc(sizeof(ASTNumFunc_LinRegVariance), OBJ_TYPE__ASTNumFunc_LinRegVariance, NULL, 0);
		func->head.interpret = interpret_LinRegVariance;
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__LinRegSlope:
	LIN_REG_SLOPE ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_LinRegSlope *func = mam_alloc(sizeof(ASTNumFunc_LinRegSlope), OBJ_TYPE__ASTNumFunc_LinRegSlope, NULL, 0);
		func->head.interpret = interpret_LinRegSlope;
		stack_pop(&AST_STACK, (void **) &(func->expdef_x));
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	LIN_REG_SLOPE ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_LinRegSlope *func = mam_alloc(sizeof(ASTNumFunc_LinRegSlope), OBJ_TYPE__ASTNumFunc_LinRegSlope, NULL, 0);
		func->head.interpret = interpret_LinRegSlope;
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

// The function 'LINREGR2' has the same logic as the function 'Correlation'.
exp_fn__LinRegR2:
	LINREGR2 ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Correlation *func = mam_alloc(sizeof(ASTNumFunc_Correlation), OBJ_TYPE__ASTNumFunc_Correlation, NULL, 0);
		func->head.interpret = interpret_correlation;
		stack_pop(&AST_STACK, (void **) &(func->expdef_x));
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	LINREGR2 ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Correlation *func = mam_alloc(sizeof(ASTNumFunc_Correlation), OBJ_TYPE__ASTNumFunc_Correlation, NULL, 0);
		func->head.interpret = interpret_correlation;
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__LinRegIntercept:
	LINREGINTERCEPT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_LinRegIntercept *func = mam_alloc(sizeof(ASTNumFunc_LinRegIntercept), OBJ_TYPE__ASTNumFunc_LinRegIntercept, NULL, 0);
		func->head.interpret = interpret_LinRegIntercept;
		stack_pop(&AST_STACK, (void **) &(func->expdef_x));
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	LINREGINTERCEPT ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_LinRegIntercept *func = mam_alloc(sizeof(ASTNumFunc_LinRegIntercept), OBJ_TYPE__ASTNumFunc_LinRegIntercept, NULL, 0);
		func->head.interpret = interpret_LinRegIntercept;
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__covariance:
	COVARIANCE ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Covariance *func = mam_alloc(sizeof(ASTNumFunc_Covariance), OBJ_TYPE__ASTNumFunc_Covariance, NULL, 0);
		func->head.interpret = interpret_covariance;
		stack_pop(&AST_STACK, (void **) &(func->expdef_x));
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	COVARIANCE ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Covariance *func = mam_alloc(sizeof(ASTNumFunc_Covariance), OBJ_TYPE__ASTNumFunc_Covariance, NULL, 0);
		func->head.interpret = interpret_covariance;
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__correlation:
	CORRELATION ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Correlation *func = mam_alloc(sizeof(ASTNumFunc_Correlation), OBJ_TYPE__ASTNumFunc_Correlation, NULL, 0);
		func->head.interpret = interpret_correlation;
		stack_pop(&AST_STACK, (void **) &(func->expdef_x));
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	CORRELATION ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Correlation *func = mam_alloc(sizeof(ASTNumFunc_Correlation), OBJ_TYPE__ASTNumFunc_Correlation, NULL, 0);
		func->head.interpret = interpret_correlation;
		stack_pop(&AST_STACK, (void **) &(func->expdef_y));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__abs:
	ABS ROUND_BRACKET_L expression ROUND_BRACKET_R {
		ASTNumFunc_Abs *func = mam_alloc(sizeof(ASTNumFunc_Abs), OBJ_TYPE__ASTNumFunc_Abs, NULL, 0);
		func->head.interpret = interpret_abs;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__rank:
	RANK ROUND_BRACKET_L tuple_statement COMMA set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Rank *func = mam_alloc(sizeof(ASTNumFunc_Rank), OBJ_TYPE__ASTNumFunc_Rank, NULL, 0);
		func->head.interpret = interpret_rank;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_pop(&AST_STACK, (void **) &(func->param1));
		stack_push(&AST_STACK, func);
	}
  |
	RANK ROUND_BRACKET_L mdm_entity_universal_path COMMA set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Rank *func = mam_alloc(sizeof(ASTNumFunc_Rank), OBJ_TYPE__ASTNumFunc_Rank, NULL, 0);
		func->head.interpret = interpret_rank;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_pop(&AST_STACK, (void **) &(func->param1));
		stack_push(&AST_STACK, func);
	}
  |
	RANK ROUND_BRACKET_L tuple_statement COMMA set_statement ROUND_BRACKET_R {
		ASTNumFunc_Rank *func = mam_alloc(sizeof(ASTNumFunc_Rank), OBJ_TYPE__ASTNumFunc_Rank, NULL, 0);
		func->head.interpret = interpret_rank;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_pop(&AST_STACK, (void **) &(func->param1));
		stack_push(&AST_STACK, func);
	}
  |
	RANK ROUND_BRACKET_L mdm_entity_universal_path COMMA set_statement ROUND_BRACKET_R {
		ASTNumFunc_Rank *func = mam_alloc(sizeof(ASTNumFunc_Rank), OBJ_TYPE__ASTNumFunc_Rank, NULL, 0);
		func->head.interpret = interpret_rank;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_pop(&AST_STACK, (void **) &(func->param1));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__median:
	MEDIAN ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Median *func = mam_alloc(sizeof(ASTNumFunc_Median), OBJ_TYPE__ASTNumFunc_Median, NULL, 0);
		func->head.interpret = interpret_median;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	MEDIAN ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_Median *func = mam_alloc(sizeof(ASTNumFunc_Median), OBJ_TYPE__ASTNumFunc_Median, NULL, 0);
		func->head.interpret = interpret_median;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__aggregate:
	AGGREGATE ROUND_BRACKET_L set_statement COMMA expression COMMA VAR {
		ASTNumFunc_Aggregate *func = mam_alloc(sizeof(ASTNumFunc_Aggregate), OBJ_TYPE__ASTNumFunc_Aggregate, NULL, 0);
		func->head.interpret = interpret_aggregate;

		if (!strcasecmp(yytext, "DEFAULT")) {
			func->opt = FAO_DEFAULT;
		} else if (!strcasecmp(yytext, "SUM")) {
			func->opt = FAO_SUM;
		} else if (!strcasecmp(yytext, "COUNT")) {
			func->opt = FAO_COUNT;
		} else if (!strcasecmp(yytext, "MAX")) {
			func->opt = FAO_MAX;
		} else if (!strcasecmp(yytext, "MIN")) {
			func->opt = FAO_MIN;
		} else if (!strcasecmp(yytext, "DistinctCount")) {
			func->opt = FAO_DISTINCT_COUNT;
		}

		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	} ROUND_BRACKET_R
  |
	AGGREGATE ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Aggregate *func = mam_alloc(sizeof(ASTNumFunc_Aggregate), OBJ_TYPE__ASTNumFunc_Aggregate, NULL, 0);
		func->head.interpret = interpret_aggregate;
		func->opt = FAO_SUM;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	AGGREGATE ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_Aggregate *func = mam_alloc(sizeof(ASTNumFunc_Aggregate), OBJ_TYPE__ASTNumFunc_Aggregate, NULL, 0);
		func->head.interpret = interpret_aggregate;
		func->opt = FAO_SUM;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn_sum:
	SUM ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_Sum *func = mam_alloc(sizeof(ASTNumFunc_Sum), OBJ_TYPE__ASTNumFunc_Sum, NULL, 0);
		func->head.interpret = interpret_sum;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  | SUM ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Sum *func = mam_alloc(sizeof(ASTNumFunc_Sum), OBJ_TYPE__ASTNumFunc_Sum, NULL, 0);
		func->head.interpret = interpret_sum;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn_count:
	COUNT ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_Count *func = mam_alloc(sizeof(ASTNumFunc_Count), OBJ_TYPE__ASTNumFunc_Count, NULL, 0);
		func->head.interpret = interpret_count;
		func->include_empty = 0;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  | COUNT ROUND_BRACKET_L set_statement COMMA EXCLUDEEMPTY ROUND_BRACKET_R {
		ASTNumFunc_Count *func = mam_alloc(sizeof(ASTNumFunc_Count), OBJ_TYPE__ASTNumFunc_Count, NULL, 0);
		func->head.interpret = interpret_count;
		func->include_empty = 0;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  | COUNT ROUND_BRACKET_L set_statement COMMA INCLUDEEMPTY ROUND_BRACKET_R {
		ASTNumFunc_Count *func = mam_alloc(sizeof(ASTNumFunc_Count), OBJ_TYPE__ASTNumFunc_Count, NULL, 0);
		func->head.interpret = interpret_count;
		func->include_empty = 1;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

exp_fn__max:
	MAX ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_MaxMin *func = mam_alloc(sizeof(ASTNumFunc_MaxMin), OBJ_TYPE__ASTNumFunc_MaxMin, NULL, 0);
		func->head.interpret = interpret_maxmin;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		func->opt = 'x';
		stack_push(&AST_STACK, func);
	}
  |
	MAX ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_MaxMin *func = mam_alloc(sizeof(ASTNumFunc_MaxMin), OBJ_TYPE__ASTNumFunc_MaxMin, NULL, 0);
		func->head.interpret = interpret_maxmin;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		func->opt = 'x';
		stack_push(&AST_STACK, func);
	}
;

exp_fn__min:
	MIN ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_MaxMin *func = mam_alloc(sizeof(ASTNumFunc_MaxMin), OBJ_TYPE__ASTNumFunc_MaxMin, NULL, 0);
		func->head.interpret = interpret_maxmin;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		func->opt = 'i';
		stack_push(&AST_STACK, func);
	}
  |
	MIN ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_MaxMin *func = mam_alloc(sizeof(ASTNumFunc_MaxMin), OBJ_TYPE__ASTNumFunc_MaxMin, NULL, 0);
		func->head.interpret = interpret_maxmin;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		func->opt = 'i';
		stack_push(&AST_STACK, func);
	}
;

exp_fn__avg:
	AVG ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTNumFunc_Avg *func = mam_alloc(sizeof(ASTNumFunc_Avg), OBJ_TYPE__ASTNumFunc_Avg, NULL, 0);
		func->head.interpret = interpret_avg;
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	AVG ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTNumFunc_Avg *func = mam_alloc(sizeof(ASTNumFunc_Avg), OBJ_TYPE__ASTNumFunc_Avg, NULL, 0);
		func->head.interpret = interpret_avg;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	AVG ROUND_BRACKET_L set_statement COMMA expression COMMA INCLUDEEMPTY ROUND_BRACKET_R {
		// IncludeEmpty
		ASTNumFunc_Avg *func = mam_alloc(sizeof(ASTNumFunc_Avg), OBJ_TYPE__ASTNumFunc_Avg, NULL, 0);
		func->head.interpret = interpret_avg;
		stack_pop(&AST_STACK, (void **) &(func->expdef));
		stack_pop(&AST_STACK, (void **) &(func->setdef));
		func->include_empty = 1;
		stack_push(&AST_STACK, func);
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
	set_func_qtd {}
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
  |
	set_func_distinct {}
  |
	set_func_drilldownlevel {}
  |
	set_func_DrilldownLevelBottom {}
  |
	set_func_DrilldownLevelTop {}
  |
	set_func_DrilldownMember {}
  |
	set_func_DrilldownMemberBottom {}
  |
	set_func_DrilldownMemberTop {}
  |
	set_func_DrillupLevel {}
  |
	set_func_DrillupMember {}
  |
	set_func_Ancestors {}
  |
	set_func_BottomCount {}
  |
	set_func_BottomSum {}
  |
	set_func_TopSum {}
  |
	set_func_Extract {}
  |
	set_func_PeriodsToDate {}
  |
	set_func_Generate {}
  |
	set_func_Head {}
  |
	set_func_Subset {}
;

set_func_Subset:
	SUB_SET ROUND_BRACKET_L set_statement COMMA decimal_value COMMA decimal_value ROUND_BRACKET_R {
		ASTSetFunc_Subset *func = mam_alloc(sizeof(ASTSetFunc_Subset), OBJ_TYPE__ASTSetFunc_Subset, NULL, 0);
		func->head.interpret = interpret_Subset;

		long count_;
		long index_;
		stack_pop(&AST_STACK, (void **)&count_);
		stack_pop(&AST_STACK, (void **)&index_);

		func->count = (int)count_;
		func->index = (int)index_;

		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	SUB_SET ROUND_BRACKET_L set_statement COMMA decimal_value ROUND_BRACKET_R {
		ASTSetFunc_Subset *func = mam_alloc(sizeof(ASTSetFunc_Subset), OBJ_TYPE__ASTSetFunc_Subset, NULL, 0);
		func->head.interpret = interpret_Subset;

		long index_;
		stack_pop(&AST_STACK, (void **)&index_);

		func->count = 1;
		func->index = (int)index_;

		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

set_func_Head:
	HEAD ROUND_BRACKET_L set_statement COMMA decimal_value ROUND_BRACKET_R {
		ASTSetFunc_Head *func = mam_alloc(sizeof(ASTSetFunc_Head), OBJ_TYPE__ASTSetFunc_Head, NULL, 0);
		func->head.interpret = interpret_Head;
		long count_;
		stack_pop(&AST_STACK, (void **)&count_);
		func->count = (int)count_;
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	HEAD ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTSetFunc_Head *func = mam_alloc(sizeof(ASTSetFunc_Head), OBJ_TYPE__ASTSetFunc_Head, NULL, 0);
		func->head.interpret = interpret_Head;
		func->count = 1;
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

set_func_Generate:
	GENERATE ROUND_BRACKET_L set_statement COMMA set_statement COMMA ALL ROUND_BRACKET_R {
		ASTSetFunc_Generate *func = mam_alloc(sizeof(ASTSetFunc_Generate), OBJ_TYPE__ASTSetFunc_Generate, NULL, 0);
		func->head.interpret = interpret_Generate;
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		func->all = 1;
		stack_push(&AST_STACK, func);
	}
  |
	GENERATE ROUND_BRACKET_L set_statement COMMA set_statement ROUND_BRACKET_R {
		ASTSetFunc_Generate *func = mam_alloc(sizeof(ASTSetFunc_Generate), OBJ_TYPE__ASTSetFunc_Generate, NULL, 0);
		func->head.interpret = interpret_Generate;
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		stack_push(&AST_STACK, func);
	}
;

// PeriodsToDate
set_func_PeriodsToDate:
	PERIODS_TO_DATE ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_PeriodsToDate *func = mam_alloc(sizeof(ASTSetFunc_PeriodsToDate), OBJ_TYPE__ASTSetFunc_PeriodsToDate, NULL, 0);
		func->head.interpret = interpret_PeriodsToDate;
		stack_pop(&AST_STACK, (void **)&(func->mrole_def));
		stack_pop(&AST_STACK, (void **)&(func->lrole_def));
		stack_push(&AST_STACK, func);
	}
  |
	PERIODS_TO_DATE ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_PeriodsToDate *func = mam_alloc(sizeof(ASTSetFunc_PeriodsToDate), OBJ_TYPE__ASTSetFunc_PeriodsToDate, NULL, 0);
		func->head.interpret = interpret_PeriodsToDate;
		stack_pop(&AST_STACK, (void **)&(func->lrole_def));
		stack_push(&AST_STACK, func);
	}
  |
	PERIODS_TO_DATE ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTSetFunc_PeriodsToDate *func = mam_alloc(sizeof(ASTSetFunc_PeriodsToDate), OBJ_TYPE__ASTSetFunc_PeriodsToDate, NULL, 0);
		func->head.interpret = interpret_PeriodsToDate;
		stack_push(&AST_STACK, func);
	}
;

set_func_Extract:
	EXTRACT ROUND_BRACKET_L extract_params ROUND_BRACKET_R {}
;

extract_params:
	set_statement COMMA mdm_entity_universal_path {
		ASTSetFunc_Extract *func = mam_alloc(sizeof(ASTSetFunc_Extract), OBJ_TYPE__ASTSetFunc_Extract, NULL, 0);
		func->head.interpret = interpret_Extract;
		func->dhlist = als_new(8, "MDMEntityUniversalPath *", THREAD_MAM, NULL);
		void *up = NULL;
		stack_pop(&AST_STACK, (void **)&up);
		als_add(func->dhlist, up);
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	extract_params COMMA mdm_entity_universal_path {
		void *up = NULL;
		stack_pop(&AST_STACK, (void **)&up);
		ASTSetFunc_Extract *func = NULL;
		stack_pop(&AST_STACK, (void **)&func);
		als_add(func->dhlist, up);
		stack_push(&AST_STACK, func);
	}
;

set_func_TopSum:
	TOP_SUM ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_BottomTopSum *func = mam_alloc(sizeof(ASTSetFunc_BottomTopSum), OBJ_TYPE__ASTSetFunc_BottomTopSum, NULL, 0);
		func->head.interpret = interpret_BottomTopSum;
		func->type = 't';
		stack_pop(&AST_STACK, (void **)&(func->expdef2));
		stack_pop(&AST_STACK, (void **)&(func->expdef1));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

set_func_BottomSum:
	BOTTOM_SUM ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_BottomTopSum *func = mam_alloc(sizeof(ASTSetFunc_BottomTopSum), OBJ_TYPE__ASTSetFunc_BottomTopSum, NULL, 0);
		func->head.interpret = interpret_BottomTopSum;
		func->type = 'b';
		stack_pop(&AST_STACK, (void **)&(func->expdef2));
		stack_pop(&AST_STACK, (void **)&(func->expdef1));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

set_func_BottomCount:
	BOTTOM_COUNT ROUND_BRACKET_L set_statement COMMA expression COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_BottomCount *func = mam_alloc(sizeof(ASTSetFunc_BottomCount), OBJ_TYPE__ASTSetFunc_BottomCount, NULL, 0);
		func->head.interpret = interpret_BottomCount;
		stack_pop(&AST_STACK, (void **)&(func->exp));
		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	BOTTOM_COUNT ROUND_BRACKET_L set_statement COMMA expression ROUND_BRACKET_R {
		ASTSetFunc_BottomCount *func = mam_alloc(sizeof(ASTSetFunc_BottomCount), OBJ_TYPE__ASTSetFunc_BottomCount, NULL, 0);
		func->head.interpret = interpret_BottomCount;
		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

set_func_Ancestors:
	ANCESTORS ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_Ancestors *func = mam_alloc(sizeof(ASTSetFunc_Ancestors), OBJ_TYPE__ASTSetFunc_Ancestors, NULL, 0);
		func->head.interpret = interpret_Ancestors;
		stack_pop(&AST_STACK, (void **)&(func->lvdef));
		stack_pop(&AST_STACK, (void **)&(func->mrdef));
		stack_push(&AST_STACK, func);
	}
  |
	ANCESTORS ROUND_BRACKET_L mdm_entity_universal_path COMMA decimal_value ROUND_BRACKET_R {
		ASTSetFunc_Ancestors *func = mam_alloc(sizeof(ASTSetFunc_Ancestors), OBJ_TYPE__ASTSetFunc_Ancestors, NULL, 0);
		func->head.interpret = interpret_Ancestors;

		long dist;
		stack_pop(&AST_STACK, (void **)&dist);
		func->distance = (unsigned int)dist;

		stack_pop(&AST_STACK, (void **)&(func->mrdef));
		stack_push(&AST_STACK, func);
	}
;

set_func_DrillupMember:
	DRILLUP_MEMBER ROUND_BRACKET_L set_statement COMMA set_statement ROUND_BRACKET_R {
		ASTSetFunc_DrillupMember *func = mam_alloc(sizeof(ASTSetFunc_DrillupMember), OBJ_TYPE__ASTSetFunc_DrillupMember, NULL, 0);
		func->head.interpret = interpret_drillupmember;
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		stack_push(&AST_STACK, func);
	}
;

set_func_DrillupLevel:
	DRILLUP_LEVEL ROUND_BRACKET_L set_statement COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_DrillupLevel *func = mam_alloc(sizeof(ASTSetFunc_DrillupLevel), OBJ_TYPE__ASTSetFunc_DrillupLevel, NULL, 0);
		func->head.interpret = interpret_drilluplevel;
		stack_pop(&AST_STACK, (void **)&(func->lrdef));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	DRILLUP_LEVEL ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTSetFunc_DrillupLevel *func = mam_alloc(sizeof(ASTSetFunc_DrillupLevel), OBJ_TYPE__ASTSetFunc_DrillupLevel, NULL, 0);
		func->head.interpret = interpret_drilluplevel;
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

// DrilldownMemberBottom(set, set, count [, numeric_expression] [, RECURSIVE] )
set_func_DrilldownMemberBottom:
	DRILL_DOWN_MEMBER_BOTTOM ROUND_BRACKET_L drilldownmember_bottomtop_params ROUND_BRACKET_R {
		ASTSetFunc_DrilldownMemberBottomTop *func = NULL;
		stack_pop(&AST_STACK, (void **)&func);
		func->type = 'b';
		stack_push(&AST_STACK, func);
	}
;

// DrilldownMemberTop(set, set, count [, numeric_expression] [, RECURSIVE] )
set_func_DrilldownMemberTop:
	DRILL_DOWN_MEMBER_TOP ROUND_BRACKET_L drilldownmember_bottomtop_params ROUND_BRACKET_R {
		ASTSetFunc_DrilldownMemberBottomTop *func = NULL;
		stack_pop(&AST_STACK, (void **)&func);
		func->type = 't';
		stack_push(&AST_STACK, func);
	}
;

drilldownmember_bottomtop_params:
	set_statement COMMA set_statement COMMA expression COMMA expression COMMA RECURSIVE {
		ASTSetFunc_DrilldownMemberBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownMemberBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownMemberBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownmemberbottomtop;
		func->recursive = 1;
		stack_pop(&AST_STACK, (void **)&(func->num_exp));
		stack_pop(&AST_STACK, (void **)&(func->count_exp));
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		stack_push(&AST_STACK, func);
	}
  |
	set_statement COMMA set_statement COMMA expression COMMA expression {
		ASTSetFunc_DrilldownMemberBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownMemberBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownMemberBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownmemberbottomtop;
		stack_pop(&AST_STACK, (void **)&(func->num_exp));
		stack_pop(&AST_STACK, (void **)&(func->count_exp));
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		stack_push(&AST_STACK, func);
	}
  |
	set_statement COMMA set_statement COMMA expression COMMA RECURSIVE {
		ASTSetFunc_DrilldownMemberBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownMemberBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownMemberBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownmemberbottomtop;
		func->recursive = 1;
		stack_pop(&AST_STACK, (void **)&(func->count_exp));
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		stack_push(&AST_STACK, func);
	}
  |
	set_statement COMMA set_statement COMMA expression {
		ASTSetFunc_DrilldownMemberBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownMemberBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownMemberBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownmemberbottomtop;
		stack_pop(&AST_STACK, (void **)&(func->count_exp));
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		stack_push(&AST_STACK, func);
	}
;

set_func_DrilldownMember:
	DRILL_DOWN_MEMBER ROUND_BRACKET_L set_statement COMMA set_statement ROUND_BRACKET_R {
		ASTSetFunc_DrilldownMember *func = mam_alloc(sizeof(ASTSetFunc_DrilldownMember), OBJ_TYPE__ASTSetFunc_DrilldownMember, NULL, 0);
		func->head.interpret = interpret_drilldownmember;
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		stack_push(&AST_STACK, func);
	}
  |
	DRILL_DOWN_MEMBER ROUND_BRACKET_L set_statement COMMA set_statement COMMA RECURSIVE ROUND_BRACKET_R {
		ASTSetFunc_DrilldownMember *func = mam_alloc(sizeof(ASTSetFunc_DrilldownMember), OBJ_TYPE__ASTSetFunc_DrilldownMember, NULL, 0);
		func->head.interpret = interpret_drilldownmember;
		func->recursive = 1;
		stack_pop(&AST_STACK, (void **)&(func->setdef2));
		stack_pop(&AST_STACK, (void **)&(func->setdef1));
		stack_push(&AST_STACK, func);
	}
;

// DrilldownLevelBottom(Set_def, Count [, Level_def] [, Expression] [, INCLUDE_CALC_MEMBERS])
set_func_DrilldownLevelBottom:
	DRILL_DOWN_LEVEL_BOTTOM ROUND_BRACKET_L DrilldownLevel_BottomAndTop_params ROUND_BRACKET_R {
		ASTSetFunc_DrilldownLevelBottomTop *func = NULL;
		stack_pop(&AST_STACK, (void **)&func);
		func->type = 'b';
		stack_push(&AST_STACK, func);
	}
;

// DrilldownLevelTop(Set_def, Count [, Level_def] [, Expression] [, INCLUDE_CALC_MEMBERS])
set_func_DrilldownLevelTop:
	DRILL_DOWN_LEVEL_TOP ROUND_BRACKET_L DrilldownLevel_BottomAndTop_params ROUND_BRACKET_R {
		ASTSetFunc_DrilldownLevelBottomTop *func = NULL;
		stack_pop(&AST_STACK, (void **)&func);
		func->type = 't';
		stack_push(&AST_STACK, func);
	}
;

DrilldownLevel_BottomAndTop_params:
	set_statement COMMA expression {
		ASTSetFunc_DrilldownLevelBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevelBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownLevelBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownlevelbottomtop;

		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	set_statement COMMA expression COMMA expression COMMA expression {
		ASTSetFunc_DrilldownLevelBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevelBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownLevelBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownlevelbottomtop;

		stack_pop(&AST_STACK, (void **)&(func->sortexp));
		stack_pop(&AST_STACK, (void **)&(func->uncertainexp));
		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	set_statement COMMA expression COMMA expression {
		ASTSetFunc_DrilldownLevelBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevelBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownLevelBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownlevelbottomtop;

		stack_pop(&AST_STACK, (void **)&(func->uncertainexp));
		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);

	}
  |
	set_statement COMMA expression COMMA INCLUDE_CALC_MEMBERS {
		ASTSetFunc_DrilldownLevelBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevelBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownLevelBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownlevelbottomtop;
		func->include_calc_members = 1;

		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);

	}
  |
	set_statement COMMA expression COMMA expression COMMA expression COMMA INCLUDE_CALC_MEMBERS {
		ASTSetFunc_DrilldownLevelBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevelBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownLevelBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownlevelbottomtop;
		func->include_calc_members = 1;

		stack_pop(&AST_STACK, (void **)&(func->sortexp));
		stack_pop(&AST_STACK, (void **)&(func->uncertainexp));
		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	set_statement COMMA expression COMMA expression COMMA INCLUDE_CALC_MEMBERS {
		ASTSetFunc_DrilldownLevelBottomTop *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevelBottomTop), OBJ_TYPE__ASTSetFunc_DrilldownLevelBottomTop, NULL, 0);
		func->head.interpret = interpret_drilldownlevelbottomtop;
		func->include_calc_members = 1;

		stack_pop(&AST_STACK, (void **)&(func->uncertainexp));
		stack_pop(&AST_STACK, (void **)&(func->countexp));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;

set_func_drilldownlevel:
	DRILL_DOWN_LEVEL ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		// DrilldownLevel(Set_Expression)
		ASTSetFunc_DrilldownLevel *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevel), OBJ_TYPE__ASTSetFunc_DrilldownLevel, NULL, 0);
		func->head.interpret = interpret_drilldownlevel;

		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	DRILL_DOWN_LEVEL ROUND_BRACKET_L set_statement COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		// DrilldownLevel(Set_Expression, Level_Expression)
		ASTSetFunc_DrilldownLevel *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevel), OBJ_TYPE__ASTSetFunc_DrilldownLevel, NULL, 0);
		func->head.interpret = interpret_drilldownlevel;

		stack_pop(&AST_STACK, (void **)&(func->lvrole_up));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	DRILL_DOWN_LEVEL ROUND_BRACKET_L set_statement COMMA decimal_value ROUND_BRACKET_R {
		// DrilldownLevel(Set_Expression, Index)
		ASTSetFunc_DrilldownLevel *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevel), OBJ_TYPE__ASTSetFunc_DrilldownLevel, NULL, 0);
		func->head.interpret = interpret_drilldownlevel;

		long lidx;
		stack_pop(&AST_STACK, (void **)&lidx);
		func->index = (int)lidx;

		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	DRILL_DOWN_LEVEL ROUND_BRACKET_L set_statement COMMA INCLUDE_CALC_MEMBERS ROUND_BRACKET_R {
		// DrilldownLevel(Set_Expression, INCLUDE_CALC_MEMBERS)
		ASTSetFunc_DrilldownLevel *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevel), OBJ_TYPE__ASTSetFunc_DrilldownLevel, NULL, 0);
		func->head.interpret = interpret_drilldownlevel;
		func->include_calc_members = 1;

		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	DRILL_DOWN_LEVEL ROUND_BRACKET_L set_statement COMMA mdm_entity_universal_path COMMA INCLUDE_CALC_MEMBERS ROUND_BRACKET_R {
		// DrilldownLevel(Set_Expression, Level_Expression, INCLUDE_CALC_MEMBERS)
		ASTSetFunc_DrilldownLevel *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevel), OBJ_TYPE__ASTSetFunc_DrilldownLevel, NULL, 0);
		func->head.interpret = interpret_drilldownlevel;
		func->include_calc_members = 1;

		stack_pop(&AST_STACK, (void **)&(func->lvrole_up));
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	DRILL_DOWN_LEVEL ROUND_BRACKET_L set_statement COMMA decimal_value COMMA INCLUDE_CALC_MEMBERS ROUND_BRACKET_R {
		// DrilldownLevel(Set_Expression, Index, INCLUDE_CALC_MEMBERS)
		ASTSetFunc_DrilldownLevel *func = mam_alloc(sizeof(ASTSetFunc_DrilldownLevel), OBJ_TYPE__ASTSetFunc_DrilldownLevel, NULL, 0);
		func->head.interpret = interpret_drilldownlevel;
		func->include_calc_members = 1;

		long lidx;
		stack_pop(&AST_STACK, (void **)&lidx);
		func->index = (int)lidx;

		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
;


set_func_distinct:
	DISTINCT ROUND_BRACKET_L set_statement ROUND_BRACKET_R {
		ASTSetFunc_Distinct *func = mam_alloc(sizeof(ASTSetFunc_Distinct), OBJ_TYPE__ASTSetFunc_Distinct, NULL, 0);
		func->head.interpret = interpret_distinct;
		stack_pop(&AST_STACK, (void **)&(func->setdef));
		stack_push(&AST_STACK, func);
	}
  |
	DISTINCT ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTSetFunc_Distinct *func = mam_alloc(sizeof(ASTSetFunc_Distinct), OBJ_TYPE__ASTSetFunc_Distinct, NULL, 0);
		func->head.interpret = interpret_distinct;
		stack_push(&AST_STACK, func);
	}
  |
	DISTINCT {
		ASTSetFunc_Distinct *func = mam_alloc(sizeof(ASTSetFunc_Distinct), OBJ_TYPE__ASTSetFunc_Distinct, NULL, 0);
		func->head.interpret = interpret_distinct;
		stack_push(&AST_STACK, func);
	}
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

set_func_qtd:
	QTD ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTSetFunc_QTD *func = mam_alloc(sizeof(ASTSetFunc_QTD), OBJ_TYPE__ASTSetFunc_QTD, NULL, 0);
		func->head.interpret = interpret_qtd;
		stack_pop(&AST_STACK, (void **)&(func->mrole_def));
		stack_push(&AST_STACK, func);
	}
  |
	QTD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTSetFunc_QTD *func = mam_alloc(sizeof(ASTSetFunc_QTD), OBJ_TYPE__ASTSetFunc_QTD, NULL, 0);
		func->head.interpret = interpret_qtd;
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
  |
	NOT boolean_term {
		BooleanTerm *bt;
		stack_pop(&AST_STACK, (void **) &bt);
		BooleanExpression *bool_exp = BooleanExpression_creat();
		bool_exp->reversed = 1;
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

member_func_last_child:
	LAST_CHILD ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_LastChild *func = mam_alloc(sizeof(ASTMemberFn_LastChild), OBJ_TYPE__ASTMemberFn_LastChild, NULL, 0);
		func->head.interpret = interpret_lastchild;
		stack_pop(&AST_STACK, (void **) &(func->mr_up));
		stack_push(&AST_STACK, func);
	}
  |
	LAST_CHILD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_LastChild *func = mam_alloc(sizeof(ASTMemberFn_LastChild), OBJ_TYPE__ASTMemberFn_LastChild, NULL, 0);
		func->head.interpret = interpret_lastchild;
		stack_push(&AST_STACK, func);
	}
  |
	LAST_CHILD {
		ASTMemberFn_LastChild *func = mam_alloc(sizeof(ASTMemberFn_LastChild), OBJ_TYPE__ASTMemberFn_LastChild, NULL, 0);
		func->head.interpret = interpret_lastchild;
		stack_push(&AST_STACK, func);
	}
;

member_func_first_sibling:
   FIRST_SIBLING ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_FirstSibling *func = mam_alloc(sizeof(ASTMemberFn_FirstSibling), OBJ_TYPE__ASTMemberFn_FirstSibling, NULL, 0);
		func->head.interpret = interpret_firstsibling;
		stack_pop(&AST_STACK, (void **) &(func->mr_up));
		stack_push(&AST_STACK, func);
	}
  |
	FIRST_SIBLING ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_FirstSibling *func = mam_alloc(sizeof(ASTMemberFn_FirstSibling), OBJ_TYPE__ASTMemberFn_FirstSibling, NULL, 0);
		func->head.interpret = interpret_firstsibling;
		stack_push(&AST_STACK, func);
	}
  |
	FIRST_SIBLING {
		ASTMemberFn_FirstSibling *func = mam_alloc(sizeof(ASTMemberFn_FirstSibling), OBJ_TYPE__ASTMemberFn_FirstSibling, NULL, 0);
		func->head.interpret = interpret_firstsibling;
		stack_push(&AST_STACK, func);
	}
;

member_func_last_sibling:
	LAST_SIBLING ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_LastSibling *func = mam_alloc(sizeof(ASTMemberFn_LastSibling), OBJ_TYPE__ASTMemberFn_LastSibling, NULL, 0);
		func->head.interpret = interpret_lastsibling;
		stack_pop(&AST_STACK, (void **) &(func->mr_up));
		stack_push(&AST_STACK, func);
	}
  |
	LAST_SIBLING ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_LastSibling *func = mam_alloc(sizeof(ASTMemberFn_LastSibling), OBJ_TYPE__ASTMemberFn_LastSibling, NULL, 0);
		func->head.interpret = interpret_lastsibling;
		stack_push(&AST_STACK, func);
	}
  |
	LAST_SIBLING {
		ASTMemberFn_LastSibling *func = mam_alloc(sizeof(ASTMemberFn_LastSibling), OBJ_TYPE__ASTMemberFn_LastSibling, NULL, 0);
		func->head.interpret = interpret_lastsibling;
		stack_push(&AST_STACK, func);
	}
;

member_func_lag:
	LAG ROUND_BRACKET_L mdm_entity_universal_path COMMA decimal_value ROUND_BRACKET_R {
		ASTMemberFn_Lag *func = mam_alloc(sizeof(ASTMemberFn_Lag), OBJ_TYPE__ASTMemberFn_Lag, NULL, 0);
		func->head.interpret = interpret_lag;
		void *ptol = NULL;
		stack_pop(&AST_STACK, (void **) &ptol);
		func->index = (long)ptol;
		stack_pop(&AST_STACK, (void **) &(func->mr_up));
		stack_push(&AST_STACK, func);
	}
  |
	LAG ROUND_BRACKET_L decimal_value ROUND_BRACKET_R {
		ASTMemberFn_Lag *func = mam_alloc(sizeof(ASTMemberFn_Lag), OBJ_TYPE__ASTMemberFn_Lag, NULL, 0);
		func->head.interpret = interpret_lag;
		void *ptol = NULL;
		stack_pop(&AST_STACK, (void **) &ptol);
		func->index = (long)ptol;
		stack_push(&AST_STACK, func);
	}
;

member_func_lead:
	LEAD ROUND_BRACKET_L mdm_entity_universal_path COMMA decimal_value ROUND_BRACKET_R {
		ASTMemberFn_Lag *func = mam_alloc(sizeof(ASTMemberFn_Lag), OBJ_TYPE__ASTMemberFn_Lag, NULL, 0);
		func->head.interpret = interpret_lag;
		void *ptol = NULL;
		stack_pop(&AST_STACK, (void **) &ptol);
		func->index = 0L - (long)ptol;
		stack_pop(&AST_STACK, (void **) &(func->mr_up));
		stack_push(&AST_STACK, func);
	}
  |
	LEAD ROUND_BRACKET_L decimal_value ROUND_BRACKET_R {
		ASTMemberFn_Lag *func = mam_alloc(sizeof(ASTMemberFn_Lag), OBJ_TYPE__ASTMemberFn_Lag, NULL, 0);
		func->head.interpret = interpret_lag;
		void *ptol = NULL;
		stack_pop(&AST_STACK, (void **) &ptol);
		func->index = 0L - (long)ptol;
		stack_push(&AST_STACK, func);
	}
;

member_func_parallel_period:
	PARALLEL_PERIOD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_ParallelPeriod *func = mam_alloc(sizeof(ASTMemberFn_ParallelPeriod), OBJ_TYPE__ASTMemberFn_ParallelPeriod, NULL, 0);
		func->head.interpret = interpret_parallelperiod;
		stack_push(&AST_STACK, func);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_ParallelPeriod *func = mam_alloc(sizeof(ASTMemberFn_ParallelPeriod), OBJ_TYPE__ASTMemberFn_ParallelPeriod, NULL, 0);
		func->head.interpret = interpret_parallelperiod;
		stack_pop(&AST_STACK, (void **) &(func->mroleup));
		stack_push(&AST_STACK, func);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L mdm_entity_universal_path COMMA expression ROUND_BRACKET_R {
		ASTMemberFn_ParallelPeriod *func = mam_alloc(sizeof(ASTMemberFn_ParallelPeriod), OBJ_TYPE__ASTMemberFn_ParallelPeriod, NULL, 0);
		func->head.interpret = interpret_parallelperiod;
		stack_pop(&AST_STACK, (void **) &(func->mroleup));
		stack_pop(&AST_STACK, (void **) &(func->index));
		stack_push(&AST_STACK, func);
	}
  | 
	PARALLEL_PERIOD ROUND_BRACKET_L mdm_entity_universal_path COMMA expression COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_ParallelPeriod *func = mam_alloc(sizeof(ASTMemberFn_ParallelPeriod), OBJ_TYPE__ASTMemberFn_ParallelPeriod, NULL, 0);
		func->head.interpret = interpret_parallelperiod;
		stack_pop(&AST_STACK, (void **) &(func->mroleup));
		stack_pop(&AST_STACK, (void **) &(func->index));
		stack_pop(&AST_STACK, (void **) &(func->lvroleup));
		stack_push(&AST_STACK, func);
	}
;

member_func_closing_period:
	CLOSING_PERIOD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_ClosingPeriod *func = mam_alloc(sizeof(ASTMemberFn_ClosingPeriod), OBJ_TYPE__ASTMemberFn_ClosingPeriod, NULL, 0);
		func->head.interpret = interpret_closingperiod;
		stack_push(&AST_STACK, func);
	}
  |
	CLOSING_PERIOD ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_ClosingPeriod *func = mam_alloc(sizeof(ASTMemberFn_ClosingPeriod), OBJ_TYPE__ASTMemberFn_ClosingPeriod, NULL, 0);
		func->head.interpret = interpret_closingperiod;
		stack_pop(&AST_STACK, (void **) &(func->lvroleup));
		stack_push(&AST_STACK, func);
	}
  |
	CLOSING_PERIOD ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_ClosingPeriod *func = mam_alloc(sizeof(ASTMemberFn_ClosingPeriod), OBJ_TYPE__ASTMemberFn_ClosingPeriod, NULL, 0);
		func->head.interpret = interpret_closingperiod;
		stack_pop(&AST_STACK, (void **) &(func->mroleup));
		stack_pop(&AST_STACK, (void **) &(func->lvroleup));
		stack_push(&AST_STACK, func);
	}
;

member_func_opening_period:
	OPENING_PERIOD ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_OpeningPeriod *func = mam_alloc(sizeof(ASTMemberFn_OpeningPeriod), OBJ_TYPE__ASTMemberFn_OpeningPeriod, NULL, 0);
		func->head.interpret = interpret_openingperiod;
		stack_push(&AST_STACK, func);
	}
  |
	OPENING_PERIOD ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_OpeningPeriod *func = mam_alloc(sizeof(ASTMemberFn_OpeningPeriod), OBJ_TYPE__ASTMemberFn_OpeningPeriod, NULL, 0);
		func->head.interpret = interpret_openingperiod;
		stack_pop(&AST_STACK, (void **) &(func->lvroleup));
		stack_push(&AST_STACK, func);
	}
  |
	OPENING_PERIOD ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_OpeningPeriod *func = mam_alloc(sizeof(ASTMemberFn_OpeningPeriod), OBJ_TYPE__ASTMemberFn_OpeningPeriod, NULL, 0);
		func->head.interpret = interpret_openingperiod;
		stack_pop(&AST_STACK, (void **) &(func->mroleup));
		stack_pop(&AST_STACK, (void **) &(func->lvroleup));
		stack_push(&AST_STACK, func);
	}
;

member_func_next_member:
	NEXT_MEMBER ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_NextMember *func = mam_alloc(sizeof(ASTMemberFn_NextMember), OBJ_TYPE__ASTMemberFn_NextMember, NULL, 0);
		func->head.interpret = interpret_nextmember;
		stack_pop(&AST_STACK, (void **) &(func->mroleup));
		stack_push(&AST_STACK, func);
	}
  |
	NEXT_MEMBER ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_NextMember *func = mam_alloc(sizeof(ASTMemberFn_NextMember), OBJ_TYPE__ASTMemberFn_NextMember, NULL, 0);
		func->head.interpret = interpret_nextmember;
		stack_push(&AST_STACK, func);
	}
  |
	NEXT_MEMBER {
		ASTMemberFn_NextMember *func = mam_alloc(sizeof(ASTMemberFn_NextMember), OBJ_TYPE__ASTMemberFn_NextMember, NULL, 0);
		func->head.interpret = interpret_nextmember;
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
  |
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
	member_func_opening_period {}
  |
	member_func_next_member {}
  |
	member_func_Ancestor {}
  |
	member_func_Cousin {}
  |
	member_func_DefaultMember {}
;

member_func_DefaultMember:
	DEFAULT_MEMBER ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_DefaultMember *func = mam_alloc(sizeof(ASTMemberFn_DefaultMember), OBJ_TYPE__ASTMemberFn_DefaultMember, NULL, 0);
		func->head.interpret = interpret_DefaultMember;
		stack_pop(&AST_STACK, (void **)&(func->dhdef));
		stack_push(&AST_STACK, func);
	}
  |
	DEFAULT_MEMBER ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTMemberFn_DefaultMember *func = mam_alloc(sizeof(ASTMemberFn_DefaultMember), OBJ_TYPE__ASTMemberFn_DefaultMember, NULL, 0);
		func->head.interpret = interpret_DefaultMember;
		stack_push(&AST_STACK, func);
	}
  |
	DEFAULT_MEMBER {
		ASTMemberFn_DefaultMember *func = mam_alloc(sizeof(ASTMemberFn_DefaultMember), OBJ_TYPE__ASTMemberFn_DefaultMember, NULL, 0);
		func->head.interpret = interpret_DefaultMember;
		stack_push(&AST_STACK, func);
	}
;

member_func_Cousin:
	COUSIN ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_Cousin *func = mam_alloc(sizeof(ASTMemberFn_Cousin), OBJ_TYPE__ASTMemberFn_Cousin, NULL, 0);
		func->head.interpret = interpret_Cousin;
		stack_pop(&AST_STACK, (void **)&(func->ancedef));
		stack_pop(&AST_STACK, (void **)&(func->mrdef));
		stack_push(&AST_STACK, func);
	}
  |
	COUSIN ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_Cousin *func = mam_alloc(sizeof(ASTMemberFn_Cousin), OBJ_TYPE__ASTMemberFn_Cousin, NULL, 0);
		func->head.interpret = interpret_Cousin;
		stack_pop(&AST_STACK, (void **)&(func->ancedef));
		stack_push(&AST_STACK, func);
	}
;

member_func_Ancestor:
	ANCESTOR ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_Ancestor *func = mam_alloc(sizeof(ASTMemberFn_Ancestor), OBJ_TYPE__ASTMemberFn_Ancestor, NULL, 0);
		func->head.interpret = interpret_Ancestor;
		stack_pop(&AST_STACK, (void **)&(func->lvdef));
		stack_pop(&AST_STACK, (void **)&(func->mrdef));
		stack_push(&AST_STACK, func);
	}
  |
	ANCESTOR ROUND_BRACKET_L mdm_entity_universal_path COMMA decimal_value ROUND_BRACKET_R {
		ASTMemberFn_Ancestor *func = mam_alloc(sizeof(ASTMemberFn_Ancestor), OBJ_TYPE__ASTMemberFn_Ancestor, NULL, 0);
		func->head.interpret = interpret_Ancestor;

		long dist;
		stack_pop(&AST_STACK, (void **)&dist);
		func->distance = (unsigned int)dist;

		stack_pop(&AST_STACK, (void **)&(func->mrdef));
		stack_push(&AST_STACK, func);
	}
  |
	ANCESTOR ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTMemberFn_Ancestor *func = mam_alloc(sizeof(ASTMemberFn_Ancestor), OBJ_TYPE__ASTMemberFn_Ancestor, NULL, 0);
		func->head.interpret = interpret_Ancestor;
		stack_pop(&AST_STACK, (void **)&(func->lvdef));
		stack_push(&AST_STACK, func);
	}
  |
	ANCESTOR ROUND_BRACKET_L decimal_value ROUND_BRACKET_R {
		ASTMemberFn_Ancestor *func = mam_alloc(sizeof(ASTMemberFn_Ancestor), OBJ_TYPE__ASTMemberFn_Ancestor, NULL, 0);
		func->head.interpret = interpret_Ancestor;

		long dist;
		stack_pop(&AST_STACK, (void **)&dist);
		func->distance = (unsigned int)dist;

		stack_push(&AST_STACK, func);
	}
;

level_function:
	lv_func_Level {}
  |
	lv_func_Levels {}
;

lv_func_Levels:
	LEVELS ROUND_BRACKET_L mdm_entity_universal_path COMMA expression ROUND_BRACKET_R {
		ASTLvFunc_Levels *func = mam_alloc(sizeof(ASTLvFunc_Levels), OBJ_TYPE__ASTLvFunc_Levels, NULL, 0);
		func->head.interpret = interpret_Levels;
		stack_pop(&AST_STACK, (void **)&(func->lvexp));
		stack_pop(&AST_STACK, (void **)&(func->dhdef));
		stack_push(&AST_STACK, func);
	}
  |
	LEVELS ROUND_BRACKET_L expression ROUND_BRACKET_R {
		ASTLvFunc_Levels *func = mam_alloc(sizeof(ASTLvFunc_Levels), OBJ_TYPE__ASTLvFunc_Levels, NULL, 0);
		func->head.interpret = interpret_Levels;
		stack_pop(&AST_STACK, (void **)&(func->lvexp));
		stack_push(&AST_STACK, func);
	}
;

lv_func_Level:
	LEVEL ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTLvFunc_Level *func = mam_alloc(sizeof(ASTLvFunc_Level), OBJ_TYPE__ASTLvFunc_Level, NULL, 0);
		func->head.interpret = interpret_Level;
		stack_pop(&AST_STACK, (void **)&(func->mrdef));
		stack_push(&AST_STACK, func);
	}
  |
	LEVEL ROUND_BRACKET_L ROUND_BRACKET_R {
		ASTLvFunc_Level *func = mam_alloc(sizeof(ASTLvFunc_Level), OBJ_TYPE__ASTLvFunc_Level, NULL, 0);
		func->head.interpret = interpret_Level;
		stack_push(&AST_STACK, func);
	}
  |
	LEVEL {
		ASTLvFunc_Level *func = mam_alloc(sizeof(ASTLvFunc_Level), OBJ_TYPE__ASTLvFunc_Level, NULL, 0);
		func->head.interpret = interpret_Level;
		stack_push(&AST_STACK, func);
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
  |
	logical_func_IsAncestor {}
  |
	logical_func_IsGeneration {}
  |
	logical_func_IsLeaf {}
  |
	logical_func_IsSibling {}
;

logical_func_IsSibling:
	IS_SIBLING ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path COMMA VAR ROUND_BRACKET_R {
		ASTLogicalFunc_IsSibling *func = mam_alloc(sizeof(ASTLogicalFunc_IsSibling), OBJ_TYPE__ASTLogicalFunc_IsSibling, NULL, 0);
		func->head.interpret = interpret_IsSibling;
		func->include_member = 1;
		stack_pop(&AST_STACK, (void **)&(func->mrdef2));
		stack_pop(&AST_STACK, (void **)&(func->mrdef1));
		stack_push(&AST_STACK, func);
	}
  |
	IS_SIBLING ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTLogicalFunc_IsSibling *func = mam_alloc(sizeof(ASTLogicalFunc_IsSibling), OBJ_TYPE__ASTLogicalFunc_IsSibling, NULL, 0);
		func->head.interpret = interpret_IsSibling;
		stack_pop(&AST_STACK, (void **)&(func->mrdef2));
		stack_pop(&AST_STACK, (void **)&(func->mrdef1));
		stack_push(&AST_STACK, func);
	}
;

logical_func_IsLeaf:
	IS_LEAF ROUND_BRACKET_L mdm_entity_universal_path ROUND_BRACKET_R {
		ASTLogicalFunc_IsLeaf *func = mam_alloc(sizeof(ASTLogicalFunc_IsLeaf), OBJ_TYPE__ASTLogicalFunc_IsLeaf, NULL, 0);
		func->head.interpret = interpret_IsLeaf;
		stack_pop(&AST_STACK, (void **)&(func->mrdef));
		stack_push(&AST_STACK, func);
	}
;

logical_func_IsGeneration:
	IS_GENERATION ROUND_BRACKET_L mdm_entity_universal_path COMMA decimal_value ROUND_BRACKET_R {
		ASTLogicalFunc_IsGeneration *func = mam_alloc(sizeof(ASTLogicalFunc_IsGeneration), OBJ_TYPE__ASTLogicalFunc_IsGeneration, NULL, 0);
		func->head.interpret = interpret_IsGeneration;

		long gnum;
		stack_pop(&AST_STACK, (void **)&gnum);
		func->generation_number = (unsigned int)gnum;

		stack_pop(&AST_STACK, (void **)&(func->mrdef));
		stack_push(&AST_STACK, func);
	}
;

logical_func_IsAncestor:
	IS_ANCESTOR ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path COMMA VAR ROUND_BRACKET_R {
		ASTLogicalFunc_IsAncestor *func = mam_alloc(sizeof(ASTLogicalFunc_IsAncestor), OBJ_TYPE__ASTLogicalFunc_IsAncestor, NULL, 0);
		func->head.interpret = interpret_IsAncestor;
		stack_pop(&AST_STACK, (void **)&(func->mrdef2));
		stack_pop(&AST_STACK, (void **)&(func->mrdef1));
		func->include_member = 1;
		stack_push(&AST_STACK, func);
	}
  |
	IS_ANCESTOR ROUND_BRACKET_L mdm_entity_universal_path COMMA mdm_entity_universal_path ROUND_BRACKET_R {
		ASTLogicalFunc_IsAncestor *func = mam_alloc(sizeof(ASTLogicalFunc_IsAncestor), OBJ_TYPE__ASTLogicalFunc_IsAncestor, NULL, 0);
		func->head.interpret = interpret_IsAncestor;
		stack_pop(&AST_STACK, (void **)&(func->mrdef2));
		stack_pop(&AST_STACK, (void **)&(func->mrdef1));
		stack_push(&AST_STACK, func);
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
	level_function {
		void *lv_func = NULL;
		stack_pop(&AST_STACK, (void **) &lv_func);
		MDMEntityUniversalPath *up = mam_alloc(sizeof(MDMEntityUniversalPath), OBJ_TYPE__MDMEntityUniversalPath, NULL, 0);
		up->list = als_new(8, NULL, THREAD_MAM, NULL);
		als_add(up->list, lv_func);
		stack_push(&AST_STACK, up);
	}
  |
	mdm_entity_universal_path DOT mdm_entity_up_segment {
		ast_func_append_to_up();
	}
  |
	mdm_entity_universal_path DOT member_function {
		ast_func_append_to_up();
	}
  |
	mdm_entity_universal_path DOT set_function {
		ast_func_append_to_up();
	}
  |
	mdm_entity_universal_path DOT string_function {
		ast_func_append_to_up();
	}
  |
	mdm_entity_universal_path DOT level_function {
		ast_func_append_to_up();
	}
  |
	mdm_entity_universal_path DOT expression_function {
		ast_func_append_to_up();
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

static void ast_func_append_to_up(void) {
	void *ast_func = NULL;
	stack_pop(&AST_STACK, (void **) &ast_func);
	MDMEntityUniversalPath *up = NULL;
	stack_pop(&AST_STACK, (void **) &up);
	als_add(up->list, ast_func);
	stack_push(&AST_STACK, up);
}