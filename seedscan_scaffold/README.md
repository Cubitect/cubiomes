# seedscan scaffold (C + CMake)

Minimal scaffold for a Minecraft Java 1.21 seed scanner.
It already parses CLI, generates an integer disk around spawn,
and writes results to CSV. The only TODOs are wiring Cubiomes in:

- `cb_init_generator(...)`
- `cb_get_spawn(...)`
- `cb_query_surface_biome(...)`

## Build (without Cubiomes - stub only)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/seedscan --mc 1.21 --radius 100 --target 20 --max-seeds 1000000
```

## Expected agent tasks
1. Add Cubiomes as a submodule in `external/cubiomes`.
2. Enable the `add_subdirectory(...)` + link/include lines in `CMakeLists.txt`.
3. Implement the three `cb_*` functions for Minecraft Java **1.21**.
4. Keep CLI and CSV columns unchanged.
5. Stop after 20 hits or `--max-seeds` is reached.
