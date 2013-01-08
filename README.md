# Custom Script Language #
Welcome to the CSL project! It aims to be a useful little tool to easily add non-intrusive scripting to your applications. It does not aim to be the fastest or most feature-filled alternative there is, but instead the goal is to be easy to use and have a small overhead. To get an idea of how the scripts work, here is a quick example:

```c++
int NumberOfMushrooms = 12;
bool ShouldThereBeMushrooms = true;
```

To include and use the above script in your application you only need to include ```Script.cpp``` and ```Script.hpp``` in your project and add this to your program:

```c++
#include "Script.hpp"

Script::Init();
Script::ParseFile("myscript.csl");

int MushNum = Script::GetInt("NumberOfMushrooms", 0);
bool Mush = Script::GetBool("ShouldThereBeMushrooms", false);

Script::CleanUp();		// deletes all funcs and variables from Script
```

Note that you can set default values when fetching data from script-set variables. This makes sure that you still have data even if something went wrong with reading the script. This makes it perfect to use CSL to add translation support to any program; just read a script with string definitions (```string LANG_EXIT = "Exit";```) and load them in your program, providing default strings for each entry.

As written above, we don´t aim for lots of features (as a matter of fact, the project goal was at the beginning to be just a tool to add translation support), but we do have some useful features. Look at this example script to get an idea:

```c++
func SetMushrooms(int Num, bool ShouldBe) {
	NumberOfMushrooms = Num;
	ShouldThereBeMushrooms = ShouldBe;
}
if ("this" == "that"); {
	SetMushrooms((124-18*2), false);
} elseif (false); {
	int dummy = 0;
} else (); {
	SetMushrooms(17, true);
}

/*
	Block comment
*/

func writel(string Var, string Text) {		// used to easily write lines to files
	write(Var, Text, true);
};

file OutFile = "myfile.txt";			// creates the file "myfile.txt"
OutFile:writel("This is written");		// writes to file
OutFile:writel("to a file.");
delete OutFile;					// deletes file handle and closes file
```
