#ifndef _BST_H_
#define _BST_H_

#include <stdlib.h>

//Inserts an additional item with same key
#define POLICY_INSERT 0
// Override any previous item with the same key if it exists
//#define POLICY_OVERRIDE 1
// Does not insert if item with same key found
#define POLICY_IGNORE -1

typedef unsigned int key_t;

struct rbtree {
	void *item;
	key_t key;

	int color;

	struct rbtree *lchild;
	struct rbtree *rchild;
	struct rbtree *parent;
};

typedef struct rbtree rbtree;

rbtree* rbtree_alloc();
void rbtree_free(rbtree* tree);

void rbtree_flatprint(rbtree* tree);
size_t rbtree_flatten(rbtree *x, void **item_buffer, key_t *key_buffer);

rbtree* rbtree_insert(rbtree *tree, void *item, key_t key);
rbtree* rbtree_insert_policy(rbtree *tree, void *item, key_t key, int policy);
int rbtree_verify(rbtree *tree);
//TODO: rbtree* *rbtree_remove(rbtree *tree, key_t key, void* item);
void *rbtree_search(rbtree *tree, key_t key);

rbtree* rbtree_remove(rbtree *tree, key_t key, void **item);

key_t rbtree_pop(rbtree *tree, void** item);

size_t rbtree_size(rbtree *tree);

int rbtree_get_default_policy();
void rbtree_set_default_policy(int policy);
#endif
