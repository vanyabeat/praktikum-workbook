#include <cstdio>
#include <iostream>
#include <iterator>
#include <vector>

template <typename Type> class BidirectionalList
{
#pragma region Node
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
#pragma endregion
  public:
#pragma region Iterator
	template <typename ValueType> class iterator
	{
	  public:
		// Класс списка объявляется дружественным, чтобы из методов списка
		// был доступ к приватной области итератора
		friend class BidirectionalList;

		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = ValueType;
		using pointer = ValueType*;	  // or also value_type*
		using reference = ValueType&; // or also value_type&

		// Конвертирующий конструктор итератора из указателя на узел списка
		explicit iterator(Node* node) : node_(node)
		{
		}
		iterator() = default;

#pragma region Operators

		// Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
		// пользовательского конструктора копирования, явно объявим оператор = и
		// попросим компилятор сгенерировать его за нас.
		iterator& operator=(const iterator& rhs) = default;

		// Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
		// Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
		[[nodiscard]] bool operator==(const iterator<const Type>& rhs) const noexcept
		{
			return this->node_ == rhs.node_;
		}

		// Оператор, проверки итераторов на неравенство
		// Противоположен !=
		[[nodiscard]] bool operator!=(const iterator<const Type>& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		// Оператор сравнения итераторов (в роли второго аргумента итератор)
		// Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
		[[nodiscard]] bool operator==(const iterator<Type>& rhs) const noexcept
		{
			return this->node_ == rhs.node_;
		}

		// Оператор, проверки итераторов на неравенство
		// Противоположен !=
		[[nodiscard]] bool operator!=(const iterator<Type>& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		// Операция разыменования. Возвращает ссылку на текущий элемент
		// Вызов этого оператора, у итератора, не указывающего на существующий элемент списка,
		// приводит к неопределённому поведению
		[[nodiscard]] reference operator*() const noexcept
		{
			assert(node_ != nullptr);
			return node_->value;
		}

		// Операция доступа к члену класса. Возвращает указатель на текущий элемент списка.
		// Вызов этого оператора, у итератора, не указывающего на существующий элемент списка,
		// приводит к неопределённому поведению
		[[nodiscard]] pointer operator->() const noexcept
		{
			assert(node_ != nullptr);
			return &node_->value;
		}

		// Оператор прединкремента. После его вызова итератор указывает на следующий элемент списка
		// Возвращает ссылку на самого себя
		// Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
		iterator& operator++() noexcept
		{
			assert(node_ != nullptr);
			node_ = node_->next_node;
			return *this;
			// Заглушка. Реализуйте оператор самостоятельно
		}

		// Оператор предекремента. После его вызова итератор указывает на предыдущий элемент списка
		// Возвращает ссылку на самого себя
		// Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
		iterator& operator--() noexcept
		{
			assert(node_ != nullptr);
			node_ = node_->prev_node;
			return *this;
			// Заглушка. Реализуйте оператор самостоятельно
		}

		// Оператор постинкремента. После его вызова итератор указывает на следующий элемент списка.
		// Возвращает прежнее значение итератора
		// Инкремент итератора, не указывающего на существующий элемент списка,
		// приводит к неопределённому поведению
		iterator operator++(int) noexcept
		{
			auto this_copy(*this);
			++(*this);
			return this_copy;
		}

		// Оператор постдекремента. После его вызова итератор указывает на предыдущий элемент списка.
		// Возвращает прежнее значение итератора
		// Инкремент итератора, не указывающего на существующий элемент списка,
		// приводит к неопределённому поведению
		iterator operator--(int) noexcept
		{
			auto this_copy(*this);
			--(*this);
			return this_copy;
		}
#pragma endregion
	  private:
		Node* node_ = nullptr;
	};
#pragma endregion
#pragma region Usings
	using value_type = Type;
	using reference = value_type&;
	using const_reference = const value_type&;

	// Итератор, допускающий изменение элементов списка
	using Iterator = iterator<Type>;
	// Константный итератор, предоставляющий доступ для чтения к элементам списка
	using ConstIterator = iterator<const Type>;
#pragma endregion
#pragma region Constructors_and_Destructors
	BidirectionalList() : size_(0), head_(Node()), tail_(Node())
	{
		head_.next_node = &tail_;
		tail_.prev_node = &head_;
	}
	~BidirectionalList()
	{
		clear();
	}
#pragma endregion
#pragma region Methods
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
#pragma endregion
#pragma region Accessors
	// Возвращает итератор, ссылающийся на первый элемент
	// Если список пустой, возвращённый итератор будет равен end()
	[[nodiscard]] Iterator begin() noexcept
	{
		return Iterator{head_.next_node};
	}

	// Возвращает итератор, указывающий на позицию, следующую за последним элементом односвязного списка
	// Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
	[[nodiscard]] Iterator end() noexcept
	{
		return Iterator{&tail_};
	}

	// Возвращает константный итератор, ссылающийся на первый элемент
	// Если список пустой, возвращённый итератор будет равен end()
	// Результат вызова эквивалентен вызову метода cbegin()
	[[nodiscard]] ConstIterator begin() const noexcept
	{
		return ConstIterator{head_.next_node};
	}

	// Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
	// Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
	// Результат вызова эквивалентен вызову метода cend()
	[[nodiscard]] ConstIterator end() const noexcept
	{
		return ConstIterator{const_cast<Node*>(&tail_)};
	}

	// Возвращает константный итератор, ссылающийся на первый элемент
	// Если список пустой, возвращённый итератор будет равен cend()
	[[nodiscard]] ConstIterator cbegin() const noexcept
	{
		return ConstIterator{head_.next_node};
	}

	// Возвращает константный итератор, указывающий на позицию, следующую за последним элементом односвязного списка
	// Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
	[[nodiscard]] ConstIterator cend() const noexcept
	{
		return ConstIterator{const_cast<Node*>(&tail_)};
	}
#pragma endregion

  private:
	size_t size_;
	Node head_;
	Node tail_;
};