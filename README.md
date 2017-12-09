<div>
	<img src="./doc/logo.png">
</div>

Megrez
====================

**NOT STABLE BY NOW !!!**

## Welcome to Megrez!

Megrez is a fast data serialization tool for memory constrained applications.

## How to use Megrez?

1. use `autobuild.sh` to build the `MegrezC.exe`(Or you can just download it in the Release files.)
2. Write the IDL file (such as schema.mgz, you can find a demo in `./test`).
3. Use `MegrezC.exe` to compile the IDL file into different languages.
4. Add `./megrez` into your compiler (Megrez requires a C++11 compatible compiler).
5. For C++, you can `#include "schema.mgz.h"` (just a example) to continue.

## To join the development?

You can make a new pull request, Pull Request is welcomed!
