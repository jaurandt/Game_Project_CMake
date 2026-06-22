# Quantum Particle in a 2D Infinite Square Well

An interactive 3D visualization of quantum mechanics using OpenGL. This application simulates and displays the probability density of a particle confined in a 2D infinite square well, allowing exploration of quantum superposition states and energy level transitions.

## Overview

This project visualizes the 2D infinite square well problem from quantum mechanics. The height and color of the 3D surface represent the probability density $\psi^2(x, z)$ at each point in the box. Users can:

- **Explore different quantum modes** (energy levels)
- **Create quantum superposition states** by mixing ground and excited states
- **Observe phase interference** as the superposition evolves in time
- **Sample measurements** from the probability distribution
- **Control animation speed** and pause/resume the simulation

### Screenshots
<img width="1497" height="787" alt="image" src="https://github.com/user-attachments/assets/a37fa28c-bb91-43e3-88ec-897f94df5762" />

<img width="1497" height="787" alt="image" src="https://github.com/user-attachments/assets/b2d919ad-6c3c-4ee6-a317-62b03b138e70" />

## Features

- **Real-time 3D rendering** with OpenGL
- **7 quantum modes** with customizable mixing
- **Phase-dependent interference** visualization
- **Interactive mode selection** and state control
- **Quantum measurement sampling** from probability distributions
- **Responsive controls** for exploration and experimentation

## Quick Start

### Prerequisites
- Windows 10+ with OpenGL support
- CMake 3.20+
- C++17 compatible compiler (MSVC recommended)

### Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Run

```bash
./Game_Project.exe
```

## Controls

| Key/Input | Action |
|-----------|--------|
| **Space** / **Right Arrow** | Next quantum mode |
| **Left Arrow** | Previous quantum mode |
| **Up Arrow** | Increase superposition mix (add excited state) |
| **Down Arrow** | Decrease superposition mix |
| **+/−** | Increase/decrease animation speed |
| **Left Mouse Drag** | Charge up and select excited state energy |
| **Right Mouse** / **M** | Sample a measurement from probability distribution |
| **P** | Pause/resume animation |
| **R** | Reset to initial state |
| **ESC** | Exit |

## Quantum Physics Explained

### The 2D Infinite Square Well

A particle confined to a 2D box with infinite potential walls has discrete energy levels characterized by quantum numbers $(n_x, n_z)$:

$$E_{n_x,n_z} = \frac{\hbar^2 \pi^2}{2m} (n_x^2 + n_z^2)$$

The ground state has $(n_x, n_z) = (1, 1)$, and higher energy modes have larger quantum numbers.

### Superposition States

The visualization shows superposition of the ground state and an excited state:

$$\psi(x, z, t) = c_1 \psi_1(x, z) e^{-i E_1 t/\hbar} + c_2 \psi_n(x, z) e^{-i E_n t/\hbar}$$

where the coefficients $(c_1, c_2)$ are controlled via the "mix" parameter.

### Probability Density

The displayed surface height represents $|\psi(x, z, t)|^2$, which gives the probability of finding the particle at position $(x, z)$.

### Phase Interference

The phase difference $\omega t = (E_n - E_1) t / \hbar$ between the ground and excited states creates interference patterns that oscillate in time. The "animation speed" multiplier scales this frequency.

### Measurement

When you perform a measurement, the simulation samples a position from the probability distribution. The yellow crosshair indicates the measured position.

## Architecture

```
OGL3D/                          # Engine library
├── include/OGL3D/
│   ├── Game/                   # Game loop and quantum simulation
│   ├── Graphics/               # OpenGL rendering subsystem
│   ├── Window/                 # Win32 window management
│   └── Math/                   # Vector and matrix utilities
├── source/OGL3D/               # Implementation files
└── vendor/                     # Third-party libraries
    ├── glad/                   # OpenGL loader
    └── stb_img/                # Image loading

Game/
└── main.cpp                    # Application entry point

Assets/
└── Shaders/
    ├── BasicShader.vert        # Vertex shader
    └── BasicShader.frag        # Fragment shader
```

## Quantum Modes

The visualization supports 7 different quantum modes:

| Mode | $(n_x, n_z)$ | Description |
|------|--------------|-------------|
| 0 | (2, 1) | First excited state |
| 1 | (1, 2) | First excited state (orthogonal) |
| 2 | (2, 2) | Two nodes in each direction |
| 3 | (3, 1) | Three nodes in x-direction |
| 4 | (1, 3) | Three nodes in z-direction |
| 5 | (3, 3) | Three nodes in both directions |
| 6 | (4, 2) | Higher energy state |

## Interaction Examples

### Explore Superposition Interference

1. Select a mode (e.g., mode 0)
2. Use **Up arrow** to gradually increase the mix
3. Observe how the probability density changes
4. If paused (press **P**), you'll see just one mode
5. If running, you'll see oscillating interference patterns

### Control the Oscillation Frequency

1. The energy difference between ground and excited states determines the oscillation frequency
2. Use **+/−** to speed up or slow down the animation
3. Different modes will oscillate at different rates (higher energy difference = faster oscillation)

### Perform Measurements

1. Press **M** or right-click to sample a measurement
2. The yellow crosshair appears at the sampled position
3. Sample many times to build a picture of the probability distribution
4. High-probability regions will be sampled more frequently

## Technical Details

- **Language**: C++17
- **Graphics API**: OpenGL 3.3+
- **Rendering**: 56×56 sample grid for smooth surfaces
- **Shaders**: GLSL vertex/fragment shaders
- **Platform**: Windows (Win32 API for windowing and input)
- **Build System**: CMake

## Development

The codebase is organized for clarity and extensibility:

- **Game.cpp** contains the quantum simulation and visualization logic
- **GraphicsEngine** manages all OpenGL state
- **GWindow** abstracts the Win32 window layer
- **ShaderProgram** handles GLSL compilation and linking
- Math utilities (Vec4, Mat4) provide graphics calculations

## Future Enhancements

- 3D infinite cube visualization
- Time-dependent Hamiltonian effects
- Visualization of quantum wavefunctions (not just probability density)
- Export rendered frames as images/video
- Different potential shapes (harmonic oscillator, finite well)

## References

- Griffiths, D. J. (2018). *Introduction to Quantum Mechanics* (3rd ed.)
- Shankar, R. (1994). *Principles of Quantum Mechanics*

## License

(Add your license here)

## Contact

(Add contact information here)
