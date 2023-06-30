#ifndef OBJ_TYPE_DEF_H
#define OBJ_TYPE_DEF_H 1

// TODO Replace the logic in the header section of the program about object hiding with the type type_obj.
typedef short type_obj;

#define BYTES_ALIGNMENT (sizeof(void *))
#define BYTES_ALIG_CHK_MASK (0x0000000000000007UL)


#define is_type_astmemberfunc(obj_type) (obj_type >= OBJ_TYPE__ASTMEMBERFUNC__MIN && obj_type <= OBJ_TYPE__ASTMEMBERFUNC__MAX)
#define is_type_astlogicalfunc(obj_type) (obj_type >= OBJ_TYPE__ASTLOGICALFUNC__MIN && obj_type <= OBJ_TYPE__ASTLOGICALFUNC__MAX)
#define is_type_ast_str_func(obj_type) (obj_type >= OBJ_TYPE__AST_STR_FUNC__MIN && obj_type <= OBJ_TYPE__AST_STR_FUNC__MAX)
#define is_type_ast_set_func(obj_type) (obj_type >= OBJ_TYPE__AST_SET_FUNC_MIN && obj_type <= OBJ_TYPE__AST_SET_FUNC_MAX)


// The range reserved for the member functions of the AST is 1 ~ 200.
#define OBJ_TYPE__ASTMEMBERFUNC__MIN 1

#define OBJ_TYPE__ASTMemberFunc_ClosingPeriod 1
#define OBJ_TYPE__ASTMemberFunc_CurrentMember 2
#define OBJ_TYPE__ASTMemberFunc_FirstChild 3
#define OBJ_TYPE__ASTMemberFunc_FirstSibling 4
#define OBJ_TYPE__ASTMemberFunc_Lag 5
#define OBJ_TYPE__ASTMemberFunc_LastChild 6
#define OBJ_TYPE__ASTMemberFunc_LastSibling 7
#define OBJ_TYPE__ASTMemberFunc_OpeningPeriod 8
#define OBJ_TYPE__ASTMemberFunc_ParallelPeriod 9
#define OBJ_TYPE__ASTMemberFunc_Parent 10
#define OBJ_TYPE__ASTMemberFunc_PrevMember 11

#define OBJ_TYPE__ASTMEMBERFUNC__MAX 200


// The range reserved for the logical functions of the AST is 201 ~ 300.
#define OBJ_TYPE__ASTLOGICALFUNC__MIN       201

#define OBJ_TYPE__ASTLogicalFunc_IsEmpty    201

#define OBJ_TYPE__ASTLOGICALFUNC__MAX       300


// The range reserved for the string functions of the AST is 301 ~ 400.
#define OBJ_TYPE__AST_STR_FUNC__MIN       301
#define OBJ_TYPE__ASTStrFunc_Name         301
#define OBJ_TYPE__AST_STR_FUNC__MAX       400


// The range reserved for the set functions of the AST is 401 ~ 500.
#define OBJ_TYPE__AST_SET_FUNC_MIN        401
#define OBJ_TYPE__ASTSetFunc_Children     401
#define OBJ_TYPE__AST_SET_FUNC_MAX        500


#define OBJ_TYPE__RAW_BYTES 10001


