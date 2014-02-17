/*
 * BELUGA, a wayland tiling window manager based on the...
 * SIMPLE WAYLAND COMPOSITER, by the venerable...
 * MICHAEL FORTNEY, mfortney.org and designed by...
 * NATHAN MCCLOSKEY, npgm.org
 *
 */
#include "cont.h"

char *terminal_command[] = {"/home/nathan/clones/dmenu/cust", NULL};
char *dwb[] = {"/home/nathan/clones/st/st", NULL};
static bl_manager_t *manager;


void
bl_commit_focus(void *data) {
	fprintf(stderr, "Commititng focus\n");
	swc_window_focus(manager->focus_window ? manager->focus_window->swc : NULL);
	manager->focus_source = NULL;
}

void
bl_focus(bl_client_t *window) {
	if (window == NULL) {
		fprintf(stderr, "Attempt to focus on Null rejected\n");
		return;
	}
	fprintf(stderr, "Focusing\n");
	if (!manager->focus_source)
		manager->focus_source = wl_event_loop_add_idle(manager->event_loop, &bl_commit_focus, NULL);
	manager->last_window = manager->focus_window;
	manager->focus_window = window;
}
	

void
bl_draw_help(bl_container_t *root, struct swc_rectangle *geom) {
	if (root == NULL) return;
	if (root->type == BL_NODE_CONTAINER) {
		fprintf(stderr, "N, ");
		struct swc_rectangle a, b;
		blt_split_rect(root->split, geom, &a, &b);
		bl_draw_help(root->achild, &a);
		bl_draw_help(root->bchild, &b);
	} else if (root->client != NULL) {
		fprintf(stderr, "C, ");
		swc_window_set_geometry(root->client->swc, geom);
		swc_window_show(root->client->swc);
	}
}

void 
bl_draw() {
	struct swc_rectangle *screen_geometry = &manager->active_screen->swc->usable_geometry;
	fprintf(stderr, "Starting recursion! ");
	bl_draw_help(manager->active_screen->tree, screen_geometry);
	fprintf(stderr, "\nDone with recursion!\n");
	bl_focus(manager->focus_cont->client);
}
void 
bl_split_vertical() {
	blt_split(manager->focus_cont, NULL, BL_SPLIT_VERTICAL);
	manager->focus_cont = manager->focus_cont->bchild;
	bl_draw();
}

void
bl_window_push(bl_client_t *window) {
	fprintf(stderr, "Pushing window\n");
	manager->focus_cont->client = window;
	bl_draw();
}

bl_client_t*
bl_prev(struct wl_list *list, struct wl_list *elem) {
		struct wl_list *ret;
		ret = (elem == list->next) ? list->next : elem->next;
		bl_client_t *window = wl_container_of(ret, window, link);
		return window;
}
bl_client_t*
bl_next(struct wl_list *list, struct wl_list *elem) {
		struct wl_list *ret;
		ret = (elem == list->prev) ? list->next : elem->next;
		bl_client_t *window = wl_container_of(ret, window, link);
		return window;
}

void
bl_window_next() {
	fprintf(stderr, "bl_window_next\n");
	if (manager->focus_window) {
		bl_client_t *window;
		swc_window_hide(manager->focus_window->swc);
		window = bl_next(&manager->active_screen->windows, &manager->focus_window->link);

		bl_window_push(window);
	} else if (manager->active_screen->num_windows > 0) {
		bl_client_t *window = wl_container_of(manager->active_screen->windows.next, window, link);
		fprintf(stderr, "Focusing next window after kill: %s\n", window->swc->title);
		bl_window_push(window);
	}
	fprintf(stderr, "bl_window_next out\n");
}
	

void
bl_window_kill(bl_client_t *window) {
	fprintf(stderr, "killing window: %s\n", window->swc->title);
	if (manager->focus_window == window) { 
		if (bl_next(&manager->active_screen->windows, &window->link) != window) {
			bl_window_next();
		} else 
			manager->focus_window = NULL;
			manager->focus_cont->client = NULL;
	}

	--(window->screen->num_windows);
	wl_list_remove(&window->link);	
	fprintf(stderr, "%d windows\n", window->screen->num_windows);
	window->screen = NULL;
	swc_window_hide(window->swc);
	free(window);
}

void
bl_window_unmap(bl_client_t *window) {
	fprintf(stderr, "unmapping window: %s\n", window->swc->title);
	--window->screen->num_windows;
	fprintf(stderr, "%d windows\n", window->screen->num_windows);
	swc_window_hide(window->swc);
	wl_list_remove(&window->link);
	wl_list_insert(&manager->unmapped_windows, &window->link);
}

