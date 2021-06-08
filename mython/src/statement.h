#pragma once

#include "runtime.h"
#include <exception>
#include <functional>
#include <utility>

namespace ast
{

using Statement = runtime::Executable;

template <typename T> class ValueStatement : public Statement
{
  public:
	explicit ValueStatement(T v) : value_(std::move(v))
	{
	}

	runtime::ObjectHolder Execute([[maybe_unused]] runtime::Closure& closure,
								  [[maybe_unused]] runtime::Context& context) override
	{
		return runtime::ObjectHolder::Share(value_);
	}

  private:
	T value_;
};

using NumericConst = ValueStatement<runtime::Number>;
using StringConst = ValueStatement<runtime::String>;
using BoolConst = ValueStatement<runtime::Bool>;

struct ObjRet : public std::runtime_error
{
  public:
	ObjRet(runtime::ObjectHolder obj) : std::runtime_error::runtime_error(std::string()), object_holder_(obj)
	{
	}

	runtime::ObjectHolder Get_ObjHldr()
	{
		return this->object_holder_;
	}

  private:
	runtime::ObjectHolder object_holder_;
};

class VariableValue : public Statement
{
  public:
	explicit VariableValue(const std::string& var_name);

	explicit VariableValue(std::vector<std::string> dot_ids);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::vector<std::string> var_names_;
};

class Assignment : public Statement
{
  public:
	Assignment(std::string var, std::unique_ptr<Statement> rv);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::string var_;
	std::unique_ptr<Statement> rv_;
};

class FieldAssignment : public Statement
{
  public:
	FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::string str_name_;
	VariableValue obj_;
	std::unique_ptr<Statement> rv_;
};

class None : public Statement
{
  public:
	runtime::ObjectHolder Execute([[maybe_unused]] runtime::Closure& closure,
								  [[maybe_unused]] runtime::Context& context) override
	{
		return {};
	}
};

class Print : public Statement
{
  public:
	explicit Print(std::unique_ptr<Statement> argument);

	explicit Print(std::vector<std::unique_ptr<Statement>> args);

	static std::unique_ptr<Print> Variable(const std::string& name);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::vector<std::unique_ptr<Statement>> args_;
};

class MethodCall : public Statement
{
  public:
	MethodCall(std::unique_ptr<Statement> object, std::string method, std::vector<std::unique_ptr<Statement>> args);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::unique_ptr<Statement> object_;
	std::string method_;
	std::vector<std::unique_ptr<Statement>> args_;
};

class NewInstance : public Statement
{
  public:
	explicit NewInstance(const runtime::Class& class_);

	NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	const runtime::Class& cls_;
	std::vector<std::unique_ptr<Statement>> args_;
};

class UnaryOperation : public Statement
{
  public:
	explicit UnaryOperation(std::unique_ptr<Statement> argument) : arg_(std::move(argument))
	{
	}
	std::unique_ptr<Statement> arg_;
};

class Stringify : public UnaryOperation
{
  public:
	using UnaryOperation::UnaryOperation;

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class BinaryOperation : public Statement
{
  public:
	BinaryOperation(std::unique_ptr<Statement> lhs, std::unique_ptr<Statement> rhs)
		: lhs_(std::move(lhs)), rhs_(std::move(rhs))
	{
	}
	std::unique_ptr<Statement> lhs_;
	std::unique_ptr<Statement> rhs_;
};

class Add : public BinaryOperation
{
  public:
	using BinaryOperation::BinaryOperation;

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Sub : public BinaryOperation
{
  public:
	using BinaryOperation::BinaryOperation;

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Mult : public BinaryOperation
{
  public:
	using BinaryOperation::BinaryOperation;

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Div : public BinaryOperation
{
  public:
	using BinaryOperation::BinaryOperation;

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Or : public BinaryOperation
{
  public:
	using BinaryOperation::BinaryOperation;

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class And : public BinaryOperation
{
  public:
	using BinaryOperation::BinaryOperation;

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Not : public UnaryOperation
{
  public:
	using UnaryOperation::UnaryOperation;

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;
};

class Compound : public Statement
{
  public:
	// Конструирует Compound из нескольких инструкций типа unique_ptr<Statement>
	template <typename... Args> explicit Compound(Args&&... args)
	{
		((this->AddStatement(std::move(args))), ...);
	}

	// Добавляет очередную инструкцию в конец составной инструкции
	void AddStatement(std::unique_ptr<Statement> stmt)
	{
		this->args_.push_back(std::move(stmt));
	}

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::vector<std::unique_ptr<Statement>> args_;
};

// Тело метода. Как правило, содержит составную инструкцию
class MethodBody : public Statement
{
  public:
	explicit MethodBody(std::unique_ptr<Statement>&& body);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::unique_ptr<Statement> body_;
};

// Выполняет инструкцию return с выражением statement
class Return : public Statement
{
  public:
	explicit Return(std::unique_ptr<Statement> statement) : st_(std::move(statement))
	{
	}

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::unique_ptr<Statement> st_;
};

// Объявляет класс
class ClassDefinition : public Statement
{
  public:
	explicit ClassDefinition(runtime::ObjectHolder cls);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	runtime::ObjectHolder cls_;
};

// Инструкция if <condition> <if_body> else <else_body>
class IfElse : public Statement
{
  public:
	// Параметр else_body может быть равен nullptr
	IfElse(std::unique_ptr<Statement> condition, std::unique_ptr<Statement> if_body,
		   std::unique_ptr<Statement> else_body);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	std::unique_ptr<Statement> condition_;
	std::unique_ptr<Statement> if_block_;
	std::unique_ptr<Statement> else_block_;
};

// Операция сравнения
class Comparison : public BinaryOperation {
  public:
	// Comparator задаёт функцию, выполняющую сравнение значений аргументов
	using Comparator =
		std::function<bool(const runtime::ObjectHolder&, const runtime::ObjectHolder&, runtime::Context&)>;

	Comparison(Comparator cmp, std::unique_ptr<Statement> lhs, std::unique_ptr<Statement> rhs);

	runtime::ObjectHolder Execute(runtime::Closure& closure, runtime::Context& context) override;

  private:
	Comparator cmp_;
};

} // namespace ast