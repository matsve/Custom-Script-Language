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
	
	printf("Listing lists:\n");
	for (std::map<std::string, Script::List>::iterator it = Script::Lists.begin(); it != Script::Lists.end(); it++)
	{
		printf("Found list '%s':\n", it->first.c_str());
		for (unsigned int i = 0; i < it->second.Row.size(); i++)
		{
			for (unsigned int j = 0; j < it->second.Row.at(i).Column.size(); j++)
			{
				printf("  - %s\n", it->second.Row.at(i).Column.at(j).c_str());
			}
			printf("\n");
		}
		printf("\n");
	}

	// Clean up after ourselves. Make sure to not have any pointers left!
	Script::CleanUp();

	return 0;
}
