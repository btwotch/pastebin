# pastebin
Pastebin for the console with syntax highlighting

## Build
*make* to make - this installs also some prerequisites (libmicrohttpd, highlight, libhttpserver)

## Usage
It is listening on port 8080 and shows some useful information on how to use it:
```
PUT: curl http://server:8080/foo.c --data-binary @foo.c
PUT with rudimentary auth: curl http://server:8080/foo.c --data-binary @foo.c --user britneyspears
GET: curl http://server:8080/foo.c
GET without highlighting: curl -http://server:8080/foo.c -A ""
```
