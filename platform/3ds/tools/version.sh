#!/bin/sh
# Prints the build version, derived from git (same scheme as ../mzm and ../sm-3ds):
#
#   - HEAD exactly on a tag vX.Y.Z:  X.Y.Z
#   - HEAD exactly on a tag vX.Y.Zc: X.Y.Zc (this fork's build of upstream X.Y.Z)
#   - on (or branched off) release/vX.Y.Z:
#                                    X.Y.Z-dev.<commits since main>[.<commits since
#                                    the release branch>]+<hash>
#   - anything else:                  <base>-dev.<commits since base>+<hash>,
#                                    where <base> is the newest tag this tree
#                                    is newer than, or - in a clone that has no
#                                    tags at all - the release pinned in
#                                    ../release_base.txt
#   - neither tags nor a pin:         0.0.0-dev+<hash>
#
# Only three-component tags count, plus this fork's "c" suffix (v0.4.7c: the
# upstream release this fork's Chinese build derives from). The upstream port
# (EstebanPdN) tags with two components (v2.1, v2.2-E1); those tags live in this
# repository too and must never be mistaken for ours. The upstream base this
# fork builds on is in platform/3ds/upstream_version.txt instead.
#
# No leading "v": the updater, the file names and the SMDH all add their own.
# TMC3DS_VERSION in the environment overrides everything.
set -eu

if [ -n "${TMC3DS_VERSION:-}" ]; then
  echo "${TMC3DS_VERSION#v}"
  exit 0
fi

cd "$(dirname "$0")"
OUR_TAGS='v[0-9]*.[0-9]*.[0-9]*'
RELEASE_BASE="../release_base.txt"

# "<version> <commit>" from release_base.txt, first data line; empty if the
# file is absent or holds nothing usable.
ReleaseBase() {
  [ -f "$RELEASE_BASE" ] || return 0
  # The file is checked out with CRLF on some hosts; a stray \r would make the
  # commit field fail its pattern and drop the pin silently. The version may
  # carry this fork's build suffix (0.4.7c), like a tag does.
  awk '{ sub(/\r$/, "") } $1 ~ /^[0-9]+\.[0-9]+\.[0-9]+[a-z]?$/ && $2 ~ /^[0-9a-f]+$/ { print $1, $2; exit }' "$RELEASE_BASE"
}

if ! git rev-parse --git-dir >/dev/null 2>&1; then
  echo "0.0.0-dev"
  exit 0
fi

is_ours() {
  # The glob above also matches v1.2.3-rc and similar; keep plain X.Y.Z, and
  # this fork's single-letter build suffix (v0.4.7c).
  echo "$1" | grep -Eq '^v[0-9]+\.[0-9]+\.[0-9]+[a-z]?$'
}

EXACT=$(git tag --points-at HEAD --list "$OUR_TAGS" 2>/dev/null | sort -V | tail -1)
if [ -n "$EXACT" ] && is_ours "$EXACT"; then
  echo "${EXACT#v}"
  exit 0
fi

HASH=$(git rev-parse --short HEAD 2>/dev/null || echo dev)
BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo HEAD)

case "$BRANCH" in
  release/*) REL="$BRANCH" ;;
  *)
    # Topic branches cut from a release branch inherit its version: the newest
    # release/* branch (local or origin) that HEAD contains.
    REL=$(git for-each-ref --format='%(refname:short)' --merged HEAD \
            'refs/heads/release/*' 'refs/remotes/origin/release/*' 2>/dev/null |
          sed 's#^origin/##' | sort -V | tail -1)
    ;;
esac

case "$REL" in
  release/v*) ;;
  *)
    # Not on a release branch (a fork's main, a CI build of a topic branch):
    # describe from the newest tag of this fork's own line instead of giving up.
    # v0.* only: the upstream port's v2.x tags live in this repository too and
    # are a different version lineage, never this build's ancestor. The second
    # pattern is this fork's build suffix (v0.4.7c), which git's glob does not
    # cover with the first. main then reads 0.4.7-dev.17+<hash>, which keeps the
    # updater's ordering meaningful and the package name readable.
    BASE=""
    BASE_REV=""
    BASE_TAG=$(git describe --tags --abbrev=0 --match 'v0.[0-9]*.[0-9]*' \
                 --match 'v0.[0-9]*.[0-9]*[a-z]' 2>/dev/null || true)
    if [ -n "$BASE_TAG" ] && is_ours "$BASE_TAG"; then
      BASE=${BASE_TAG#v}
      BASE_REV=$(git rev-parse --verify -q "${BASE_TAG}^{commit}" || true)
    else
      # No tag at all, which is what a fork's CI sees: tags are not pushed to a
      # fork, because a tag push runs build-release.yml as of that tag's commit
      # and would publish every historical build. release_base.txt pins the same
      # two facts a tag carries, so the count below still matches a clone that
      # does have the tags.
      set -- $(ReleaseBase)
      BASE=${1:-}
      BASE_REV=${2:-}
    fi
    if [ -n "$BASE" ] && [ -n "$BASE_REV" ] &&
       git rev-parse --verify -q "${BASE_REV}^{commit}" >/dev/null 2>&1; then
      AHEAD=$(git rev-list --count "${BASE_REV}..HEAD" 2>/dev/null || echo 0)
      echo "${BASE}-dev.${AHEAD}+$HASH"
    elif [ -n "$BASE" ]; then
      echo "${BASE}-dev+$HASH"
    else
      echo "0.0.0-dev+$HASH"
    fi
    exit 0
    ;;
esac

if git rev-parse --verify -q "refs/heads/$REL" >/dev/null; then
  TARGET="refs/heads/$REL"
else
  TARGET="refs/remotes/origin/$REL"
fi
if git rev-parse --verify -q refs/remotes/origin/main >/dev/null; then
  BASE=refs/remotes/origin/main
else
  BASE=main
fi
REL_CNT=$(git rev-list --count "$BASE..$TARGET" 2>/dev/null || echo 1)
DEV_CNT=$(git rev-list --count "$TARGET..HEAD" 2>/dev/null || echo 0)
NUM=${REL#release/v}

if [ "$DEV_CNT" = 0 ]; then
  echo "$NUM-dev.$REL_CNT+$HASH"
else
  echo "$NUM-dev.$REL_CNT.$DEV_CNT+$HASH"
fi
