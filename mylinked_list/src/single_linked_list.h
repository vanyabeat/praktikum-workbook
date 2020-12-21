#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <typeinfo>
#include <utility>

template <typename Type> class SingleLinkedList
{
	// Узел списка
	struct Node
	{
		Node() = default;
		Node(const Type& val, Node* next) : value(val), next_node(next)
		{
		}
		Type value;
		Node* next_node = nullptr;
	};

	// Шаблон класса Базовый Итератор.
	// Определяет поведение итератора на элементы односвязного списка
	// ValueType - совпадает с Type (для Iterator) либо с const Type (для ConstIterator)
	template <typename ValueType> class BasicIterator
	{
		// Класс списка объявляется дружественным, чтобы из методов списка
		// был доступ к приватной области итератора
		friend class SingleLinkedList;

		// Конвертирующий конструктор итератора из указателя на узел списка
		explicit BasicIterator(Node* node) : node_(node)
		{
		}

	  public:
		// Объявленные ниже типы сообщают стандартной библиотеке о свойствах этого итератора

		// Категория итератора - forward iterator
		// (итератор, который поддерживает операции инкремента и многократное разыменование)
		using iterator_category = std::forward_iterator_tag;
		// Тип элементов, по которым перемещается итератор
		using value_type = Type;
		// Тип, используемый для хранения смещения между итераторами
		using difference_type = std::ptrdiff_t;
		// Тип указателя на итерируемое значение
		using pointer = ValueType*;
		// Тип ссылки на итерируемое значение
		using reference = ValueType&;

		BasicIterator() = default;

		// Конвертирующий конструктор/конструктор копирования
		// При ValueType, совпадающем с Type, играет роль копирующего конструктора
		// При ValueType, совпадающем с const Type, играет роль конвертирующего конструктора
		BasicIterator(const BasicIterator<Type>& other) noexcept : node_(other.node_)
		{
		}

		// Чтобы компилятор не выдавал предупреждение об отсутствии оператора = при наличии
		// пользовательского конструктора копирования, явно объявим оператор = и
		// попросим компилятор сгенерировать его за нас.
		BasicIterator& operator=(const BasicIterator& rhs) = default;

		// Оператор сравнения итераторов (в роли второго аргумента выступает константный итератор)
		// Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
		[[nodiscard]] bool operator==(const BasicIterator<const Type>& rhs) const noexcept
		{
			return this->node_ == rhs.node_;
		}

		// Оператор, проверки итераторов на неравенство
		// Противоположен !=
		[[nodiscard]] bool operator!=(const BasicIterator<const Type>& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		// Оператор сравнения итераторов (в роли второго аргумента итератор)
		// Два итератора равны, если они ссылаются на один и тот же элемент списка, либо на end()
		[[nodiscard]] bool operator==(const BasicIterator<Type>& rhs) const noexcept
		{
			return this->node_ == rhs.node_;
		}

		// Оператор, проверки итераторов на неравенство
		// Противоположен !=
		[[nodiscard]] bool operator!=(const BasicIterator<Type>& rhs) const noexcept
		{
			return !(*this == rhs);
		}

		// Оператор прединкремента. После его вызова итератор указывает на следующий элемент списка
		// Возвращает ссылку на самого себя
		// Инкремент итератора, не указывающего на существующий элемент списка, приводит к неопределённому поведению
		BasicIterator& operator++() noexcept
		{
			assert(node_ != nullptr);
			node_ = node_->next_node;
			return *this;
			// Заглушка. Реализуйте оператор самостоятельно
		}

		// Оператор постинкремента. После его вызова итератор указывает на следующий элемент списка.
		// Возвращает прежнее значение итератора
		// Инкремент итератора, не указывающего на существующий элемент списка,
		// приводит к неопределённому поведению
		BasicIterator operator++(int) noexcept
		{
			auto this_copy(*this);
			++(*this);
			return this_copy;
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

	  private:
		Node* node_ = nullptr;
	};

  public:
	using value_type = Type;
	using reference = value_type&;
	using const_reference = const value_type&;

	// Итератор, допускающий изменение элементов списка
	using Iterator = BasicIterator<Type>;
	// Константный итератор, предоставляющий доступ для чтения к элементам списка
	using ConstIterator = BasicIterator<const Type>;

	SingleLinkedList() : size_(0), head_({})
	{
	}

	SingleLinkedList(std::initializer_list<Type> values) : size_(0), head_({})
	{
		for (auto it = std::rbegin(values); it != std::rend(values); ++it)
		{
			PushFront(*it);
		}
	}

	SingleLinkedList(const SingleLinkedList& other)
	{
		Assign(other.begin(), other.end());
	}

	SingleLinkedList& operator=(const SingleLinkedList& other)
	{
		if (this != &other)
		{
			if (other.IsEmpty())
			{
				Clear();
			}
			else
			{
				auto copy(other);
				swap(copy);
			}
		}
		return *this;
	}

	~SingleLinkedList()
	{
		Clear();
	}

	// добавляет в начало
	void PushFront(const Type& value)
	{
		head_.next_node = new Node(value, head_.next_node);
		++size_;
	}

	void Clear() noexcept
	{
		while (head_.next_node != nullptr)
		{
			auto next_element = head_.next_node;
			head_.next_node = next_element->next_node;
			delete next_element;
		}

		size_ = 0;
	}

	// Сообщает, пустой ли список за время O(1)
	[[nodiscard]] bool IsEmpty() const noexcept
	{

		return (GetSize() == 0);
	}

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
		return {};
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
		return {};
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
		return {};
	}

	// Возвращает итератор, указывающий на позицию перед первым элементом односвязного списка.
	// Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
	[[nodiscard]] Iterator before_begin() noexcept
	{
		return Iterator(&head_);
	}

	// Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
	// Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
	[[nodiscard]] ConstIterator cbefore_begin() const noexcept
	{
		return ConstIterator(const_cast<Node*>(&head_));
	}

	// Возвращает константный итератор, указывающий на позицию перед первым элементом односвязного списка.
	// Разыменовывать этот итератор нельзя - попытка разыменования приведёт к неопределённому поведению
	[[nodiscard]] ConstIterator before_begin() const noexcept
	{
		// Реализуйте самостоятельно
		return ConstIterator(&head_);
	}

	/*
	 * Вставляет элемент value после элемента, на который указывает pos.
	 * Возвращает итератор на вставленный элемент
	 * Если при создании элемента будет выброшено исключение, список останется в прежнем состоянии
	 */
	Iterator InsertAfter(ConstIterator pos, const Type& value)
	{
		Node * new_node = new Node(value, pos.node_->next_node);
		pos.node_->next_node = new_node;
		++size_;
		return Iterator{new_node};
	}

	void PopFront() noexcept
	{
		if (IsEmpty())
		{
			return;
		}
		else
		{
			Node* first_item = head_.next_node;
			Node* second_item = first_item->next_node;
			delete first_item;
			head_.next_node = second_item;
			--size_;
		}
	}

	/*
	 * Удаляет элемент, следующий за pos.
	 * Возвращает итератор на элемент, следующий за удалённым
	 */
	Iterator EraseAfter(ConstIterator pos) noexcept
	{
		// Заглушка. Реализуйте метод самостоятельно
		return {};
	}

	[[nodiscard]] size_t size() const noexcept
	{
		return GetSize();
	}

	// Возвращает количество элементов в списке за время O(1)
	[[nodiscard]] size_t GetSize() const noexcept
	{
		return size_;
	}

	// Обменивает содержимое списков за время O(1)
	void swap(SingleLinkedList& other) noexcept
	{
		std::swap(head_.next_node, other.head_.next_node);
		std::swap(size_, other.size_);
	}

	friend bool operator==(const SingleLinkedList& own, const SingleLinkedList& other)
	{
		if (own.size() != other.size())
		{
			return false;
		}
		else
		{
			Node this_head = own.head_;
			Node other_head = other.head_;
			while (this_head.next_node != nullptr && other_head.next_node != nullptr)
			{
				auto this_next_element = this_head.next_node;
				this_head.next_node = this_next_element->next_node;

				auto other_next_element = other_head.next_node;
				other_head.next_node = other_next_element->next_node;
				if (this_next_element->value != other_next_element->value)
				{
					return false;
				}
			}
		}
		return true;
	}

	friend bool operator!=(const SingleLinkedList& own, const SingleLinkedList& other)
	{
		return !(own == other);
	}

	friend bool operator<(const SingleLinkedList& lhs, const SingleLinkedList& rhs)
	{

		return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
	}

	friend bool operator>(const SingleLinkedList& lhs, const SingleLinkedList& rhs)
	{

		return (rhs < lhs);
	}

	friend bool operator<=(const SingleLinkedList& lhs, const SingleLinkedList& rhs)
	{
		return (lhs == rhs) || (lhs < rhs);
	}

	friend bool operator>=(const SingleLinkedList& lhs, const SingleLinkedList& rhs)
	{
		return (rhs <= lhs);
	}

	friend void swap(SingleLinkedList& lhs, SingleLinkedList& rhs) noexcept
	{
		lhs.swap(rhs);
	}

  private:
	template <typename InputIterator> void Assign(InputIterator from, InputIterator to)
	{
		SingleLinkedList<Type> tmp;
		Node** node_ptr = &tmp.head_.next_node;
		while (from != to)
		{
			assert(*node_ptr == nullptr);
			*node_ptr = new Node(*from, nullptr);
			++tmp.size_;
			node_ptr = &((*node_ptr)->next_node);
			++from;
		}

		swap(tmp);
	}
	size_t size_;
	Node head_;
};