#define OBJ_TYPE__ArrayList 10002
#define OBJ_TYPE__Axis 10003
#define OBJ_TYPE__AxisDef 10004
#define OBJ_TYPE__BooleanExpression 10005
#define OBJ_TYPE__BooleanFactory 10006
#define OBJ_TYPE__BooleanTerm 10007
#define OBJ_TYPE__ByteBuf 10008
#define OBJ_TYPE__CoordinateSystem 10009
#define OBJ_TYPE__Cube 10010
#define OBJ_TYPE__CubeDef 10011
#define OBJ_TYPE__Dimension 10012
#define OBJ_TYPE__DimensionRole 10013
#define OBJ_TYPE__DimRoleDef 10014
#define OBJ_TYPE__EuclidCommand 10015
#define OBJ_TYPE__ExpFnCoalesceEmpty 10016
#define OBJ_TYPE__ExpFnCount 10017
#define OBJ_TYPE__ExpFnIif 10018
#define OBJ_TYPE__ExpFnLookUpCube 10019
#define OBJ_TYPE__ExpFnSum 10020
#define OBJ_TYPE__Expression 10021
#define OBJ_TYPE__Factory 10022
#define OBJ_TYPE__FormulaContext 10023
#define OBJ_TYPE__GeneralChainExpression 10024
#define OBJ_TYPE__IDSVectorMears 10025
#define OBJ_TYPE__Level 10026
#define OBJ_TYPE__LevelRole 10027
#define OBJ_TYPE__LevelRoleDef 10028
#define OBJ_TYPE__LinkedQueue 10029
#define OBJ_TYPE__LinkedQueueNode 10030
#define OBJ_TYPE__MDContext 10031
#define OBJ_TYPE__MddAxis 10032
#define OBJ_TYPE__MddMemberRole 10033
#define OBJ_TYPE__MddSet 10034
#define OBJ_TYPE__MddTuple 10035
#define OBJ_TYPE__MDMEntityUniversalPath 10036
#define OBJ_TYPE__MdmEntityUpSegment 10037
#define OBJ_TYPE__MeasureSpace 10038
#define OBJ_TYPE__MemAllocMng 10039
#define OBJ_TYPE__Member 10040
#define OBJ_TYPE__MemberDef 10041
#define OBJ_TYPE__MemberFormula 10042
#define OBJ_TYPE__MemberRoleFuncCurrentMember 10043
#define OBJ_TYPE__MemberRoleFuncFirstChild 10044
#define OBJ_TYPE__MemberRoleFuncFirstSibling 10045
#define OBJ_TYPE__MemberRoleFuncLag 10046
#define OBJ_TYPE__MemberRoleFuncLastChild 10047
#define OBJ_TYPE__MemberRoleFuncLastSibling 10048
#define OBJ_TYPE__MemberRoleFuncLead 10049
#define OBJ_TYPE__MemberRoleFuncParent 10050
#define OBJ_TYPE__MemberRoleFuncPrevMember 10051
#define OBJ_TYPE__MembersDef 10052
#define OBJ_TYPE__MultiDimResult 10053
#define OBJ_TYPE__RBNode 10054
#define OBJ_TYPE__RedBlackTree 10055
#define OBJ_TYPE__Scale 10056
#define OBJ_TYPE__ScaleOffsetRange 10057
#define OBJ_TYPE__SelectDef 10058
#define OBJ_TYPE__SetDef 10059
#define OBJ_TYPE__SetFnBottomOrTopPercent 10060
// #define OBJ_TYPE__SetFnChildren 10061
#define OBJ_TYPE__SetFnCrossJoin 10062
#define OBJ_TYPE__SetFnDescendants 10063
#define OBJ_TYPE__SetFnExcept 10064
#define OBJ_TYPE__SetFnFilter 10065
#define OBJ_TYPE__SetFnIntersect 10066
#define OBJ_TYPE__SetFnLateralMembers 10067
#define OBJ_TYPE__SetFnMembers 10068
#define OBJ_TYPE__SetFnOrder 10069
#define OBJ_TYPE__SetFnTail 10070
#define OBJ_TYPE__SetFnTopCount 10071
#define OBJ_TYPE__SetFnUnion 10072
#define OBJ_TYPE__SetFnYTD 10073
#define OBJ_TYPE__SetFormula 10074
// #define OBJ_TYPE__SetFuncChildren 10075
#define OBJ_TYPE__SetFuncMembers 10076
#define OBJ_TYPE__SockIntentThread 10077
#define OBJ_TYPE__StrArr 10078
#define OBJ_TYPE__STRING 10079
#define OBJ_TYPE__Term 10080
#define OBJ_TYPE__TupleDef 10081
#define OBJ_TYPE__Hierarchy 10082
#define OBJ_TYPE__HierarchyRole 10083
#define OBJ_TYPE__GridData 10084
#define OBJ_TYPE__ASTStrExp 10085

#endif