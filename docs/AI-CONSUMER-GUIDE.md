# AI consumer guide — using this melonDS for project testing

This melonDS tree exists to support **other** NDS projects' single-player and local-multiplayer debug loops.

## Absolute paths

| Role | Path |
|------|------|
| Source | `C:\~Coding\Emulation\Nintendo DS\melonDS\` |
| Deployed exe + scripts | `C:\~Coding\Emulation\~Emulators\` |
| Agent launch sheet | `C:\~Coding\Emulation\~Emulators\AI-MELONDS-LAUNCH.md` |
| Enhancement note | `C:\~Coding\Emulation\~Emulators\ENHANCEMENT-CLI-INSTANCES.md` |
| Example consumer | `C:\~Coding\Emulation\Nintendo DS\pokeplatinum-MP\` |

## Do / don't

| Do | Don't |
|----|--------|
| `melonDS --instances 2 --mp-layout auto rom.nds` | Start two `melonDS.exe` for local MP |
| Console tee scripts for `[GAME_LOG]` | Assume GUI launch writes the log |
| Reuse `.sav` / `.sav.2` / … (missing → blank) | Expect P2+ to clone P1's `.sav` |
| Right-click → Linked input for synced control | Expect instances to share keybinds by default |

## Auto layout (`--mp-layout auto`)

| N | Arrange | Sizing | Lock |
|---|---------|--------|------|
| 2 | Side by side | Dual screen (Even) | On |
| 4+ | 2×2 | Top only | On |

Override: File → Multiplayer. Top only disables touch until Dual screen is restored.

## Linked input

Right-click any instance → **Linked input** (or File → Multiplayer → Linked input). When ON, all instances use P1's keybinds and receive identical button/touch state each frame. Useful for walking all players in lockstep. Toggle off to resume independent control.

## Launch matrix (scripts in `~Emulators`)

| Goal | Script |
|------|--------|
| SP console + log | `launch-pokeplatinum-console.ps1` |
| 2x MP (+ auto layout) | `launch-pokeplatinum-dual-console.ps1` |
| 4x MP (+ auto layout) | `launch-pokeplatinum-quad-console.ps1` |
| N instances | `launch-pokeplatinum-console.ps1 -Players N` |
| GDB | `launch-pokeplatinum-gdb.ps1` (SP only) |

```text
melonDS.exe --instances N --mp-layout auto "C:\path\to\rom.nds"
```

## Build and deploy

```powershell
cd "C:\~Coding\Emulation\Nintendo DS\melonDS"
ninja -C build melonDS
Copy-Item build\melonDS.exe "C:\~Coding\Emulation\~Emulators\melonDS.exe" -Force
```

Always deploy to `melonDS.exe` — never a `-new` or other suffix. Launch scripts reference `melonDS.exe`.

## pokeplatinum-MP integration

| Artifact | Path |
|----------|------|
| Agent launcher | `scripts\launch-debug-emulator.ps1` (`-Dual` / `-Quad`) |
| Agent doc | `docs\single-screen\AI-RULES\MELONDS.md` |
| Skill | `.cursor\skills\mp-emulator-debug\SKILL.md` (on `ai-docs`) |

AssertFresh remains mandatory after console launch.

See [CLI-INSTANCES.md](CLI-INSTANCES.md).
