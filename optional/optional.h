#include <stdexcept>
#include <utility>
#include <algorithm>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char *what() const noexcept override {
        return "Bad optional access";
    }
};

template<typename T>
class Optional {
public:
    Optional() = default;

    Optional(const T &value) {
        is_initialized_ = true;
        T *val = new(&data_[0]) T{value};
        (void) (&val);
    }

    Optional(T &&value) {
        is_initialized_ = true;
        T *val = new(&data_[0]) T(std::move(value));
        (void) (&val);
    }

    Optional(const Optional &other) {
        if (is_initialized_ && other.is_initialized_) {
            this->Value() = other.Value();
        } else if (!is_initialized_ && other.is_initialized_) {
            T *val = new(&data_[0]) T(other.Value());
            is_initialized_ = true;
            (void) (&val);
        } else if (is_initialized_ && !other.is_initialized_) {
            this->Reset();
        } else if (!is_initialized_ && !other.is_initialized_) {
            (void) (&data_);
        }
    }

    Optional(Optional &&other) {
        if (is_initialized_ && other.is_initialized_) {
            this->Value() = std::move(other.Value());
        } else if (!is_initialized_ && other.is_initialized_) {
            T *val = new(&data_[0]) T(std::move(other.Value()));
            is_initialized_ = true;
            (void) (&val);
        } else if (is_initialized_ && !other.is_initialized_) {
            this->Reset();
        } else if (!is_initialized_ && !other.is_initialized_) {
            (void) (&data_);
        }
    }

    Optional &operator=(const T &value) {
        if (is_initialized_) {
            this->Value() = value;
        } else {
            T *val = new(&data_[0]) T(value);
            is_initialized_ = true;
            (void) (&val);
        }
        return *this;
    }

    Optional &operator=(T &&rhs) {
        if (is_initialized_) {
            this->Value() = std::move(rhs);
        } else {
            T *val = new(&data_[0]) T(std::move(rhs));
            is_initialized_ = true;
            (void) (&val);
        }

        return *this;
    }

    Optional &operator=(const Optional &rhs) {
        if (this == &rhs) {
            return *this;
        }

        if (is_initialized_ && rhs.is_initialized_) {
            this->Value() = rhs.Value();
        } else if (is_initialized_ && !rhs.is_initialized_) {
            this->Reset();
        } else if (!is_initialized_ && rhs.is_initialized_) {
            is_initialized_ = true;
            T *val = new(&data_[0]) T(rhs.Value());
            (void) (&val);
        } else if (!is_initialized_ && !rhs.is_initialized_) {
            return *this;
        }

        return *this;
    }

    Optional &operator=(Optional &&rhs) {
        if (is_initialized_ && rhs.is_initialized_) {
            this->Value() = std::move(rhs.Value());
        } else if (is_initialized_ && !rhs.is_initialized_) {
            static_cast<T * >((void *) &data_[0])->~T();
            is_initialized_ = false;
        } else if (!is_initialized_ && rhs.is_initialized_) {
            is_initialized_ = true;
            T *val = new(&data_[0]) T{std::move(rhs.Value())};
            (void) (&val);
        } else if (!is_initialized_ && !rhs.is_initialized_) {
            return *this;
        }

        return *this;
    }

    ~Optional() {
        if (is_initialized_) {
            T *ptr = static_cast<T * >((void *) &data_[0]);
            ptr->~T();
        }

    };

    bool HasValue() const {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T &operator*() &{
        return *(static_cast<T * >((void *) &data_[0]));
    }

    const T &operator*() const &{
        return *(static_cast<const T * >((void *) &data_[0]));
    }

    T *operator->() {
        return static_cast<T * >((void *) &data_[0]);
    }

    const T *operator->() const {
        return static_cast<const T * >((void *) &data_[0]);
    }

    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T &Value() &{
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return reinterpret_cast<T &>(*(T *) (&data_[0]));
    }

    const T &Value() const &{
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return reinterpret_cast<T &>(*(T *) (&data_[0]));
    }

    T &&operator*() &&{
        return std::move(*(static_cast<T * >((void *) &data_[0])));
    }

    T &&Value() &&{
        if (!is_initialized_) {
            throw BadOptionalAccess();
        }
        return std::move(reinterpret_cast<T &>(*(T *) (&data_[0])));
    }

    void Reset() {
        if (is_initialized_) {
            static_cast<T * >((void *) &data_[0])->~T();
            is_initialized_ = false;
        }

    }

    template<typename... Args>
    void Emplace(Args &&... args) {
        if (is_initialized_) {
            Reset();
        }
        is_initialized_ = true;
        T *val = new(&data_[0]) T(std::forward<Args>(args)...);
        (void) (&val);

    }

    friend void swap(Optional<T> &first, Optional<T> &second) {
        std::swap(first.data_, second.data_);
        std::swap(first.is_initialized_, second.is_initialized_);
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    bool is_initialized_ = false;
};