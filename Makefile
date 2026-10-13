PREFIX ?= /usr/local
WLR_PROTOCOLS ?= $(shell pkg-config --variable=pkgdatadir wlr-protocols)
PROTO = $(WLR_PROTOCOLS)/unstable/wlr-virtual-pointer-unstable-v1.xml
CFLAGS ?= -O2 -Wall
CFLAGS += $(shell pkg-config --cflags wayland-client) -I.
LDLIBS += $(shell pkg-config --libs wayland-client)

GEN = wlr-virtual-pointer-unstable-v1-client-protocol.h wlr-virtual-pointer-unstable-v1.c

unstick: unstick.c $(GEN)
	$(CC) $(CFLAGS) -o $@ unstick.c wlr-virtual-pointer-unstable-v1.c $(LDFLAGS) $(LDLIBS)

wlr-virtual-pointer-unstable-v1-client-protocol.h: $(PROTO)
	wayland-scanner client-header $< $@

wlr-virtual-pointer-unstable-v1.c: $(PROTO)
	wayland-scanner private-code $< $@

install: unstick
	install -Dm755 unstick $(DESTDIR)$(PREFIX)/bin/sway-unstick

clean:
	rm -f unstick $(GEN)

.PHONY: install clean
