# objectdb

`objectdb` is a GTK3-based Linux desktop browser for Frontier `.root` files built on top of `libodb`.

User documentation:

- See [USER_GUIDE.md](./USER_GUIDE.md) for a full usage guide.

Build via the top-level CMake project:

```sh
cmake -S . -B build_libodb
cmake --build build_libodb --target objectdb
```
