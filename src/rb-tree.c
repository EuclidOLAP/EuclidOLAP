#include <stdio.h>
// #include <stdlib.h>
#include <string.h>
// #include <sys/time.h>
// #include <arpa/inet.h>
// #include <unistd.h>

#include "rb-tree.h"
#include "utils.h"

RedBlackTree *rbt_create(char *desc, int (*comparison_func)(void *obj, void *other), void *(*release_obj_func)(void *obj))
{
	RedBlackTree *t = __objAlloc__(sizeof(RedBlackTree), OBJ_TYPE__RedBlackTree);
	t->comparison_func = comparison_func;
	t->release_obj_func = release_obj_func;
	if (desc)
	{
		t->desc = __objAlloc__(strlen(desc) + 1, OBJ_TYPE__RAW_BYTES);
		sprintf(t->desc, "%s", desc);
	}
	return t;
}

void rbt_add(RedBlackTree *rbt, void *obj)
{
	RBNode *node = rbt_create_node(obj, rbt, RED_NODE);

	// [1] The new node has no parent node
	if (rbt->root == NULL)
	{
		rbt_set_node_color(node, BLACK_NODE);
		rbt->root = node;
		return;
	}

	// If the mount point is empty, the obj node already exists in the red-black tree.
	RBNode *mount_p = rbt_find_mount_point(rbt, node);
	if (mount_p == NULL)
		return;

	int cmp_rs = rbt->comparison_func(obj, mount_p->obj);
	if (cmp_rs < 0)
	{
		mount_p->child_left = node;
		node->parent = mount_p->child_left;
	}
	else
	{
		mount_p->child_right = node;
		node->parent = mount_p->child_right;
	}

	// [2] The parent node(mount_p) of the new node is black
	if ((mount_p->attrs & 0x01) == BLACK_NODE)
		return;

	// [3] The parent node(mount_p) of the new node is red
	// u_node is sibling node of mount_p
	RBNode *u_node = rbt_find_sibling_node(mount_p);

	RBNode *_G = mount_p->parent;
	RBNode *_P = mount_p;
	RBNode *_N = node;
	RBNode *_U = u_node;
	if (u_node)
	{
		// [3-1] u_node is the red data node
		rbt_set_node_color(_G, RED_NODE);
		rbt_set_node_color(_P, BLACK_NODE);
		rbt_set_node_color(_U, BLACK_NODE);
		rbt_adjust__sub_tree_red_top(rbt, _G);
	}
	else
	{
		// [3-2] u_node is Nil
		int p_g__relp = rbt->comparison_func(_P->obj, _G->obj);
		int n_p__relp = cmp_rs;

		if (p_g__relp < 0 && n_p__relp < 0)
		{
			rbt_set_node_color(_G, RED_NODE);
			rbt_set_node_color(_P, BLACK_NODE);
			rbt_rotate_right(rbt, _G);
		}
		else if (p_g__relp > 0 && n_p__relp < 0)
		{
			rbt_set_node_color(_G, RED_NODE);
			rbt_set_node_color(_N, BLACK_NODE);
			rbt_rotate_right(rbt, _P);
			rbt_rotate_left(rbt, _G);
		}
		else if (p_g__relp < 0 && n_p__relp > 0)
		{
			rbt_set_node_color(_G, RED_NODE);
			rbt_set_node_color(_N, BLACK_NODE);
			rbt_rotate_left(rbt, _P);
			rbt_rotate_right(rbt, _G);
		}
		else if (p_g__relp > 0 && n_p__relp > 0)
		{
			rbt_set_node_color(_G, RED_NODE);
			rbt_set_node_color(_P, BLACK_NODE);
			rbt_rotate_left(rbt, _G);
		}
	}
}

RBNode *rbt_create_node(void *obj, RedBlackTree *tree, char _attrs)
{
	RBNode *n = __objAlloc__(sizeof(RBNode), OBJ_TYPE__RBNode);
	n->obj = obj;
	n->attrs = _attrs;
	n->tree = tree;
	return n;
}

RBNode *rbt_find_mount_point(RedBlackTree *rbt, RBNode *new_node)
{
	RBNode *c_node = rbt->root;
	while (1)
	{
		int cmp_rs = rbt->comparison_func(new_node->obj, c_node->obj);
		if (cmp_rs < 0)
		{
			if (c_node->child_left == NULL)
				return c_node;
			c_node = c_node->child_left;
		}
		else if (cmp_rs > 0)
		{
			if (c_node->child_right == NULL)
				return c_node;
			c_node = c_node->child_right;
		}
		else
		{
			return NULL;
		}
	}
}

RBNode *rbt_find_sibling_node(RBNode *node)
{
	RBNode *p = node->parent;
	if (p)
		return p->child_left == node ? p->child_right : p->child_left;
	return NULL;
}

RBNode *rbt__find(RedBlackTree *t, void *obj)
{
	RBNode *current_node = t->root;
	while (1)
	{
		if (current_node == NULL)
			return NULL;

		int cmp = t->comparison_func(current_node->obj, obj);

		if (cmp == 0)
			return current_node;

		current_node = cmp < 0 ? current_node->child_right : current_node->child_left;
	}
}

