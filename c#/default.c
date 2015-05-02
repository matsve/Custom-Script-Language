int def = 12;

int mos = (def + 7);

string test = ("Testing string" & " lurk");
callme("mos", test);


file MyFile = "file.txt";

func boo(string li, string mo) {
    MyFile:writel(li, mo);
};

MyFile:writel("Data", "Data2", "Data3");
boo("boo1");
boo("boo2", "boo3");
boo("boo2", "boo3");
boo("boo2", "boo3");
boo("boo2", "boo3");
MyFile:writel("Data4", "Data5", "Data6");

delete MyFile;
