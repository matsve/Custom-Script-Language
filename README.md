# Custom Script Language #
Welcome to the CSL project! It aims to be a useful little tool to easily add non-intrusive scripting to your applications. It does not aim to be the fastest or most feature-filled alternative there is, but instead the goal is to be easy to use and have a small overhead. To get an idea of how the scripts work, here is a quick example:

'''c++
int NumberOfMushrooms = 12;
bool ShouldThereBeMushrooms = true;
func SetMushrooms(int Num, bool ShouldBe) {
	NumberOfMushrooms = Num;
	ShouldThereBeMushrooms = ShouldBe;
}
if ("this" == "that"); {
	SetMushrooms((124-18*2), false);
} else () {
	SetMushrooms(17, true);
}
'''

To include and use the above script in your application you only need to include '''Script.cpp''' and '''Script.hpp''' in your project and add this to your program:

'''c++
#include "Script.hpp"

Script::Init();
Script::ParseFile("myscript.csl");
Script::CleanUp();

int MushNum = Script::GetInt("NumberOfMushrooms", 0);
bool Mush = Script::GetBool("ShouldThereBeMushrooms", false);
'''

Note that you can set default values when fetching data from script-set variables. This makes sure that you still have data even if something went wrong with reading the script.

More to come in this file...
