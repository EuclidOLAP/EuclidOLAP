#ifndef EUCLID__RB_TREE_H
#define EUCLID__RB_TREE_H 1

#include "utils.h"

#define RED_NODE 0x01
#define BLACK_NODE 0x00

typedef struct _redblack_tree_node_
{
	// struct _redblack_tree_ *tree;

	struct _redblack_tree_node_ *parent;
	struct _redblack_tree_node_ *child_left;
	struct _redblack_tree_node_ *child_right;

	void *obj;
	unsigned int index;

	// The lowest bit represents the color of the current node, 0 is black, and 1 is red.
	char attrs;

} RBNode;

typedef struct _redblack_tree_
{
	char *desc;
	RBNode *root;
	int (*comparison_func)(void *obj, void *other);
	void *(*release_obj_func)(void *obj);
	unsigned int size;
} RedBlackTree;

void rbt__clear(RedBlackTree *);

void rbt__scan_do(RedBlackTree *t, void *callback_param, void *(*callback)(RBNode *node, void *param));

void rbt__reordering(RedBlackTree *);

void rbt__destory_node(RBNode *);

void rbt__destory(RedBlackTree *);

/**
 * @param desc
 * @param comparison_func when `other` is sorted before `obj`, return -1
 * @param release_obj_func
 * @param strat Memory allocation strategy.
 * @param mam When the strat parameter is SPEC_MAM, mam cannot be NULL.
 */
RedBlackTree *rbt_create(char *desc, int (*comparison_func)(void *obj, void *other), void *(*release_obj_func)(void *obj), enum_oms strat, MemAllocMng *mam);

void rbt_add(RedBlackTree *rbt, void *obj);

RBNode *rbt_create_node(void *obj, RedBlackTree *tree, char _attrs);

int rbt__size(RedBlackTree *);

RBNode *rbt__find(RedBlackTree *t, void *obj);

// Find a node, which is the parent node of the new node after the new node is added to the red-black tree.
RBNode *rbt_find_mount_point(RedBlackTree *rbt, RBNode *new_node);

RBNode *rbt_find_sibling_node(RBNode *node);

void rbt_set_node_color(RBNode *n, char color);

void rbt_rotate_left(RedBlackTree *tree, RBNode *node);

void rbt_rotate_right(RedBlackTree *tree, RBNode *node);

/*
 * [4]
 * One top node of a subtree in the red-black tree has changed from black to red (black height has not changed),
 * adjust the red-black tree to keep its characteristics.
 */
void rbt_adjust__sub_tree_red_top(RedBlackTree *tree, RBNode *top_node);

int rbt_get_node_color(RBNode *node);

#endif
