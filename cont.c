#include "cont.h"

bl_container_t*
blt_init_root() {
	fprintf(stderr, "initializing root container\n");
	bl_container_t *root = calloc(1, sizeof(bl_container_t));
	root->type = BL_CLIENT_CONTAINER;
	root->split = BL_SPLIT_NONE;
	root->isroot = 1;
	return root;
}

bl_container_t*
blt_init_client(bl_client_t *client) {
	fprintf(stderr, "initializing client container\n");
	bl_container_t *cont = calloc(1, sizeof(bl_container_t));
	cont->client = client;
	cont->type = BL_CLIENT_CONTAINER;
	cont->split = BL_SPLIT_NONE;
	cont->isroot = 0;
	return cont;
}

bl_container_t*
blt_init_node(bl_split_t split) {
	fprintf(stderr, "initializing node container\n");
	bl_container_t *node = calloc(1, sizeof(bl_container_t));
	node->type = BL_NODE_CONTAINER;
	node->split = split;
	node->isroot = 0;
	return node;
}


int
blt_split(bl_container_t *node, bl_client_t *new, bl_split_t split) {
	if (node->type == BL_NODE_CONTAINER) {
		fprintf(stderr, "attempt to split on node");
		return 1;
	}
	/* maintain current node;
	bl_container_t *node = blt_init_node(split);
	node->achild = current;
	node->parent = current->parent;
	node->bchild = blt_init_client(new);

	node->bchild->parent = node;
	node->achild->parent = node;
	*/

	node->achild = blt_init_client(node->client);
	node->bchild = blt_init_client(new);
	node->split = split;
	node->type = BL_NODE_CONTAINER;
	node->bchild->parent = node;
	node->achild->parent = node;

	return 0;
}

void
blt_split_rect(bl_split_t split, struct swc_rectangle *geom, struct swc_rectangle *a, struct swc_rectangle *b) {
	if (split == BL_SPLIT_NONE) {
		fprintf(stderr, "Attempting to split rectangle for no split\n");
		return;
	} else if (split == BL_SPLIT_VERTICAL) {
		a->x = geom->x;
		a->y = geom->y;
		a->width = geom->width/2;
		a->height = geom->height;
		b->x = geom->x + geom->width/2;
		b->y = geom->y;
		b->width = geom->width/2;
		b->height = geom->height;
	} else {
		a->x = geom->x;
		a->y = geom->y;
		a->width = geom->width;
		a->height = geom->height/2;
		b->x = geom->x;
		b->y = geom->y + geom->height/2;
		b->width = geom->width;
		b->height = geom->height/2;
	}
}

bl_container_t*
blt_next_sibling(bl_container_t *root) {
	return ((root->parent->bchild != root) ? root->parent->bchild : NULL);
}

bl_container_t*
blt_next_container(bl_container_t *current) {
	fprintf(stderr, "Traversing tree...\n");
	bl_container_t* ct = current;
	while (ct != NULL) {
		if (ct->type == BL_NODE_CONTAINER) {
			fprintf(stderr, "N\n");
			ct = ct->achild;
			if (ct->type == BL_CLIENT_CONTAINER) 
				break;
		} else {
			fprintf(stderr, "C\n");
			while (ct->isroot != 1 && blt_next_sibling(ct) == NULL) {
				ct = ct->parent;
				fprintf(stderr, "Step up parent\n");
			}
			if (ct->isroot)
				continue;
			ct = blt_next_sibling(ct);
			if (ct->type == BL_CLIENT_CONTAINER)
				break;
		}
	}
	fprintf(stderr, "Done\n");
	return ct;
}

				

