# Quantum Box Explorer 1.0

Explore a quantum particle in a two-dimensional infinite square well using an
interactive probability-density surface, seven excited modes, superposition
controls, relative-phase animation, and position sampling.

Download `QuantumBoxExplorer-1.0.0-windows-x64.zip`, extract the entire archive,
and run `QuantumBoxExplorer.exe`. Keep `Assets` beside the executable.
Requires 64-bit Windows 10 or newer and a graphics driver supporting OpenGL 4.6.
The MSVC runtime is linked statically; no compiler or CMake installation is needed.

This release includes a persistent inspector, energy and probability readouts,
pause/reset controls, and the latest sampled position. Sampling follows the Born
distribution and leaves the visualized wavefunction unchanged. The surface shows
relative density; it is not a particle trajectory. Animation uses scaled time,
not calibrated physical seconds.

Physics verification covers wall boundaries, normalization, interference,
energy gaps, input limits, and seeded sampling distributions. Windows automation
builds and tests Release binaries, verifies package contents, and publishes a ZIP
with a SHA256 checksum when the `v1.0.0` tag is pushed.

To verify the download in PowerShell, run
`Get-FileHash .\QuantumBoxExplorer-1.0.0-windows-x64.zip -Algorithm SHA256`
and compare it with the accompanying `.sha256` file.