static void _rbt__scan_nodes_do(RBNode *n, void *callback_param, void *(*callback)(RBNode *node, void *param))
{
	if (n->child_left)
		_rbt__scan_nodes_do(n->child_left, callback_param, callback);

	callback(n, callback_param);

	if (n->child_right)
		_rbt__scan_nodes_do(n->child_right, callback_param, callback);
}

void rbt__scan_do(RedBlackTree *t, void *callback_param, void *(*callback)(RBNode *node, void *param))
{
	if (t->root)
		_rbt__scan_nodes_do(t->root, callback_param, callback);
}

void rbt_set_node_color(RBNode *n, char color)
{
	n->attrs = (n->attrs & 0xFE) + (color & 0x01);
}

void rbt_rotate_left(RedBlackTree *tree, RBNode *node)
{
	node->child_right->child_left = node;
	if (node->parent)
	{
		if (tree->comparison_func(node->obj, node->parent->obj) < 0)
		{
			node->parent->child_left = node->child_right;
		}
		else
		{
			node->parent->child_right = node->child_right;
		}
		node->child_right->parent = node->parent;
	}
	else
	{
		tree->root = node->child_right;
		node->child_right->parent = NULL;
	}
	node->parent = node->child_right;
	node->child_right = NULL;
}

void rbt_rotate_right(RedBlackTree *tree, RBNode *node)
{
	node->child_left->child_right = node;
	if (node->parent)
	{
		if (tree->comparison_func(node->obj, node->parent->obj) < 0)
		{
			node->parent->child_left = node->child_left;
		}
		else
		{
			node->parent->child_right = node->child_left;
		}
		node->child_left->parent = node->parent;
	}
	else
	{
		tree->root = node->child_left;
		node->child_left->parent = NULL;
	}
	node->parent = node->child_left;
	node->child_left = NULL;
}

void rbt_adjust__sub_tree_red_top(RedBlackTree *tree, RBNode *top_node)
{
	// [4-1] top_node is root node
	if (top_node->parent == NULL)
	{
		rbt_set_node_color(top_node, BLACK_NODE);
		return;
	}

	// [4-2-2] top_node's parent is black
	if (rbt_get_node_color(top_node->parent) == BLACK_NODE)
		return;

	// [4-2-1]
	RBNode *_X = top_node;
	RBNode *_P = _X->parent;
	RBNode *_B = rbt_find_sibling_node(_X);
	RBNode *_G = _P->parent;
	RBNode *_U = rbt_find_sibling_node(_P);

	// [4-2-1-2] _U is red
	if (rbt_get_node_color(_U) == RED_NODE)
	{
		rbt_set_node_color(_G, RED_NODE);
		rbt_set_node_color(_P, BLACK_NODE);
		rbt_set_node_color(_U, BLACK_NODE);
		rbt_adjust__sub_tree_red_top(tree, _G);
		return;
	}

	// [4-2-1-1] _U is black
	int x_p__relp = tree->comparison_func(_X->obj, _P->obj);
	int p_g__relp = tree->comparison_func(_P->obj, _G->obj);

	// [4-2-1-1-1]
	if (x_p__relp < 0 && p_g__relp < 0)
	{
		rbt_rotate_right(tree, _G);
		rbt_set_node_color(_P, BLACK_NODE);
		rbt_set_node_color(_G, RED_NODE);
	}
	// [4-2-1-1-2]
	else if (x_p__relp > 0 && p_g__relp < 0)
	{
		rbt_rotate_left(tree, _P);
		rbt_rotate_right(tree, _G);
		rbt_set_node_color(_X, BLACK_NODE);
		rbt_set_node_color(_G, RED_NODE);
	}
	// [4-2-1-1-3]
	else if (x_p__relp < 0 && p_g__relp > 0)
	{
		rbt_rotate_right(tree, _P);
		rbt_rotate_left(tree, _G);
		rbt_set_node_color(_X, BLACK_NODE);
		rbt_set_node_color(_G, RED_NODE);
	}
	// [4-2-1-1-4]
	else if (x_p__relp > 0 && p_g__relp > 0)
	{
		rbt_rotate_left(tree, _G);
		rbt_set_node_color(_P, BLACK_NODE);
		rbt_set_node_color(_G, RED_NODE);
	}
}

int rbt_get_node_color(RBNode *node)
{
	return node->attrs & 0x01;
}

static void rbt__do_reordering(RBNode *n, unsigned int *idx)
{
	if (n->child_left)
		rbt__do_reordering(n->child_left, idx);

	n->index = (*idx)++;

	if (n->child_right)
		rbt__do_reordering(n->child_right, idx);
}

void rbt__reordering(RedBlackTree *t)
{
	if (t->root == NULL)
		return;

	unsigned int idx = 0;
	rbt__do_reordering(t->root, &idx);
	t->size = idx;
}

int rbt__size(RedBlackTree *t)
{
	return t->size;
}

static void _release_rbt_node(RBNode *n)
{
	if (n->child_left)
		_release_rbt_node(n->child_left);

	if (n->child_right)
		_release_rbt_node(n->child_right);

	if (n->obj)
		n->tree->release_obj_func(n->obj);

	rbt__destory_node(n);
}

void rbt__clear(RedBlackTree *t)
{
	if (t->root)
		_release_rbt_node(t->root);

	t->root = NULL;
}

void rbt__destory_node(RBNode *n)
{
	// _release_mem_(n);
}

void rbt__destory(RedBlackTree *t)
{
	rbt__clear(t);
	// _release_mem_(t);
}