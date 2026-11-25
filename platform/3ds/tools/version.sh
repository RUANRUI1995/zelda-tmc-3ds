#!/bin/sh
# Prints the build version, derived from git (same scheme as ../mzm and ../sm-3ds):
#
#   - HEAD exactly on a tag vX.Y.Z:  X.Y.Z
#   - on (or branched off) release/vX.Y.Z:
#                                    X.Y.Z-dev.<commits since main>[.<commits since
#                                    the release branch>]+<hash>
#   - anything else:                 0.0.0-dev+<hash>
#
# Only three-component tags count. The upstream port (EstebanPdN) tags with two
# components (v2.1, v2.2-E1); those tags live in this repository too and must
# never be mistaken for ours. The upstream base this fork builds on is in
# platform/3ds/upstream_version.txt instead.
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

if ! git rev-parse --git-dir >/dev/null 2>&1; then
  echo "0.0.0-dev"
  exit 0
fi

is_ours() {
  # The glob above also matches v1.2.3-rc and similar; keep plain X.Y.Z only.
  echo "$1" | grep -Eq '^v[0-9]+\.[0-9]+\.[0-9]+$'
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
    # are a different version lineage, never this build's ancestor.
    # main then reads 0.4.7-dev.7+<hash>, which keeps the updater's ordering
    # meaningful and the package name readable.
    BASE_TAG=$(git describe --tags --abbrev=0 --match 'v0.[0-9]*.[0-9]*' 2>/dev/null || true)
    if [ -n "$BASE_TAG" ] && is_ours "$BASE_TAG"; then
      AHEAD=$(git rev-list --count "${BASE_TAG}..HEAD" 2>/dev/null || echo 0)
      echo "${BASE_TAG#v}-dev.${AHEAD}+$HASH"
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
