#pragma once // Keep the shared physics definitions included once.
#include <algorithm> // Clamp amplitudes and roundoff in the density.
#include <array> // Store the seven selectable quantum states.
#include <cmath> // Evaluate eigenfunctions and relative phases.
#include <random> // Sample positions with an injectable random engine.
#include <utility> // Return sampled normalized coordinates together.

namespace QuantumBox { // Share one model between the renderer and tests.
    inline constexpr double Pi = 3.14159265358979323846; // Use double precision for normalized physics.
    inline constexpr std::array<int, 7> ModeNx = {2, 1, 2, 3, 1, 3, 4}; // Define the horizontal quantum numbers.
    inline constexpr std::array<int, 7> ModeNz = {1, 2, 2, 1, 3, 3, 2}; // Define the matching vertical quantum numbers.
    inline constexpr int ModeCount = static_cast<int>(ModeNx.size()); // Keep navigation aligned with the model.
    
    inline double energyRatio(int mode) { // Express excited energy relative to the ground energy.
        const int nx = ModeNx.at(mode), nz = ModeNz.at(mode); // Reject invalid modes before indexing.
        return 0.5 * (nx * nx + nz * nz); // Divide the mode sum by the ground sum of two.
    } // Finish the dimensionless energy calculation.
    
    inline double phaseRate(int mode) { // Use units with hbar, mass, and box length equal to one.
        return Pi * Pi * (energyRatio(mode) - 1.0); // Compute the excited minus ground angular frequency.
    } // Finish the relative phase calculation.
    
    inline double density(double u, double v, int mode, double mix, double phase) { // Return density normalized on the unit square.
        const int nx = ModeNx.at(mode), nz = ModeNz.at(mode); // Validate the selected eigenstate.
        if (u <= 0.0 || u >= 1.0 || v <= 0.0 || v >= 1.0) return 0.0; // Enforce exact infinite-wall boundaries.
        const double excited = std::clamp(mix, 0.0, 0.95); // Match the amplitude limits of the controls.
        const double ground = std::sqrt(1.0 - excited * excited); // Preserve total eigenstate probability.
        const double a = 2.0 * ground * std::sin(Pi * u) * std::sin(Pi * v); // Normalize the ground eigenfunction.
        const double b = 2.0 * excited * std::sin(nx * Pi * u) * std::sin(nz * Pi * v); // Normalize the excited eigenfunction.
        return std::max(0.0, a * a + b * b + 2.0 * a * b * std::cos(phase)); // Include interference without negative roundoff.
    } // Finish the probability density calculation.
    
    inline std::pair<double, double> sample(int mode, double mix, double phase, std::mt19937& rng) { // Allow deterministic sampling tests.
        static_cast<void>(ModeNx.at(mode)); // Reject an invalid mode before consuming any random values.
        std::uniform_real_distribution<double> unit(0.0, 1.0); // Propose uniform positions and acceptance thresholds.
        for (;;) { // Continue until acceptance to avoid a biased fallback position.
            const double u = unit(rng), v = unit(rng); // Propose a position within the unit square.
            if (8.0 * unit(rng) < density(u, v, mode, mix, phase)) return {u, v}; // Eight bounds all normalized superposition densities.
        } // Retry rejected proposals without changing the quantum state.
    } // Finish unbiased position sampling.
} // Close the shared physics namespace.
