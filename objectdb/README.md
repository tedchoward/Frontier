# objectdb

`objectdb` is a GTK3-based Linux desktop browser for Frontier `.root` files built on top of `libodb`.

Current status:

- Phase 1 scaffold only
- Starts a GTK3 application window
- Data browser UI not implemented yet

Build via the top-level CMake project:

```sh
cmake -S . -B build_libodb
cmake --build build_libodb --target objectdb
```
