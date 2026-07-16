# CLI: `--instances` + `--mp-layout` local multiplayer

## Summary

```text
melonDS --instances N [--mp-layout auto|none] path\to\game.nds
```

| Flag | Meaning |
|------|---------|
| `-n` / `--instances <1..16>` | In-process LocalMP instances (vanilla `.sav` / `.sav.2` / …) |
| `--mp-layout none` | Default — spawn only |
| `--mp-layout auto` | After spawn: **2** → side-by-side + dual screen + lock; **4+** → 2×2 + top only + lock |

## Menu (File → Multiplayer)

- Arrange side by side / Arrange 2×2
- Dual screen (all instances) / Top only (all instances)
- Lock window positions (group drag + group resize)
- **Linked input** — mirror P1's controller to all instances (see below)
- **System → Reset all** resets every active local-MP instance (Reset stays per-window)

While locked, resizing one primary window applies the same client size to all peers and snaps the panel to content aspect (Top only 4:3 / Dual Even ~2:3) so screens fill without black bars. Integer scaling is forced off for the group. Group panel size (`WindowGroup.PanelWidth/Height`) persists across launches.

Saves: instance 0 → `rom.sav`; instance 1 → `rom.sav.2`; instance 2 → `rom.sav.3`; instance 3 → `rom.sav.4`. Missing instance saves start blank (not seeded from P1).

`--mp-layout auto` also enables **View → Hide menu bar** (menubar collapsed like fullscreen; OS title bar remains). Right-click the screen: Pause/Reset/Stop, Linked input, cascading File/System/View/Config/Help (same `QAction`s), and Hide menu bar at the bottom to restore the bar. Closing any primary instance window closes all of them.

## Linked input

Toggle via **right-click → Linked input** or **File → Multiplayer → Linked input** (visible when 2+ instances are running).

When **ON**, all instances share **instance 0 (P1)'s key bindings**:

- Keyboard press/release on any window is evaluated against P1's mappings; the resulting button bits are applied identically to every instance.
- Joystick input from P1's `inputProcess` is copied to all peers each frame.
- Touch screen taps/drags/releases are mirrored to all peers at the same coordinates.
- Instances stay in sync — same buttons pressed on the same frames.

When **OFF**, each instance uses its own independent keybinds as normal.

OSD confirms "Linked input ON" / "Linked input OFF" on toggle.

## Constraints

- LocalMP is **same-process only** — never two `melonDS.exe` for local wireless
- `--instances > 1` and `--mp-layout auto` require an NDS ROM
- Top only disables touch until Dual screen is restored
- Linked input shares P1 keybinds temporarily; per-instance binds are not overwritten

## Source

- `src/frontend/qt_sdl/CLI.*`, `main.cpp`, `WindowGroup.*`, `Window.cpp`, `EmuInstanceInput.cpp`

## Scripts

`C:\~Coding\Emulation\~Emulators\AI-MELONDS-LAUNCH.md`