void
bl_window_map(bl_client_t *window) {
	fprintf(stderr, "Mapping %s!\n", window->swc->title);
	window->screen = manager->active_screen;
	++window->screen->num_windows;
	fprintf(stderr, "%d windows\n", window->screen->num_windows);
	wl_list_insert(&window->screen->windows, &window->link);
	bl_window_push(window);
}

static void spawn(uint32_t time, uint32_t value, void *data) {
		char * const * command = data;
		if(fork() == 0) {
				fprintf(stderr, "Execing command, %s", command[0]);
				execvp(command[0], command);
				exit(EXIT_FAILURE);
		}
}
/* locally set keybindings for swc */
void
bl_set_keybindings() {
	//void set keybindings
	swc_add_key_binding(SWC_MOD_LOGO, XKB_KEY_Return, &spawn, terminal_command);
	swc_add_key_binding(SWC_MOD_LOGO, XKB_KEY_d, &spawn, dwb);
	swc_add_key_binding(SWC_MOD_LOGO, XKB_KEY_n, &bl_window_next,NULL);
	swc_add_key_binding(SWC_MOD_LOGO, XKB_KEY_v, &bl_split_vertical,NULL);
}

static void
bl_window_event(struct wl_listener *listener, void *data) {
	struct swc_event *event = data;
	bl_client_t *window = NULL;
	window = wl_container_of(listener, window, event_listener);
	fprintf(stderr, "Window Event On: %s\n", window->swc->title);

	switch(event->type) {
		/* remove from data structure */
		case SWC_WINDOW_DESTROYED:
			bl_window_kill(window);
			break;

		/* monitor for switch to top level so that we map a new window */
		case SWC_WINDOW_STATE_CHANGED:
			fprintf(stderr, "StateChange: \n");
			if (window->swc->state == SWC_WINDOW_STATE_TOPLEVEL) 
				bl_window_map(window);
			else
				bl_window_unmap(window);
			break;

		/* We don't care to monitor these */
		case SWC_WINDOW_CLASS_CHANGED:
		case SWC_WINDOW_TITLE_CHANGED:
		case SWC_WINDOW_ENTERED:
		case SWC_WINDOW_RESIZED:
			break;
	}
}


static void
bl_screen_event(struct wl_listener *listener, void *data) {
	struct swc_event *event = data;
	bl_screen_t *screen = NULL;
	screen = wl_container_of(listener, screen, event_listener);

	switch(event->type) {
		case SWC_SCREEN_GEOMETRY_CHANGED:
		case SWC_SCREEN_USABLE_GEOMETRY_CHANGED:
			//bl_map_window(screen);
			break;
		case SWC_SCREEN_DESTROYED:
			//bl_kill_screen(screen);
			break;
	}
}

/* new screen handler, passed to swc through the swc_manager */
static void 
bl_new_screen(struct swc_screen *swc) {
	//add new screen 
	
	bl_screen_t *screen;
	screen = malloc(sizeof(*screen));
	if (!screen)
		return;
	screen->tree = blt_init_root();
	screen->swc = swc;
	screen->event_listener.notify = &bl_screen_event;
	screen->num_windows = 0;
	wl_list_init(&screen->windows);

	wl_signal_add(&swc->event_signal, &screen->event_listener);
	manager->active_screen = screen;
	manager->focus_cont = screen->tree;
}

/* new window handler, passed to swc through the swc_manager */
static void 
bl_new_window(struct swc_window *swc) {
	//add new window
	bl_client_t *window;
	window = malloc(sizeof(*window));

	if(!window) return;
	fprintf(stderr, "New Window: %s\n", swc->title);

	window->swc = swc;
	window->event_listener.notify = &bl_window_event;
	window->screen = NULL;

	wl_signal_add(&swc->event_signal, &window->event_listener);
}

/* pointers to the main swc event handlers */
const struct swc_manager callbacks =  {
	&bl_new_window,
	&bl_new_screen
};

void
bl_init() {
	manager = malloc(sizeof(*manager));
}

int
main(int argc, char **argv) {
	/* core global object */
	bl_init();
	if (!(manager->display = wl_display_create())) return EXIT_FAILURE;
		fprintf(stderr, "INIT1\n");

	if (wl_display_add_socket(manager->display, NULL) != 0) return EXIT_FAILURE;
	if (wl_display_init_shm(manager->display) != 0) return EXIT_FAILURE;
		fprintf(stderr, "INIT2\n");

	/* initialize swc with the swc_manager */
	if (!swc_initialize(manager->display, NULL, &callbacks)) return EXIT_FAILURE;

	/* locally set up keybindings */
	bl_set_keybindings();

	manager->event_loop = wl_display_get_event_loop(manager->display);
	wl_display_run(manager->display);
	return EXIT_SUCCESS;
}

