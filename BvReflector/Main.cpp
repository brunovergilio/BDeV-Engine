#include "BDeV\Container\BvString.h"
#include <BDeV/System/File/BvFile.h>
#include <BDeV/System/File/BvPath.h>
#include "BvReflector.h"
#include <BvReflector/BvLexer.h>


bool GetNextStruct(const BvVector<BvString>& symbols, u32& objectNameIndex, u32& first, u32& last, u32 startFrom = 0)
{
	auto rttiIndex = startFrom;
	auto classDeclIndex = 0u;
	bool rttiFound = false;
	for (; rttiIndex < symbols.Size(); rttiIndex++)
	{
		if (!rttiFound && symbols[rttiIndex].Contains("BV_RTTI"))
		{
			rttiFound = true;
			objectNameIndex = rttiIndex + 2;
			auto currIndex = rttiIndex - 1;
			while (symbols[currIndex] != "class" && symbols[currIndex] != "struct")
			{
				--currIndex;
			}
			first = currIndex;

			break;
		}
	}

	if (!rttiFound)
	{
		return false;
	}

	u32 bracketCount = 0;
	for (rttiIndex = first; rttiIndex < symbols.Size(); rttiIndex++)
	{
		if (symbols[rttiIndex] == '{')
		{
			++bracketCount;
		}
		else if (symbols[rttiIndex] == '}')
		{
			if (--bracketCount == 0)
			{
				last = rttiIndex;
				break;
			}
		}
	}

	return true;
}


enum class ObjectPosition : u8
{
	kInClassDecl,
	kInVariablesAndMethods
};


void ReadBaseClasses(const BvVector<BvString>& symbols, BvVector<BvString>& baseClasses, u32& curr, u32 last)
{
	for (; curr <= last; curr++)
	{
		if (symbols[curr] == "{")
		{
			break;
		}

		if (symbols[curr] == "public" || symbols[curr] == "protected" || symbols[curr] == "private"
			|| symbols[curr] == "virtual" || symbols[curr].GetLastChar() == ',')
		{
			continue;
		}
		else
		{
			baseClasses.EmplaceBack(symbols[curr]);
		}
	}
}


struct Variable
{
	BvString type;
	BvString name;
};

struct Method
{
	BvString type;
	BvString name;
	BvVector<BvString> params;
	bool isVirtual;
};


void ReadVariablesAndMethods(const BvVector<BvString>& symbols, BvVector<Variable>& vars, BvVector<Method>& methods, u32& curr, u32 last)
{
	BvVector<BvString> methodParams;
	for (; curr <= last; curr++)
	{
		if (curr == last && symbols[curr] == '}')
		{
			break;
		}

		BvString typeAndNameTags;
		if (symbols[curr] == ';')
		{
			continue;
		}

		bool isMethod = false;
		bool isVirtual = false;
		if (symbols[curr] == "public" || symbols[curr] == "protected" || symbols[curr] == "private")
		{
			curr += 2;
		}

		// skip these
		if (symbols[curr] == "typedef" || symbols[curr] == "using" || symbols[curr] == "friend" || symbols[curr].Contains("BV_RTTI"))
		{
			while (symbols[++curr] != ';')
			{
			}
			++curr;
		}

		if (symbols[curr] == "virtual")
		{
			isMethod = true;
			isVirtual = true;
			++curr;
		}

		u32 typeAndNameTagCount = 0;
		while (symbols[curr] != '(' && symbols[curr] != '=' && symbols[curr] != '{' && symbols[curr] != ';')
		{
			if (typeAndNameTagCount > 0)
			{
				typeAndNameTags.Append(' ');
			}
			typeAndNameTags.Append(symbols[curr]);
			++typeAndNameTagCount;
			++curr;
		}

		bool isFunctionPointer = false;
		bool isMethodReturningFunctionPointer = false;
		bool isPointer = false;
		if (symbols[curr] == '(')
		{
			if (symbols[curr + 1] == '*') // Pointer variable, possibly a function pointer or method returning a function pointer
			{
				isPointer = true;
				curr += 2;
				typeAndNameTags.Append(' ');
				typeAndNameTags.Append("(*");
				typeAndNameTags.Append(symbols[curr++]);

				while (symbols[curr] != ')')
				{
					// If there's another set of parenthesis inside, then it's a method returning a function pointer
					if (symbols[curr] == '(')
					{
						isMethod = true;
						isMethodReturningFunctionPointer = true;
						typeAndNameTags.Append('(');
						++curr;
						while (symbols[curr] != ')')
						{
							typeAndNameTags.Append(symbols[curr]);
							++curr;
						}
						typeAndNameTags.Append(')');
					}
					++curr;
				}
				typeAndNameTags.Append(')');
				++curr;

				if (symbols[curr] == '(')
				{
					if (!isMethodReturningFunctionPointer)
					{
						isFunctionPointer = true;
					}
					typeAndNameTags.Append('(');
					++curr;
					// Function pointer or method returning a function pointer
					while (symbols[curr] != ')')
					{
						typeAndNameTags.Append(symbols[curr]);
						++curr;
					}
					typeAndNameTags.Append(')');
				}
			}
			else
			{
				isMethod = true;
				++curr;
				while (symbols[curr++] != ')')
				{
					if (symbols[curr] == '=')
					{
						while (symbols[curr] != ',')
						{
							++curr;
						}
					}

					typeAndNameTags.Append(' ');
					typeAndNameTags.Append(symbols[curr]);
				}
			}
		}
		if (symbols[curr] == '=')
		{
			if (isMethod && isVirtual && symbols[curr + 1] == '0')
			{
				curr += 2;
				u32 bracketCount = 0;
				do
				{
					if (symbols[curr] == '{')
					{
						++bracketCount;
					}
					else if (symbols[curr] == '}')
					{
						--bracketCount;
					}
					else if (symbols[curr] == ';')
					{
						break;
					}
					++curr;
				} while (1);

				++curr;
				while (symbols[curr] != ';' && symbols[curr] != '}')
				{
					++curr;
				}
			}

			while (symbols[curr] != ';' && symbols[curr] != '}')
			{
				++curr;
			}
			++curr;
		}
		if (symbols[curr] == '{')
		{
			if (!isMethod)
			{
				while (symbols[curr] != ';')
				{
					++curr;
				}
				++curr;
			}
			else
			{
				++curr;
				u32 bracketCount = 1;
				while (bracketCount != 0)
				{
					if (symbols[curr] == '{')
					{
						++bracketCount;
					}
					else if (symbols[curr] == '}')
					{
						if (--bracketCount == 0)
						{
							break;
						}
					}
					++curr;
				}

				if (!isMethod)
				{
					while (symbols[curr++] != ';')
					{
					}
				}
			}
		}

		BvString type;
		BvString name;
		u32 nameIndex = typeAndNameTags.FindLastOf(' ');
		auto p = typeAndNameTags.Find("(*");
		if (p == BvString::kInvalidIndex)
		{
			type = typeAndNameTags.Substr(0, nameIndex);
			name = typeAndNameTags.Substr(nameIndex + 1, typeAndNameTags.Size() - nameIndex + 1);
		}
		else
		{
			p += 2;
			auto p2 = typeAndNameTags.Find(')', p);
			name = typeAndNameTags.Substr(p, p2 - p);
			type = typeAndNameTags;
			type.Erase(p, name.Size());
			if (!isFunctionPointer)
			{
				p = type.Find('(');
				type.Erase(p, 1);
				p = type.Find(')');
				type.Erase(p, 1);
			}
		}

		if (isMethod)
		{
			auto& m = methods.EmplaceBack(Method());
			m.name = std::move(name);
			m.type = std::move(type);
			m.params = std::move(methodParams);
			m.isVirtual = isVirtual;
		}
		else
		{
			auto& v = vars.EmplaceBack(Variable());
			v.name = std::move(name);
			v.type = std::move(type);
		}
	}
}


