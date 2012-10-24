#include "Script.hpp"

int main()
{
	Script::Init();
	Script::ParseFile("init.csl");
	Script::CleanUp();

	return 0;
}
