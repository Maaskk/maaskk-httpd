#!/bin/sh
set -eu

httpd_binary=${1:-./build/maaskk-httpd}
httpd_port=${HTTPD_TEST_PORT:-18080}
output_file=$(mktemp)
server_log=$(mktemp)

cleanup() {
    if [ -n "${server_pid:-}" ]; then
        kill "$server_pid" 2>/dev/null || true
    fi
    rm -f "$output_file" "$server_log"
}
trap cleanup EXIT INT TERM

"$httpd_binary" --address 127.0.0.1 --port "$httpd_port" --once >"$server_log" 2>&1 &
server_pid=$!

attempt=0
connected=0
while [ "$attempt" -lt 50 ]; do
    if curl --silent --fail "http://127.0.0.1:$httpd_port/" >"$output_file" 2>/dev/null; then
        connected=1
        break
    fi
    attempt=$((attempt + 1))
    sleep 0.05
done

if [ "$connected" -ne 1 ]; then
    printf 'server did not become ready\n' >&2
    cat "$server_log" >&2
    exit 1
fi

wait "$server_pid"
server_pid=

expected='maaskk-httpd alive'
actual=$(tr -d '\r\n' <"$output_file")
if [ "$actual" != "$expected" ]; then
    printf 'expected "%s", got "%s"\n' "$expected" "$actual" >&2
    exit 1
fi

printf 'smoke test passed\n'
