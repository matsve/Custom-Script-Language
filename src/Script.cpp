#include "Script.hpp"

namespace Script
{
    std::map<std::string, File> Files;
    std::vector<Scope> Scopes;
    int MessageLevel = MSGL_INFO;
	std::map<std::string, Variable> Variables;
	std::map<std::string, Function> Functions;
	std::map<std::string, ExternalType> ExternalTypes;

    void Init()
    {
        // Make sure we start off clean
        CleanUp();
		// Setup default scope, which should not be removed
		// until breaking down script class
        PushScope();
        BindExternalType("file", &File_Init, &File_Assign, &File_AsString, &File_Delete);
    }
    void SetMessageLevel(int nMessageLevel)
    {
        MessageLevel = nMessageLevel;
        if (MessageLevel <= MSGL_INFO)
            printf("MessageLevel set to %d\n", nMessageLevel);
    }
    bool Msg(int sMsgl)
    {
        return (MessageLevel <= sMsgl) ? true : false;
    }
    void CleanUp()
    {
		printf("vars[%d]\n", Variables.size());
        Scopes.clear();
        Files.clear();
    }

    void PushScope(bool ExecScope)
    {
        Scope nScope;
        nScope.Blockcomment = false;
        nScope.Col = 0;
        nScope.DidRun = false;
        nScope.Escape = false;
        nScope.Execnext = ExecScope;
		nScope.ExecnextScope = true;
        nScope.Linecomment = false;
        nScope.Parentheses.clear();
        nScope.RecordBlock = false;
        nScope.Gotone = false;
        nScope.RecordType = 0;
        nScope.Row = 0;
        nScope.String = false;
        Scopes.push_back(nScope);
        PushParen();
        //if (Msg(MSGL_DEBUG)) printf("Debug at %s: Pushing scope.\n", StringPosition().c_str());
    }
    void PopScope()
    {
        if (Scopes.size() > 1)
            Scopes.pop_back();
        else
            if (Msg(MSGL_WARNING)) printf("Warning at %s! Tried to pop last scope!\n", StringPosition().c_str());
    }
    void PushParen()
    {
        Parenthesis nP;
        nP.Keyword = "";
        nP.Memory = "";
        nP.Operator = "";
        nP.Escape = false;
        nP.Funccall = false;
        nP.Funccreate = false;
        nP.Expect = EXPECT_NONE;
        CScope.Parentheses.push_back(nP);
        //if (Msg(MSGL_DEBUG)) printf("Debug at %s: Pushing parenthesis.\n", StringPosition().c_str());
    }
    void PopParen()
    {
        if (CScope.Parentheses.size() > 1)
            CScope.Parentheses.pop_back();
        else
            if (Msg(MSGL_WARNING)) printf("Warning at %s! Tried to pop last parenthesis!\n", StringPosition().c_str());
    }
    void BindExternalType(std::string newName, void (*newInit)(std::string Name), void (*newAssign)(std::string Name, std::string Value), std::string (*newAsString)(std::string Name), void (*newDelete)(std::string Name))
    {
    	ExternalType tmp;
    	tmp.Init = newInit;
    	tmp.Assign = newAssign;
    	tmp.AsString = newAsString;
    	tmp.Delete = newDelete;
    	ExternalTypes[newName] = tmp;
	}
	void BindNativeFunction(std::string Name, int MinParam, int MaxParam, std::string (*NewFunction)(FunctionCall Data))
	{
		printf("Adding function '%s'\n",Name.c_str());
		Function MyFunc;
		MyFunc.Name = Name;
		MyFunc.IsNative = true;
		MyFunc.NativeFunction = NewFunction;
		MyFunc.MinParam = MinParam;
		MyFunc.MaxParam = MaxParam;
		Functions[Name] = MyFunc;
	}
	void BindUserFunction(std::string Name, std::vector<Parameter> Params, std::vector<std::string> TextCode)
	{
		Function MyFunc;
		MyFunc.Name = Name;
		MyFunc.IsNative = false;
		MyFunc.Parameters = Params;
		MyFunc.TextCode = TextCode;
		MyFunc.MinParam = Params.size();
		MyFunc.MaxParam = -1;
		Functions[Name] = MyFunc;
	}
	bool IsFunc(std::string FuncName)
	{
		return (Functions.find(FuncName) != Functions.end());
	}
	bool RemFunc(std::string FuncName)
	{
		if (Functions.find(FuncName) == Functions.end())
		{
			return false;
		}
		else
		{
			Functions.erase(FuncName);
			return true;
		}
	}

