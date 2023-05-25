#include "BvReflector.h"
#include <BDeV/System/File/BvPath.h>


enum class ObjectPart : u8
{
	kNameDeclaration,
	kFinalModifier,
	kBaseClasses,
	kClassBeginBracket,
};


BvReflector::BvReflector()
{
}


void BvReflector::Process(const wchar_t* projectDir)
{
	BvPath path(projectDir);
	auto files = path.GetFileList("*.*");

	for (auto&& file : files)
	{
		if (file.GetExt() == L".cpp" || file.GetExt() == L".h")
		{
			BvFile hFile(file.AsFile(BvFileAccessMode::kRead));
			if (hFile.IsValid())
			{
				ParseHeaderFile(hFile);
			}
		}
	}
}


void BvReflector::ParseHeaderFile(BvFile& file)
{
	BvString data((u32)file.GetSize() + 1);
	file.Read(&data[0], data.Size());

	// Remove comments for parsing the objects
	u32 startOffset = 0, endOffset = 0;
	bool isInsideString = false;
	while (startOffset < data.Size() - 1)
	{
		if (data[startOffset] == '"')
		{
			isInsideString = !isInsideString;
		}

		// Single-line comment
		if (!isInsideString && data[startOffset] == '/' && data[startOffset + 1] == '/')
		{
			endOffset = startOffset + 2;
			// Go until there's a line break
			while (endOffset < data.Size() && data[endOffset] != '\n')
			{
				endOffset++;
			}

			data.Erase(startOffset, endOffset - startOffset);
		}
		// Multi-line comment
		else if (!isInsideString && data[startOffset] == '/' && data[startOffset + 1] == '*')
		{
			endOffset = startOffset + 2;
			while (endOffset < data.Size() - 1 && !(data[endOffset] == '*' && data[endOffset + 1] == '/'))
			{
				endOffset++;
			}

			data.Erase(startOffset, endOffset - startOffset + 2);
		}
		else
		{
			startOffset++;
		}
	}

	BvVector<BvString> symbols;
	data.Split(symbols, " \n\t\r", "()*{}:,;/");

	for (auto i = 0u; i < symbols.Size(); i++)
	{
		if (symbols[i] == "BV_RSTRUCT" || symbols[i] == "BV_RCLASS")
		{
			m_Objects.EmplaceBack();
			ParseObject(symbols, ++i);
		}
		else if (symbols[i] == "BV_RENUM")
		{
			ParseEnum(symbols, ++i);
		}
	}
}


void BvReflector::ParseObject(const BvVector<BvString>& symbols, u32& symbolIndex)
{
	auto& object = m_Objects.Back();

	// Skip BV_RSTRUCT / BV_RCLASS declaration and attributes
	while (symbols[symbolIndex++] != ')')
	{
		// Parse possible attributes
	}

	// Parse struct / class name, along with base classes, if any
	symbolIndex++;
	object.name = symbols[symbolIndex++];
	if (symbols[symbolIndex] != '{')
	{
		// Process base classes
		if (symbols[symbolIndex] == ':')
		{
			symbolIndex++;
			object.baseObjects.EmplaceBack();
			auto baseIndex = 0;
			while (symbolIndex < symbols.Size() && symbols[symbolIndex] != '{')
			{
				if (symbols[symbolIndex] == ',')
				{
					object.baseObjects.EmplaceBack();
					++baseIndex;
				}
				else if (!(symbols[symbolIndex] == "public" || symbols[symbolIndex] == "protected" || symbols[symbolIndex] == "private"))
				{
					object.baseObjects[baseIndex].Append(symbols[symbolIndex]);
					object.baseObjects[baseIndex].Append(' ');
				}
				symbolIndex++;
			}
		}
		symbolIndex++;
	}
	symbolIndex++;

	u32 bracketCount = 1;
	u32 lastSymbolIndexForClass = symbolIndex;
	while (lastSymbolIndexForClass < symbols.Size() && bracketCount != 0)
	{
		if (symbols[lastSymbolIndexForClass] == '{')
		{
			++bracketCount;
		}
		if (symbols[lastSymbolIndexForClass] == '}')
		{
			--bracketCount;
		}
		++lastSymbolIndexForClass;
	}

	while (symbolIndex < lastSymbolIndexForClass)
	{
		if (symbols[symbolIndex] == "BV_RMETHOD")
		{
			object.methods.EmplaceBack();
			ParseMethod(symbols, symbolIndex, lastSymbolIndexForClass);
		}
		else if (symbols[symbolIndex] == "BV_RVAR")
		{
			object.properties.EmplaceBack();
			ParseVariable(symbols, symbolIndex, lastSymbolIndexForClass);
		}
		else
		{
			symbolIndex++;
		}
	}
}


