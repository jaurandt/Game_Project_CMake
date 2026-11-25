#include <OGL3D/Physics/QuantumBox.h> // Exercise production physics without creating a window.
#include <iostream> // Report failures to CTest.
#include <stdexcept> // Fail checks in both Debug and Release.
#include <string> // Select independent test groups.

using namespace QuantumBox; // Keep numerical checks focused on model invariants.

void require(bool ok, const char* message) { 
    if (!ok) 
        throw std::runtime_error(message); 
} // Retain checks with NDEBUG enabled.

void near(double actual, double expected, double tolerance, const char* message) { 
    require(std::abs(actual - expected) <= tolerance, message); 
} // Check numerical errors explicitly.

double integrate(int mode, double mix, double phase, double xmax = 1.0) { // Independently integrate using midpoint quadrature.
    double sum = 0.0; // Accumulate the density over the requested region.
    constexpr int steps = 240; // Resolve every supported eigenstate accurately.
    
    for (int x = 0; x < steps; ++x) { // Traverse horizontal integration cells.
        for (int z = 0; z < steps; ++z) {
            sum += density(xmax * (x + 0.5) / steps, (z + 0.5) / steps, mode, mix, phase); // Evaluate cell centers.
        }
    } // Finish the horizontal integral.

    return xmax * sum / (steps * steps); // Apply the area of each cell.
} // Finish independent numerical integration.

