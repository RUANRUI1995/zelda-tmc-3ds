#include "platform_3ds.h"
#include "tmc3ds_version.h"
#include "updater.h"
#include "port_second_screen_theme.h"

#include "port_audio.h"
#include "port_ppu.h"
#include "port_rom.h"
#include "port_runtime_config.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "app_paths_3ds.h"
#include "port_rom_hash.h"
#include "port_rom_profile.h"
#include "port_text_i18n.h"
#include "upstream_import_3ds.h"
#include "boot_text_3ds.h"
#include "tmc3ds_build_config.h"
#if TMC3DS_DEBUG_TOOLS
#include "debug_log_3ds.h"
#include "debug_perf_3ds.h"
#include "debug_switches_3ds.h"
#endif
#include "ftp_server_3ds.h"
#include "retro_ach.h"

#define APP_DIR TMC3DS_APP_DIR
#define ROM_PATH_SIZE 512
extern void AgbMain(void);

static const PortRomProfile* sSelectedRomProfile;
static PortRomHashes sSelectedRomHashes;
static const PortRomProfile* sMismatchedRomProfile;

/* The compile-time baseline of this package. A ROM's region family decides which
 * offsets, data stubs and asset baseline are valid, so a JP ROM must never be
 * booted by a USA/EU build nor the other way round. Within the JP family the
 * exact revision (clean retail vs Angel SP4) is decided by the ROM profile. */
static const char* BuildBaselineName(void) {
#if defined(JP)
    return "JP";
#elif defined(EU)
    return "EU";
#else
    return "USA";
#endif
}

static int RomMatchesBuildFamily(const PortRomProfile* profile) {
#if defined(JP)
    return profile->region == ROM_REGION_JP;
#else
    return profile->region == ROM_REGION_USA || profile->region == ROM_REGION_EU;
#endif
}

static const char* RequiredBuildBaseline(const PortRomProfile* profile) {
    if (profile != NULL && profile->region == ROM_REGION_JP) return "JP";
    return "USA";
}

static int PrepareStorage(void) {
    mkdir("sdmc:/3ds", 0777);
    if (mkdir(APP_DIR, 0777) != 0 && errno != EEXIST) return 0;
    return chdir(APP_DIR) == 0;
}

static int HasGbaExtension(const char* name) {
    const size_t length = strlen(name);
    if (length < 5) return 0;
    const char* ext = name + length - 4;
    return tolower((unsigned char)ext[0]) == '.' &&
           tolower((unsigned char)ext[1]) == 'g' &&
           tolower((unsigned char)ext[2]) == 'b' &&
           tolower((unsigned char)ext[3]) == 'a';
}

/* ROM hash cache.
 *
 * Identifying a ROM by profile means hashing all 16 MiB of it, which costs more
 * than the rest of the boot put together, and the answer cannot change unless
 * the file does. Cache it in the app folder keyed by name + size + mtime, so a
 * normal second launch skips hashing entirely. */
#define ROM_HASH_CACHE_PATH "romhash.cache"
#define ROM_HASH_CACHE_MAX 8

typedef struct {
    char path[ROM_PATH_SIZE];
    long long size;
    long long mtime;
    char sha1[PORT_SHA1_HEX_LENGTH + 1];
    char sha256[PORT_SHA256_HEX_LENGTH + 1];
} RomHashCacheEntry;

static int LoadRomHashCache(RomHashCacheEntry* entries, int capacity) {
    FILE* f = fopen(ROM_HASH_CACHE_PATH, "r");
    int n = 0;
    if (f == NULL) return 0;
    while (n < capacity && fscanf(f, "%511s %lld %lld %40s %64s", entries[n].path, &entries[n].size,
                                  &entries[n].mtime, entries[n].sha1, entries[n].sha256) == 5) {
        ++n;
    }
    fclose(f);
    return n;
}

static void StoreRomHashCache(const RomHashCacheEntry* entries, int n) {
    FILE* f = fopen(ROM_HASH_CACHE_PATH, "w");
    if (f == NULL) return;
    for (int i = 0; i < n; ++i) {
        fprintf(f, "%s %lld %lld %s %s\n", entries[i].path, entries[i].size, entries[i].mtime, entries[i].sha1,
                entries[i].sha256);
    }
    fclose(f);
}