void BvReflector::ParseVariable(const BvVector<BvString>& symbols, u32& symbolIndex, u32 lastSymbolIndex)
{
	auto& object = m_Objects.Back();

	// Skip BV_RVAR declaration and attributes
	while (symbolIndex < lastSymbolIndex && symbols[symbolIndex++] != ')')
	{
		// Parse possible attributes
	}

	BvString varTypeAndName;
	bool addSpace = false;
	while (symbolIndex < lastSymbolIndex && symbols[symbolIndex] != ';')
	{
		// Don't care for default values
		if (symbols[symbolIndex] == '=' || symbols[symbolIndex] == '{')
		{
			while (symbolIndex < lastSymbolIndex && symbols[symbolIndex++] != ';')
			{
			}
			break;
		}

		if (addSpace)
		{
			varTypeAndName.Append(' ');
		}
		addSpace = true;

		varTypeAndName.Append(symbols[symbolIndex]);
		symbolIndex++;
	}
	addSpace = false;

	auto& variable = object.properties.Back();
	bool firstAdd = true;
	auto parIndex = varTypeAndName.Find('(');
	if (parIndex == BvString::kInvalidIndex)
	{
		auto lastSpace = varTypeAndName.FindLastOf(' ');
		auto bracketIndex = varTypeAndName.Find('[');
		if (bracketIndex == BvString::kInvalidIndex)
		{
			variable.type = varTypeAndName.Substr(0, lastSpace);
			variable.name = varTypeAndName.Substr(lastSpace + 1, varTypeAndName.Size() - lastSpace + 1);
		}
		else
		{
			variable.type = varTypeAndName.Substr(0, lastSpace);
			variable.type.Append(varTypeAndName, bracketIndex, varTypeAndName.Size() - bracketIndex);
			variable.name = varTypeAndName.Substr(lastSpace + 1, bracketIndex - (lastSpace + 1));
		}
	}
	else
	{
		BvVector<BvString> varSymbols;
		varTypeAndName.Split(varSymbols, ' ');
		u32 varSymbolIndex = 0;
		bool hasSecondParenthesis = varTypeAndName.Find('(', parIndex + 1) != BvString::kInvalidIndex;
		if (hasSecondParenthesis)
		{
			variable.isFunctionPointer = true;
		}
		bool firstParenthesis = false;

		while (varSymbolIndex < varSymbols.Size())
		{
			if (varSymbols[varSymbolIndex] == '(')
			{
				if (!firstParenthesis)
				{
					firstParenthesis = true;
				}

				if (hasSecondParenthesis)
				{
					variable.type += varSymbols[varSymbolIndex];
				}
				varSymbolIndex++;
				if (varSymbols[varSymbolIndex] == '*')
				{
					if (!variable.isFunctionPointer)
					{
						variable.isPointer = true;
					}
					variable.type += varSymbols[varSymbolIndex];
					varSymbolIndex++;

					variable.name = varSymbols[varSymbolIndex];
				}
				else if (variable.name.Empty())
				{
					variable.name = varSymbols[varSymbolIndex];
				}
				else
				{
					variable.type += varSymbols[varSymbolIndex];
				}
			}
			else if (varSymbols[varSymbolIndex] == ')')
			{
				if (hasSecondParenthesis)
				{
					variable.type += varSymbols[varSymbolIndex];
				}
			}
			else
			{
				if (!firstAdd)
				{
					variable.type += ' ';
				}
				firstAdd = false;
				variable.type += varSymbols[varSymbolIndex];
			}
			varSymbolIndex++;
		}

		auto bracketIndex = variable.name.Find('[');
		if (bracketIndex != BvString::kInvalidIndex)
		{
			variable.type.Append(variable.name, bracketIndex, variable.name.Size() - bracketIndex);
			variable.name.Erase(bracketIndex, variable.name.Size() - bracketIndex);
		}
	}
}


