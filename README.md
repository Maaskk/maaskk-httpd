# maaskk-httpd

`maaskk-httpd` is a strict HTTP/1.1 server written in C to study sockets, protocol parsing, nonblocking I/O, filesystem boundaries, and server hardening. The first milestone is deliberately small: accept a TCP connection, read one request, return a valid response, and report every syscall failure.

## Current state

The networking vertical slice is working. It binds to loopback by default, supports a configurable IPv4 address and port, handles partial writes, and can exit after one request for deterministic testing.

```bash
cmake -S . -B build
cmake --build build
./build/maaskk-httpd --port 8080
curl -i http://127.0.0.1:8080/
```

Run the smoke test:

```bash
chmod +x tests/smoke.sh
./tests/smoke.sh ./build/maaskk-httpd
```

## Next milestone

Replace the blocking connection loop with an `epoll` engine on Linux. Each connection will own its read buffer, write buffer, parser state, output offset, and last-activity timestamp.

## Scope

The v0.1 target includes incremental request parsing, GET and HEAD, safe static files, keep-alive, timeouts, bounded headers, partial writes, sanitizer tests, and clean shutdown. It will not implement TLS, HTTP/2, CGI, or application routing.

## License

MIT

