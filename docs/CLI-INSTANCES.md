# CLI: `--instances` local multiplayer

## Summary

`-n` / `--instances <count>` starts **1..16** in-process emu instances (same as File → Multiplayer → Launch new instance), auto-loads the same NDS/GBA ROMs on each, and uses **vanilla** save suffixes (`.sav`, `.sav.2`, …).

```text
melonDS --instances 2 path\to\game.nds
```

## Constraints

- LocalMP is **same-process only**. Do not spawn multiple `melonDS.exe` for local wireless.
- `--instances > 1` requires an NDS ROM argument.
- Max is `kMaxEmuInstances` (16).

## Saves

Unchanged `EmuInstance::instanceFileSuffix()`:

| ID | Suffix |
|----|--------|
| 0 | _(none)_ |
| 1 | `.2` |
| 2 | `.3` |
| … | `.(id+1)` |

## Source

- `src/frontend/qt_sdl/CLI.h`, `CLI.cpp`
- `src/frontend/qt_sdl/main.cpp` (post-preload spawn loop)

## Scripts

See `C:\~Coding\Emulation\~Emulators\AI-MELONDS-LAUNCH.md` and [AI-CONSUMER-GUIDE.md](AI-CONSUMER-GUIDE.md).
