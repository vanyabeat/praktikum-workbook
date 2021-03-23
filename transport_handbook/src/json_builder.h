#pragma once
#include "json.h"
#include <memory>
#include <stack>

namespace json
{

	class DictValueCtx;
	class DictItemCtx;
	class ArrayItemContext;

	class Builder
	{
	  public:
		json::DictValueCtx Key(std::string key);

		Builder& Value(json::Node::Value value);

		json::DictItemCtx StartDict();

		Builder& EndDict();

		json::ArrayItemContext StartArray();

		Builder& EndArray();

		json::Node Build();

	  private:
		bool IsKeyOnTop()
		{
			return (stack_.size() > 1) && stack_.top()->IsString();
		}
		enum class State
		{
			EMPTY,
			STAGING,
			ENDED
		};
		State state_ = State::EMPTY;
		std::stack<std::unique_ptr<Node>> stack_;
	};

	class ArrayItemContext
	{
	  public:
		ArrayItemContext(Builder& b) : b_(b)
		{
		}
		ArrayItemContext Value(const json::Node::Value& value)
		{
			return ArrayItemContext(b_.Value(value));
		}
		DictItemCtx StartDict();

		ArrayItemContext StartArray()
		{
			return b_.StartArray();
		}
		Builder& EndArray()
		{
			return b_.EndArray();
		}

	  private:
		Builder& b_;
	};

	class DictItemCtx
	{
	  public:
		DictItemCtx(Builder& b) : b_(b)
		{
		}

		DictValueCtx Key(const std::string& key);

		Builder& EndDict()
		{
			return b_.EndDict();
		}

	  private:
		Builder& b_;
	};

	class DictValueCtx
	{
	  public:
		DictValueCtx(Builder& b) : b_(b)
		{
		}
		DictItemCtx Value(const json::Node::Value& value)
		{
			return DictItemCtx(b_.Value(value));
		}
		DictItemCtx StartDict()
		{
			return b_.StartDict();
		}
		ArrayItemContext StartArray()
		{
			return b_.StartArray();
		}

	  private:
		Builder& b_;
	};
} // namespace json
