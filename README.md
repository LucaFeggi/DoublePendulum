# DoublePendulum

DoublePendulum is a lightweight real-time double pendulum simulation written in C11 and rendered with SDL2. It is meant to be small enough to read, modify, and use as an educational codebase for numerical integration, simple rendering, and real-time simulation loops.

Current scope is narrow on purpose: RK4 is the only integrator, SDL2 is the only renderer, and most runtime behavior is configured with compile-time macros under `src/config/`.

## Preview

![Single double-pendulum simulation preview](assets/img/single_pendulum.png)

![Multiple double-pendulum simulation preview](assets/img/multiple_pendulums.png)

## Quick Start

Clone with the vendored SDL2 submodule:

```sh
git clone --recurse-submodules <repository-url>
cd DoublePendulum
```

If you already cloned without submodules:

```sh
git submodule update --init --recursive
```

Build and run a debug build for your host platform.

Windows, from PowerShell:

```powershell
cmake --workflow --preset win32-rk4-sdl-debug
.\.build\win32-rk4-sdl-debug\bin\DoublePendulum.exe
```

Linux, from a POSIX shell:

```sh
cmake --workflow --preset posix-rk4-sdl-debug
./.build/posix-rk4-sdl-debug/bin/DoublePendulum
```

Use the matching `*-release` preset for an optimized build.

## Controls

| Input | Action |
| --- | --- |
| Escape | Exit the simulation |
| Window close button | Exit the simulation |

There are no command-line options.

## Configuration

Most behavior is controlled by compile-time macros. Edit the headers in `src/config/`, then rebuild.

Common changes:

| Goal | Edit |
| --- | --- |
| Change the number of pendulums | `TOTAL_PENDULUMS` in `src/config/simulation_config.h` |
| Speed up or slow down simulation time | `SIMULATION_TIME_SCALE` in `src/config/simulation_config.h` |
| Change integration timestep | `SIMULATION_STEPS_PER_SECOND` in `src/config/simulation_config.h` |
| Change starting angles, rod lengths, or masses | `PENDULUM_INIT_MODE` and the default/custom rod macros in `src/config/simulation_config.h` |
| Enable or disable trails | `TRAIL` in `src/config/render_config.h` |
| Change rod or trail thickness | `ROD_WIDTH_PER_MILLE`, `TRAIL_WIDTH_PER_MILLE` in `src/config/render_config.h` |

Configuration files:

| File | Important macros |
| --- | --- |
| `src/config/simulation_config.h` | `PENDULUM_INIT_MODE`, `DEFAULT_*`, `CUSTOM_*`, `GRAVITY_CENTI`, `G`, `SIMULATION_STEPS_PER_SECOND`, `SIMULATION_DT`, `SIMULATION_TIME_SCALE`, `TOTAL_PENDULUMS` |
| `src/config/render_config.h` | `COLOR_DECAY_PER_MILLE`, `COLOR_DECAY_REFERENCE_FPS`, `ROD_WIDTH_PER_MILLE`, `ROD_WIDTH_PIXELS`, `TRAIL`, `TRAIL_WIDTH_PER_MILLE`, `TRAIL_DURATION_MILLISECONDS`, `TRAIL_BUCKET_MILLISECONDS`, `TRAIL_FADE_GAMMA_PER_MILLE` |
| `src/config/app_config.h` | `MIN_SUPPORTED_RENDER_FPS`, `MAX_SIMULATION_STEPS_PER_FRAME`, `THREADPOOL_MIN_ITEMS_PER_JOB` |
| `src/config/config_validation.h` | `_Static_assert` checks for supported ranges and modes |

`PENDULUM_INIT_MODE` selects how initial conditions are created:

| Value | Behavior |
| --- | --- |
| `0` | Both rods use the `DEFAULT_*` values |
| `1` | Rod 1 and rod 2 use separate `CUSTOM_*_ROD1` and `CUSTOM_*_ROD2` values |

When `TOTAL_PENDULUMS` is greater than 1, each pendulum receives a small index-based angle offset from the relevant `*_ANGLE_ADDER` macro. This makes chaotic divergence visible.

## Architecture

Headers live next to implementation files under `src/`; there is no top-level `include/` directory.

```text
.
|-- CMakeLists.txt
|-- CMakePresets.json
|-- assets/
|   |-- icon/
|   `-- img/
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

Main code areas:

