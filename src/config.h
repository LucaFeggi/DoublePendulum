#ifndef CONFIG_H
#define CONFIG_H


// ----- Physics parameters -----

#define PENDULUM_INIT_MODE 0		// 0 -> default		1 -> custom

// the default rods are both equally initialized, so just one group of defines
#define DEFAULT_ANG_VEL 0.0
#define DEFAULT_ANG_ACC 0.0
#define DEFAULT_LEN 100.0
#define DEFAULT_MASS 10.0
#define DEFAULT_ANGLE 2.4
#define DEFAULT_ANGLE_ADDER 0.001	// used if multiple pendulums are spawned, to make them locate a little far apart

// the 2 custom rods can be different to each other, so 2 groups of defines
#define CUSTOM_ANG_VEL_ROD1 0.0
#define CUSTOM_ANG_ACC_ROD1 0.0
#define CUSTOM_LEN_ROD1 100.0
#define CUSTOM_MASS_ROD1 10.0
#define CUSTOM_ANGLE_ROD1 1.28
#define CUSTOM_ANGLE_ADDER_ROD1 0.001

#define CUSTOM_ANG_VEL_ROD2 0.0
#define CUSTOM_ANG_ACC_ROD2 0.0
#define CUSTOM_LEN_ROD2 50.0
#define CUSTOM_MASS_ROD2 5.0
#define CUSTOM_ANGLE_ROD2 2.28
#define CUSTOM_ANGLE_ADDER_ROD2 0.001

// simulation variables
#define G 9.81
#define DT 1.0/1000.0		// dt for physics update rate (1.0/1000.0 -> simulation updates at 1000Hz)
#define SPEED_FACTOR 3.0	// speed factor for faster pendulum movement

#define TOTAL_PENDULUMS 2


// ----- Performance parameters -----

#define MULTITHREADING_THRESHOLD 1000	// number of pendulums above which CPU multithreading is used (exclusive)
#define THREADPOOL_NUM_THREADS 4		// C11 has no standard API for querying the CPU core count


// ----- Rendering parameters -----

#define RENDER_MODE 0				// 0 -> SDL2 renderer		1 -> Vulkan renderer

#define COLOR_DECAY 0.995			// decay for not having full blue or red pendulum colors 

#define PENDULUM_GLOWING 0			// 0 -> glowing off		1 -> glowing on

#define TRAIL 1						// 1 -> trail on		0 -> trail off
#define TOTAL_TRAIL_SAMPLES 150		// trail_samples = trail_duration_in_seconds * renderer_fps
#define TRAIL_GLOWING 0				// 0 -> glowing off		1 -> glowing on


#endif

/*
DoublePendulum/
├── CMakeLists.txt              # Entry point principale
├── src/
│   ├── core/                   # SEMPRE compilato (Agnostico)
│   │   ├── simulation.c        # Logica fisica
│   │   ├── physics_jobs.c      # Funzioni wrapper per il threadpool
│   │   └── types.h             # real_t (float/double), Pendulum struct
│   ├── platform/               # Scelta da CMake (Hardware dependent)
│   │   ├── desktop/            # Se target == Win/Lin
│   │   │   ├── threadpool_win.c
│   │   │   └── sdl_renderer.c
│   │   ├── esp32/              # Se target == ESP32
│   │   │   ├── threadpool_esp.c
│   │   │   └── display_driver.c
│   │   └── common/             # Interfacce (Header con i prototipi)
│   │       ├── threadpool.h
│   │       └── renderer_interface.h
│   └── app/                    # Il "Game Loop" (Compilato sempre)
│       ├── main.c
│       └── render_bridge.c     # Contiene i Job di preparazione grafica
└── cmake/                      # Script per toolchain (opzionale)
*/
