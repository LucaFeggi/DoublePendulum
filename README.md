# DoublePendulum

## Overview

DoublePendulum is a real-time double pendulum simulation written in C11 and rendered with SDL2. The project focuses on visualizing chaotic motion, experimenting with numerical simulation, rendering a real-time physics system, and keeping the implementation lightweight enough to understand and modify.

## Preview

![Single pendulum preview](assets/img/single_pendulum.png)

![Multiple pendulums preview](assets/img/multiple_pendulums.png)

## Architecture

The project keeps headers next to their implementation files under `src/`; there is no separate top-level `include/` directory.

```text
.
|-- CMakeLists.txt
|-- CMakePresets.json
|-- assets/
|   `-- icon/
|       `-- icon.bmp
|-- externals/
|   `-- SDL2-2.32.10/
`-- src/
    |-- main.c
    |-- app/
    |-- config/
    |-- renderer/
    |-- simulation/
    `-- utils/
```

## Cloning

The project vendors SDL2 as a Git submodule. Clone it with submodules enabled:

```sh
git clone --recurse-submodules <repository-url>
cd DoublePendulum
```

If the repository was already cloned without submodules, initialize them before configuring:

```sh
git submodule update --init --recursive
```

## Build System and Presets

The project uses CMake with the Ninja generator and GCC. `CMakeLists.txt` rejects non-Ninja generators and non-GNU C compilers. It also requires CMake 3.25 or newer and C11 support.

SDL2 is vendored as a Git submodule at `externals/SDL2-2.32.10` and is configured as a shared library. On Windows with MinGW, the CMake configuration may fetch [`jtsiomb/c11threads`](https://github.com/jtsiomb/c11threads.git) to provide a C11 `<threads.h>` compatibility layer.

### Available Presets

List the available workflow presets with:

```sh
cmake --workflow --list-presets
```

| Preset | Host | Build type | Platform | Integrator | Renderer | Output executable |
| --- | --- | --- | --- | --- | --- | --- |
| `win32-rk4-sdl-debug` | Windows | Debug | `win32` | `rk4` | `sdl` | `.build\win32-rk4-sdl-debug\bin\DoublePendulum.exe` |
| `win32-rk4-sdl-release` | Windows | Release | `win32` | `rk4` | `sdl` | `.build\win32-rk4-sdl-release\bin\DoublePendulum.exe` |
| `posix-rk4-sdl-debug` | Linux | Debug | `posix` | `rk4` | `sdl` | `.build/posix-rk4-sdl-debug/bin/DoublePendulum` |
| `posix-rk4-sdl-release` | Linux | Release | `posix` | `rk4` | `sdl` | `.build/posix-rk4-sdl-release/bin/DoublePendulum` |

Release builds enable CMake's Release configuration and try to enable IPO/LTO when the toolchain supports it.

### Integrator and Renderer Selection

The codebase is structured so integrators and renderers can be selected at compile time. CMake currently binds `DP_INTEGRATOR=rk4` and `DP_RENDERER=sdl` through the workflow presets, and `CMakeLists.txt` validates those values against the supported lists.

At the moment, `rk4` is the only supported integrator and `sdl` is the only supported renderer. Future integrators and renderer backends can be added by extending the supported CMake values, wiring their source files in `CMakeLists.txt`, and adding workflow presets that select the desired combination.

### Configure and Build

Use the workflow preset for your host platform and build type. Workflow presets run the configure and build steps defined in `CMakePresets.json`.

Windows debug:

```powershell
cmake --workflow --preset win32-rk4-sdl-debug
```

Windows release:

```powershell
cmake --workflow --preset win32-rk4-sdl-release
```

Linux debug:

```sh
cmake --workflow --preset posix-rk4-sdl-debug
```

Linux release:

```sh
cmake --workflow --preset posix-rk4-sdl-release
```

## Running the Simulation

Run the executable produced by the selected preset.

Windows debug:

```powershell
.\.build\win32-rk4-sdl-debug\bin\DoublePendulum.exe
```

Windows release:

```powershell
.\.build\win32-rk4-sdl-release\bin\DoublePendulum.exe
```

Linux debug:

```sh
./.build/posix-rk4-sdl-debug/bin/DoublePendulum
```

Linux release:

```sh
./.build/posix-rk4-sdl-release/bin/DoublePendulum
```

The app has no command-line options. Close the window or press Escape to quit.

The executable loads `assets/icon/icon.bmp` with a relative path. The build copies `assets/` into the executable directory after build, and the same `assets/` directory also exists at the repository root.

## Modifying Simulation Parameters

Most runtime behavior is configured with compile-time macros. After changing them, reconfigure or rebuild as needed.

### Physics Parameters

Edit `src/config/simulation_config.h`:

- `PENDULUM_INIT_MODE`: `0` selects the default initialization, `1` selects the custom rod-specific initialization.
- `DEFAULT_ANG_VEL`, `DEFAULT_LEN`, `DEFAULT_MASS`, `DEFAULT_ANGLE`, `DEFAULT_ANGLE_ADDER`: default mode values shared by both rods.
- `CUSTOM_ANG_VEL_ROD1`, `CUSTOM_LEN_ROD1`, `CUSTOM_MASS_ROD1`, `CUSTOM_ANGLE_ROD1`, `CUSTOM_ANGLE_ADDER_ROD1`: custom mode values for rod 1.
- `CUSTOM_ANG_VEL_ROD2`, `CUSTOM_LEN_ROD2`, `CUSTOM_MASS_ROD2`, `CUSTOM_ANGLE_ROD2`, `CUSTOM_ANGLE_ADDER_ROD2`: custom mode values for rod 2.
- `GRAVITY_CENTI` and `G`: gravitational acceleration. The default is `981`, which makes `G` equal to `9.81`.
- `SIMULATION_STEPS_PER_SECOND` and `SIMULATION_DT`: fixed simulation timestep.
- `SIMULATION_TIME_SCALE`: multiplier applied to elapsed real time before consuming simulation steps.
- `TOTAL_PENDULUMS`: number of pendulums allocated and rendered.

The simulation can run one or more pendulums. The current default is `TOTAL_PENDULUMS == 5`; each pendulum starts with an index-based angle offset from the relevant `*_ANGLE_ADDER` macro, which makes chaotic divergence visible over time.

### Rendering Parameters

Edit `src/config/render_config.h`:

- `COLOR_DECAY_PER_MILLE` and `COLOR_DECAY_REFERENCE_FPS`: control decay of the maximum angular velocity used for color normalization.
- `ROD_WIDTH_PER_MILLE` and `ROD_WIDTH_PIXELS`: rod thickness.
- `TRAIL`: enables or disables trail rendering.
- `TRAIL_WIDTH_PER_MILLE` and `TRAIL_WIDTH_PIXELS`: trail thickness.
- `TRAIL_DURATION_MILLISECONDS`: total visible trail lifetime.
- `TRAIL_BUCKET_MILLISECONDS`: duration represented by each trail render-target bucket.
- `TRAIL_FADE_GAMMA_PER_MILLE`: shapes trail alpha falloff.

Rod colors are defined in the `spectrum` array in `src/renderer/sdl/color.c`.

### Application Loop Parameters

Edit `src/config/app_config.h`:

- `MIN_SUPPORTED_RENDER_FPS`: lowest render FPS where the app should still try to keep the configured simulation speed before dropping accumulated simulation time.
- `MAX_SIMULATION_STEPS_PER_FRAME`: derived cap for fixed simulation steps consumed in one rendered frame.
- `THREADPOOL_MIN_ITEMS_PER_JOB`: controls how many pendulums are needed before the app creates useful worker jobs.

`src/config/config_validation.h` validates these values with `_Static_assert`.

## Technical Notes

The implementation is split by responsibility:

- `src/app/`: SDL window setup, input, frame timing, and the fixed-step application loop.
- `src/simulation/`: pendulum state, equations of motion, initialization, and RK4 integration.
- `src/renderer/sdl/`: SDL rendering, screen-space conversion, rods, trails, and color mapping.
- `src/config/`: compile-time parameters and validation.

A few design choices are useful to know before changing behavior:

- Simulation advances with fixed `SIMULATION_DT` steps; the app accumulates scaled frame time and caps per-frame catch-up work with `MAX_SIMULATION_STEPS_PER_FRAME`.
- Positions are derived for rendering instead of stored in the simulation state.
- Multiple pendulums use index-based initial angle offsets to make chaotic divergence visible.
- Rods and trails are batched through `SDL_RenderGeometry`.
- Trail history is stored in render-target texture buckets rather than rebuilt from CPU-side point lists each frame.
- Worker threads are only useful for larger `TOTAL_PENDULUMS` values; the default workload is intentionally small.

## Dependencies

Required tools and libraries:

- CMake 3.25 or newer.
- Ninja.
- GCC with C11 support.
- C11 threads support through `<threads.h>`.
- Git, for initializing the SDL2 submodule.
- SDL2 2.32.10, vendored under `externals/SDL2-2.32.10`.

Platform notes:

- Windows builds are intended for GCC/MinGW and use the `win32-*` presets.
- Linux builds use the `posix-*` presets and link against the C math library and CMake `Threads::Threads`.
- Other host platforms are rejected by `CMakeLists.txt`.