void BvReflector::ParseMethod(const BvVector<BvString>& symbols, u32& symbolIndex, u32 lastSymbolIndex)
{
	auto& object = m_Objects.Back();

	// Skip BV_RMETHOD declaration and attributes
	while (symbolIndex < lastSymbolIndex && symbols[symbolIndex++] != ')')
	{
		// Parse possible attributes
	}

	BvString methodTypeAndName;
	bool isInParamList = false;
	while (symbolIndex < lastSymbolIndex)
	{
		// Don't care for parameters' default values
		if (isInParamList && symbols[symbolIndex] == '=')
		{
			while (symbolIndex < lastSymbolIndex && (symbols[symbolIndex] != ',' && symbols[symbolIndex] != ')'))
			{
				symbolIndex++;
			}
		}

		// If we've reached a semi-colon, we're done
		if (symbols[symbolIndex] == ';')
		{
			symbolIndex++;
			break;
		}
		// If it's a function body, skip it as well
		else if (symbols[symbolIndex] == '{')
		{
			u32 curlyBraceCount = 1;
			symbolIndex++;

			while (symbolIndex < lastSymbolIndex && curlyBraceCount != 0)
			{
				if (symbols[symbolIndex] == '{')
				{
					++curlyBraceCount;
				}
				if (symbols[symbolIndex] == '}')
				{
					--curlyBraceCount;
				}
				symbolIndex++;
			}
			break;
		}
		// For any other character that pops, we process it
		else
		{
			if (symbols[symbolIndex] == '(')
			{
				isInParamList = true;
			}
			else if (symbols[symbolIndex] == ')')
			{
				isInParamList = false;
			}

			methodTypeAndName.Append(symbols[symbolIndex]);
			methodTypeAndName.Append(' ');
		}
		symbolIndex++;
	}

	auto& method = object.methods.Back();
	BvVector<BvString> methodSymbols;
	methodTypeAndName.Split(methodSymbols, ' ');
	u32 methodSymbolIndex = 0;
	while (methodSymbolIndex < methodSymbols.Size())
	{
		if (methodSymbolIndex < methodSymbols.Size() - 1 && methodSymbols[methodSymbolIndex + 1] == '(')
		{
			// Probably a method returning a function pointer
			if (methodSymbols[methodSymbolIndex + 2] == '*')
			{
				methodSymbolIndex += 3;
				method.name = methodSymbols[methodSymbolIndex];
			}
			// Otherwise just a regular method
			else
			{
				method.name = methodSymbols[methodSymbolIndex];
			}
			break;
		}
		methodSymbolIndex++;
	}
}


void BvReflector::ParseEnum(const BvVector<BvString>& symbols, u32& symbolIndex)
{
	auto& enumObjects = m_Enums.EmplaceBack();

	// Skip BV_RMETHOD declaration and attributes
	while (symbolIndex < symbols.Size() && symbols[symbolIndex++] != ')')
	{
		// Parse possible attributes
	}
	symbolIndex++;
	if (symbols[symbolIndex] == "class")
	{
		symbolIndex++;
	}

	enumObjects.name = symbols[symbolIndex++];
	while (symbolIndex < symbols.Size() && symbols[symbolIndex] != '{')
	{
		symbolIndex++;
	}
	symbolIndex++;

	while (symbolIndex < symbols.Size() && symbols[symbolIndex] != '}')
	{
		if (symbols[symbolIndex] == ',')
		{
		}
		else if (symbols[symbolIndex] == '=')
		{
			while (symbolIndex < symbols.Size() && symbols[symbolIndex] == ',')
			{
				symbolIndex++;
			}
		}
		else
		{
			enumObjects.constants.EmplaceBack(symbols[symbolIndex]);
		}
		symbolIndex++;
	}
}


void BvReflector::GenerateMetadata()
{
}