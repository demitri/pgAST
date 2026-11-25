#!/usr/bin/env bash
#
# Build pgAST, install it into a staging directory, start a throwaway
# PostgreSQL cluster that loads the extension from there, and run the
# pg_regress suite (src/test/sql, src/test/expected) against it.
#
# Nothing is installed into the PostgreSQL installation itself, so no root
# access is needed. Requires PostgreSQL >= 18 (extension_control_path).
#
# Usage:
#   AST=/usr/local/ast scripts/run_tests.sh
#
# Environment:
#   AST        Starlink AST installation to build against (required).
#   PG_CONFIG  pg_config of the PostgreSQL to test with (default: pg_config on PATH).
#   KEEP_TMP   If set, the temporary directory is kept for inspection.

set -euo pipefail

: "${AST:?Set AST to a Starlink AST installation, e.g. AST=/usr/local/ast}"
PG_CONFIG="${PG_CONFIG:-pg_config}"

repo="$(cd "$(dirname "$0")/.." && pwd)"
bindir="$("$PG_CONFIG" --bindir)"
pg_major="$("$PG_CONFIG" --version | sed -E 's/^PostgreSQL ([0-9]+).*/\1/')"
if (( pg_major < 18 )); then
    echo "run_tests.sh needs PostgreSQL >= 18 (extension_control_path); found $pg_major." >&2
    exit 1
fi

tmp="$(mktemp -d "${TMPDIR:-/tmp}/pgast-test.XXXXXX")"
stage="$tmp/stage"
data="$tmp/data"
sock="$tmp/sock"
mkdir -p "$sock"

cleanup() {
    if [[ -f "$data/postmaster.pid" ]]; then
        "$bindir/pg_ctl" -D "$data" -m immediate stop >/dev/null 2>&1 || true
    fi
    if [[ -z "${KEEP_TMP:-}" ]]; then
        rm -rf "$tmp"
    else
        echo "Temporary directory kept: $tmp"
    fi
}
trap cleanup EXIT

# On macOS, PGXS compiles with the SDK PostgreSQL was built against
# (PG_SYSROOT). After an Xcode update that SDK path can be gone; use the
# current one instead.
make_args=(AST="$AST" PG_CONFIG="$PG_CONFIG")
if [[ "$(uname)" == Darwin ]]; then
    pg_sysroot="$(sed -nE 's/^PG_SYSROOT = (.*)$/\1/p' "$("$PG_CONFIG" --pgxs | sed 's|/pgxs/src/makefiles/pgxs.mk$|/pgxs/src/Makefile.global|')")"
    if [[ -n "$pg_sysroot" && ! -d "$pg_sysroot" ]]; then
        make_args+=(PG_SYSROOT="$(xcrun --show-sdk-path)")
    fi
fi

make -C "$repo/src" "${make_args[@]}" clean >/dev/null
make -C "$repo/src" "${make_args[@]}"
make -C "$repo/src" "${make_args[@]}" install DESTDIR="$stage" >/dev/null

# Staged paths mirror the real installation under $stage.
sharedir="$stage$("$PG_CONFIG" --sharedir)"
pkglibdir="$stage$("$PG_CONFIG" --pkglibdir)"

"$bindir/initdb" -D "$data" -A trust -U postgres --no-instructions >/dev/null
cat >> "$data/postgresql.conf" <<EOF
listen_addresses = ''
unix_socket_directories = '$sock'
port = 54329
extension_control_path = '$sharedir:\$system'
dynamic_library_path = '$pkglibdir:\$libdir'
EOF
"$bindir/pg_ctl" -D "$data" -l "$tmp/server.log" -w start >/dev/null

status=0
PGHOST="$sock" PGPORT=54329 PGUSER=postgres \
    make -C "$repo/src" "${make_args[@]}" installcheck || status=$?

if (( status != 0 )) && [[ -f "$repo/src/test/regression.diffs" ]]; then
    cat "$repo/src/test/regression.diffs"
fi
exit "$status"
