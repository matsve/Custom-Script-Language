using System.Data.Script;

public static class MainClass
{
	public static void Main(string[] args)
	{
		//var script = new Script("default.c");
		var script = new Script();
		script.BindNativeFunction("callme", 0, -1, CallMe);
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
}