void run(const std::string& group) { // Dispatch named physical invariants.
    if (group == "boundaries") { // Verify confinement and the analytic ground state.
        for (int m = 0; m < ModeCount; ++m) { // Cover every selectable state.
            for (double t : {0.0, 0.21, 0.5, 0.79, 1.0}) { // Check nodes along all four walls.
                for (double a : {0.0, 0.45, 0.95}) {
                    require(density(0, t, m, a, 1.7) == 0 && density(1, t, m, a, 1.7) == 0 && density(t, 0, m, a, 1.7) == 0 && density(t, 1, m, a, 1.7) == 0, "wall density must vanish"); // Require exact boundaries.
                }
            } // Finish wall coordinates.
        } // Finish supported modes.
        near(density(0.5, 0.5, 0, 0, 0), 4.0, 1e-12, "ground-state central density"); // Check normalized analytic peak.
        near(density(0.25, 0.5, 0, 0, 0), 2.0, 1e-12, "ground-state quarter density"); // Check another analytic point.
        require(density(-0.1, 0.5, 0, 0.4, 0) == 0 && density(0.5, 1.1, 0, 0.4, 0) == 0, "density outside box"); // Verify confinement outside the walls.
    } else if (group == "normalization") { // Verify integrated probability and positivity.
        for (int m = 0; m < ModeCount; ++m) { // Check the full supported spectrum.
            for (double a : {0.0, 0.45, 0.95}) { // Check pure ground and mixed states.
                for (double p : {0.0, Pi / 2, Pi, 5.2}) { // Check constructive, quadrature, and destructive phases.
                    near(integrate(m, a, p), 1.0, 1e-10, "probability must integrate to one"); // Test normalized orthogonal superpositions.
                    for (int x = 0; x <= 50; ++x) 
                        for (int z = 0; z <= 50; ++z) 
                            require(density(x / 50.0, z / 50.0, m, a, p) >= 0 && density(x / 50.0, z / 50.0, m, a, p) <= 8.0, "density outside rejection bound"); // Check nonnegativity and sampling envelope.
                } // Finish phase coverage.
            } // Finish amplitude coverage.
        } // Finish mode coverage.
    } else if (group == "interference") { // Test coherent addition against independent analytic values.
        const double a = std::sqrt(0.5), b = std::sqrt(2.0); // Ground and excited eigenfunctions at u=1/4,v=1/2 with equal weights.
        near(density(0.25, 0.5, 0, a, 0), std::pow(1.0 + b, 2), 1e-12, "constructive interference"); // Compare the squared sum of amplitudes.
        near(density(0.25, 0.5, 0, a, Pi), std::pow(1.0 - b, 2), 1e-12, "destructive interference"); // Compare the squared difference of amplitudes.
        for (int m = 0; m < ModeCount; ++m) { // Check phase invariants for all eigenstates.
            near(density(0.23, 0.61, m, 0.45, 0.8), density(0.23, 0.61, m, 0.45, 0.8 + 2 * Pi), 1e-12, "phase periodicity"); // Require one full relative phase cycle.
            near(density(0.23, 0.61, m, 0, 0), density(0.23, 0.61, m, 0, 2.7), 1e-12, "stationary ground density"); // A single eigenstate cannot oscillate in density.
        } // Finish interference invariants.
    } else if (group == "energy") { // Compare the known square-well energy spectrum.
        const double expected[] = {2.5, 2.5, 4.0, 5.0, 5.0, 9.0, 10.0}; // Specify independently derived mode-to-ground ratios.
        for (int m = 0; m < ModeCount; ++m) { // Cover every selected mode.
            near(energyRatio(m), expected[m], 1e-12, "energy ratio"); // Verify the discrete energy levels.
            near(phaseRate(m), Pi * Pi * (expected[m] - 1), 1e-12, "relative phase rate"); // Verify the gap used by animation.
        } // Finish energy coverage.
    } else if (group == "sampling") { // Compare deterministic samples with integrated Born probabilities.
        std::mt19937 rng(20260930); // Make distribution failures repeatable.
        for (int mode : {0, 2, 6}) { // Include degenerate first, nodal, and high-energy modes.
            for (double mix : {0.0, 0.45, 0.95}) { // Include ground and strong superposition distributions.
                for (double phase : {0.0, Pi}) { // Verify constructive and destructive interference sampling.
                    constexpr int count = 20000; int left = 0; // Count positions in a fixed quarter-box region.
                    for (int i = 0; i < count; ++i) { // Draw enough samples to detect biased distributions.
                        const auto point = sample(mode, mix, phase, rng); // Exercise the production rejection sampler.
                        require(point.first > 0 && point.first < 1 && point.second > 0 && point.second < 1, "sample outside box"); // Require valid normalized coordinates.
                        if (point.first < 0.25) 
                            ++left; // Count samples in the integration region.
                    } // Finish independent draws.
                    const double probability = integrate(mode, mix, phase, 0.25); // Compute the expected probability independently.
                    const double tolerance = 6 * std::sqrt(probability * (1 - probability) / count) + 0.001; // Allow six standard deviations and quadrature error.
                    near(left / double(count), probability, tolerance, "samples disagree with Born distribution"); // Detect bias and incorrect phase dependence.
                } // Finish phase coverage.
            } // Finish amplitude coverage.
        } // Finish sampling modes.
    } else if (group == "validation") { // Check public input limits and invalid modes.
        near(density(0.2, 0.3, 0, -1,0), density(0.2, 0.3, 0, 0, 0), 0, "lower amplitude clamp"); // Keep negative controls at ground state.
        near(density(0.2, 0.3, 0, 2, 0), density(0.2, 0.3, 0, 0.95, 0), 0, "upper amplitude clamp"); // Match the maximum UI amplitude.
        for (int m : {-1, ModeCount}) { // Exercise both invalid mode boundaries.
            bool caught = false; 
            try { 
                density(0.2, 0.3, m, 0.4, 0); 
            } 
            catch (const std::out_of_range&) { 
                caught = true; 
            } // Require checked mode indexing.
            require(caught, "invalid mode accepted"); // Fail explicitly if an invalid mode slips through.

            std::mt19937 rng(42); // Start from a reproducible random engine state.
            const auto originalRng = rng; // Keep a copy to detect any rejected draw.
            bool sampleRejected = false; // Track rejection independently of the density check.
            try { 
                sample(m, 0.4, 0.0, rng); 
            } // Attempt a sample with the same invalid mode.
            catch (const std::out_of_range&) { 
                sampleRejected = true; 
            } // Require the sampler to report the invalid mode.
            require(sampleRejected, "sampler accepted an invalid mode"); // Verify the sampling API rejects invalid modes.
            require(rng == originalRng, "invalid sample advanced the random engine"); // Verify rejection happens before any random draw.
        } // Finish mode validation.
    } else 
        throw std::runtime_error("unknown test group"); // Avoid silently passing misspelled CTest registrations.
} // Finish the invariant dispatcher.

int main(int argc,char** argv) { 
    // Provide a small CTest-compatible executable.
    try { 
        require(argc == 2, "expected test group"); 
        run(argv[1]); std::cout << argv[1] << " passed\n"; 
        return 0; 
    } // Return success only after all checks execute.
    catch (const std::exception& error) { 
        std::cerr << error.what() << '\n'; 
        return 1; 
    } // Surface numerical failures as nonzero test results.
} // Finish the physics test entry point.