static RomHashCacheEntry sRomHashCache[ROM_HASH_CACHE_MAX];
static int sRomHashCacheCount;
static int sRomHashCacheDirty;

/* Returns 1 when the cache answered for `path`. */
static int RomHashesFromCache(const char* path, const struct stat* info, PortRomHashes* out) {
    for (int i = 0; i < sRomHashCacheCount; ++i) {
        const RomHashCacheEntry* e = &sRomHashCache[i];
        if (e->size != (long long)info->st_size || e->mtime != (long long)info->st_mtime ||
            strcmp(e->path, path) != 0) {
            continue;
        }
        memset(out, 0, sizeof(*out));
        out->size = (size_t)info->st_size;
        snprintf(out->sha1, sizeof(out->sha1), "%s", e->sha1);
        snprintf(out->sha256, sizeof(out->sha256), "%s", e->sha256);
        return 1;
    }
    return 0;
}

static void RomHashesStoreCache(const char* path, const struct stat* info, const PortRomHashes* hashes) {
    int slot = -1;
    for (int i = 0; i < sRomHashCacheCount; ++i) {
        if (strcmp(sRomHashCache[i].path, path) == 0) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        if (sRomHashCacheCount < ROM_HASH_CACHE_MAX) {
            slot = sRomHashCacheCount++;
        } else {
            slot = 0; /* evict the oldest slot; the list is tiny and unordered */
        }
    }
    snprintf(sRomHashCache[slot].path, sizeof(sRomHashCache[slot].path), "%s", path);
    sRomHashCache[slot].size = (long long)info->st_size;
    sRomHashCache[slot].mtime = (long long)info->st_mtime;
    snprintf(sRomHashCache[slot].sha1, sizeof(sRomHashCache[slot].sha1), "%s", hashes->sha1);
    snprintf(sRomHashCache[slot].sha256, sizeof(sRomHashCache[slot].sha256), "%s", hashes->sha256);
    sRomHashCacheDirty = 1;
}

/* 1 supported, 0 not a supported profile, -1 a supported profile of the other
 * build family (reported to the user instead of being silently skipped). */
static int RomIsSupported(const char* path, const struct stat* info) {
    PortRomHashes hashes = { 0 };
    const PortRomProfile* profile;
    if (info != NULL && RomHashesFromCache(path, info, &hashes)) {
        char gameCode[4] = { 0 };
        FILE* gc = fopen(path, "rb");
        if (gc != NULL) {
            if (fseek(gc, 0xAC, SEEK_SET) == 0) (void)fread(gameCode, 1, sizeof(gameCode), gc);
            fclose(gc);
        }
        profile = Port_IdentifyRomHashes(&hashes, gameCode);
    } else {
        profile = Port_IdentifyRomFile(path, &hashes);
        if (info != NULL && profile != NULL) RomHashesStoreCache(path, info, &hashes);
    }
    if (!Port_RomProfileIsPlayable(profile)) {
        if (hashes.sha256[0] != '\0') {
            printf("Skipping unsupported ROM: %s\nSHA-256: %s\n", path, hashes.sha256);
        }
        return 0;
    }
    if (!RomMatchesBuildFamily(profile)) {
        if (sMismatchedRomProfile == NULL) sMismatchedRomProfile = profile;
        printf("Skipping ROM for a different build family: %s\nProfile: %s\nBuild baseline: %s\n", path,
               profile->displayName, BuildBaselineName());
        return -1;
    }
    sSelectedRomProfile = profile;
    sSelectedRomHashes = hashes;
    /* A Chinese ROM (Angel SP4) reports Japanese and would otherwise leave the
     * port's own bottom screen in English; follow its translation instead. */
    Port_Text_SetChineseUi(profile->variant == PORT_ROM_VARIANT_JP_ANGEL_SP4);
    return 1;
}

/* The first supported .gba of `dirPath` into `out`, prefixed with `prefix`
 * ("" for the working directory). 1 found, -1 only unsupported .gba files,
 * -2 a supported ROM of the other build family, 0 none. */
