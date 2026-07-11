# AI consumer guide — using this melonDS for project testing

This melonDS tree exists to support **other** NDS projects’ single-player and local-multiplayer debug loops.

## Absolute paths

| Role | Path |
|------|------|
| Source | `C:\~Coding\Emulation\Nintendo DS\melonDS\` |
| Deployed exe + scripts | `C:\~Coding\Emulation\~Emulators\` |
| Agent launch sheet | `C:\~Coding\Emulation\~Emulators\AI-MELONDS-LAUNCH.md` |
| Enhancement note | `C:\~Coding\Emulation\~Emulators\ENHANCEMENT-CLI-INSTANCES.md` |
| Example consumer | `C:\~Coding\Emulation\Nintendo DS\pokeplatinum-MP\` |

## Do / don’t

| Do | Don’t |
|----|--------|
| `melonDS --instances 2 rom.nds` (one process) | Start two `melonDS.exe` for local MP |
| Console tee scripts for `[GAME_LOG]` | Assume GUI launch writes the log |
| Reuse `.sav` / `.sav.2` from prior dual sessions | Invent a shared-save CLI |

## Launch matrix (scripts in `~Emulators`)

| Goal | Script |
|------|--------|
| SP console + log | `launch-pokeplatinum-console.ps1` |
| 2x MP console + log | `launch-pokeplatinum-dual-console.ps1` |
| 4x MP console + log | `launch-pokeplatinum-quad-console.ps1` |
| N instances | `launch-pokeplatinum-console.ps1 -Players N` (1..16) |
| SP/N GUI | `launch-pokeplatinum.ps1 -Players N` |
| GDB | `launch-pokeplatinum-gdb.ps1` (SP only) |

Generic CLI (any ROM):

```text
melonDS.exe --instances N "C:\path\to\rom.nds"
```

Scripts prepend `C:\msys64\mingw64\bin` for dynamic Qt/SDL deps.

## pokeplatinum-MP integration

| Artifact | Path |
|----------|------|
| Agent launcher | `pokeplatinum-MP\scripts\launch-debug-emulator.ps1` (`-Dual`, `-Quad`, `-Players`) |
| Skill | `.cursor\skills\mp-emulator-debug\SKILL.md` |
| Rules | `.cursor\rules\nds-emulator-verify.mdc`, `nds-emulator-logging.mdc` |
| MP launch doc | `docs\mp\MELONDS-DUAL-LAUNCH.md` |
| Logging | `docs\logging.md` |

AssertFresh remains mandatory after console launch:

```powershell
.\scripts\read-game-log.ps1 -AssertFresh
```

## Rebuild / deploy

```bash
# MSYS2 mingw64
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
# copy build/melonDS.exe → ~Emulators\melonDS.exe
# vanilla rollback: ~Emulators\melonDS-vanilla.exe
```

See also [CLI-INSTANCES.md](CLI-INSTANCES.md).