    bool ParseString(std::string String)
    {
        std::vector<std::string> Data;
        Data.push_back(String);
        bool result = Parse(Data);
        return result;
    }
    bool ParseFile(std::string Filename, bool Forcereload)
    {
        // Read a new file and cache it even if it already exists
        if (Forcereload || Files.find(Filename) == Files.end())
        {
            std::vector<std::string> Data;
            std::string Line;
            std::ifstream File(Filename.c_str());
            if (File.is_open())
            {
                while (!File.eof())
                {
                    getline(File, Line);
                    Data.push_back(Line);
                }
                File.close();
            } else return false;
            // Cache the file
            Files[Filename].Data = Data;
            return Parse(Data);
        }
        // Parse an already cached file
        else if (Files.find(Filename) != Files.end())
        {
            return Parse(Files[Filename].Data);
        }
        else return false;
    }
    bool Parse(std::vector<std::string> Data)
    {
        std::string Char;
		bool DidP = false, DidB = false, DidD = false;
		int ScopeLevels = Scopes.size();
        PushScope();
        for (CScope.Row = 0; CScope.Row < Data.size(); CScope.Row++)
        {
            for (CScope.Col = 0; CScope.Col < Data.at(CScope.Row).size(); CScope.Col++)
            {
                Char = Data.at(CScope.Row).substr(CScope.Col, 1);
				DidP = false; DidB = false; DidD = false;
                if (CScope.Blockcomment)
                {
                    if (CScope.Gotone)
                    {
                        if (Char == "/")
                        {
                            CScope.Blockcomment = false;
							DidB = true;
                            //if (Msg(MSGL_DEBUG)) printf("Debug at %s: Ended block comment.\n", StringPosition().c_str());
                        }
                        else
                            CScope.Gotone = false;
                    }
                    else if (Char == "*")
                        CScope.Gotone = true;
                }
                else if (CScope.Linecomment)
                {
                    // dummy, will end automatically at newline
                }
                else if (CScope.String)
                {
                    if (Char == "\"" && !CParen.Escape)
                    {
                        CScope.String = false;
                        CParen.Keyword = "\"" + CParen.Keyword + "\"";
                        //if (Msg(MSGL_DEBUG)) printf("Debug at %s: Got string '%s'.\n", StringPosition().c_str(),CParen.Keyword.c_str());
                    }
                    else if (Char == "\\" && !CParen.Escape)
                    {
                        CParen.Escape = true;
                    }
                    else
                    {
                        CParen.Keyword += Char;
						CParen.Escape = false;
                    }
                }
                else
                {
                    if (Char == "{")
                    {
						//printf("		@%s: creating scope; exnxtscp(%s)\n", StringPosition().c_str(), Bool2Str(CScope.ExecnextScope).c_str());
						int TRow = CScope.Row, TCol = CScope.Col;
						std::string FName = CParen.Memory;
						bool Rec = CScope.RecordBlock;//, Ex = CScope.Execnext;
						//printf("___________________----GOT FNAME:'%s' at r:%d c:%d\n", FName.c_str(), TRow, TCol);
						PushScope(CScope.ExecnextScope);
						//printf("			@%s: scope; exnxtscp(%s)\n", StringPosition().c_str(), Bool2Str(CScope.Execnext).c_str());
						if (Rec)
						{
							Functions[FName].Startcol = TCol+1;
							Functions[FName].Startrow = TRow;
							CScope.Execnext = false;
						}
						/*else if (Ex)
						{
							CScope.Execnext = false;
						}*/
						CScope.Row = TRow;
						CScope.Col = TCol;
						//printf("				@%s: scope; exnxtscp(%s)\n", StringPosition().c_str(), Bool2Str(CScope.Execnext).c_str());
                    }
                    else if (Char == "}")
                    {
						int TRow = CScope.Row, TCol = CScope.Col;
						PopScope();
						CScope.Row = TRow;
						CScope.Col = TCol;
						if (CScope.RecordBlock)
						{
							CScope.RecordBlock = false;
							Functions[CParen.Memory].Endcol = TCol;
							Functions[CParen.Memory].Endrow = TRow;
							//printf("ended recording '%s' at %s\n", CParen.Memory.c_str(), StringPosition().c_str());
							Functions[CParen.Memory].TextCode = CropStrList(Data, Functions[CParen.Memory].Startcol, Functions[CParen.Memory].Startrow, Functions[CParen.Memory].Endcol, Functions[CParen.Memory].Endrow);
							//printf(":- CROP @%d, %d - %d, %d\n", Functions[CParen.Memory].Startcol, Functions[CParen.Memory].Startrow, Functions[CParen.Memory].Endcol, Functions[CParen.Memory].Endrow);
							//for (unsigned int h = 0; h < Functions[CParen.Memory].TextCode.size(); h++)
							//{
							//	printf(":%s\n", Functions[CParen.Memory].TextCode.at(h).c_str());
							//}
							//printf(":- DONE\n");
						}
                    }
					else
					{
						if (Char == "/")
						{
							if (CParen.Operator == "/")
							{
								CParen.Operator = "";
								CScope.Linecomment = true;
								DidD = true;
								//if (Msg(MSGL_DEBUG)) printf("Debug at %s: Got line comment.\n", StringPosition().c_str());
							}
							else
							{
								if (DidB) CParen.Operator = "";
								else CParen.Operator = "/";
								//if (Msg(MSGL_DEBUG)) printf("Debug at %s: Got delimiter '/'.\n", StringPosition().c_str());
							}
						}
						else if (Char == "*")
						{
							if (CParen.Operator == "/")
							{
								CParen.Operator = "";
								CScope.Blockcomment = true;
								DidD = true;
								//if (Msg(MSGL_DEBUG)) printf("Debug at %s: Got block comment.\n", StringPosition().c_str());
							}
							else
							{
								CParen.Operator = "*";
							}
						}
						if (!CScope.RecordBlock && CScope.Execnext)
						{
							if (IsProcessChar(Char) && !DidD)
							{
								if (IsType(CParen.Memory))
								{
									if (CParen.Memory == "if" && Char == ";")
									{
										bool BoolParam = Str2Bool(GetVar(CParen.Keyword));
										CScope.ExecnextScope = BoolParam;
										CScope.DidRun = BoolParam;
											
									}
									else if (CParen.Memory == "else" && Char == ";")
									{
										bool Do = CScope.DidRun ? false : true;
										CScope.ExecnextScope = Do;
									}
									else if (CParen.Memory == "elseif" && Char == ";")
									{
										bool BoolParam = Str2Bool(GetVar(CParen.Keyword));
										bool DidRun = CScope.DidRun;
								
										if (DidRun)
										{
											CScope.ExecnextScope = false;
										}
										else
										{
											CScope.ExecnextScope = BoolParam;
											CScope.DidRun = BoolParam;
										}
									}
									else if (CParen.Memory == "return" && Char == ";")
									{
										std::string StrVal = GetVar(CParen.Keyword);
										while (Scopes.size() > ScopeLevels)
										{
											PopScope();
										}
										CParen.Keyword = StrVal;
										return true;
									}
									else if (CParen.Memory == "func" && Char == "(")
									{
										//declare function
										//if (Msg(MSGL_DEBUG)) printf("Debug at %s: Declaring function %s %s.\n", StringPosition().c_str(), CParen.Memory.c_str(), CParen.Keyword.c_str());
										std::string Temp = CParen.Keyword;
										PushParen();
										DidP = true;
										CParen.Funccall = true;
										CParen.FuncCall.FunctionName = Temp;
										CParen.Expect = EXPECT_PARAM_LIST;
									}
									else if (CParen.Memory == "delete" && Char == ";")
									{
										if (IsVar(CParen.Keyword))
										{
											RemVar(CParen.Keyword);
										}
										else if (IsFunc(CParen.Keyword))
										{
											RemFunc(CParen.Keyword);
										}
										else
										{
											// Cannot remove any more
										}
									}
									else if (CParen.Memory.size() > 0 && CParen.Keyword.size() > 0)
									{
										//declare variable
										if (CParen.Expect == EXPECT_PARAM_LIST)
										{
											//if (Msg(MSGL_DEBUG)) printf("Debug at %s: Pushing variable %s %s.\n", StringPosition().c_str(), CParen.Memory.c_str(), CParen.Keyword.c_str());
											Variable TempVar;
											TempVar.Type = Str2Type(CParen.Memory);
											TempVar.StringValue = CParen.Keyword; // Borrow stringvalue field to store variable name
											CParen.FuncCall.Vars.push_back(TempVar);
											CParen.Memory = "";
										}
										else
										{
											//if (Msg(MSGL_DEBUG)) printf("Debug at %s: Declaring variable %s %s.\n", StringPosition().c_str(), CParen.Memory.c_str(), CParen.Keyword.c_str());
											SetVar(CParen.Keyword, CParen.Memory, "");
											CParen.Memory = CParen.Keyword;
										}
										CParen.Keyword = "";
									}
									else if (!IsType(CParen.Memory))
									{
										if (Msg(MSGL_WARNING)) printf("Warning at %s: No symbol name specified!\n", StringPosition().c_str());
									}
								}
								else if (CParen.Memory.size() > 0 && CParen.Keyword.size() > 0 && CParen.Operator.size() > 0 && Char != "(")
								{
									//printf("assigning\n");
									std::string TLeft = CParen.Memory;
									std::string TRight = CParen.Keyword;
									//if (IsVar(CParen.Memory))
										TLeft = GetVar(CParen.Memory);
									//if (IsVar(CParen.Keyword))
										TRight = GetVar(CParen.Keyword);
									printf("    d@%s: assigning values '%s' '%s' '%s'\n", StringPosition().c_str(), (CParen.Operator == "=" ? CParen.Memory.c_str() : TLeft.c_str()), CParen.Operator.c_str(), TRight.c_str());
									if (CParen.Operator == "=")
									{
										SetVar(CParen.Memory, TRight);
									}
									else if (CParen.Operator == "+")
									{
										CParen.Memory = Float2Str(Str2Float(TLeft) + Str2Float(TRight));
									}
									else if (CParen.Operator == "-")
									{
										CParen.Memory = Float2Str(Str2Float(TLeft) - Str2Float(TRight));
									}
									else if (CParen.Operator == "*")
									{
										CParen.Memory = Float2Str(Str2Float(TLeft) * Str2Float(TRight));
									}
									else if (CParen.Operator == "/")
									{
										CParen.Memory = Float2Str(Str2Float(TLeft) / Str2Float(TRight));
									}
									else if (CParen.Operator == "&")
									{
										CParen.Memory = "\"" + TLeft + TRight + "\"";
									}
									else if (CParen.Operator == "&&")
									{
										CParen.Memory = Bool2Str(Str2Bool(TLeft) && Str2Bool(TRight));
									}
									else if (CParen.Operator == "||")
									{
										CParen.Memory = Bool2Str(Str2Bool(TLeft) || Str2Bool(TRight));
									}
									else if (CParen.Operator == ">")
									{
										CParen.Memory = Bool2Str(Str2Float(TLeft) > Str2Float(TRight));
									}
									else if (CParen.Operator == "<")
									{
										CParen.Memory = Bool2Str(Str2Float(TLeft) < Str2Float(TRight));
									}
									else if (CParen.Operator == ">=")
									{
										CParen.Memory = Bool2Str(Str2Float(TLeft) >= Str2Float(TRight));
									}
									else if (CParen.Operator == "<=")
									{
										CParen.Memory = Bool2Str(Str2Float(TLeft) <= Str2Float(TRight));
									}
									else if (CParen.Operator == "==")
									{
										CParen.Memory = Bool2Str(TLeft == TRight);
									}
									else if (CParen.Operator == "!=")
									{
										CParen.Memory = Bool2Str(TLeft != TRight);
									}
									CParen.Keyword = "";
									CParen.Operator = "";
								}
								else if (/*CParen.Memory.size() < 1 &&*/ CParen.Keyword.size() > 0 && Char == "(")
								{
									// start function call
									//if (Msg(MSGL_DEBUG)) printf("Debug at %s: Starting function call to %s.\n", StringPosition().c_str(), CParen.Keyword.c_str());
									std::string TFN = CParen.Keyword;
									PushParen();
									DidP = true;
									CParen.Funccall = true;
									CParen.FuncCall.FunctionName = TFN;
									CParen.Expect = EXPECT_VALUE;
								}
								else if (CParen.Memory.size() < 1)
								{
									if (CParen.Keyword.size() > 0)
									{
										if (CParen.Expect == EXPECT_VALUE /*&& (Char == "," || Char == ")")*/)
										{
											Variable TV;
											TV.Type = TYPE_NONE;
											//if (IsVar(CParen.Keyword))
												TV.StringValue = GetVar(CParen.Keyword);
											//else
											//	TV.StringValue = CParen.Keyword;
											CParen.FuncCall.Vars.push_back(TV);
											//printf("_____VALUE:'%s'\n", CParen.Keyword.c_str());
											CParen.Keyword = "";
										}
										else
										{
											CParen.Memory = CParen.Keyword;
											CParen.Keyword = "";
										}
									}
								}
							}
							if (!DidD)
							{
								if (Char == "+" || Char == "-" || Char == ">" || Char == "<" || Char == "!" || (Char == "*" && !DidB && !DidD) || (Char == "/" && !DidB && !DidD))
								{
									CParen.Operator = Char;
								}
								else if (Char == "=" || Char == "&" || Char == "|")
								{
									if (CParen.Operator.size() < 2)
										CParen.Operator += Char;
									else
										CParen.Operator = Char;
								}
								else if (Char == "\"")
								{
									CScope.String = true;
								}
								else if (Char == "(")
								{
									if (!DidP)
										PushParen();
								}
								else if (Char == ")")
								{
									if (CParen.Expect == EXPECT_PARAM_LIST)
									{
										// create function
										Function TF;
										TF.Name = CParen.FuncCall.FunctionName;
										printf("Created function %s(", CParen.FuncCall.FunctionName.c_str());
										for (unsigned int i = 0; i < CParen.FuncCall.Vars.size(); i++)
										{
											if (i == (CParen.FuncCall.Vars.size() - 1))
											{
												printf("%s %s", Type2Str(CParen.FuncCall.Vars.at(i).Type).c_str(), CParen.FuncCall.Vars.at(i).StringValue.c_str());
											}
											else
											{
												printf("%s %s, ", Type2Str(CParen.FuncCall.Vars.at(i).Type).c_str(), CParen.FuncCall.Vars.at(i).StringValue.c_str());
											}
											Parameter TP; TP.Name = CParen.FuncCall.Vars.at(i).StringValue; TP.Type = CParen.FuncCall.Vars.at(i).Type;
											TF.Parameters.push_back(TP);
										}
										printf(")\n");
										TF.IsNative = false; TF.NativeFunction = NULL;
										TF.Filename = ""; TF.MinParam = CParen.FuncCall.Vars.size(); TF.MaxParam = CParen.FuncCall.Vars.size();
										Functions[TF.Name] = TF;
										PopParen();
										CParen.Memory = TF.Name;
										CParen.Operator = "";
										CParen.Keyword = "";
										CScope.RecordBlock = true;
									}
									else //if (CParen.Expect == EXPECT_VALUE)
									{
										Parenthesis TP = CParen;
										PopParen();
										// call function, return value?
										if (TP.Funccall)
										{
											if (Functions.find(TP.FuncCall.FunctionName) == Functions.end())
											{
												printf("Warning at %s: tried to call undefined function '%s'!\n", StringPosition().c_str(), TP.FuncCall.FunctionName.c_str());
											}
											else
											{
												if (TP.FuncCall.Vars.size() >= (unsigned int)Functions[TP.FuncCall.FunctionName].MinParam && (Functions[TP.FuncCall.FunctionName].MaxParam == -1 || TP.FuncCall.Vars.size() <= (unsigned int)Functions[TP.FuncCall.FunctionName].MaxParam))
												{
													if (Functions[TP.FuncCall.FunctionName].IsNative)
													{
														CParen.Keyword = Functions[TP.FuncCall.FunctionName].NativeFunction(TP.FuncCall);
													}
													else
													{
														/*printf("calling: %s(", TP.FuncCall.FunctionName.c_str());
														for (unsigned int i = 0; i < TP.FuncCall.Vars.size(); i++)
														{
															printf((i<TP.FuncCall.Vars.size()-1?"%s, ":"%s"), TP.FuncCall.Vars.at(i).StringValue.c_str());
														}
														printf(")\n");
														CParen.Keyword = "[R#(" + TP.Memory + ")]";*/
														std::vector<Variable> backup;
														for (unsigned int i = 0; i < Functions[TP.FuncCall.FunctionName].Parameters.size();i++)
														{
															if (Variables.find(Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name)!=Variables.end())
															{
																Variable tv;
																tv.Name = Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name;
																tv.Type = Str2Type(Functions[TP.FuncCall.FunctionName].Parameters.at(i).Type);
																tv.StringValue = GetVar(Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name);
																backup.push_back(tv);
															}
															Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].Type = Str2Type(Functions[TP.FuncCall.FunctionName].Parameters.at(i).Type);
															switch (Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].Type)
															{
																case TYPE_BOOL:
																	Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].BoolValue = Str2Bool(TP.FuncCall.Vars.at(i).StringValue);
																	break;
																case TYPE_FLOAT:
																	Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].FloatValue = Str2Float(TP.FuncCall.Vars.at(i).StringValue);
																	break;
																case TYPE_INT:
																	Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].IntValue = Str2Int(TP.FuncCall.Vars.at(i).StringValue);
																	break;
																default:
																	Variables[Functions[TP.FuncCall.FunctionName].Parameters.at(i).Name].StringValue = TP.FuncCall.Vars.at(i).StringValue;
																	break;
															}
														}
														Parse(Functions[TP.FuncCall.FunctionName].TextCode);
														for (unsigned int i = 0; i < backup.size(); i++)
														{
															SetVar(backup.at(i).Name, Type2Str(backup.at(i).Type), backup.at(i).StringValue);
														}
													}
												}
												else
												{
													printf("Warning at %s: wrong parameter count for '%s', %d entered, %d to %d expected!\n", StringPosition().c_str(), TP.FuncCall.FunctionName.c_str(), TP.FuncCall.Vars.size(), Functions[TP.FuncCall.FunctionName].MinParam, Functions[TP.FuncCall.FunctionName].MaxParam);
												}
											}
										}
										else
											CParen.Keyword = TP.Memory;
									}
								}
								else if (Char == ";")
								{
									CParen.Memory = "";
									CParen.Keyword = "";
									CParen.Operator = "";
									CParen.Expect = EXPECT_NONE;
								}
								else if (Char == ",")
								{
									CParen.Memory = "";
									CParen.Keyword = "";
									CParen.Operator = "";
								}
								else
								{
									if (!IsProcessChar(Char))
									{
										CParen.Keyword += Char;
									}
								}
							}
						}
					}
                }
            }
            // On new line, line comment will be false
            //if (CScope.Linecomment) {if (Msg(MSGL_DEBUG)) printf("Debug at %s: Ended line comment.\n", StringPosition().c_str());}
            CScope.Linecomment = false;
			CParen.Operator = "";
        }
        PopScope();
        return true;
    }
    std::string StringPosition()
    {
        //return "Row: " + Int2Str(CScope.Row) + ", Col: " + Int2Str(CScope.Col);
        return Int2Str(CScope.Row+1) + ", " + Int2Str(CScope.Col+1);
    }

    bool IsType(std::string str)
    {
        if (str == "if") return true;
        if (str == "else") return true;
        if (str == "elseif") return true;
        if (str == "return") return true;
        if (str == "int") return true;
        if (str == "string") return true;
        if (str == "bool") return true;
        if (str == "float") return true;
        if (str == "func") return true;
        if (str == "delete") return true;
        if (ExternalTypes.find(str) != ExternalTypes.end()) return true;
        return false;
    }
	int Str2Type(std::string Type)
	{
		if (Type == "int") return TYPE_INT;
		if (Type == "bool") return TYPE_BOOL;
		if (Type == "float") return TYPE_FLOAT;
		if (Type == "func") return TYPE_FUNC;
		if (ExternalTypes.find(Type) != ExternalTypes.end()) return TYPE_EXTERNAL;
		//if (Type == "string") return TYPE_STRING;
		return TYPE_STRING;
	}
	std::string Type2Str(int Type)
	{
		switch(Type)
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
	bool IsProcessChar(std::string Char)
	{
		std::string Chars = " =+-*/&|*;!<>(),\t\n\r\"";
		return (Chars.find(Char) == Chars.npos) ? false : true;
	}
	bool IsAssagnChar(std::string Char)
	{
		std::string Chars = "+-*/&!<>|=";
		return (Chars.find(Char) == Chars.npos) ? false : true;
	}

	bool IsVar(std::string VarName) { return (Variables.find(VarName) == Variables.end()) ? false : true; }
	bool RemVar(std::string VarName)
	{
		if (Variables.find(VarName) == Variables.end())
		{
			return false;
		}
		else
		{
			if (Variables[VarName].Type == TYPE_EXTERNAL) Variables[VarName].Ext->Delete(VarName);
			Variables.erase(VarName);
			return true;
		}
	}
	std::string GetVar(std::string VarName)
	{
		if (IsStr(VarName))
		{
			return UnStr(VarName);
		}
		else if (Variables.find(VarName) == Variables.end())
		{
			return VarName;
		}
		else
		{
			Variable TVar = Variables[VarName];
			switch(TVar.Type)
			{
				case TYPE_INT:
					return Int2Str(TVar.IntValue);
				case TYPE_BOOL:
					return Bool2Str(TVar.BoolValue);
				case TYPE_FLOAT:
					return Float2Str(TVar.FloatValue);
				case TYPE_EXTERNAL:
					return TVar.Ext->AsString(VarName);
				default:
					return TVar.StringValue;
			}
		}
	}
	bool SetInt(std::string VarName, int Value) { if (Variables.find(VarName) == Variables.end()) { Variable Temp; Temp.Type = TYPE_INT; Temp.IntValue = Value; Variables[VarName] = Temp; return false; } else { Variables[VarName].IntValue = Value; return true; } }
	int GetInt(std::string VarName, int DefaultValue) { if (Variables.find(VarName) == Variables.end()) { return DefaultValue; } else { return Variables[VarName].IntValue; } }
	int *GetIntPtr(std::string VarName) { if (Variables.find(VarName) == Variables.end()) { return 0; } else { return &Variables[VarName].IntValue; } }
	bool SetBool(std::string VarName, bool Value) { if (Variables.find(VarName) == Variables.end()) { Variable Temp; Temp.Type = TYPE_BOOL; Temp.BoolValue = Value; Variables[VarName] = Temp; return false; } else { Variables[VarName].BoolValue = Value; return true; } }
	bool GetBool(std::string VarName, bool DefaultValue) { if (Variables.find(VarName) == Variables.end()) { return DefaultValue; } else { return Variables[VarName].BoolValue; } }
	bool *GetBoolPtr(std::string VarName) { if (Variables.find(VarName) == Variables.end()) { return 0; } else { return &Variables[VarName].BoolValue; } }
	bool SetFloat(std::string VarName, float Value) { if (Variables.find(VarName) == Variables.end()) { Variable Temp; Temp.Type = TYPE_FLOAT; Temp.FloatValue = Value; Variables[VarName] = Temp; return false; } else { Variables[VarName].FloatValue = Value; return true; } }
	float GetFloat(std::string VarName, float DefaultValue) { if (Variables.find(VarName) == Variables.end()) { return DefaultValue; } else { return Variables[VarName].FloatValue; } }
	float *GetFloatPtr(std::string VarName) { if (Variables.find(VarName) == Variables.end()) { return 0; } else { return &Variables[VarName].FloatValue; } }
	bool SetString(std::string VarName, std::string Value) { if (Variables.find(VarName) == Variables.end()) { Variable Temp; Temp.Type = TYPE_STRING; Temp.StringValue = Value; Variables[VarName] = Temp; return false; } else { Variables[VarName].StringValue = Value; return true; } }
	std::string GetString(std::string VarName, std::string DefaultValue) { if (Variables.find(VarName) == Variables.end()) { return DefaultValue; } else { return Variables[VarName].StringValue; } }
	std::string *GetStringPtr(std::string VarName) { if (Variables.find(VarName) == Variables.end()) { return 0; } else { return &Variables[VarName].StringValue; } }
	bool SetVar(std::string VarName, std::string VarType, std::string Value)
	{
		//printf("SETVAR n:'%s' t:'%s' v:'%s'\n", VarName.c_str(), VarType.c_str(), Value.c_str());
		if (VarType == "int")
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
		{
			if (ExternalTypes.find(VarType) != ExternalTypes.end())
			{
				bool res = SetString(VarName, Value);
				Variables[VarName].Ext = &ExternalTypes[VarType];
				Variables[VarName].Type = TYPE_EXTERNAL;
				if (!res) Variables[VarName].Ext->Init(VarName);
				Variables[VarName].Ext->Assign(VarName, Value);
				return res;
			}
			else if (VarType != "string")
			{
				// warning message
			}
			return SetString(VarName, Value);
		}
	}
	void SetVar(std::string VarName, std::string Value)
	{
		if (Variables.find(VarName) == Variables.end())
			return;
		else
		{
			switch(Variables[VarName].Type)
			{
				case TYPE_INT:
					Variables[VarName].IntValue = Str2Int(Value);
					return;
				case TYPE_BOOL:
					Variables[VarName].BoolValue = Str2Bool(Value);
					return;
				case TYPE_FLOAT:
					Variables[VarName].FloatValue = Str2Float(Value);
					return;
				case TYPE_EXTERNAL:
					Variables[VarName].Ext->Assign(VarName, Value);
					return;
				default:
					Variables[VarName].StringValue = Value;
					return;
			}
		}
	}

    int Str2Int( std::string str ) { std::stringstream ss( str ); int n; ss >> n; return n; }
    std::string Int2Str( int aint ) { std::stringstream ss; ss << aint; return ss.str(); }
    bool Str2Bool( std::string str ) { if ( str == "true" ) return true; else return false; }
    std::string Bool2Str( bool abool ) { if ( abool ) return "true"; else return "false"; }
    float Str2Float( std::string str ) { return (float)strtod( str.c_str(), NULL ); }
    std::string Float2Str( float afloat ) { std::stringstream ss; ss << afloat; return ss.str(); }
    bool IsStr(std::string instr)
    {
	    return (instr.substr(0,1) == "\"" && instr.substr(instr.size()-1) == "\"") ? true : false;
    }
    std::string UnStr(std::string instr)
    {
	    if (IsStr(instr))
	    {
		    return instr.substr(1, instr.size()-2);
	    }
	    else
	    {
		    return instr;
	    }
    }
    std::vector<std::string> CropStrList(std::vector<std::string> List, int x, int y, int tox, int toy)
    {
        std::vector<std::string> trg;
        for (int Row = y; Row <= toy; Row++)
        {
            std::string rw = "";
            for (int Col = (Row==y?x:0); Col < (Row==toy?tox:(int)List.at(Row).size()); Col++ )
            {
                rw += List.at(Row).substr(Col,1);
            }
            trg.push_back(rw);
        }
        //for (unsigned int i = 0; i < trg.size(); i++) printf("        %s\n",trg.at(i).c_str());
        return trg;
    }

    void File_Init(std::string Name)
    {
    	printf("        Creating file '%s'\n", Name.c_str());
	}
	void File_Assign(std::string Name, std::string Value)
	{
    	printf("        Assigning file '%s' to path '%s'\n", Name.c_str(), Value.c_str());
	}
	std::string File_AsString(std::string Name)
	{
    	printf("        Getting file from '%s'\n", Name.c_str());
		return "dummy";
	}
	void File_Delete(std::string Name)
	{
    	printf("        Deleting file '%s'\n", Name.c_str());
	}

}
