#include <OGL3D/Game/Game.h>
#include <iostream>
#include <filesystem> // Resolve shipped assets independently of the launch directory.
#include <windows.h> // Locate the running Windows executable.

int main() 
{
	std::cout << "Quantum Box Explorer 1.0" << std::endl; // Identify the released application in diagnostic output.

	try
	{
		wchar_t executablePath[32768] = {}; // Reserve the Windows maximum extended path length.
		const DWORD length = GetModuleFileNameW(nullptr, executablePath, 32768); // Locate the executable rather than the caller's directory.
		if (length == 0 || length >= 32768) {
			throw std::runtime_error("Unable to locate the executable"); // Reject unusable or truncated paths.
		}

		std::filesystem::current_path(std::filesystem::path(executablePath).parent_path()); // Find the packaged Assets folder after any launch method.
		
		Game game; // default Game constructor
		
		std::cout << "Graphics context initialized." << std::endl; // Identify successful driver startup in runtime diagnostics.
		game.Run();
	}
	catch (const std::exception& e)
	{
		std::cout << "An error occurred: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}
