#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// This branch carries its own ROM-profile layer (Chinese Angel SP4 ROMs, JP
// baseline), so the updater must never pull tinaut1986's build over it: that
// release would replace this port with the English-only original. Point it at
// this fork. With no release published here the check lands on the existing
// "NO RELEASE AVAILABLE" message instead of installing the wrong build.
// Restore the upstream value when merging this work back into tinaut1986.
#define UPDATE_REPOSITORY "RUANRUI1995/zelda-tmc-3ds"
#define UPDATE_MAX_FILE (32u * 1024u * 1024u)
typedef struct UpdateRelease {
  char version[48];
  char notes[12289];
  char url[512];
  char sha256[65];
  uint32_t size;
  bool prerelease;
} UpdateRelease;
// -1: invalid response, 0: no publication in this channel, 1: valid candidate.
int Update_ParseRelease(const char *data, size_t size, bool prerelease,
                        bool homebrew, UpdateRelease *out);
// The channel's newest release out of GitHub's list (/releases): "betas" also counts
// pre-releases, else only stable ones. When `installed` is given and the candidate is
// newer, the notes cover every release of the channel in between, newest first.
// -1: invalid response, 0: nothing in this channel, 1: valid candidate.
int Update_ParseReleases(const char *data, size_t size, bool betas, bool homebrew,
                         const char *installed, UpdateRelease *out);
bool Update_IsNewer(const char *candidate, const char *installed);
bool Update_ValidVersion(const char *version);
bool Update_AllowedDownloadUrl(const char *url);

unsigned Update_FormatNotes(const char *markdown, char lines[][43], unsigned capacity);
