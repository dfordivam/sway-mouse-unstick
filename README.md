# sway-unstick

Fixes sway when **the pointer moves but mouse clicks do nothing** after a
virtual input tool (lan-mouse, input-leap, wayvnc, ...) stopped running.

## Symptom

- Pointer moves; keyboard works.
- Clicks do nothing, on every output, with every physical mouse.
- Wayland clients never get a `wl_pointer.enter` (check with `wev`).
- `swaymsg seat seat0 cursor release button1` does not help.

## Cause

sway counts held buttons in `cursor->pressed_button_count`. The count
changes only when a real input device sends a button event
(`handle_pointer_button`, `sway/input/cursor.c`). While the count is above 0,
sway stays in its "button held" mode (`seatop_down`). In that mode the
pointer stays tied to the surface that got the original press, and nothing
else receives pointer events.

When a virtual pointer is destroyed while a button is held (for example,
lan-mouse is stopped mid-click), the release never arrives and the count
stays at 1 or more.

Why the obvious fixes don't work:

| Attempt | Why it fails |
|---|---|
| `swaymsg seat … cursor release` | calls `dispatch_cursor_button` directly and never touches the count |
| `ydotool click 0x80` (release only) | libinput drops a release from a device that never sent the matching press |
| click (press + release) on any device | adds 1, then removes 1: no net change |

## Fix

`sway-unstick` creates a `zwlr_virtual_pointer_v1` device. That protocol
doesn't go through libinput. It sends only left-button *release* events, and
each one lowers the count by 1. Once the count is 0, sway goes back to its
normal pointer mode. Extra releases are harmless: sway just logs
`Pressed button count was wrong`.

## Usage

```sh
nix run path:.            # send 5 releases (default)
nix run path:. -- 10      # send 10
```

Or build it:

```sh
nix build path:.          # ./result/bin/sway-unstick
nix develop path:. -c make && ./unstick
```

Without nix, you need `wayland-client`, `wayland-scanner`, `wlr-protocols`
and `pkg-config`, then run `make && sudo make install`.

### Checking that it worked

sway logs one `Pressed button count was wrong` line for each release beyond
what was needed. If you sent N releases and see N−k such lines, the count
was stuck at k:

```sh
journalctl --user --since -1min | grep -c 'Pressed button count was wrong'
```

## Requirements

- A wlroots compositor that offers `zwlr_virtual_pointer_manager_v1`
  (sway does).
- `WAYLAND_DISPLAY` set to the affected session.

Tested on sway 1.12.
