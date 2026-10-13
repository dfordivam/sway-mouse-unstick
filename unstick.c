// sway-unstick: send release-only button events through a
// wlr-virtual-pointer device to reset sway's pressed_button_count after a
// virtual pointer (e.g. lan-mouse) vanished while a button was held.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include "wlr-virtual-pointer-unstable-v1-client-protocol.h"

#define BTN_LEFT 0x110

static struct wl_seat *seat;
static struct zwlr_virtual_pointer_manager_v1 *mgr;

static void reg_global(void *d, struct wl_registry *r, uint32_t name,
		const char *iface, uint32_t ver) {
	if (!strcmp(iface, wl_seat_interface.name) && !seat)
		seat = wl_registry_bind(r, name, &wl_seat_interface, 1);
	else if (!strcmp(iface, zwlr_virtual_pointer_manager_v1_interface.name))
		mgr = wl_registry_bind(r, name,
			&zwlr_virtual_pointer_manager_v1_interface, 1);
}
static void reg_remove(void *d, struct wl_registry *r, uint32_t n) {}
static const struct wl_registry_listener reg_l = { reg_global, reg_remove };

int main(int argc, char **argv) {
	int n = 5;
	if (argc > 2 || (argc == 2 && (n = atoi(argv[1])) <= 0)) {
		fprintf(stderr, "usage: %s [count]\n"
			"  send <count> (default 5) left-button releases\n", argv[0]);
		return 2;
	}

	struct wl_display *dpy = wl_display_connect(NULL);
	if (!dpy) {
		fprintf(stderr, "cannot connect to wayland display\n");
		return 1;
	}
	struct wl_registry *reg = wl_display_get_registry(dpy);
	wl_registry_add_listener(reg, &reg_l, NULL);
	wl_display_roundtrip(dpy);
	if (!mgr) {
		fprintf(stderr, "compositor lacks zwlr_virtual_pointer_manager_v1\n");
		return 1;
	}

	struct zwlr_virtual_pointer_v1 *vp =
		zwlr_virtual_pointer_manager_v1_create_virtual_pointer(mgr, seat);
	wl_display_roundtrip(dpy);
	for (int i = 0; i < n; i++) {
		// state 0 = WL_POINTER_BUTTON_STATE_RELEASED
		zwlr_virtual_pointer_v1_button(vp, 0, BTN_LEFT, 0);
		zwlr_virtual_pointer_v1_frame(vp);
	}
	wl_display_roundtrip(dpy);
	zwlr_virtual_pointer_v1_destroy(vp);
	wl_display_roundtrip(dpy);
	wl_display_disconnect(dpy);
	printf("sent %d release(s)\n", n);
	return 0;
}
