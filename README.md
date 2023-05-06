# acacia.h
![Discord](https://img.shields.io/discord/1096149563871613099?style=for-the-badge&label=exploiting%20c)
![Lines of code](https://img.shields.io/tokei/lines/github/Oderjunkie/acacia.h?style=for-the-badge)
![GitHub all releases](https://img.shields.io/github/downloads/Oderjunkie/acacia.h/total?style=for-the-badge)
![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/Oderjunkie/acacia.h?style=for-the-badge)
![Minimum C version: 99](https://img.shields.io/badge/Minimum%20C%20Version-99-blueviolet?style=for-the-badge)
![GitHub](https://img.shields.io/github/license/Oderjunkie/acacia.h?style=for-the-badge)

a simple C web serving library

```c
#include <stdio.h>
#include "acacia.h"

int main(void) {
  listen (80)
    route {
      GET("/"): fprintf(netout, "<h1>index!</h1>\n<a href=\"/foo\">go to foo</a>\n");
      GET("/foo"): fprintf(netout, "<h1>foo!</h1>\n<a href=\"/bar\">go to bar</a>\n");
      GET("/bar"): fprintf(netout, "<h1>bar!</h1>\n<a href=\"/\">go to index</a>\n");
      /* use connection->path to refer to the path currently requested */
      GET("/baz/**"): fprintf(netout, "<h1>baz! (from %s)</h1>\n<a href=\"/\">go to index</a>\n", connection->path);
    } else {
      fprintf(netout, "<h1>404!</h1>\n");
    }
  return 0;
}
```
