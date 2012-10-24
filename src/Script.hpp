#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#define CScope Scopes.back()
#define CParen Scopes.back().Parentheses.back()

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <cstdlib>

namespace Script
{
    enum MessageLevels {MSGL_DEBUG, MSGL_INFO, MSGL_WARNING, MSGL_ERROR};
    enum Expect {EXPECT_NONE, EXPECT_PARAM_LIST, EXPECT_VALUE};
	enum Types {TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_STRING, TYPE_FUNC, TYPE_NONE};
	enum Record {RECORD_LOOP, RECORD_FUNC};

    struct File
    {
        std::string Filename;
        std::vector<std::string> Data;
    };
	struct Variable
	{
		int Type, IntValue;
		bool BoolValue;
		float FloatValue;
		std::string StringValue, Name;
	};
	struct FunctionCall;
	struct Parameter{std::string Type, Name;};
	struct Function
	{
		bool IsNative;
		std::string (*NativeFunction)(FunctionCall Data);
		int MinParam, MaxParam;
		// ----- -----
		std::string Name, Filename;
		int Startrow, Startcol, Endrow, Endcol;
		std::vector<std::string> TextCode;
		std::vector<Parameter> Parameters;
	};
	struct FunctionCall
	{
		std::string FunctionName;
		std::vector<Variable> Vars;
		std::string FileName;
		int Row;
	};
    struct Parenthesis
    {
        std::string Keyword, Memory, Operator;
        bool Escape, Funccall, Funccreate;
        int Expect;
		FunctionCall FuncCall;
    };
    struct Scope
    {
        unsigned int Row, Col;
        bool DidRun, Linecomment, Blockcomment, String, Escape, Execnext, ExecnextScope, Gotone;
        std::vector<Parenthesis> Parentheses;
        bool RecordBlock; int RecordType;
    };

    extern std::map<std::string, File> Files;
    extern std::string Delimiters;
    extern std::vector<Scope> Scopes;
    extern int MessageLevel;
	extern std::map<std::string, Variable> Variables;
	extern std::map<std::string, Function> Functions;

    void Init();
    void SetMessageLevel(int nMessageLevel);
    bool Msg(int sMsgl);
    void CleanUp();

    void PushScope(bool ExecScope = true);
    void PopScope();
    void PushParen();
    void PopParen();

	void BindNativeFunction(std::string Name, int MinParam, int MaxParam, std::string (*NewFunction)(FunctionCall Data));
	void BindUserFunction(std::string Name, std::vector<Parameter> Params, std::vector<std::string> TextCode);

    bool ParseString(std::string String);
    bool ParseFile(std::string Filename, bool Forcereload = false);
    bool Parse(std::vector<std::string> Data);
    std::string StringPosition();

    bool IsType(std::string str);
	int Str2Type(std::string Type);
	std::string Type2Str(int Type);
	bool IsProcessChar(std::string Char);
	bool IsAssagnChar(std::string Char);

	bool IsVar(std::string VarName);
	std::string GetVar(std::string VarName);
	bool SetInt(std::string VarName, int Value);
	int GetInt(std::string VarName, int DefaultValue);
	int *GetIntPtr(std::string VarName);
	bool SetBool(std::string VarName, bool Value);
	bool GetBool(std::string VarName, bool DefaultValue);
	bool *GetBoolPtr(std::string VarName);
	bool SetFloat(std::string VarName, float Value);
	float GetFloat(std::string VarName, float DefaultValue);
	float *GetFloatPtr(std::string VarName);
	bool SetString(std::string VarName, std::string Value);
	std::string GetString(std::string VarName, std::string DefaultValue);
	std::string *GetStringPtr(std::string VarName);
	bool SetVar(std::string VarName, std::string VarType, std::string Value);
	void SetVar(std::string VarName, std::string Value);

    int Str2Int( std::string str );
    std::string Int2Str( int aint );
    bool Str2Bool( std::string str );
    std::string Bool2Str( bool abool );
    float Str2Float( std::string str );
    std::string Float2Str( float afloat );
    std::vector<std::string> CropStrList(std::vector<std::string> List, int x, int y, int tox, int toy);
};

#endif // SCRIPT_HPP
