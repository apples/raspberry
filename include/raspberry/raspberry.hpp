#ifndef RASPBERRY_HPP
#define RASPBERRY_HPP

#include <memory>
#include <utility>

#define RASPBERRY_DECL_METHOD(ConceptName, FuncName) \
template <typename Func> \
struct ConceptName; \
template <typename R, typename... Args> \
struct ConceptName<R(Args...)> { \
private: \
    template <typename...> \
    friend class raspberry::_detail::Any; \
    template <typename> \
    friend class raspberry::_detail::Any_BeamConf; \
    template <typename Next, typename Ancestor> \
    struct Virtual : Next { \
        using Next::FuncName; \
        virtual R FuncName(Args...) = 0; \
    }; \
    template <typename Next> \
    struct Virtual<Next,void> : Next { \
        virtual R FuncName(Args...) = 0; \
    }; \
    template <typename Impl, typename Base> \
    struct VirtualImpl : Base { \
        virtual R FuncName(Args... args) override final { \
            return static_cast<Impl&>(*this).get_value().FuncName(std::forward<Args>(args)...); \
        } \
    }; \
    template <typename Impl, typename Next, typename Ancestor> \
    struct NonVirtual : Next { \
        using Next::FuncName; \
        R FuncName(Args... args) { \
            return static_cast<Impl&>(*this).get_ptr()->FuncName(std::forward<Args>(args)...); \
        } \
    }; \
    template <typename Impl, typename Next> \
    struct NonVirtual<Impl,Next,void> : Next { \
        R FuncName(Args... args) { \
            return static_cast<Impl&>(*this).get_ptr()->FuncName(std::forward<Args>(args)...); \
        } \
    }; \
};\
template <typename R, typename... Args> \
struct ConceptName<R(Args...)const> { \
private: \
    template <typename...> \
    friend class raspberry::_detail::Any; \
    template <typename> \
    friend class raspberry::_detail::Any_BeamConf; \
    template <typename Next, typename Ancestor> \
    struct Virtual : Next { \
        using Next::FuncName; \
        virtual R FuncName(Args...) const = 0; \
    }; \
    template <typename Next> \
    struct Virtual<Next,void> : Next { \
        virtual R FuncName(Args...) const = 0; \
    }; \
    template <typename Impl, typename Base> \
    struct VirtualImpl : Base { \
        virtual R FuncName(Args... args) const override final { \
            return static_cast<const Impl&>(*this).get_value().FuncName(std::forward<Args>(args)...); \
        } \
    }; \
    template <typename Impl, typename Next, typename Ancestor> \
    struct NonVirtual : Next { \
        using Next::FuncName; \
        R FuncName(Args... args) const { \
            return static_cast<const Impl&>(*this).get_ptr()->FuncName(std::forward<Args>(args)...); \
        } \
    }; \
    template <typename Impl, typename Next> \
    struct NonVirtual<Impl,Next,void> : Next { \
        R FuncName(Args... args) const { \
            return static_cast<const Impl&>(*this).get_ptr()->FuncName(std::forward<Args>(args)...); \
        } \
    }; \
}

namespace raspberry {

namespace _detail {

template<typename, typename...>
struct BeamInheritance;

template<typename Conf, typename... Types>
using BeamInheritance_t = typename BeamInheritance<Conf, Types...>::type;

template<typename Conf>
struct BeamInheritance<Conf> {
    using type = typename Conf::Base;
};

template<typename Conf, typename HeadConcept, typename... TailConcepts>
struct BeamInheritance<Conf, HeadConcept, TailConcepts...> {
    using type = typename Conf::template Link<HeadConcept, BeamInheritance_t<Conf, TailConcepts...>, TailConcepts...>;
};

template <typename, typename...>
struct FindOverloadOrVoid;

template <typename T, typename... Us>
using FindOverloadOrVoid_t = typename FindOverloadOrVoid<T,Us...>::type;

template <typename T>
struct FindOverloadOrVoid<T> {
    using type = void;
};

template <typename T, typename U, typename... Vs>
struct FindOverloadOrVoid<T,U,Vs...> {
    using type = FindOverloadOrVoid_t<T,Vs...>;
};

template <template <typename> class C, typename T, typename U, typename... Vs>
struct FindOverloadOrVoid<C<T>,C<U>,Vs...> {
    using type = C<U>;
};

template <typename Any>
struct Any_BeamConf {
    using Base = struct{};
    template <typename Concept, typename Next, typename... TailConcepts>
    using Link = typename Concept::template NonVirtual<Any,Next,FindOverloadOrVoid_t<Concept,TailConcepts...>>;
};

template <typename... Concepts>
class Any final : public BeamInheritance_t<Any_BeamConf<Any<Concepts...>>, Concepts...> {

    struct AnyImplBase_BeamConf {
        using Base = struct {};
        template <typename Concept, typename Next, typename... TailConcepts>
        using Link = typename Concept::template Virtual<Next,FindOverloadOrVoid_t<Concept,TailConcepts...>>;
    };

    struct AnyImplBase : BeamInheritance_t<AnyImplBase_BeamConf, Concepts...> {
        virtual ~AnyImplBase() = default;
    };

    template <typename AnyImpl>
    struct AnyImpl_BeamConf {
        using Base = AnyImplBase;
        template <typename Concept, typename Next, typename... TailConcepts>
        using Link = typename Concept::template VirtualImpl<AnyImpl, Next>;
    };

    template <typename T, bool B = std::is_empty<T>::value>
    struct AnyImpl;

    template <typename T>
    struct AnyImpl<T,false> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T>>, Concepts...> {
        T value;
        AnyImpl(const T& value) : value(value) {}
        AnyImpl(T&& value) : value(std::move(value)) {}
        const T& get_value() const { return value; }
        T& get_value() { return value; }
    };

    template <typename T>
    struct AnyImpl<T,true> final : T, BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T>>, Concepts...> {
        AnyImpl(const T& value) : T(value) {}
        AnyImpl(T&& value) : T(std::move(value)) {}
        const T& get_value() const { return *this; }
        T& get_value() { return *this; }
    };

    template <typename T>
    struct AnyImpl<T&,false> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T&>>, Concepts...> {
        T& value;
        AnyImpl(T& value) : value(value) {}
        const T& get_value() const { return value; }
        T& get_value() { return value; }
    };

    std::unique_ptr<AnyImplBase> impl_ptr;

public:

    Any() = default;

    template <typename T>
    Any(T&& t) : impl_ptr(std::make_unique<AnyImpl<std::remove_reference_t<T>>>(std::forward<T>(t)))
    {}

    template <typename T>
    Any(std::reference_wrapper<T>&& t) : impl_ptr(std::make_unique<AnyImpl<T&>>(t.get()))
    {}

    AnyImplBase* get_ptr() {
        return impl_ptr.get();
    }

    const AnyImplBase* get_ptr() const {
        return impl_ptr.get();
    }
};

} // namespace _detail

using _detail::Any;

} // namespace Raspberry

#endif // RASPBERRY_HPP
