# JP (BZMJ) version support on the PC port

*Status (2026-06-13): **booting and core-correct.** A JP build runs against the
retail JP ROM with all data tables resolving correctly — verified by boot test.
Remaining for full speedrun parity: Japanese text rendering and JP script-address
tables (see "Remaining gaps").*

The Minish Cap speedrun scene runs the **Japanese** version (RNG manipulations are
version-exclusive and were authored for JP). The decomp supports JP at the source
level (`#ifdef JP` throughout `src/`); this port now wires JP through the build and
runtime, and supplies the JP ROM data-table offsets.

## Build & run a JP port

1. **Provide a legal JP baserom** (`BZMJ`, SHA-1 `6c5404a1effb17f481f352181d0f1c61a2765c5d`,
   see `tmc_jp.sha1`) as `baserom_jp.gba` in the repo root. (Gitignored — never committed.)
2. **Build:** `python build.py --jp` (or `xmake f --game_version=JP -y && xmake build -y tmc_pc`).
3. **Run:** point the binary at the JP ROM. It auto-detects `JP (BZMJ)` and selects the
   JP offset table. Verified boot log:
   ```
   ROM region detected: JP (BZMJ)
   Using offsets for JP (game code: BZMJ)
   gMapData loaded (13482224 bytes from ROM offset 0x324710).
   Area data tables loaded (0x90 areas, 2-level pointers resolved).
   Entering AgbMain...
   ```

## What's wired (all committed, USA build unaffected)

- `ROM_REGION_JP`, `kRomOffsets_JP` (real values), `BZMJ` detection, JP/USA offset
  selection, JP-aware region messages — `port/port_config.h`, `port/port_rom.c`
- `port/port_offset_JP.h` (asset-blob offsets) + `#if defined(JP)` include — `port/port_main.c`
- `JP`/`JAPANESE` in `pc_versions` — `xmake.lua`; `--jp` + JP version entry — `build.py`
- JP-only symbol guards so the JP port links (`sub_0807FC24`, `sub_08088658` are
  USA-only in the decomp) — `port/port_script_funcs.c`
- `-I port` on the decomp ROM build's preprocess so committed `src/` port-includes
  resolve (header-resolution only; no PC code enters the ROM build) — `xmake.lua`

## How `kRomOffsets_JP` was derived (content-anchoring)

This tree's decomp ROM build is **non-matching** (port edits shift symbols ~0xC44),
so `build/JP/tmc_jp.map` is NOT a valid source — its addresses don't match the retail
ROM the port actually loads. Instead the offsets come straight from the **retail USA +
JP ROMs**: the USA addresses are known-correct, so each USA table is located in the JP
ROM. Per-field method (all 28 address fields, see the comment block in `port_rom.c`):

- **direct content-anchor** — unique 64-byte signature of the table start (version-stable
  tables: gfx/palette/map data);
- **pointer-table dereference / shift-search** — for tables of `0x08xxxxxx` pointers,
  find the single shift under which the whole pointer array is internally consistent;
- **text/translations cluster** — uniform `-0x33C` shift, fixed by 4 independent anchors.

Regional shifts grow monotonically with address (`0 → -0x260 → ~-0x338 → -0x33C →
-0x3D4`), each value corroborated by neighbours. The whole table is validated by the
boot test above (2-level area-pointer resolution across all 0x90 areas cannot succeed
with wrong offsets). `/tmp` scratch scripts that produced these aren't committed; the
final values + provenance live in `port_rom.c`. Count/size fields are content-invariant
(identical USA==EU) and carried over.

## Remaining gaps (JP runs but is not yet speedrun-faithful)

The game boots and core gameplay — rendering, RNG, rooms, movement, area data,
and scripted cutscenes/NPCs — is correct.

Resolved since the original writeup:

- **Script addresses (was: `port/port_scripts.h` hardcoded USA).**
  `Port_TranslateScriptAddr` (`port/port_script_addrs.c`) now remaps the
  **entire** script bytecode section — all 576 data scripts, USA→EU/JP by exact
  symbol lookup in the retail maps — not just the ~100 `GBA_script_*` macros.
  This covers scripts referenced only by raw entity-data blobs in
  `port/data_const_stubs.c` (e.g. `script_ZeldaOutsideLinksHouse`, the prologue
  Business Scrub orchestrators). Those were previously untranslated and ran
  garbage bytecode on JP/EU — the cause of the intro Zelda "wrong position" and
  the prologue scrub not spitting.
