
#include <string>
#include <iostream>

#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libsolidity/Scanner.h>
#include <libsolidity/Parser.h>
#include <libsolidity/ASTPrinter.h>
#include <libsolidity/NameAndTypeResolver.h>
#include <libsolidity/Exceptions.h>
#include <libsolidity/Compiler.h>

using namespace dev;
using namespace solidity;


/// Helper class that extracts the first expression in an AST.
class FirstExpressionExtractor: private ASTVisitor
{
public:
	FirstExpressionExtractor(ASTNode& _node): m_expression(nullptr) { _node.accept(*this); }
	Expression* getExpression() const { return m_expression; }
private:
	virtual bool visit(Expression& _expression) override { return checkExpression(_expression); }
	virtual bool visit(Assignment& _expression) override { return checkExpression(_expression); }
	virtual bool visit(UnaryOperation& _expression) override { return checkExpression(_expression); }
	virtual bool visit(BinaryOperation& _expression) override { return checkExpression(_expression); }
	virtual bool visit(FunctionCall& _expression) override { return checkExpression(_expression); }
	virtual bool visit(MemberAccess& _expression) override { return checkExpression(_expression); }
	virtual bool visit(IndexAccess& _expression) override { return checkExpression(_expression); }
	virtual bool visit(PrimaryExpression& _expression) override { return checkExpression(_expression); }
	virtual bool visit(Identifier& _expression) override { return checkExpression(_expression); }
	virtual bool visit(ElementaryTypeNameExpression& _expression) override { return checkExpression(_expression); }
	virtual bool visit(Literal& _expression) override { return checkExpression(_expression); }
	bool checkExpression(Expression& _expression)
	{
		if (m_expression == nullptr)
			m_expression = &_expression;
		return false;
	}
private:
	Expression* m_expression;
};

void printSourcePart(std::ostream& _stream, Location const& _location, Scanner const& _scanner)
{
	int startLine;
	int startColumn;
	std::tie(startLine, startColumn) = _scanner.translatePositionToLineColumn(_location.start);
	_stream << " starting at line " << (startLine + 1) << ", column " << (startColumn + 1) << "\n";
	int endLine;
	int endColumn;
	std::tie(endLine, endColumn) = _scanner.translatePositionToLineColumn(_location.end);
	if (startLine == endLine)
	{
		_stream << _scanner.getLineAtPosition(_location.start) << "\n"
				<< std::string(startColumn, ' ') << "^";
		if (endColumn > startColumn + 2)
			_stream << std::string(endColumn - startColumn - 2, '-');
		if (endColumn > startColumn + 1)
			_stream << "^";
		_stream << "\n";
	}
	else
	{
		_stream << _scanner.getLineAtPosition(_location.start) << "\n"
				<< std::string(startColumn, ' ') << "^\n"
				<< "Spanning multiple lines.\n";
	}
}

std::string compile(std::string _input)
{
	ASTPointer<ContractDefinition> ast;
	std::shared_ptr<Scanner> scanner = std::make_shared<Scanner>(CharStream(_input));
	Parser parser;
	try
	{
		ast = parser.parse(scanner);
	}
	catch (ParserError const& exc)
	{
		int line;
		int column;
		std::tie(line, column) = scanner->translatePositionToLineColumn(exc.getPosition());
		std::ostringstream error;
		error << exc.what() << " at line " << (line + 1) << ", column " << (column + 1) << std::endl;
		error << scanner->getLineAtPosition(exc.getPosition()) << std::endl;
		error << std::string(column, ' ') << "^" << std::endl;
		return error.str();
	}

	dev::solidity::NameAndTypeResolver resolver;
	try
	{
		resolver.resolveNamesAndTypes(*ast.get());
	}
	catch (DeclarationError const& exc)
	{
		std::ostringstream error;
		error << exc.what() << std::endl;
		printSourcePart(std::cerr, exc.getLocation(), *scanner);
		return error.str();
	}
	catch (TypeError const& exc)
	{
		std::ostringstream error;
		error << exc.what() << std::endl;
		printSourcePart(std::cerr, exc.getLocation(), *scanner);
		return error.str();
	}

	std::ostringstream output;
	FirstExpressionExtractor extractor(*ast);
	CompilerContext context;
	ExpressionCompiler compiler(context);
	if (extractor.getExpression())
	{
		compiler.compile(*extractor.getExpression());
		bytes instructions = compiler.getAssembledBytecode();

		output << "Bytecode for the first expression: " << std::endl;
		output << eth::disassemble(instructions) << std::endl;
	}

	output << "\n-------------------------------------\n"
		   << "Syntax tree for the contract:" << std::endl;
	dev::solidity::ASTPrinter printer(ast, _input);
	printer.print(output);

	return output.str();
}


static std::string outputBuffer;

extern "C"
{
extern char const* compileString(char const* _input)
{
	outputBuffer = compile(_input);
	return outputBuffer.c_str();
}
}
