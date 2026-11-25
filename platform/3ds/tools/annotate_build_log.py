#!/usr/bin/env python3
"""Republish the reason a build failed as annotations.

A failed CI run only says "step failed"; the reason is buried in a job log that
has to be opened by hand. The tail of a compiler/linker log is where the reason
is, so this republishes the lines immediately before the build tool gave up,
plus any recognised diagnostics. GitHub then shows them on the commit and the
PR, and they are readable through the checks API.

Workflow commands with an oversized payload are dropped silently, so each line
becomes its own small annotation rather than one big dump.

Usage: annotate_build_log.py <logfile>
Exit status is always 0: the caller keeps the build's own status.
"""

import re
import sys

# The build tool's own "I gave up" lines. What precedes them explains why.
GAVE_UP = re.compile(r"(?:gmake|make)(?:\[\d+\])?: \*\*\*|ld returned \d+ exit status|"
                     r"collect2: error|ninja: build stopped|Error \d+$")

# Recognised diagnostics, most specific first: these are worth showing even when
# they are not adjacent to the build tool's final line (parallel builds interleave).
DIAGNOSTIC = re.compile(r"multiple definition of|undefined reference to|undefined symbol|"
                        r"(?<![.\w])in function |\berror\b\s*:|fatal error|cannot find -l|"
                        r"No rule to make target|CMake Error|overflowed by|will not fit")

MAX_ANNOTATIONS = 10
MAX_LINE_CHARS = 200


def escape(text):
    """Workflow-command escaping: the payload travels on a single line."""
    return text.replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")


def emit(message):
    print("::error::" + escape(message))


def main():
    if len(sys.argv) != 2:
        return 0
    try:
        with open(sys.argv[1], "r", errors="replace") as handle:
            lines = [line.rstrip() for line in handle.read().splitlines()]
    except OSError:
        return 0

    picked = []

    def add(line):
        stripped = line.strip()[:MAX_LINE_CHARS]
        if stripped and stripped not in picked:
            picked.append(stripped)

    gave_up_at = next((i for i, line in enumerate(lines) if GAVE_UP.search(line)), None)
    if gave_up_at is not None:
        # The context window: the linker/compiler block right before the failure.
        for line in lines[max(0, gave_up_at - MAX_ANNOTATIONS):gave_up_at]:
            add(line)

    # Then anything that is unmistakably a diagnostic, wherever it appears.
    for line in lines:
        if len(picked) >= MAX_ANNOTATIONS:
            break
        if DIAGNOSTIC.search(line):
            add(line)

    if not picked:
        for line in lines[-MAX_ANNOTATIONS:]:
            add(line)

    for line in picked[:MAX_ANNOTATIONS]:
        emit(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
