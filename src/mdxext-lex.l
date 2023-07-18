%{
// lex_f_001
#include "mdxext-yacc.tab.h"
extern int yyerror(const char *);
int yywrap(void);
// lex_f_002
%}

%%

[ \n\t]+ {
	// ignore white space
}

\x2F\x2F.* {
	// ASCII 0x2F is '/'
	// comment
}

create {
	return CREATE;
}

dimensions {
	return DIMENSIONS;
}

hierarchy {
	return HIERARCHY;
}

Level {
	return LEVEL;
}

levels {
	return LEVELS;
}

members {
	return MEMBERS;
}

build	{
	return BUILD;
}

cube	{
	return CUBE;
}

measures	{
	return MEASURES;
}

insert	{
	return INSERT;
}

make	{
	return MAKE;
}

with	{
	return WITH;
}

select	{
	return SELECT;
}

from	{
	return FROM;
}

on	{
	return ON;
}

columns {
	return COLUMNS;
}

rows {
	return ROWS;
}

pages {
	return PAGES;
}

chapters {
	return CHAPTERS;
}

sections {
	return SECTIONS;
}

null {
	return NIL;
}

where	{
	return WHERE;
}

member	{
	return MEMBER;
}

as	{
	return AS;
}

set {
	return SET;
}

children {
	return CHILDREN;
}

parent {
	return PARENT;
}

sum {
	return SUM;
}

Avg {
	return AVG;
}

MAX {
	return MAX;
}

MIN {
	return MIN;
}

Aggregate {
	return AGGREGATE;
}

Median {
	return MEDIAN;
}

Rank {
	return RANK;
}

Abs {
	return ABS;
}

Correlation {
	return CORRELATION;
}

Covariance {
	return COVARIANCE;
}

LinRegIntercept {
	return LINREGINTERCEPT;
}

LinRegR2 {
	return LINREGR2;
}

LinRegVariance {
	return LIN_REG_VARIANCE;
}

Ordinal {
	return ORDINAL;
}

Stdev {
	return STDEV;
}

LinRegSlope {
	return LIN_REG_SLOPE;
}

Var {
	return FN_VAR;
}

Ancestors {
	return ANCESTORS;
}

BottomCount {
	return BOTTOM_COUNT;
}

BottomSum {
	return BOTTOM_SUM;
}

TopSum {
	return TOP_SUM;
}

Extract {
	return EXTRACT;
}

crossjoin {
	return CROSS_JOIN;
}

Generate {
	return GENERATE;
}

DrilldownLevel {
	return DRILL_DOWN_LEVEL;
}

Include_Calc_Members {
	return INCLUDE_CALC_MEMBERS;
}

DrilldownLevelBottom {
	return DRILL_DOWN_LEVEL_BOTTOM;
}

DrilldownLevelTop {
	return DRILL_DOWN_LEVEL_TOP;
}

DrillDownMember {
	return DRILL_DOWN_MEMBER;
}

DrillDownMemberBottom {
	return DRILL_DOWN_MEMBER_BOTTOM;
}

DrillDownMemberTop {
	return DRILL_DOWN_MEMBER_TOP;
}

RECURSIVE {
	return RECURSIVE;
}

DrillupLevel {
	return DRILLUP_LEVEL;
}

DrillupMember {
	return DRILLUP_MEMBER;
}

count {
	return COUNT;
}

EXCLUDEEMPTY {
	return EXCLUDEEMPTY;
}

INCLUDEEMPTY {
	return INCLUDEEMPTY;
}

filter {
	return FILTER;
}

and {
	return AND;
}

or {
	return OR;
}

currentmember {
	return CURRENT_MEMBER;
}

prevmember {
	return PREV_MEMBER;
}

parallelperiod {
	return PARALLEL_PERIOD;
}

ClosingPeriod {
	return CLOSING_PERIOD;
}

OpeningPeriod {
	return OPENING_PERIOD;
}

FirstChild {
	return FIRST_CHILD;
}

LastChild {
	return LAST_CHILD;
}

FirstSibling {
	return FIRST_SIBLING;
}

LastSibling {
	return LAST_SIBLING;
}

Ancestor {
	return ANCESTOR;
}

