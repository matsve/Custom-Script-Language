using System.Data.Script;
using System.Collections.Generic;
using System.IO;

public static class MainClass
{
	public static void Main(string[] args)
	{
		//var script = new Script("default.c");
		var script = new Script();
		script.BindNativeFunction("callme", 0, -1, CallMe);
		script.BindNativeFunction("write", 2, -1, Write);
		script.BindNativeFunction("writel", 2, -1, WriteLine);
		script.BindExternalType("file", FileInit, FileAssign, FileAsString, FileDelete);

		script.ParseFile("default.c");

		System.Console.Write("Num: " + script.GetInt("def") + " " + script.GetString("test"));
		System.Console.ReadKey();

		return;
	}

	public static string CallMe(Script.FunctionCall data) {
		System.Console.WriteLine("Writing to the console");
		foreach (var v in data.variables) {
			System.Console.WriteLine("Arg: " + v.value);
		}
		return "";
	}

	public static Dictionary<string, StreamWriter> Files = new Dictionary<string, StreamWriter>();
	public static string Write(Script.FunctionCall data) {
		var fileName = "";
		var i = 0;
		foreach (var v in data.variables) {
			if (i == 0) {
				fileName = v.value;
			}
			else {
				Files[fileName].Write(v.value);
				//Files[fileName].
			}
			i++;
		}
		return "";
	}
	public static string WriteLine(Script.FunctionCall data) {
		var fileName = "";
		var i = 0;
		foreach (var v in data.variables) {
			if (i == 0) {
				fileName = v.value;
			}
			else {
				Files[fileName].WriteLine(v.value);
				//Files[fileName].
			}
			i++;
		}
		return "";
	}
	public static void FileInit(string name) {
		try {
			Files[name] = null;
			System.Console.WriteLine("!! File init '"+name+"'");
		} catch {
			System.Console.WriteLine("ERROR!");
		}
	}
	public static void FileAssign(string name, string value) {
		try {
			if (value != string.Empty) {
				System.Console.WriteLine("Opening file '"+name+"' at '"+value+"'");
				var file = System.IO.File.Open(value, FileMode.OpenOrCreate);
				Files[name] = new StreamWriter(file);
			}
		} catch {
			System.Console.WriteLine("ERROR!");
		}
	}
	public static string FileAsString(string name) {
		return name;
	}
	public static void FileDelete(string name) {
		Files[name].Close();
		Files.Remove(name);
	}
}
