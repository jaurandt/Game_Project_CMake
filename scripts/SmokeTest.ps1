param([string]$Executable = "$PSScriptRoot/../build-release/Release/QuantumBoxExplorer.exe") # Allow checking any built or extracted executable.
$ErrorActionPreference = 'Stop' # Fail the smoke test on every unexpected error.
# Compile the native window inspection helper used by the runtime assertions.
Add-Type @'
using System; // Represent native handles and callbacks.
using System.Text; // Retrieve inspector text safely.
using System.Runtime.InteropServices; // Call Windows APIs without extra dependencies.
public static class ExplorerSmoke { // Keep native helpers scoped to the runtime check.
    public delegate bool Callback(IntPtr window, IntPtr parameter); // Visit inspector controls.
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr FindWindow(string cls, string title); // Locate the initialized application window.
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr window, Callback callback, IntPtr parameter); // Enumerate inspector text and buttons.
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr window, StringBuilder text, int capacity); // Read live inspector values.
    [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr window, uint message, IntPtr word, IntPtr data); // Exercise the native controls and clean shutdown.
    [DllImport("user32.dll")] public static extern IntPtr GetDlgItem(IntPtr window, int id); // Locate the inspector action receiver.
    [DllImport("kernel32.dll", SetLastError=true)] public static extern bool GetExitCodeProcess(IntPtr process, out uint exitCode); // Read the exit status from a retained process handle.
} // Finish native inspection helpers.
'@ # End the native window inspection helper.
function Get-InspectorText([IntPtr]$Window) { # Read the state shown to the user.
    $script:inspectorText = [System.Collections.Generic.List[string]]::new() # Collect all child-window text.
    $callback = [ExplorerSmoke+Callback]{ param($child, $unused) $buffer = [System.Text.StringBuilder]::new(2048); [void][ExplorerSmoke]::GetWindowText($child, $buffer, 2048); $script:inspectorText.Add($buffer.ToString()); return $true } # Gather labels without modifying their state.
    [void][ExplorerSmoke]::EnumChildWindows($Window, $callback, [IntPtr]::Zero) # Visit all inspector descendants.
    return $script:inspectorText -join "`n" # Return a readable snapshot for assertions.
} # Finish inspector text inspection.
function Assert-Text([IntPtr]$Window, [string]$Expected) { # Require observable runtime behavior.
    $deadline = [DateTime]::UtcNow.AddSeconds(5) # Allow asynchronous rendering to consume actions.
    do { $text = Get-InspectorText $Window; if ($text.Contains($Expected)) { Write-Output "Verified: $Expected"; return }; Start-Sleep -Milliseconds 100 } while ([DateTime]::UtcNow -lt $deadline) # Wait briefly for the next rendered frame.
    throw "Expected '$Expected' in inspector; got: $text" # Report live UI state when a behavior fails.
} # Finish observable-state assertions.
$resolvedExecutable = (Resolve-Path -LiteralPath $Executable).Path # Resolve a concrete binary before launching.
$outputDirectory = Split-Path -Parent $resolvedExecutable # Store diagnostics beside the tested binary.
$process = Start-Process -FilePath $resolvedExecutable -WorkingDirectory $PSScriptRoot -WindowStyle Hidden -PassThru -RedirectStandardOutput "$outputDirectory/smoke.stdout.log" -RedirectStandardError "$outputDirectory/smoke.stderr.log" # Test asset loading from a different working directory.
$processHandle = $process.Handle # Retain the native handle before PowerShell loses exit-status access after shutdown.
try { # Ensure the test-owned application is closed after verification.
    $deadline = [DateTime]::UtcNow.AddSeconds(60); $window = [IntPtr]::Zero # Allow first-launch graphics-driver initialization and shader compilation.
    do { $process.Refresh(); if ($process.HasExited) { throw "Application exited during startup: $(Get-Content "$outputDirectory/smoke.stdout.log" -Raw)" }; $window = [ExplorerSmoke]::FindWindow('OpenGLWindowClass', 'Quantum Box Explorer 1.0'); if ($window -ne [IntPtr]::Zero) { break }; Start-Sleep -Milliseconds 100 } while ([DateTime]::UtcNow -lt $deadline) # Use the explicit class because PowerShell marshals null strings as empty strings.
    if ($window -eq [IntPtr]::Zero) { throw "Initialized application window did not appear; startup log: $(Get-Content "$outputDirectory/smoke.stdout.log" -Raw)" } # Report the last completed initialization stage.
    Assert-Text $window 'RUNNING' # Require the render loop to update the inspector.
    $panel = [ExplorerSmoke]::GetDlgItem($window, 100) # Target the inspector's actual command handler.
    [void][ExplorerSmoke]::SendMessage($panel, 0x111, [IntPtr]1, [IntPtr]::Zero); Assert-Text $window 'PAUSED' # Exercise pause through the native control path.
    [void][ExplorerSmoke]::SendMessage($panel, 0x111, [IntPtr]4, [IntPtr]::Zero); Assert-Text $window '(1, 2)' # Exercise mode navigation.
    [void][ExplorerSmoke]::SendMessage($panel, 0x111, [IntPtr]6, [IntPtr]::Zero); Assert-Text $window 'Mix amplitude: 0.50' # Exercise superposition changes.
    [void][ExplorerSmoke]::SendMessage($panel, 0x111, [IntPtr]9, [IntPtr]::Zero); Assert-Text $window 'Sample #1' # Require successful production measurement sampling.
    [void][ExplorerSmoke]::SendMessage($panel, 0x111, [IntPtr]2, [IntPtr]::Zero); Assert-Text $window 'No position sampled yet.'; Assert-Text $window 'RUNNING' # Exercise reset and resumed rendering.
    [void][ExplorerSmoke]::SendMessage($window, 0x10, [IntPtr]::Zero, [IntPtr]::Zero) # Request the same shutdown as closing the window.
    if (!$process.WaitForExit(5000)) { throw 'Application did not close normally' } # Detect shutdown hangs.
    [uint32]$exitCode = 0; if (![ExplorerSmoke]::GetExitCodeProcess($processHandle, [ref]$exitCode)) { throw 'Unable to read application exit status' } # Query the retained native handle after shutdown.
    if ($exitCode -ne 0) { throw "Application exited with code $exitCode" } # Require successful resource teardown.
    Write-Output 'Runtime smoke test passed: startup, rendering loop, pause, mode, mix, sampling, reset, and clean exit.' # Summarize the verified behavior.
} finally { # Clean up only the process started by this test.
    $process.Refresh(); if (!$process.HasExited) { Stop-Process -Id $process.Id } # Avoid leaving a failed test running.
} # Finish application lifecycle verification.
