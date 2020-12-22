#include <cstdio>
#include <iostream>
#include <iterator>
#include <vector>

template <typename Type> class BidirectionalList
{
  public:
	// Узел списка
	struct Node
	{
		Node() = default;
		Node(Node* prev, const Type& val, Node* next) : prev_node(prev), value(val), next_node(next)
		{
		}
		Type value;
		Node* next_node = nullptr;
		Node* prev_node = nullptr;
	};
#pragma region Constructors_and_Destructors
	BidirectionalList() : size_(0), head_(Node()), tail_(Node())
	{
		head_.next_node = &tail_;
		tail_.prev_node = &head_;
	}
	~BidirectionalList(){
		clear();
	}
#pragma endregion
#pragma region Iterator
	template <typename ValueType> struct iterator
	{
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = ValueType;
		using pointer = ValueType*;	  // or also value_type*
		using reference = ValueType&; // or also value_type&

		iterator(pointer ptr) : iterator_ptr_(ptr)
		{
		}
		pointer iterator_ptr_;
	};
#pragma endregion

	// добавляет в начало O(1)
	void push_front(const Type& value)
	{
		if (empty())
		{
			Node* tmp = new Node(&head_, value, &tail_);
			head_.next_node = tmp;
			tail_.prev_node = tmp;
		}
		else
		{
			head_.next_node = new Node(&head_, value, head_.next_node);
		}
		++size_;
	}

	// добавляет в конец O(1)
	void push_back(const Type& value)
	{
		if (empty())
		{
			Node* tmp = new Node(&head_, value, &tail_);
			head_.next_node = tmp;
			tail_.prev_node = tmp;
		}
		else
		{
			tail_.prev_node = new Node(tail_.prev_node, value, &tail_);
		}
		++size_;
	}

	void clear() noexcept
	{
		while (head_.next_node != &tail_)
		{
			Node* next_element = head_.next_node;
			head_.next_node = next_element->next_node;
			delete next_element;
		}

		size_ = 0;
	}

	[[nodiscard]] size_t size() const noexcept
	{
		return size_;
	}

	[[nodiscard]] bool empty() const noexcept
	{
		if (size_ == 0u)
		{
			return true;
		}
		return false;
	}

  private:
	size_t size_;
	Node head_;
	Node tail_;
};