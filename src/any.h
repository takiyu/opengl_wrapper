#ifndef ANY_H_20180617
#define ANY_H_20180617

#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <typeinfo>

namespace oglw {

class BadAnyCast : public std::logic_error {
public:
    BadAnyCast(const std::type_info& tried_type_,
               const std::type_info& correct_type_)
        : std::logic_error("BadAnyCast"),
          tried_type(&tried_type_),
          correct_type(&correct_type_),
          m_msg(std::string("Any::cast() failed : (tried: ") +
                tried_type_.name() + std::string(", correct: ") +
                correct_type_.name() + std::string(")")) {}

    const char* what() const noexcept {
        return m_msg.c_str();
    }

    const std::type_info* tried_type;
    const std::type_info* correct_type;

private:
    std::string m_msg;
};

class Any {
public:
    Any() {}

    Any(const Any& other) {
        other.m_cloner(*this, other);
    }

    Any& operator=(const Any& other) {
        other.m_cloner(*this, other);
        return *this;
    }

    Any(Any&& other) {
        m_data = other.m_data;
        m_type = other.m_type;
        m_cloner = other.m_cloner;
        // Clear other's data
        other.m_data.reset();
        other.m_type = &typeid(void);
    }

    Any& operator=(Any&& other) {
        m_data = other.m_data;
        m_type = other.m_type;
        m_cloner = other.m_cloner;
        // Clear other's data
        other.m_data.reset();
        other.m_type = &typeid(void);
        return *this;
    }

    template <typename T>
    Any(T&& value) {
        using TypeCVR = T;
        using TypeCV = typename std::remove_reference<TypeCVR>::type;
        using Type = typename std::remove_cv<TypeCV>::type;
        emplace<Type>(std::forward<TypeCV>(value));
    }

    template <typename T, typename Decayed = std::decay_t<T>>
    using Decay = std::enable_if_t<!std::is_same<Decayed, Any>::value, Decayed>;

    template <typename T>
    using CopyConstractable =
            std::enable_if_t<std::is_copy_constructible<Decay<T>>::value, Any&>;

    template <typename T>
    CopyConstractable<T> operator=(T&& value) {
        using TypeCVR = T;
        using TypeCV = typename std::remove_reference<TypeCVR>::type;
        using Type = typename std::remove_cv<TypeCV>::type;
        emplace<Type>(std::forward<TypeCV>(value));
        return *this;
    }

    ~Any() {}

    template <typename T, typename... Args>
    void emplace(Args&&... args) {
        m_data = std::make_shared<T>(std::forward<Args>(args)...);
        m_type = &typeid(T);
        m_cloner = [](Any& dst, const Any& src) {
            dst.emplace<T>(src.cast<T>());
        };
    }

    void reset() noexcept {
        m_data.reset();
        m_type = &typeid(void);
    }

    bool hasValue() const noexcept {
        return m_type != &typeid(void);
    }

    const std::type_info& type() const {
        return *m_type;
    }

    template <typename T>
    T cast() const {
        if (typeid(T) != *m_type) {
            // Invalid type cast
            throw(BadAnyCast(typeid(T), *m_type));
        }
        return *std::static_pointer_cast<T>(m_data);
    }

private:
    std::shared_ptr<void> m_data;
    const std::type_info* m_type = &typeid(void);
    std::function<void(Any&, const Any&)> m_cloner;
};

}  // namespace oglw

#endif /* end of include guard */
