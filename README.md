### HTTPinC
minimal HTTP server writen from scratch in C using BSD sockets. Built as a learning project to understand TCP/HTTP request-response cycle at syscall level, without networking libraries or frameworks.


##### Core Functions:
- Listens for TCP connections on port `8080`
- Accepts one client connection at a time
- Reads the incomming HTTP request via `recv()`
- If the request is a `GET`, reads `index.html` from disk and serves it back with a valid response, headers, and content-length computed from the actual file size
- Closes the per-clinet conection and loops back to accept the next one.


##### Systems:
- Linux (POSIX sockets: `<sys/socket.h>`, `<netdb.h>`, `<unistd.h>`)
- gcc


##### Build:
```
gcc server.c -o server
```

##### Run:
```
./server
```
the server prints `Bind result is: 0` on success, then `Got a connection!` each time a client connects.

##### Test:
```
curl -v http://localhost:8080
```
or navigate to `http://localhost:8080` using a browser, with explicit `http://` call as some browsers auto-upgrade bare `localhost:8080` to HTTPS via HTTPS-Only Mode.


##### Files:
- `server.c`
- `index.html` - the page served in response to `GET` request (this is only a dummy html)


##### How it works:
1. `getaddrinfo()` resolves the local address/port to bind to (`0.0.0.0:8080`)
2. `socket()` creates the listening socket, using the family/type/protocol `getaddrinfo` resolved
3. `bind()` attaches the socket to the port; `SO_REUSEADDR` is set so the server can restart quickly without causing the "address already in use" issue
4. `listen()` marks the socket as ready to accept incoming connections (backlog size of 20)
5. The server loops `accept()`, which blocks until a client connect and returns a new socket (`newfd`) dedicated to that client
6. `recv()` reads the client's raw HTTP requrest off `newfd` 
7. If it is a `GET`, `index.html` is read into a fixed sized buffer via `fread()`, a response header is built with `snprintf()` (which includes the real content-length) and both header and body are sent back with `send()`
8. `close(newfd)` ends that client's connection: loop returns to `accept()` for the next one.

##### Limitation:
- Handles only one connection at a time 
- Serves only a single hardcoded file regardles of the requested path
- No parsing of the request line/headers beyond checking for the `GET` method prefix
- File buffer is fixed sized stack array 
- No response for non-GET requests
- No `Connection: keep-alive` support 