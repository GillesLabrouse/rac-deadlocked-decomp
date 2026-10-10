#!/usr/bin/env bash
# Run the Windows toolchain natively in Git Bash, or through Wine elsewhere.
# PYTHON may point to the active environment's Python executable.
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) TC_RUN=() ;;
    *) TC_RUN=(wine); export WINEDEBUG=-all ;;
esac
if [ -z "${PYTHON:-}" ]; then
    if [ -x venv/bin/python ]; then
        PYTHON=venv/bin/python
    elif [ -f venv/Scripts/python.exe ]; then
        PYTHON=venv/Scripts/python.exe
    else
        PYTHON=python3
    fi
fi
