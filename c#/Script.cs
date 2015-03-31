using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace System.Data.Script
{
    public class Script
    {
        public delegate string ScriptFunction(FunctionCall data);
        public delegate void TypeInit(string name);
        public delegate void TypeAssign(string name, string value);
        public delegate string TypeAsString(string name);
        public delegate void TypeDelete(string name);

        public struct ExternalType
        {
            public string name;
            public TypeInit typeInit;
            public TypeAssign typeAssign;
            public TypeAsString typeAsString;
            public TypeDelete typeDelete;
        }
        private struct Parameter
        {
            public string name, type;
        }
        public struct Variable
	    {
		    public int type;
		    public string value, name; //stringvalue is external type name
		    public ExternalType externalType;
	    }
        public struct FunctionCall
	    {
		    public string functionName, fileName;
		    public List<Variable> variables;
		    public int row;
	    }
        private struct Parenthesis
        {
            public string keyword, memory, oper;
            public bool escape, funcCall, funcCreate;
            public int expect;
            public FunctionCall functionCall;
        }
        private struct Scope
        {
            public bool blockComment, didRun, escape, execNext, execNextScope, lineComment, recordBlock, gotOne, inString;
            public int row, col, recordType;
            public List<Parenthesis> parentheses;
        }
        private struct Function
        {
            public int startCol, startRow, endCol, endRow, minParam, maxParam;
            public string name, fileName;
            public bool isNative;
            public ScriptFunction NativeFunction;
            public List<string> textCode;
            public List<Parameter> parameters;
        }

        private List<Scope> scopes;
        private Dictionary<string, ExternalType> externalTypes;
        private Dictionary<string, Function> functions;
        private Dictionary<string, Variable> variables;
        private Scope currentScope;
        private Parenthesis currentParen;
        const int EXPECT_NONE = 0, EXPECT_PARAM_LIST = 1, EXPECT_VALUE = 2;
        const int TYPE_NONE = 0, TYPE_STRING = 1, TYPE_INT = 2, TYPE_BOOL = 3, TYPE_FLOAT = 4, TYPE_EXTERNAL = 5, TYPE_FUNC = 6;
        static string processChars = " =+-*/&|*:;!<>(),\t\n\r\"";
        static string assignChars = "+-*/&!<>|=";

        public Script()
        {
            scopes = new List<Scope>();
            externalTypes = new Dictionary<string, ExternalType>();
            functions = new Dictionary<string, Function>();
            variables = new Dictionary<string, Variable>();
        }
        public Script(string fileName) : base()
        {
			this.ParseFile(fileName);
        }

        public void BindExternalType(string name, TypeInit typeInit, TypeAssign typeAssign, TypeAsString typeAsString, TypeDelete typeDelete)
        {
            ExternalType t;
            t.name = name;
            t.typeInit = typeInit;
            t.typeAssign = typeAssign;
            t.typeAsString = typeAsString;
            t.typeDelete = typeDelete;
            externalTypes[name] = t;
        }
        public void BindNativeFunction(string name, int minParam, int maxParam, ScriptFunction function)
	    {
		    Function MyFunc = new Function();
		    MyFunc.name = name;
		    MyFunc.isNative = true;
		    MyFunc.NativeFunction = function;
		    MyFunc.minParam = minParam;
		    MyFunc.maxParam = maxParam;
		    functions[name] = MyFunc;
	    }
        bool IsProcessChar(string Char)
        {
            return (processChars.IndexOf(Char) == -1) ? false : true;
        }
        bool IsAssignChar(string Char)
        {
            return (assignChars.IndexOf(Char) == -1) ? false : true;
        }
        bool IsType(string name)
        {
            if (name == "if") return true;
            if (name == "else") return true;
            if (name == "elseif") return true;
            if (name == "return") return true;
            if (name == "int") return true;
            if (name == "string") return true;
            if (name == "bool") return true;
            if (name == "float") return true;
            if (name == "func") return true;
            if (name == "delete") return true;
            if (name == "include") return true;
            if (externalTypes.ContainsKey(name)) return true;
            return false;
        }
        int Str2Type(string type)
	    {
		    if (type == "int") return TYPE_INT;
		    if (type == "bool") return TYPE_BOOL;
		    if (type == "float") return TYPE_FLOAT;
		    if (type == "func") return TYPE_FUNC;
		    if (externalTypes.ContainsKey(type)) return TYPE_EXTERNAL;
		    //if (Type == "string") return TYPE_STRING;
		    return TYPE_STRING;
	    }
        private string Type2Str(int type)
	    {
		    switch (type)
		    {
			    case TYPE_INT:
				    return "int";
			    case TYPE_BOOL:
				    return "bool";
			    case TYPE_FLOAT:
				    return "float";
			    case TYPE_FUNC:
				    return "func";
			    case TYPE_EXTERNAL:
				    return "external";
			    //case TYPE_STRING:
			    //	return "string";
			    default:
				    return "string";
		    }
	    }
        private bool IsFunc(string funcName)
	    {
		    return (functions.ContainsKey(funcName));
	    }
	    private bool RemFunc(string funcName)
	    {
		    if (!functions.ContainsKey(funcName))
		    {
			    return false;
		    }
		    else
		    {
			    functions.Remove(funcName);
			    return true;
		    }
	    }

        private bool IsStr(string inString)
        {
	        return (inString.Substring(0,1) == "\"" && inString.Substring(inString.Count()-1) == "\"") ? true : false;
        }
        private string UnStr(string inString)
        {
	        if (IsStr(inString))
	        {
		        return inString.Substring(1, inString.Count()-2);
	        }
	        else
	        {
		        return inString;
	        }
        }
        private bool IsVar(string varName)
        {
            return (!variables.ContainsKey(varName)) ? false : true;
        }
	    private bool RemVar(string varName)
	    {
		    if (!variables.ContainsKey(varName))
		    {
			    return false;
		    }
		    else
		    {
                if (variables[varName].type == TYPE_EXTERNAL) variables[varName].externalType.typeDelete(varName);
                variables.Remove(varName);
			    return true;
		    }
	    }
        bool SetString(string varName, string value)
        {
            if (!variables.ContainsKey(varName))
            {
                Variable Temp = new Variable();
                Temp.type = TYPE_STRING;
                Temp.value = value;
                variables[varName] = Temp;
                return false;
            }
            else
            {
                Variable tempVar = variables[varName];
                tempVar.value = value;
                variables[varName] = tempVar;
                return true;
            }
        }
        private bool SetVar(string varName, string varType, string value)
	    {
		    /*if (varType == "int")
		    {
			    return SetInt(VarName, Str2Int(Value));
		    }
		    else if (VarType == "bool")
		    {
			    return SetBool(VarName, Str2Bool(Value));
		    }
		    else if (VarType == "float")
		    {
			    return SetFloat(VarName, Str2Float(Value));
		    }
		    else
		    {*/
			    if (externalTypes.ContainsKey(varType))
			    {
				    bool res = SetString(varName, value);
                    Variable tempVar = variables[varName];
                    tempVar.externalType = externalTypes[varType];
				    tempVar.type = TYPE_EXTERNAL;
                    if (!res) tempVar.externalType.typeInit(varName);
				    tempVar.externalType.typeAssign(varName, value);
                    variables[varName] = tempVar;
				    return res;
			    }
			    else if (varType != "string")
			    {
				    // warning message
			    }
			    return SetString(varName, value);
		    //}
	    }
	    void SetVar(string varName, string value)
	    {
		    if (!variables.ContainsKey(varName))
			    return;
		    else
		    {
			    switch(variables[varName].type)
			    {
				    /*case TYPE_INT:
					    Variables[VarName].IntValue = Str2Int(Value);
					    return;
				    case TYPE_BOOL:
					    Variables[VarName].BoolValue = Str2Bool(Value);
					    return;
				    case TYPE_FLOAT:
					    Variables[VarName].FloatValue = Str2Float(Value);
					    return;*/
				    case TYPE_EXTERNAL:
                        variables[varName].externalType.typeAssign(varName, value);
					    return;
				    default:
                        Variable tempVar = variables[varName];
                        tempVar.value = value;
					    variables[varName] = tempVar;
					    return;
			    }
		    }
	    }
        private string GetVar(string varName)
        {
	        if (IsStr(varName))
	        {
		        return UnStr(varName);
	        }
	        else if (variables.ContainsKey(varName))
	        {
		        Variable TVar = variables[varName];
		        switch(TVar.type)
		        {
			        /*case TYPE_INT:
				        return Int2Str(TVar.IntValue);
			        case TYPE_BOOL:
				        return Bool2Str(TVar.BoolValue);
			        case TYPE_FLOAT:
				        return Float2Str(TVar.FloatValue);*/
			        case TYPE_EXTERNAL:
                        return TVar.externalType.typeAsString(varName);
			        default:
				        return TVar.value;
		        }
	        }
	        else
	        {
		        return varName;
	        }
        }
        public string GetString(string name)
        {
            return GetVar(name);
        }
        public int GetInt(string name)
        {
            return (int)Str2Float(GetVar(name));
        }
        public float GetFloat(string name)
        {
            return Str2Float(GetVar(name));
        }
        public bool GetBool(string name)
        {
            return Str2Bool(GetVar(name));
        }

        public string StringParameter(FunctionCall data, int num)
        {
            if (data.variables.Count() >= num && num >= 1)
            {
                return GetVar(data.variables[num - 1].value);
            }
            else
                return "NULL";
        }
        public int IntParameter(FunctionCall data, int num)
        {
            if (data.variables.Count() >= num && num >= 1)
            {
                return (int)Script.Str2Float(GetVar(data.variables[num - 1].value));
            }
            else
                return -1;
        }
        public float FloatParameter(FunctionCall data, int num)
        {
            if (data.variables.Count() >= num && num >= 1)
            {
                return Script.Str2Float(GetVar(data.variables[num - 1].value));
            }
            else
                return -1.0f;
        }

        private void PushParen()
        {
            if (currentScope.parentheses.Count() > 0) currentScope.parentheses[currentScope.parentheses.Count() - 1] = currentParen;
            Parenthesis paren;
            paren.keyword = "";
            paren.memory = "";
            paren.oper = "";
            paren.escape = false;
            paren.funcCall = false;
            paren.funcCreate = false;
            paren.expect = EXPECT_NONE;
            paren.functionCall = new FunctionCall();
            paren.functionCall.fileName = "";
            paren.functionCall.row = 0;
            paren.functionCall.variables = new List<Variable>();
            currentScope.parentheses.Add(paren);
            //if (Msg(MSGL_DEBUG)) printf("Debug at %s: Pushing parenthesis.\n", StringPosition().c_str());
        }
        private void PushScope(bool execScope = true)
        {
            if (scopes.Count() > 0) scopes[scopes.Count() - 1] = currentScope;
            Scope scope = new Scope();
            scope.blockComment = false;
            scope.lineComment = false;
            scope.recordBlock = false;
            scope.inString = false;
            scope.didRun = false;
            scope.escape = false;
            scope.gotOne = false;
            scope.execNextScope = true;
            scope.execNext = execScope;
            scope.row = 0;
            scope.col = 0;
            scope.recordType = 0;
            scope.parentheses = new List<Parenthesis>();
            scopes.Add(scope);
            CurrentScope();
            PushParen();
            CurrentParen();
        }
        private void PopParen()
        {
            if (currentScope.parentheses.Count() > 1)
            {
                currentScope.parentheses.RemoveAt(currentScope.parentheses.Count() - 1);
                CurrentParen();
            }
        }
        private void PopScope()
        {
            if (scopes.Count() > 1)
            {
                scopes.RemoveAt(scopes.Count() - 1);
                CurrentScope();
                CurrentParen();
            }
        }
        private void CurrentScope()
        {
            currentScope = scopes[scopes.Count() - 1];
        }
        private void CurrentParen()
        {
            currentParen = currentScope.parentheses[currentScope.parentheses.Count() - 1];
        }

        public bool ParseString(string data)
        {
            List<string> result = new List<string>();
            result.Add(data);
            return Parse(result);
        }
        public bool ParseFile(string fileName)
        {
            string line = "";
            List<string> result = new List<string>();
            System.IO.StreamReader file = new System.IO.StreamReader(fileName);
            while ((line = file.ReadLine()) != null)
            {
                result.Add(line);
            }
            file.Close();
            return Parse(result);
        }
        public bool Parse(List<string> data)
        {
            string Char;
		    bool DidP = false, DidB = false, DidD = false;
		    int ScopeLevels = scopes.Count();
            PushScope();
            for (currentScope.row = 0; currentScope.row < data.Count(); currentScope.row++)
            {
                for (currentScope.col = 0; currentScope.col < data[currentScope.row].Count(); currentScope.col++)
                {
                    Char = data[currentScope.row].Substring(currentScope.col, 1);
				    DidP = false; DidB = false; DidD = false;
                    if (currentScope.blockComment)
                    {
                        if (currentScope.gotOne)
                        {
                            if (Char == "/")
                            {
                                currentScope.blockComment = false;
							    DidB = true;
                            }
                            else
                                currentScope.gotOne = false;
                        }
                        else if (Char == "*")
                            currentScope.gotOne = true;
                    }
                    else if (currentScope.lineComment)
                    {
                        // dummy, will end automatically at newline
                    }
                    else if (currentScope.inString)
                    {
                        if (Char == "\"" && !currentParen.escape)
                        {
                            currentScope.inString = false;
                            currentParen.keyword = "\"" + currentParen.keyword + "\"";
                        }
                        else if (Char == "\\" && !currentParen.escape)
                        {
                            currentParen.escape = true;
                        }
                        else
                        {
                            currentParen.keyword += Char;
						    currentParen.escape = false;
                        }
                    }
                    else
                    {
                        if (Char == "{")
                        {
						    int TRow = currentScope.row, TCol = currentScope.col;
						    string FName = currentParen.memory;
						    bool Rec = currentScope.recordBlock;//, Ex = CScope.Execnext;
						    PushScope(currentScope.execNextScope);
						    if (Rec)
						    {
                                Function tempf = functions[FName];
                                tempf.startCol = TCol + 1;
                                tempf.startRow = TRow;
                                functions[FName] = tempf;
							    currentScope.execNext = false;
						    }
						    currentScope.row = TRow;
						    currentScope.col = TCol;
						}
                        else if (Char == "}")
                        {
						    int TRow = currentScope.row, TCol = currentScope.col;
						    PopScope();
						    currentScope.row = TRow;
						    currentScope.col = TCol;
						    if (currentScope.recordBlock)
						    {
							    currentScope.recordBlock = false;
                                Function tempf = functions[currentParen.keyword];
							    tempf.endCol = TCol;
							    tempf.endRow = TRow;
							    tempf.textCode = CropStrList(data, functions[currentParen.keyword].startCol, functions[currentParen.keyword].startRow, TCol, TRow);
                                functions[currentParen.keyword] = tempf;
                                Console.WriteLine("--- FUNCTION! ({0}, {1}, {2}, {3})", functions[currentParen.keyword].startCol, functions[currentParen.keyword].startRow, TCol, TRow);
                                foreach (string row in tempf.textCode)
                                {
                                    Console.WriteLine("    : " + row);
                                }
                                Console.WriteLine("--- /FUNCTION!");
						    }
                            currentParen.functionCall = new FunctionCall();
                            currentParen.functionCall.variables = new List<Variable>();
                        }
					    else
					    {
						    if (Char == "/")
						    {
							    if (currentParen.oper == "/")
							    {
								    currentParen.oper = "";
								    currentScope.lineComment = true;
								    DidD = true;
							    }
							    else
							    {
								    if (DidB) currentParen.oper = "";
								    else currentParen.oper = "/";
							    }
						    }
						    else if (Char == "*")
						    {
							    if (currentParen.oper == "/")
							    {
								    currentParen.oper = "";
								    currentScope.blockComment = true;
								    DidD = true;
							    }
							    else
							    {
								    currentParen.oper = "*";
							    }
						    }
						    if (!currentScope.recordBlock && currentScope.execNext)
						    {
							    if (IsProcessChar(Char) && !DidD)
							    {
								    if (IsType(currentParen.memory))
								    {
									    if (currentParen.memory == "if" && Char == ";")
									    {
										    bool BoolParam = Str2Bool(GetVar(currentParen.keyword));
										    currentScope.execNextScope = BoolParam;
										    currentScope.didRun = BoolParam;
									    }
									    else if (currentParen.memory == "else" && Char == ";")
									    {
										    bool Do = currentScope.didRun ? false : true;
										    currentScope.execNextScope = Do;
									    }
									    else if (currentParen.memory == "elseif" && Char == ";")
									    {
										    bool BoolParam = Str2Bool(GetVar(currentParen.keyword));
										    bool DidRun = currentScope.didRun;
								
										    if (DidRun)
										    {
											    currentScope.execNextScope = false;
										    }
										    else
										    {
											    currentScope.execNextScope = BoolParam;
											    currentScope.didRun = BoolParam;
										    }
									    }
									    else if (currentParen.memory == "return" && Char == ";")
									    {
										    string StrVal = GetVar(currentParen.keyword);
										    while (scopes.Count() > ScopeLevels)
										    {
											    PopScope();
										    }
										    currentParen.keyword = StrVal;
										    return true;
									    }
									    else if (currentParen.memory == "func" && Char == "(")
									    {
                                            Console.WriteLine("Creating func '{0}/{1}'", currentParen.memory, currentParen.keyword);
										    string temp = currentParen.keyword;
                                            string temp2 = currentParen.memory;
										    PushParen();
										    DidP = true;
										    currentParen.funcCall = true;
                                            currentParen.functionCall.functionName = temp;
										    currentParen.expect = EXPECT_PARAM_LIST;
                                            currentParen.keyword = "";
                                            currentParen.memory = "";
									    }
									    else if (currentParen.memory == "delete" && Char == ";")
									    {
										    if (IsVar(currentParen.keyword))
										    {
											    RemVar(currentParen.keyword);
										    }
										    else if (IsFunc(currentParen.keyword))
										    {
											    RemFunc(currentParen.keyword);
										    }
										    else
										    {
											    // Cannot remove any more
										    }
									    }
									    else if (currentParen.memory == "include" && Char == ";")
									    {
										    string StrVal = GetVar(currentParen.keyword);
										    ParseFile(StrVal);
									    }
									    else if (currentParen.memory.Count() > 0 && currentParen.keyword.Count() > 0)
									    {
										    if (currentParen.expect == EXPECT_PARAM_LIST)
										    {
                                                //Console.WriteLine("ADDING PARAMETER: '{0} {1}'", currentParen.memory, currentParen.keyword);
											    Variable TempVar = new Variable();
											    TempVar.type = Str2Type(currentParen.memory);
											    TempVar.value = currentParen.keyword;
											    currentParen.functionCall.variables.Add(TempVar);
											    currentParen.memory = "";
										    }
										    else
										    {
                                                //Console.WriteLine("Setting var '{0} {1}'", currentParen.keyword, currentParen.memory);
											    SetVar(currentParen.keyword, currentParen.memory, "");
											    currentParen.memory = currentParen.keyword;
										    }
										    currentParen.keyword = "";
									    }
									    else if (!IsType(currentParen.memory))
									    {
										    //if (Msg(MSGL_WARNING)) printf("Warning at %s: No symbol name specified!\n", StringPosition().c_str());
									    }
								    }
								    else if (currentParen.memory.Count() > 0 && currentParen.keyword.Count() > 0 && currentParen.oper.Count() > 0 && Char != "(" && Char != ":")
								    {
									    string TLeft = currentParen.memory;
									    string TRight = currentParen.keyword;
									    TLeft = GetVar(currentParen.memory);
									    TRight = GetVar(currentParen.keyword);
									    Console.WriteLine("Debug: assigning values '{0}' '{1}' '{2}'", (currentParen.oper == "=" ? currentParen.memory : TLeft), currentParen.oper, TRight);
									    if (currentParen.oper == "=")
									    {
										    SetVar(currentParen.memory, TRight);
									    }
									    else if (currentParen.oper == "+")
									    {
										    currentParen.memory = Float2Str(Str2Float(TLeft) + Str2Float(TRight));
									    }
									    else if (currentParen.oper == "-")
									    {
										    currentParen.memory = Float2Str(Str2Float(TLeft) - Str2Float(TRight));
									    }
									    else if (currentParen.oper == "*")
									    {
										    currentParen.memory = Float2Str(Str2Float(TLeft) * Str2Float(TRight));
									    }
									    else if (currentParen.oper == "/")
									    {
										    currentParen.memory = Float2Str(Str2Float(TLeft) / Str2Float(TRight));
									    }
									    else if (currentParen.oper == "&")
									    {
										    currentParen.memory = "\"" + TLeft + TRight + "\"";
									    }
									    else if (currentParen.oper == "&&")
									    {
										    currentParen.memory = Bool2Str(Str2Bool(TLeft) && Str2Bool(TRight));
									    }
									    else if (currentParen.oper == "||")
									    {
										    currentParen.memory = Bool2Str(Str2Bool(TLeft) || Str2Bool(TRight));
									    }
									    else if (currentParen.oper == ">")
									    {
										    currentParen.memory = Bool2Str(Str2Float(TLeft) > Str2Float(TRight));
									    }
									    else if (currentParen.oper == "<")
									    {
										    currentParen.memory = Bool2Str(Str2Float(TLeft) < Str2Float(TRight));
									    }
									    else if (currentParen.oper == ">=")
									    {
										    currentParen.memory = Bool2Str(Str2Float(TLeft) >= Str2Float(TRight));
									    }
									    else if (currentParen.oper == "<=")
									    {
										    currentParen.memory = Bool2Str(Str2Float(TLeft) <= Str2Float(TRight));
									    }
									    else if (currentParen.oper == "==")
									    {
										    currentParen.memory = Bool2Str(TLeft == TRight);
									    }
									    else if (currentParen.oper == "!=")
									    {
										    currentParen.memory = Bool2Str(TLeft != TRight);
									    }
									    currentParen.keyword = "";
									    currentParen.oper = "";
								    }
								    else if (/*CParen.Memory.size() < 1 &&*/ currentParen.keyword.Count() > 0 && (Char == "(" || Char == ":"))
								    {
									    // start function call
									    //if (Msg(MSGL_DEBUG)) printf("Debug at %s: Starting function call to '%s' with '%s' isf:%s.\n", StringPosition().c_str(), CParen.Keyword.c_str(), Char.c_str(), CParen.Funccall?"true":"false");
									    if (currentParen.funcCall)
									    {
										    if (Char == "(")
										    {
											    DidP = true;
											    currentParen.functionCall.functionName = currentParen.keyword;
											    currentParen.expect = EXPECT_VALUE;
											    currentParen.keyword = "";
										    }
										    else
										    {
											    Variable tvar = new Variable();
											    tvar.type = TYPE_STRING;
											    tvar.value = currentParen.keyword;
											    tvar.name = currentParen.keyword;
                                                currentParen.functionCall.variables.Add(tvar);
                                                currentParen.expect = EXPECT_VALUE;
                                                currentParen.keyword = "";
										    }
									    }
									    else
									    {
										    if (Char == "(")
										    {
											    string TFN = currentParen.keyword;
											    PushParen();
											    DidP = true;
											    currentParen.funcCall = true;
											    currentParen.functionCall.functionName = TFN;
											    currentParen.expect = EXPECT_VALUE;
										    }
										    else
										    {
											    Variable tvar = new Variable();
											    tvar.type = TYPE_STRING;
											    tvar.value = currentParen.keyword;
											    tvar.name = currentParen.keyword;
											    currentParen.keyword = "";
											    PushParen();
											    currentParen.funcCall = true;
											    currentParen.functionCall.variables.Add(tvar);
											    currentParen.expect = EXPECT_NONE;
											    currentParen.expect = EXPECT_VALUE;
										    }
									    }
								    }
								    else if (currentParen.memory.Count() < 1)
								    {
									    if (currentParen.keyword.Count() > 0)
									    {
										    if (currentParen.expect == EXPECT_VALUE)
										    {
                                                //Console.WriteLine("Pushing var: '{0},{1}'", currentParen.keyword, currentParen.memory);
											    Variable TV = new Variable();
											    TV.type = TYPE_NONE;
											    TV.value = GetVar(currentParen.keyword);
											    currentParen.functionCall.variables.Add(TV);
											    currentParen.keyword = "";
										    }
										    else
										    {
											    currentParen.memory = currentParen.keyword;
											    currentParen.keyword = "";
										    }
									    }
								    }
							    }
							    if (!DidD)
							    {
								    if (Char == "+" || Char == "-" || Char == ">" || Char == "<" || Char == "!" || (Char == "*" && !DidB && !DidD) || (Char == "/" && !DidB && !DidD))
								    {
									    currentParen.oper = Char;
								    }
								    else if (Char == "=" || Char == "&" || Char == "|")
								    {
									    if (currentParen.oper.Count() < 2)
										    currentParen.oper += Char;
									    else
										    currentParen.oper = Char;
								    }
								    else if (Char == "\"")
								    {
									    currentScope.inString = true;
								    }
								    else if (Char == "(")
								    {
									    if (!DidP)
										    PushParen();
								    }
								    else if (Char == ")")
								    {
									    if (currentParen.expect == EXPECT_PARAM_LIST)
									    {
										    Function TF = new Function();
                                            TF.parameters = new List<Parameter>();
										    TF.name = currentParen.functionCall.functionName;
                                            Console.WriteLine("Creating function '{0}' with {1} param(s)", TF.name, currentParen.functionCall.variables.Count().ToString());
										    for (int i = 0; i < currentParen.functionCall.variables.Count(); i++)
										    {
                                                Parameter TP = new Parameter(); TP.name = currentParen.functionCall.variables[i].value; TP.type = Type2Str(currentParen.functionCall.variables[i].type);
											    TF.parameters.Add(TP);
										    }
										    TF.isNative = false; TF.NativeFunction = null;
										    TF.fileName = ""; TF.minParam = currentParen.functionCall.variables.Count(); TF.maxParam = currentParen.functionCall.variables.Count();
										    functions[TF.name] = TF;
										    PopParen();
										    currentParen.memory = TF.name;
										    currentParen.oper = "";
										    currentParen.keyword = "";
										    currentScope.recordBlock = true;
                                            currentParen.functionCall = new FunctionCall();
									    }
									    else
									    {
                                            Parenthesis TP = currentParen;
										    PopParen();
                                            currentParen.functionCall = new FunctionCall();
                                            currentParen.functionCall.variables = new List<Variable>();
										    if (TP.funcCall)
										    {
											    if (!functions.ContainsKey(TP.functionCall.functionName))
											    {
												    Console.WriteLine("Warning: tried to call undefined function '{0}'!", TP.functionCall.functionName);
											    }
											    else
											    {
												    if (TP.functionCall.variables.Count() >= functions[TP.functionCall.functionName].minParam && (functions[TP.functionCall.functionName].maxParam == -1 || TP.functionCall.variables.Count() <= functions[TP.functionCall.functionName].maxParam))
												    {
													    if (functions[TP.functionCall.functionName].isNative)
													    {
														    currentParen.keyword = functions[TP.functionCall.functionName].NativeFunction(TP.functionCall);
													    }
													    else
													    {
														    List<Variable> backup = new List<Variable>();
														    for (int i = 0; i < functions[TP.functionCall.functionName].parameters.Count(); i++)
														    {
															    if (variables.ContainsKey(functions[TP.functionCall.functionName].parameters[i].name))
															    {
																    Variable tv = new Variable();
																    tv.name = functions[TP.functionCall.functionName].parameters[i].name;
																    tv.type = Str2Type(functions[TP.functionCall.functionName].parameters[i].type);
																    tv.value = GetVar(functions[TP.functionCall.functionName].parameters[i].name);
																    backup.Add(tv);
															    }
                                                                Variable tempVar = new Variable(); //variables[functions[TP.functionCall.functionName].parameters[i].name];
															    tempVar.type = Str2Type(functions[TP.functionCall.functionName].parameters[i].type);
                                                                variables[functions[TP.functionCall.functionName].parameters[i].name] = tempVar;
															    switch (variables[functions[TP.functionCall.functionName].parameters[i].name].type)
															    {
																    /*case TYPE_BOOL:
																	    Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].BoolValue = Str2Bool(TP.FuncCall.Vars.at(i).StringValue);
																	    break;
																    case TYPE_FLOAT:
																	    Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].FloatValue = Str2Float(TP.FuncCall.Vars.at(i).StringValue);
																	    break;
																    case TYPE_INT:
																	    Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].IntValue = Str2Int(TP.FuncCall.Vars.at(i).StringValue);
																	    break;*/
																    default:
                                                                        tempVar = variables[functions[TP.functionCall.functionName].parameters[i].name];
																	    tempVar.value = TP.functionCall.variables[i].value;
                                                                        variables[functions[TP.functionCall.functionName].parameters[i].name] = tempVar;
																	    break;
															    }
														    }
														    Parse(functions[TP.functionCall.functionName].textCode);
														    for (int i = 0; i < backup.Count(); i++)
														    {
															    SetVar(backup[i].name, Type2Str(backup[i].type), backup[i].value);
														    }
													    }
												    }
												    else
												    {
													    Console.WriteLine("Warning: wrong parameter count for '{0}', {1} entered, {2} to {3} expected!\n", TP.functionCall.functionName, TP.functionCall.variables.Count(), functions[TP.functionCall.functionName].minParam, functions[TP.functionCall.functionName].maxParam);
                                                        for (int j = 0; j < TP.functionCall.variables.Count(); j++)
                                                        {
                                                            Console.WriteLine("    {0}", TP.functionCall.variables[j].value);
                                                        }
												    }
											    }
										    }
										    else
                                                currentParen.keyword = TP.memory;
									    }
								    }
								    else if (Char == ";")
								    {
                                        currentParen.memory = "";
                                        currentParen.keyword = "";
                                        currentParen.oper = "";
                                        currentParen.expect = EXPECT_NONE;
								    }
								    else if (Char == ",")
								    {
                                        currentParen.memory = "";
                                        currentParen.keyword = "";
                                        currentParen.oper = "";
								    }
								    else
								    {
									    if (!IsProcessChar(Char))
									    {
                                            currentParen.keyword += Char;
									    }
								    }
							    }
						    }
					    }
                    }
                }
                // On new line, line comment will be false
                currentScope.lineComment = false;
                currentParen.oper = "";
            }
            PopScope();
            return true;
        }

        List<string> CropStrList(List<string> list, int x, int y, int toX, int toY)
        {
            List<string> result = new List<string>();
            for (int Row = y; Row <= toY; Row++)
            {
                string rw = "";
                for (int Col = (Row==y?x:0); Col < (Row==toY?toX:list[Row].Count()); Col++)
                {
                    rw += list[Row].Substring(Col,1);
                }
                result.Add(rw);
            }
            return result;
        }

        public static bool Str2Bool(string value)
        {
            return value == "true" || value == "True" || value == "yes" || value == "Yes";
        }
        public static string Bool2Str(bool value)
        {
            return value ? "true" : "false";
        }
        public static string Float2Str(float value)
        {
            return value.ToString();
        }
        public static float Str2Float(string value)
        {
            float result = 0;
            if (float.TryParse(value, out result))
            {
                return float.Parse(value);
            }
            else
                return 0;
        }
    }
}
