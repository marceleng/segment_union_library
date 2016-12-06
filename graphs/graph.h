#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "matrice.h"

struct node {
	double x;
	double y;

	size_t number_of_neigh;
	size_t max_number_of_neigh;
	struct node **neigh;

	size_t id;

	int flag;
};

typedef struct node node;

node* node_alloc();
void node_free(node *n);

void node_urandom(node *n, double size_x, double size_y);
node* node_alloc_urandom(double size_x, double size_y);

double node_distance(node *x, node *y);

#endif