- **`port/port_script_funcs.c` native-call table.** Has per-region tables
  (`sScriptFuncTable_JP` / `_EU`, selected at runtime via `REGION_IS_*`); the
  two USA-only functions are excluded for JP.

Still needing JP treatment:

1. **Japanese text rendering.** Basic kana render (the intro `こっちよ` textbox
   is correct), but the full JP text *system* (kanji 2-byte encoding, font
   widths/layout) is unverified — audit menus/dialogue against this JP build.

This is the remaining path from "JP boots" to "JP speedrun-faithful". Pair a JP
build with `--console-parity` for hardware-equivalent JP runs.

### Patch profiles on the JP baseline

The JP baseline is also the base for patches that only replace text and font
data. The Angel Team Chinese SP4 patch keeps `BZMJ` and the JP gameplay data but
moves the glyph table (to `0xDC9F00`), the glyph remap table (to `0xE4F000`,
`0xA0` bytes) and carries 16 glyph banks with wide glyphs from bank 3. It also
renumbers the `0x0B..0x0F` text prefixes and makes `0x0F` variable length, so its
byte stream is not decodable with the retail rule.

That is handled by the ROM profile layer rather than by `#ifdef`:

- `port/port_rom_profile.c` — exact profiles, their per-variant tables and a
  validator that checks every offset against the loaded ROM.
- `port/port_text_codec.c` — the prefix decoder (`PORT_TEXT_CODEC_RETAIL` vs
  `PORT_TEXT_CODEC_ANGEL_SP4`).
- `src/text.c` — takes the bank count and the first wide bank from the profile.

The retail JP gaps listed above are unchanged by this: a clean JP ROM still uses
`PORT_TEXT_CODEC_RETAIL` and the 9/5 bank geometry, so its menus need the same
audit as before.

### The port's own UI language

Only the bottom screen's text belongs to the port; the game's dialogue and menus
are drawn by the ROM itself. Its table is `port/port_text_i18n.c` — one column per
language — and `port/port_text_i18n.h` decides which column is read:

- the language picked in the game's file-select screen,
- Chinese when the loaded ROM is the Angel SP4 patch (it reports Japanese, so the
  language above cannot tell it apart from a clean JP ROM),
- or whatever `OPTIONS > LANGUAGE` holds (`ui_language` in `tmc3ds.ini`), which
  wins over both; `-1` means "follow" again. The cell steps from the language in
  effect, so a Chinese ROM opens on English.

The Chinese column is drawn with a 12x12 face generated from Ark Pixel 12px
(SIL OFL 1.1) into `platform/3ds/source/ui_cjk_font.inc` — only the characters the
column uses, about 436 glyphs, plus `EXTRA_DRAWN` for strings the UI draws that are
not table keys (the language names). Re-run
`platform/3ds/tools/gen_cjk_font.py` after adding UI strings. A Chinese cell label
holds five full-width glyphs (68 px, the content width of a quarter-width cell); the
i18n unit test enforces that.

## Version and updater on this fork

Two things differ from tinaut1986's tree so a build of this branch cannot be
mistaken for, or replaced by, the English-only original:

- `platform/3ds/tools/version.sh` describes a build off the last `v0.*` tag
  (`0.4.7-dev.17+<hash>`) instead of falling through to `0.0.0-dev+<hash>`. The
  upstream port's `v2.x` tags are a different lineage and are deliberately
  excluded from the match. Tags win when a clone has them.
- `platform/3ds/release_base.txt` pins the release this branch descends from
  (version and the commit its tag points at), because a fork's CI clones the
  fork and the fork carries no tags. Pushing the upstream tags there is not the
  fix: a tag push runs `build-release.yml` as of that tag's commit, so it would
  publish every historical build, including a release older than this work.
  With the pin, a tag-less clone computes the same string a tagged one does;
  `build-3ds.yml` fails the run rather than upload a `0.0.0` package.
- `UPDATE_REPOSITORY` in `platform/3ds/source/update_manifest.h` points at this
  fork. With no release published here, CHECK FOR UPDATE ends on the existing
  "NO RELEASE AVAILABLE" page rather than offering tinaut1986's build, which
  would install the original port over the profile layer.

Both are merge points: restore the upstream value when this work goes back.

## Related

- Console-Parity mode (`--console-parity`) — run-integrity switch.
