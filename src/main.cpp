#include "Script.hpp"

int main(int argc, char *args[])
{
	// Initialize script
	Script::Init();

	// Try to parse every file added to the command line, else use "init.csl"
	if (argc > 1)
	{
		for (int i = 0; i < argc; i++)
		{
			Script::ParseFile(args[i]);
		}
	}
	else
	{
		Script::ParseFile("init.csl");
	}

	// Clean up after ourselves. Make sure to not have any pointers left!
	Script::CleanUp();

	return 0;
}
