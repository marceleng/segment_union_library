#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "rbtree.h"

#define COLOR_RED 1
#define COLOR_BLACK 2
#define COLOR_UNDEF 0

#define LEFT 1
#define RIGHT 0

static int DEFAULT_POLICY = POLICY_IGNORE;

void rbtree_set_default_policy(int policy)
{
	assert(policy == POLICY_IGNORE || policy==POLICY_INSERT
#ifdef POLICY_OVERRIDE
			|| policy==POLICY_OVERRIDE
#endif
			);

	DEFAULT_POLICY = policy;
}

int rbtree_get_default_policy()
{
	return DEFAULT_POLICY;
}

size_t rbtree_size(rbtree *tree)
{
	if (!tree)
		return 0;
	else
		return 1+rbtree_size(tree->lchild)+rbtree_size(tree->rchild);
}

void _rbtree_update_child(rbtree * tree, rbtree *child, int left)
{
	if (left) 
		tree->lchild = child;
	else
		tree->rchild = child;

	if (child)
		child->parent = tree;
}

rbtree * _rbtree_bst_insert(rbtree *tree, rbtree *node, int policy, int *modified)
{
	*modified = 1;
	if (!tree) {
		tree = node;
	}
	else if (node->key == tree->key && policy != POLICY_INSERT) {
#ifdef POLICY_OVERRIDE
		if (policy == POLICY_OVERRIDE) {
			//TODO: figure out what happens with tree->item. Should it be freed? (no)
			tree->item = node->item;
		}
#endif
		*modified = 0;
	}
	else if (node->key < tree->key) {
		_rbtree_update_child(tree, _rbtree_bst_insert(tree->lchild, node, policy, modified), LEFT);
	}
	else {
		_rbtree_update_child(tree, _rbtree_bst_insert(tree->rchild, node, policy, modified), RIGHT);
	}

	return tree;
}

rbtree* rbtree_alloc()
{
	rbtree *node = (rbtree *) malloc(sizeof(rbtree));
	memset(node, 0, sizeof(rbtree));
	return node;
}

void rbtree_free(rbtree *tree)
{
	if (tree->lchild)
		rbtree_free(tree->lchild);
	else if (tree->rchild)
		rbtree_free(tree->rchild);
	free(tree->item);
	free(tree);
}

rbtree* _rbtree_gp(rbtree *node)
{
	if (!node->parent)
		return NULL;
	else 
		return node->parent->parent;
}

rbtree* _rbtree_s(rbtree *node, int *side)
{
	rbtree *ret = NULL;
	if (node->parent) {
		if (node == node->parent->lchild){
			*side = LEFT;
			ret = node->parent->rchild;
		}
		else {
			*side = RIGHT;
			ret = node->parent->lchild;
		}
	}
	return ret;
}

rbtree* _rbtree_u(rbtree *node)
{
	rbtree *ret = _rbtree_gp(node);
	if (ret) {
		if (node->parent == ret->lchild)
			ret = ret->rchild;
		else
			ret = ret->lchild;
	}
	return ret;
}

rbtree* _rbtree_rotate_left(rbtree *tree)
{
	rbtree *old_root = tree->parent;

	assert(old_root && tree == old_root->rchild);

	_rbtree_update_child(old_root, tree->lchild, RIGHT);
	if (old_root->parent) {
		int side = (old_root == old_root->parent->lchild) ? LEFT : RIGHT;
		_rbtree_update_child(old_root->parent, tree, side);
	}
	else { /* Old root has not parent -> It's the root */
		tree->parent = NULL;
	}
	_rbtree_update_child(tree, old_root, LEFT);

	return tree;
}

rbtree* _rbtree_rotate_right(rbtree *tree)
{

	rbtree *old_root = tree->parent;

	assert(old_root && tree == old_root->lchild);

	if (old_root->parent) {
		int side = (old_root == old_root->parent->lchild) ? LEFT : RIGHT;
		_rbtree_update_child(old_root->parent, tree, side);
	}
	else { /* Old root has not parent -> It's the root */
		tree->parent = NULL;
	}
	_rbtree_update_child(old_root, tree->rchild, LEFT);
	_rbtree_update_child(tree, old_root, RIGHT);

	return tree;
}


rbtree* rbtree_insert(rbtree *tree, void *item, size_t key)
{
	return rbtree_insert_policy(tree, item, key, DEFAULT_POLICY);
}

rbtree* rbtree_insert_policy(rbtree *tree, void *item, size_t key, int policy)
{
	rbtree *node = rbtree_alloc();
	node->key = key;
	node->item = item;
	node->color = COLOR_RED;

	int modified;

	tree = _rbtree_bst_insert(tree, node, policy, &modified);

	if (!modified) {
		free(node);
	}
	else {
		while (node->parent && node->parent->color == COLOR_RED) {
			/* TODO: Put breakpoint here and debug for x[i]=8 */
			rbtree *u = _rbtree_u(node), *gp = _rbtree_gp(node), *p = node->parent;
			if(u && u->color==COLOR_RED) {//parent and uncle are red-> switch them to black
				node->parent->color = COLOR_BLACK;
				u->color = COLOR_BLACK;
				gp->color = COLOR_RED;
				node = gp;
			}
			else {
				if(p==gp->lchild) {
					if (node==p->rchild) {
						_rbtree_rotate_left(node);	
						node = node->lchild;
						p = node->parent;
					}
					_rbtree_rotate_right(p);
					node->color = COLOR_RED;
					p->color = COLOR_BLACK;
					p->rchild->color = COLOR_RED;
				}
				else { /* p==gp->lchild */
					if(node==p->lchild) {
						_rbtree_rotate_right(node);
						node = node->rchild;
						p = node->parent;
					}
					_rbtree_rotate_left(p);
					node->color = COLOR_RED;
					p->color = COLOR_BLACK;
					p->lchild->color = COLOR_RED;
				}
			}
		}

		while (tree->parent)
			tree = tree->parent;

		tree->color = COLOR_BLACK; //Root must be black
	}
	return tree;
}

void _rbtree_flatten(rbtree *x, void **item_buffer, size_t *key_buffer, size_t *count)
{
	if (x) {
		_rbtree_flatten(x->lchild, item_buffer, key_buffer, count);
		key_buffer[*count] = x->key;
		item_buffer[(*count)++] = x->item;
		_rbtree_flatten(x->rchild, item_buffer, key_buffer, count);
	}
}

size_t rbtree_flatten(rbtree *x, void **item_buffer, size_t *key_buffer)
{
	size_t count = 0;
	_rbtree_flatten(x, item_buffer, key_buffer, &count);
	return count;
}

void rbtree_flatprint(rbtree *x)
{
	if(x) {
		rbtree_flatprint(x->lchild);
		printf("%zu ", x->key);
		rbtree_flatprint(x->rchild);
	}
}
