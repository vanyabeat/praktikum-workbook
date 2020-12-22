#include <cstdio>
#include <iterator>

template <typename Type> class BidirectionalList
{
  public:
	// Узел списка
	struct Node
	{
		Node() = default;
		Node(const Type& val, Node* prev, Node* next) : value(val), prev_node(prev), next_node(next)
		{
		}
		Type value;
		Node* next_node = nullptr;
		Node* prev_node = nullptr;
	};
#pragma region Constructors
	BidirectionalList() : size_(0), head_(Node())
	{
	}
#pragma endregion
#pragma region Iterator
	template <typename ValueType> struct iterator
	{
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = ValueType;
		using pointer = ValueType*;	// or also value_type*
		using reference = ValueType&; // or also value_type&
	};
#pragma endregion
	size_t size() const
	{
		return size_;
	}

	bool empty() const
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
};