Cousin {
	return COUSIN;
}

DefaultMember {
	return DEFAULT_MEMBER;
}

Lag {
	return LAG;
}

Lead {
	return LEAD;
}

NextMember {
	return NEXT_MEMBER;
}

Distinct {
	return DISTINCT;
}

lateralMembers {
	return LATERAL_MEMBERS;
}

order {
	return ORDER;
}

topCount {
	return TOP_COUNT;
}

except {
	return EXCEPT;
}

ALL {
	return ALL;
}

PeriodsToDate {
	return PERIODS_TO_DATE;
}

Ytd {
	return YTD;
}

Qtd {
	return QTD;
}

descendants {
	return DESCENDANTS;
}

Tail {
	return TAIL;
}

bottomPercent {
	return BOTTOM_PERCENT;
}

topPercent {
	return TOP_PERCENT;
}

union {
	return UNION;
}

intersect {
	return INTERSECT;
}

lookUpCube {
	return LOOK_UP_CUBE;
}

iif {
	return IIF;
}

coalesceEmpty {
	return COALESCE_EMPTY;
}

IsAncestor {
	return IS_ANCESTOR;
}

IsGeneration {
	return IS_GENERATION;
}

IsLeaf {
	return IS_LEAF;
}

IsSibling {
	return IS_SIBLING;
}

IsEmpty {
	return IS_EMPTY;
}

Not {
	return NOT;
}

Case {
	return CASE;
}

Else {
	return ELSE;
}

End {
	return END;
}

When {
	return WHEN;
}

Then {
	return THEN;
}

Name {
	return NAME;
}

[_a-zA-Z][_a-zA-Z0-9]*	{
	return VAR;
}

[0-9]+(\x2E[0-9]+)? {
	// ASCII 0x2E is .
	return DECIMAL;
}

\x40\x40EXP {
	// ASCII 0x40 is @
	return FLAG_EXP;
}

","	{
	return COMMA;
}

"." {
	return DOT;
}

"&" {
	return AMPERSAND;
}

"@" {
	return AT_SIGN;
}

"("	{
	return ROUND_BRACKET_L;
}

")"	{
	return ROUND_BRACKET_R;
}

"[" {
	return OPENING_BRACKET;
}

"]" {
	return CLOSING_BRACKET;
}

"{" {
	return BRACE_L;
}

"}" {
	return BRACE_R;
}

"+" {
	return PLUS;
}

"-" {
	return MINUS;
}

"*" {
	return MULTIPLIED;
}

"/" {
	return DIVIDED;
}

"<" {
	return LESS;
}

"<=" {
	return LESS_EQ;
}

"=" {
	return EQ;
}

"<>" {
	return NOT_EQ;
}

">" {
	return GREA;
}

">=" {
	return GREA_EQ;
}

":" {
	return COLON;
}

";" {
	return SEMICOLON;
}

"->" {
	return EQUIVALENT_TO;
}

\x5B([^\x5D]|\x5D\x5D)+\x5D {
	// ASCII 0x5B is [
	// ASCII 0x5D is ]
	return BLOCK;
}

\x22[^\x22]+\x22 {
	// ASCII 0x22 is "
	return STRING;
}

<<EOF>> {
	return EOF_;
}

. {
	// do nothing
	// TODO An MDX lexical parsing-related exception should be thrown here.
}

%%
// lex_f_003

static YY_BUFFER_STATE ecp_str_buf_stat;

int eucparser_scan_string(const char *s) {

    // insist on cleaning up any existing buffer before setting up a new one
	if (ecp_str_buf_stat != NULL)
		return -1;

    // Set up and switch to a buffer for scanning the contents of the
    // specified string.  A copy of the string will be made.
    ecp_str_buf_stat = yy_scan_string(s);
    return ecp_str_buf_stat == NULL ? -1 : 0;
}

void eucparser_cleanup() {
    // No effect if ecp_str_buf_stat is NULL
    yy_delete_buffer(ecp_str_buf_stat);
    // ... but avoid trying to free the same buffer twice
    ecp_str_buf_stat = NULL;
}

int yywrap(void)
{
	return 1;
}
// lex_f_004