static int FindRomIn(const char* dirPath, const char* prefix, char* out, size_t outSize) {
    DIR* dir = opendir(dirPath);
    if (!dir) return 0;

    int foundGba = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (!HasGbaExtension(entry->d_name)) continue;

        char path[ROM_PATH_SIZE];
        snprintf(path, sizeof(path), "%s%s", prefix, entry->d_name);
        struct stat info;
        if (stat(path, &info) != 0 || !S_ISREG(info.st_mode)) continue;
        foundGba = 1;
        const int supported = RomIsSupported(path, &info);
        if (supported <= 0) continue;
        snprintf(out, outSize, "%s", path);
        closedir(dir);
        return 1;
    }

    closedir(dir);
    if (sMismatchedRomProfile != NULL) return -2;
    return foundGba ? -1 : 0;
}

/* Our folder first; without a ROM there, the upstream port's folder, read in place
 * so the ROM is not on the SD twice. */
static int FindRom(char* out, size_t outSize) {
    sRomHashCacheCount = LoadRomHashCache(sRomHashCache, ROM_HASH_CACHE_MAX);
    sRomHashCacheDirty = 0;

    const int own = FindRomIn(".", "", out, outSize);
    int result = own;
    if (own <= 0) {
        const int upstream = FindRomIn(UPSTREAM_APP_DIR, UPSTREAM_APP_DIR "/", out, outSize);
        if (upstream != 0) result = upstream;
    }

    if (sRomHashCacheDirty) StoreRomHashCache(sRomHashCache, sRomHashCacheCount);
    return result;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    if (!Platform3DS_Init()) return 1;
    Platform3DS_SilenceBootConsole();
    Platform3DS_ShowSplash();

    Platform3DS_BootAnimStart();
    if (!PrepareStorage()) {
        Platform3DS_BootAnimStop();
        char message[256];
        snprintf(message, sizeof(message), "%s\n%s", BootText_Tr("COULD NOT OPEN THIS FOLDER:"), APP_DIR);
        Platform3DS_ShowFatal("STORAGE ERROR", message);
        Platform3DS_Shutdown();
        return 1;
    }

    /* Before the game starts: it reads its saves once, at boot. */
    if (UpstreamImport_ShouldOffer()) {
        Platform3DS_BootAnimStop();
        UpstreamImport_OfferSaves();
        Platform3DS_BootAnimStart();
    }

    char romPath[ROM_PATH_SIZE];
    int romResult = FindRom(romPath, sizeof(romPath));
    if (romResult <= 0) {
        /* In the console's language (port_text_i18n.c); paths and hashes as they are.
         * A string with no translation entry is shown in English. */
        char message[900];
        if (romResult == -2 && sMismatchedRomProfile != NULL) {
            snprintf(message, sizeof(message),
                     "Found %s (%s), but this package uses the %s compile-time baseline.\n\n"
                     "Use a build with TMC3DS_REGION=%s for this ROM.\n"
                     "JP builds accept clean Japan and Angel SP4; USA/EU builds accept the original USA "
                     "and Europe ROMs.",
                     sMismatchedRomProfile->displayName, sMismatchedRomProfile->id, BuildBaselineName(),
                     RequiredBuildBaseline(sMismatchedRomProfile));
        } else if (romResult < 0) {
            snprintf(message, sizeof(message), "%s\n\n%s\n%s\n%s\n\n%s\nUSA: %s\n%s %s\n%s %s\n%s %s",
                     BootText_Tr("NONE OF THE .GBA FILES IS A SUPPORTED USA (BZME) OR EUROPE (BZMP) ROM."),
                     BootText_Tr("FOLDERS SEARCHED:"), APP_DIR, UPSTREAM_APP_DIR, BootText_Tr("EXPECTED SHA-1:"),
                     "b4bd50e4131b027c334547b4524e2dbbd4227130", BootText_Tr("EUROPE:"),
                     "cff199b36ff173fb6faf152653d1bccf87c26fb7", "JAPAN:",
                     "6c5404a1effb17f481f352181d0f1c61a2765c5d", "ANGEL SP4:",
                     "ba04cfbe93d12d2ad684c52234472fa12a5b53d7");
        } else {
            snprintf(message, sizeof(message), "%s\n%s\n%s\n%s\n\n%s\n\n%s\nUSA: %s\n%s %s\n%s %s\n%s %s",
                     BootText_Tr("COPY YOUR CLEAN USA OR EUROPE ROM TO:"), APP_DIR,
                     BootText_Tr("THE ORIGINAL PORT'S FOLDER WORKS TOO:"), UPSTREAM_APP_DIR,
                     BootText_Tr("ANY .GBA FILENAME IS ACCEPTED."), BootText_Tr("EXPECTED SHA-1:"),
                     "b4bd50e4131b027c334547b4524e2dbbd4227130", BootText_Tr("EUROPE:"),
                     "cff199b36ff173fb6faf152653d1bccf87c26fb7", "JAPAN:",
                     "6c5404a1effb17f481f352181d0f1c61a2765c5d", "ANGEL SP4:",
                     "ba04cfbe93d12d2ad684c52234472fa12a5b53d7");
        }
        Platform3DS_BootAnimStop();
        Platform3DS_ShowFatal(romResult == -2 ? "ROM/BUILD MISMATCH"
                              : romResult < 0 ? "UNSUPPORTED ROM"
                                              : "ROM NOT FOUND",
                              message);
        Platform3DS_Shutdown();
        return 1;
    }
    printf("ROM profile: %s (%s)\nROM SHA-1: %s\n", sSelectedRomProfile->displayName, sSelectedRomProfile->id,
           sSelectedRomHashes.sha1);

    FILE* rom = fopen(romPath, "rb");
    if (!rom) {
        Platform3DS_BootAnimStop();
        Platform3DS_ShowFatal("ROM ERROR", BootText_Tr("COULD NOT OPEN THE SELECTED .GBA FILE."));
        Platform3DS_Shutdown();
        return 1;
    }
    fclose(rom);

    Port_Config_Load("tmc3ds.ini");
#if TMC3DS_DEBUG_TOOLS
    /* Debug builds log from boot unless the DEBUG tab left it off; the FTP
     * server comes back on its own once Wi-Fi is up (port_ppu_3ds.c). */
    if (Port_Config_DebugLogOn()) {
        DebugLog_SetDirect(Port_Config_DebugLogDirect());
        DebugLog_Start();
    }
#endif
    Port_LoadRom(romPath);
    Port_SecondScreenTheme_Ready();
    Platform3DS_BootAnimStop();
    Port_PPU_Init(NULL);
    if (!Port_Audio_Init()) {
#if TMC3DS_DEBUG_TOOLS
        DebugLog_Printf("audio is unavailable");
#endif
    }

    Platform3DS_EnterGameplayDisplay();
    Updater_Init(argc > 0 ? argv[0] : NULL, Port_Config_GetAutoUpdate());
    /* After the ROM (its hash) and in the data folder (retroachievements.ini). */
    RetroAch_Init();
    AgbMain();
#if TMC3DS_DEBUG_TOOLS
    /* Each step is flushed: if one never returns, the log says which. */
#define EXIT_STEP(name)                         \
    do {                                        \
        DebugLog_Printf("exit: %s", name);      \
        DebugLog_Flush();                       \
    } while (0)
    EXIT_STEP("engine returned");
    if (DebugPerf_Running()) DebugPerf_Stop();
#else
#define EXIT_STEP(name) ((void)0)
#endif
    /* Production builds too: the install-only server (dev-ftp.txt). */
    if (FtpServer_Running()) FtpServer_Stop();
    EXIT_STEP("ftp stopped");
    Updater_Shutdown();
    EXIT_STEP("updater stopped");
    RetroAch_Shutdown();
    EXIT_STEP("retroachievements stopped");
    Port_PPU_Shutdown();
    EXIT_STEP("ppu stopped");
#if TMC3DS_DEBUG_TOOLS
    DebugLog_Stop();
#endif
    Platform3DS_Shutdown();
    return 0;
}
