#include "runtime.h"
#include <functional>
#include <gtest/gtest.h>

using namespace runtime;
class Logger : public Object
{
  public:
	static int instance_count;

	explicit Logger(int value_ = 0) : id_(value_)
	{
		++instance_count;
	}

	Logger(const Logger& rhs) : id_(rhs.id_)
	{
		++instance_count;
	}

	Logger(Logger&& rhs) noexcept : id_(rhs.id_)
	{
		++instance_count;
	}

	Logger& operator=(const Logger& /*rhs*/) = default;
	Logger& operator=(Logger&& /*rhs*/) = default;

	[[nodiscard]] int GetId() const
	{
		return id_;
	}

	~Logger()
	{
		--instance_count;
	}

	void Print(std::ostream& os, [[maybe_unused]] runtime::Context& context) override
	{
		os << id_;
	}

  private:
	int id_;
};

int Logger::instance_count = 0;

TEST(RuntimeTest, TestNumber)
{
	using namespace std;
	runtime::Number num(127);

	runtime::DummyContext context;

	num.Print(context.output, context);
	ASSERT_EQ(context.output.str(), "127"s);
	ASSERT_EQ(num.GetValue(), 127);
}

TEST(RuntimeTest, TestString)
{
	using namespace std;

	String word("hello!"s);

	DummyContext context;
	word.Print(context.output, context);
	ASSERT_EQ(context.output.str(), "hello!"s);
	ASSERT_EQ(word.GetValue(), "hello!"s);
}

struct TestMethodBody : Executable
{
	using Fn = std::function<ObjectHolder(Closure& closure, Context& context)>;
	Fn body;

	explicit TestMethodBody(Fn body) : body(std::move(body))
	{
	}

	ObjectHolder Execute(Closure& closure, Context& context) override
	{
		if (body)
		{
			return body(closure, context);
		}
		return {};
	}
};

TEST(RuntimeTest, TestNonowning)
{
	using namespace std;
	ASSERT_EQ(Logger::instance_count, 0);
	Logger logger(784);
	{
		auto oh = ObjectHolder::Share(logger);
		ASSERT_TRUE(oh);
	}
	ASSERT_EQ(Logger::instance_count, 1);

	auto oh = ObjectHolder::Share(logger);
	ASSERT_TRUE(oh);
	ASSERT_TRUE(oh.Get() == &logger);

	DummyContext context;
	oh->Print(context.output, context);

	ASSERT_EQ(context.output.str(), "784"sv);
}

TEST(RuntimeTest, TestOwning)
{
	using namespace std;
	ASSERT_EQ(Logger::instance_count, 0);
	{
		auto oh = ObjectHolder::Own(Logger());
		ASSERT_TRUE(oh);
		ASSERT_EQ(Logger::instance_count, 1);
	}
	ASSERT_EQ(Logger::instance_count, 0);

	auto oh = ObjectHolder::Own(Logger(312));
	ASSERT_TRUE(oh);
	ASSERT_EQ(Logger::instance_count, 1);

	DummyContext context;
	oh->Print(context.output, context);

	ASSERT_EQ(context.output.str(), "312"sv);
}

TEST(RuntimeTest, TestMove)
{
	{
		ASSERT_EQ(Logger::instance_count, 0);
		Logger logger;

		auto one = ObjectHolder::Share(logger);
		ObjectHolder two = std::move(one);

		ASSERT_EQ(Logger::instance_count, 1);
		ASSERT_TRUE(two.Get() == &logger);
	}
	{
		ASSERT_EQ(Logger::instance_count, 0);
		auto one = ObjectHolder::Own(Logger());
		ASSERT_EQ(Logger::instance_count, 1);
		Object* stored = one.Get();
		ObjectHolder two = std::move(one);
		ASSERT_EQ(Logger::instance_count, 1);

		ASSERT_TRUE(two.Get() == stored);
		ASSERT_TRUE(!one); // NOLINT
	}
}

TEST(RuntimeTest, TestNullptr)
{
	using namespace std;
	ObjectHolder oh;
	ASSERT_TRUE(!oh);
	ASSERT_TRUE(!oh.Get());
}