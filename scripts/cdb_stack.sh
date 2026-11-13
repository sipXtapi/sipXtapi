#!/bin/bash
#
# Copyright (C) 2026 SIPez LLC.  All rights reserved.
#
# Print stacks from a Windows crash or hang dump with cdb, over ssh, no GUI.
#
#   scripts/cdb_stack.sh <dump>            crashing thread (exception context)
#   scripts/cdb_stack.sh --all <dump>      every thread (use for hang dumps)
#   scripts/cdb_stack.sh --analyze <dump>  !analyze -v plus crashing thread
#
# Needs Debugging Tools for Windows (cdb.exe), run from Cygwin. Symbols:
#   - sipX: the PDBs in x64/Release of this repo;
#   - Windows: Visual Studio's symbol cache first (cdb cannot resolve the
#     Microsoft symbol server from an ssh session), then the server itself.
# Override with CDB=<path to cdb.exe> or SIPX_SYMCACHE=<cache directory>.

set -u

mode=crash
case "${1:-}" in
   --all) mode=all; shift ;;
   --analyze) mode=analyze; shift ;;
   -h|--help|"")
      sed -n '3,15p' "$0" | sed 's/^# \{0,1\}//'
      exit 1 ;;
esac

dump="${1:-}"
if [ ! -f "$dump" ]; then
   echo "cdb_stack.sh: no such dump: $dump" >&2
   exit 1
fi

cdb="${CDB:-/cygdrive/c/Program Files (x86)/Windows Kits/10/Debuggers/x64/cdb.exe}"
if [ ! -x "$cdb" ]; then
   echo "cdb_stack.sh: cdb.exe not found at $cdb (install Debugging Tools for Windows, or set CDB)" >&2
   exit 1
fi

repo="$(cd "$(dirname "$0")/.." && pwd)"
cache="${SIPX_SYMCACHE:-/cygdrive/c/Users/$USER/AppData/Local/Temp/SymbolCache}"
sympath="srv*$(cygpath -w "$cache")*https://msdl.microsoft.com/download/symbols;$(cygpath -w "$repo/x64/Release")"

case "$mode" in
   crash)   commands='.ecxr; kn 40; q' ;;
   all)     commands='~*kn 40; q' ;;
   analyze) commands='!analyze -v; .ecxr; kn 40; q' ;;
esac

"$cdb" -lines -z "$(cygpath -w "$dump")" -y "$sympath" -c "$commands" 2>&1 \
   | grep -viE 'natvis script (un)?loaded'
