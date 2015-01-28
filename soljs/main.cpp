/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Javascript interface for the solidity compiler.
 */

#include <string>
#include <iostream>

#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libevmcore/Instruction.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/ASTPrinter.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/CompilerStack.h>
#include <libsolidity/SourceReferenceFormatter.h>

using namespace std;
using namespace dev;
using namespace solidity;

string wordwrap(string const& _text)
{
	const int c_softLimit = 80;
	const int c_hardLimit = 128;
	int lineLength = 0;
	string output;
	for (char c: _text) {
		if (c == '\n')
			lineLength = 0;
		else
			lineLength++;
		output.push_back(c);
		if ((lineLength >= c_softLimit && c == ' ') || lineLength >= c_hardLimit) {
			output.push_back('\n');
			lineLength = 0;
		}
	}
	return output;
}

string compile(string _input, bool optimize)
{
	ostringstream output;

	CompilerStack compiler;
	try
	{
		compiler.compile(_input, optimize);
	}
	catch (ParserError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(output, exception, "Parser error", compiler);
		return output.str();
	}
	catch (DeclarationError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(output, exception, "Declaration error", compiler);
		return output.str();
	}
	catch (TypeError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(output, exception, "Type error", compiler);
		return output.str();
	}
	catch (CompilerError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(output, exception, "Compiler error", compiler);
		return output.str();
	}
	catch (InternalCompilerError const& exception)
	{
		SourceReferenceFormatter::printExceptionInformation(output, exception, "Internal compiler error", compiler);
		return output.str();
	}
	catch (Exception const& exception)
	{
		output << "Exception during compilation: " << boost::diagnostic_information(exception) << endl;
		return output.str();
	}
	catch (...)
	{
		output << "Unknown exception during compilation." << endl;
		return output.str();
	}

	vector<string> contracts = compiler.getContractNames();
	output << endl << "Contracts:" << endl;
	for (string const& contract: contracts)
	{
		output << endl << "======= " << contract << " =======" << endl
			<< "Opcodes:" << endl
			<< wordwrap(eth::disassemble(compiler.getBytecode(contract))) << endl
			<< "Hex: " << wordwrap(toHex(compiler.getBytecode(contract))) << endl
			<< "EVM assembly:" << endl;
		compiler.streamAssembly(output, contract);
	}

	output << "Syntax tree:" << endl;
	dev::solidity::ASTPrinter printer(compiler.getAST(), _input);
	printer.print(output);
	return output.str();
}

static string outputBuffer;

extern "C"
{
extern char const* compileString(char const* _input, bool optimize)
{
	outputBuffer = compile(_input, optimize);
	return outputBuffer.c_str();
}
}
