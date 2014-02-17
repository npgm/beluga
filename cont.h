#include <stdlib.h>
#include <stdio.h>
#include <swc.h>
#include <unistd.h>
#include <wayland-server.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#ifndef CONT_H
#define CONT_H



struct bl_container_t;
typedef enum bl_split_t{
		BL_SPLIT_HORIZONTAL, 
		BL_SPLIT_VERTICAL,
		BL_SPLIT_NONE
} bl_split_t;

typedef enum {
		BL_NODE_CONTAINER, 
		BL_CLIENT_CONTAINER
} bl_container_form_t;

typedef struct { /* screen has a list of windows */
		struct swc_screen *swc;
		struct wl_listener event_listener;
		struct wl_list windows;
		unsigned num_windows;
		struct bl_container_t *tree;
} bl_screen_t;

typedef struct { /* client has a link in the list */
		struct swc_window *swc;
		struct wl_listener event_listener;
		bl_screen_t *screen;
		struct wl_list link;
} bl_client_t;

typedef struct bl_container_t{
		bl_client_t *client; 

		bl_container_form_t type;
		bl_split_t split;
		int isroot;

		struct bl_container_t *parent;
		struct bl_container_t *achild;
		struct bl_container_t *bchild;
} bl_container_t;


typedef struct { /* manager has a list of unmapped windows */
	struct wl_list unmapped_windows;
	bl_client_t *last_window; /* last_window */
	bl_client_t *focus_window; /* currently focused window */
	bl_container_t *focus_cont; /* currently focused window */
	bl_screen_t *active_screen; /* active screen */
	struct wl_event_source_t *focus_source; 
	struct wl_event_loop *event_loop;
	struct wl_display *display; 
} bl_manager_t;


bl_container_t* blt_init_root(void);
int blt_split(bl_container_t *current, bl_client_t *new, bl_split_t split);
void blt_split_rect(bl_split_t split, struct swc_rectangle *geom, struct swc_rectangle *a, struct swc_rectangle *b);
#endif //CONT_H
