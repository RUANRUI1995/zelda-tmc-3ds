#pragma once

/* Where this port keeps the ROM, saves, config, dumps and update downloads.
 * It is not the upstream port's "The Minish Cap 3DS" folder: this fork installs
 * as its own application (its own title ID, cia/tmc3ds.rsf) and must never read
 * or rewrite the other one's config or saves. */
#define TMC3DS_APP_DIR_FS "/3ds/Minish Cap 3DS"
#define TMC3DS_APP_DIR "sdmc:" TMC3DS_APP_DIR_FS

/* The CIA's title ID: UniqueId 0x5A22E in cia/tmc3ds.rsf (upstream is 0x5A22D).
 * The updater refuses a downloaded CIA that would install anything else.
 *
 * A package built against another region baseline is a different application,
 * not an update of this one: its asset baseline and JP-only data make it
 * incompatible with the USA/EU package. build.sh stamps the matching UniqueId
 * into a generated .rsf, so both can be installed side by side and each
 * updater can only ever replace itself. */
#if defined(JP)
#define TMC3DS_TITLE_ID 0x0004000005a22f00ull
#elif defined(EU)
#define TMC3DS_TITLE_ID 0x0004000005a23000ull
#else
#define TMC3DS_TITLE_ID 0x0004000005a22e00ull
#endif
