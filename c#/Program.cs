using System.Data.Script;

public static class MainClass
{
	public static void Main(string[] args)
	{
		System.Console.ReadKey();
	
		//var script = new Script("default.c");
		var script = new Script();
		script.ParseFile("default.c");
		
		System.Console.Write("Num: " + script.GetInt("def"));
		System.Console.ReadKey();
		return;
	}
}