void ParseObject(const BvVector<BvString>& symbols, u32 first, u32 last)
{
	ObjectPosition pos = ObjectPosition::kInClassDecl;
	BvVector<BvString> baseClasses;
	BvVector<Variable> vars;
	BvVector<Method> methods;
	for (auto i = first; i <= last; i++)
	{
		if (symbols[i] == ':')
		{
			if (pos == ObjectPosition::kInClassDecl)
			{
				ReadBaseClasses(symbols, baseClasses, ++i, last);
			}
		}

		if (symbols[i] == '{')
		{
			if (pos == ObjectPosition::kInClassDecl)
			{
				pos = ObjectPosition::kInVariablesAndMethods;

				ReadVariablesAndMethods(symbols, vars, methods, ++i, last);
			}
		}
	}
}


void Process(const char* pFolderName)
{
	BvPath path(pFolderName);
	auto files = path.GetFileList("*.*");

	for (auto&& file : files)
	{
		if (file.GetExt() == L".cpp" || file.GetExt() == L".h")
		{
			BvFile hFile(file.AsFile(BvFileAccessMode::kRead));
			BvString fileData((u32)hFile.GetSize() + 1);
			hFile.Read(&fileData[0], fileData.Size());
			
			BvVector<BvString> parts;
			fileData.Split(parts, " \n\t\r", "()*{}:,;");

			u32 first = 0, last = 0, startFrom = 0, objectNameIndex = 0;
			while (GetNextStruct(parts, objectNameIndex, first, last, startFrom))
			{
				ParseObject(parts, first, last);
			}

			bool isFinal = false;
			u32 bracketCount = 0;
			bool derivedPart = false;
			BvVector<BvString> derivedClasses;
			for (auto i = first + 1; i < parts.Size(); i++)
			{
				if (parts[i][0] == '{')
				{
					if (++bracketCount == 1)
					{
						derivedPart = false;
					}
				}
				else if (parts[i][0] == ':')
				{
					++i;
					if (bracketCount == 0)
					{
						derivedPart = true;
					}
				}
				else
				{
					if (derivedPart)
					{
						if (parts[i].Contains("public") || parts[i].Contains("protected") || parts[i].Contains("private") || parts[i].Contains(','))
						{
							++i;
						}
						else
						{
							derivedClasses.EmplaceBack(parts[i]);
						}
					}

					if (!(parts[i].Contains("public:") || parts[i].Contains("protected:") || parts[i].Contains("private:") || parts[i].Contains("BV_RTTI")))
					{
						while (!parts[i].Contains(';') && !parts[i].Contains('('))
						{
							i++;
						}


					}
				}
			}


			//auto& pp = parts[classDeclIndex];
		}
	}
}


int main(int argc, char* argv[])
{
	BvReflector r;
	r.Process(LR"(C:\Study\BDeV-Engine\TestProjects\CoreTests)");
	return 0;
}