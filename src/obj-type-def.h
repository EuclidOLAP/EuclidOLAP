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
#define is_type_ast_num_func(obj_type) (obj_type >= OBJ_TYPE__ASTNUMFUNC_MIN && obj_type <= OBJ_TYPE__ASTNUMFUNC_MAX)


// The range reserved for the member functions of the AST is 1 ~ 200.
#define OBJ_TYPE__ASTMEMBERFUNC__MIN 1

#define OBJ_TYPE__ASTMemberFn_Parent 12
#define OBJ_TYPE__ASTMemberFn_CurrentMember 13
#define OBJ_TYPE__ASTMemberFn_PrevMember 14
#define OBJ_TYPE__ASTMemberFn_FirstChild 15
#define OBJ_TYPE__ASTMemberFn_LastChild 16
#define OBJ_TYPE__ASTMemberFn_FirstSibling 17
#define OBJ_TYPE__ASTMemberFn_LastSibling 18
#define OBJ_TYPE__ASTMemberFn_Lag 19
#define OBJ_TYPE__ASTMemberFn_ParallelPeriod 20
#define OBJ_TYPE__ASTMemberFn_ClosingPeriod 21
#define OBJ_TYPE__ASTMemberFn_OpeningPeriod 22
#define OBJ_TYPE__ASTMemberFn_NextMember 23
#define OBJ_TYPE__ASTMemberFn_Ancestor 24
#define OBJ_TYPE__ASTMemberFn_Cousin 25
#define OBJ_TYPE__ASTMemberFn_DefaultMember 26

#define OBJ_TYPE__ASTMEMBERFUNC__MAX 200


// The range reserved for the logical functions of the AST is 201 ~ 300.
#define OBJ_TYPE__ASTLOGICALFUNC__MIN       201

#define OBJ_TYPE__ASTLogicalFunc_IsEmpty    201
#define OBJ_TYPE__ASTLogicalFunc_IsAncestor    202
#define OBJ_TYPE__ASTLogicalFunc_IsGeneration    203
#define OBJ_TYPE__ASTLogicalFunc_IsLeaf    204
#define OBJ_TYPE__ASTLogicalFunc_IsSibling    205

#define OBJ_TYPE__ASTLOGICALFUNC__MAX       300


// The range reserved for the string functions of the AST is 301 ~ 400.
#define OBJ_TYPE__AST_STR_FUNC__MIN       301
#define OBJ_TYPE__ASTStrFunc_Name         301
#define OBJ_TYPE__AST_STR_FUNC__MAX       400


// The range reserved for the set functions of the AST is 401 ~ 500.
#define OBJ_TYPE__AST_SET_FUNC_MIN        401
#define OBJ_TYPE__ASTSetFunc_Children     401
#define OBJ_TYPE__ASTSetFunc_Members      402
#define OBJ_TYPE__ASTSetFunc_CrossJoin    403
#define OBJ_TYPE__ASTSetFunc_Filter       404
#define OBJ_TYPE__ASTSetFunc_LateralMembers       405
#define OBJ_TYPE__ASTSetFunc_Order        406
#define OBJ_TYPE__ASTSetFunc_TopCount     407
#define OBJ_TYPE__ASTSetFunc_Except       408
#define OBJ_TYPE__ASTSetFunc_YTD          409
#define OBJ_TYPE__ASTSetFunc_Descendants          410
#define OBJ_TYPE__ASTSetFunc_Tail         411
#define OBJ_TYPE__ASTSetFunc_BottomOrTopPercent         412
#define OBJ_TYPE__ASTSetFunc_Union         413
#define OBJ_TYPE__ASTSetFunc_Intersect         414
#define OBJ_TYPE__ASTSetFunc_Distinct         415
#define OBJ_TYPE__ASTSetFunc_DrilldownLevel         416
#define OBJ_TYPE__ASTSetFunc_DrilldownLevelBottomTop         417
#define OBJ_TYPE__ASTSetFunc_DrilldownMember         418
#define OBJ_TYPE__ASTSetFunc_DrilldownMemberBottomTop         419
#define OBJ_TYPE__ASTSetFunc_DrillupLevel         420
#define OBJ_TYPE__ASTSetFunc_DrillupMember         421
#define OBJ_TYPE__ASTSetFunc_Ancestors         422
#define OBJ_TYPE__ASTSetFunc_BottomCount         423
#define OBJ_TYPE__ASTSetFunc_BottomTopSum         424
#define OBJ_TYPE__ASTSetFunc_Extract         425
#define OBJ_TYPE__ASTSetFunc_PeriodsToDate         425
#define OBJ_TYPE__ASTSetFunc_QTD         426
#define OBJ_TYPE__AST_SET_FUNC_MAX        500


// The range reserved for the Numeric functions of the AST is 501 ~ 600.
#define OBJ_TYPE__ASTNUMFUNC_MIN        501
#define OBJ_TYPE__ASTNumFunc_Avg        501
#define OBJ_TYPE__ASTNumFunc_MaxMin        502
#define OBJ_TYPE__ASTNumFunc_Sum        503
#define OBJ_TYPE__ASTNumFunc_Count        504
#define OBJ_TYPE__ASTNumFunc_Aggregate        505
#define OBJ_TYPE__ASTNumFunc_Median        506
#define OBJ_TYPE__ASTNumFunc_Rank        507
#define OBJ_TYPE__ASTNumFunc_Abs        508
#define OBJ_TYPE__ASTNumFunc_Correlation        509
#define OBJ_TYPE__ASTNumFunc_Covariance        510
#define OBJ_TYPE__ASTNumFunc_LinRegIntercept        511
#define OBJ_TYPE__ASTNumFunc_LinRegSlope        512
#define OBJ_TYPE__ASTNumFunc_LinRegVariance        513
#define OBJ_TYPE__ASTNumFunc_Stdev        514
#define OBJ_TYPE__ASTNumFunc_Var        515
#define OBJ_TYPE__ASTNumFunc_CaseStatement        516
#define OBJ_TYPE__ASTNUMFUNC_MAX        600


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
// #define OBJ_TYPE__ExpFnCount 10017
#define OBJ_TYPE__ExpFnIif 10018
#define OBJ_TYPE__ExpFnLookUpCube 10019
// #define OBJ_TYPE__ExpFnSum 10020
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
#define OBJ_TYPE__MembersDef 10052
#define OBJ_TYPE__MultiDimResult 10053
#define OBJ_TYPE__RBNode 10054
#define OBJ_TYPE__RedBlackTree 10055
#define OBJ_TYPE__Scale 10056
#define OBJ_TYPE__ScaleOffsetRange 10057
#define OBJ_TYPE__SelectDef 10058
#define OBJ_TYPE__SetDef 10059
#define OBJ_TYPE__SetFormula 10074
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