| Path | Role |
| --- | --- |
| `src/app/` | SDL setup, window lifecycle, input, frame timing, fixed-step app loop |
| `src/simulation/` | Pendulum state, equations of motion, initialization, integrator interface |
| `src/simulation/integrators/rk4.c` | RK4 integration implementation |
| `src/renderer/` | Renderer-facing frame data |
| `src/renderer/sdl/` | SDL renderer backend, rod batches, trails, color mapping |
| `src/utils/threadpool.*` | Small worker pool used for larger pendulum counts |
| `src/config/` | Compile-time configuration and validation |

The source tree is arranged so more integrators and renderer backends can be added later, but the current supported values are only `rk4` and `sdl`.

## Build System and Presets

The supported build path is intentionally narrow to keep configuration predictable:

- CMake 3.25 or newer.
- Ninja generator.
- GCC C compiler with C11 support.
- Windows or Linux host.
- SDL2 vendored as a Git submodule at `externals/SDL2-2.32.10`.

`CMakeLists.txt` validates the host platform, generator, compiler, integrator, and renderer. It builds SDL2 as a shared library, disables SDL tests, and copies `assets/` into the executable directory after building.

On Windows with MinGW, the configuration may fetch a pinned `jtsiomb/c11threads` revision to provide a C11 `<threads.h>` compatibility layer. On Linux, the project links `Threads::Threads` and the C math library.

Available workflow presets:

| Preset | Host | Build type | Platform | Integrator | Renderer | Output executable |
| --- | --- | --- | --- | --- | --- | --- |
| `win32-rk4-sdl-debug` | Windows | Debug | `win32` | `rk4` | `sdl` | `.build\win32-rk4-sdl-debug\bin\DoublePendulum.exe` |
| `win32-rk4-sdl-release` | Windows | Release | `win32` | `rk4` | `sdl` | `.build\win32-rk4-sdl-release\bin\DoublePendulum.exe` |
| `posix-rk4-sdl-debug` | Linux | Debug | `posix` | `rk4` | `sdl` | `.build/posix-rk4-sdl-debug/bin/DoublePendulum` |
| `posix-rk4-sdl-release` | Linux | Release | `posix` | `rk4` | `sdl` | `.build/posix-rk4-sdl-release/bin/DoublePendulum` |

List presets:

```sh
cmake --workflow --list-presets
```

Run a full configure-and-build workflow:

```sh
cmake --workflow --preset <preset>
```

Release builds use CMake's `Release` configuration and try to enable IPO/LTO when supported by the toolchain.

## Running the Simulation

Run the executable from the matching preset output directory.

Windows:

```powershell
.\.build\win32-rk4-sdl-debug\bin\DoublePendulum.exe
.\.build\win32-rk4-sdl-release\bin\DoublePendulum.exe
```

Linux:

```sh
./.build/posix-rk4-sdl-debug/bin/DoublePendulum
./.build/posix-rk4-sdl-release/bin/DoublePendulum
```

The executable loads `assets/icon/icon.bmp` through a relative path. The build copies the full `assets/` directory into the executable directory, so the copied binary has the icon and preview assets beside it.

## Technical Notes

- Simulation advances in fixed `SIMULATION_DT` steps.
- Real frame time is scaled by `SIMULATION_TIME_SCALE` before fixed steps are consumed.
- `MAX_SIMULATION_STEPS_PER_FRAME` caps catch-up work if rendering falls behind.
- Window title updates with approximate simulation steps per second and renderer FPS.
- Pendulum positions are derived for rendering instead of stored as simulation state.
- Rods and trails are batched through SDL geometry calls.
- Trails are stored in render-target texture buckets and faded by age.
- Worker threads are capped by CPU count and by `THREADPOOL_MIN_ITEMS_PER_JOB`; with the default `TOTAL_PENDULUMS == 5`, the workload stays intentionally small.

## Dependencies

Required:

| Dependency | Notes |
| --- | --- |
| CMake 3.25+ | Uses workflow presets version 6 |
| Ninja | Required generator |
| GCC | Required C compiler |
| Git | Needed for SDL2 submodule setup |
| SDL2 2.32.10 | Vendored under `externals/SDL2-2.32.10` |
| C11 threads support | Native `<threads.h>` where available; MinGW fallback handled during CMake configure |

## Platform Notes

Windows users should use the `win32-*` presets from a shell where `gcc`, `cmake`, and `ninja` are available.

Linux users should use the `posix-*` presets. Other host platforms are currently rejected by the CMake configuration.

## Project Status

DoublePendulum is an educational simulation project, not a general physics engine. Current supported combinations are:

| Area | Current support |
| --- | --- |
| Integrator | RK4 only |
| Renderer | SDL2 only |
| Platforms | Windows and Linux |
| Tests | No project-specific test targets found; vendored SDL tests are disabled by this build |
