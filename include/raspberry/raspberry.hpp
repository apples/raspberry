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
    template <typename> \
    friend class raspberry::_detail::BaseAny; \
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
    template <typename> \
    friend class raspberry::_detail::BaseAny; \
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

// Forward declarations.

template <typename Config>
class BaseAny;

template <typename... Concepts>
class Any;

/// A simple list of types.
template <typename...>
struct TypeList {};

template<typename, typename, typename>
struct BeamInheritance;

/// Converts multiple inheritance into a chain of single inheritance (to avoid arithmetic on `this` pointer).
/// `Conf::Base` is the leaf base class.
/// `Conf::Link<H, B, T...>` is invoked using mutual recursion.
/// - `H` is the current element in `Types...`
/// - `B` is the current base class that `Link` must inherit from (it will either be the next `Link` or `Base`)
/// - `T...` is a list of the remaining elements of `Types...`
template<typename Conf, typename Types, typename SuperConf = Conf>
using BeamInheritance_t = typename BeamInheritance<Conf, Types, SuperConf>::type;

template<typename Conf, typename SuperConf>
struct BeamInheritance<Conf, TypeList<>, SuperConf> {
    using type = typename Conf::Base;
};

template<typename Conf, typename HeadConcept, typename... TailConcepts, typename SuperConf>
struct BeamInheritance<Conf, TypeList<HeadConcept, TailConcepts...>, SuperConf> {
    using type = typename Conf::template Link<HeadConcept, BeamInheritance_t<SuperConf, TypeList<TailConcepts...>>, TailConcepts...>;
};

template <typename, typename...>
struct FindOverloadOrVoid;

/// Given T = C<X>, searches Us for C<Y> and returns it, else returns void.
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

template <typename, typename>
struct InheritAll;

/// Inherits from each `Config::Base<T>` for each `T` in `Types`.
template <typename Config, typename Types>
using InheritAll_t = typename InheritAll<Config,Types>::type;

template <typename Config, typename... Ts>
struct InheritAll<Config, TypeList<Ts...>> {
    struct type : Config::template Base<Ts>... {};
};

template <typename Any>
struct GetConfig;

/// Given a `BaseAny<Config>`, returns `Config`.
template <typename Any>
using GetConfig_t = typename GetConfig<Any>::type;

template <typename Config>
struct GetConfig<BaseAny<Config>> {
    using type = Config;
};

template <typename Any>
struct Any_BeamConf {
    struct BaseConfig {
        template <typename SuperAny>
        using Base = BeamInheritance_t<Any_BeamConf, typename GetConfig_t<SuperAny>::Concepts, Any_BeamConf<SuperAny>>;
    };
    using Base = InheritAll_t<BaseConfig, typename GetConfig_t<Any>::Bases>;
    template <typename Concept, typename Next, typename... TailConcepts>
    using Link = typename Concept::template NonVirtual<Any,Next,FindOverloadOrVoid_t<Concept,TailConcepts...>>;
};

/// Tags used for BaseAny constructor dispatching.
struct Tags {
    using Default = struct{};
    using Reference = struct{};
    using Derived = struct{};
    using Unrelated = struct{};
};

/// The backbone of Any.
template <typename Config>
class BaseAny : public BeamInheritance_t<Any_BeamConf<BaseAny<Config>>, typename Config::Concepts> {
    template <typename>
    friend class BaseAny;

    struct AnyImplBase_BeamConf {
        struct BaseConfig {
            template <typename Any>
            using Base = typename Any::AnyImplBase;
        };
        using Base = InheritAll_t<BaseConfig, typename Config::Bases>;
        template <typename Concept, typename Next, typename... TailConcepts>
        using Link = typename Concept::template Virtual<Next,FindOverloadOrVoid_t<Concept,TailConcepts...>>;
    };

    struct AnyImplBase : BeamInheritance_t<AnyImplBase_BeamConf, typename Config::Concepts> {
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
    struct AnyImpl<T,false> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T>>, typename Config::AllConcepts> {
        T value;
        AnyImpl(const T& value) : value(value) {}
        AnyImpl(T&& value) : value(std::move(value)) {}
        const T& get_value() const { return value; }
        T& get_value() { return value; }
    };

    template <typename T>
    struct AnyImpl<T,true> final : T, BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T>>, typename Config::AllConcepts> {
        AnyImpl(const T& value) : T(value) {}
        AnyImpl(T&& value) : T(std::move(value)) {}
        const T& get_value() const { return *this; }
        T& get_value() { return *this; }
    };

    template <typename T>
    struct AnyImpl<T&,false> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T&>>, typename Config::AllConcepts> {
        T& value;
        AnyImpl(T& value) : value(value) {}
        const T& get_value() const { return value; }
        T& get_value() { return value; }
    };

    template <typename>
    struct GetTag {
        using type = Tags::Default;
    };

    /// Selects the appropriate tag for constructor dispatching.
    template <typename T>
    using GetTag_t = typename GetTag<std::decay_t<T>>::type;

    template <typename T>
    struct GetTag<std::reference_wrapper<T>> {
        using type = Tags::Reference;
    };

    template <typename... Ts>
    struct GetTag<Any<Ts...>> {
        using type = std::conditional_t<std::is_base_of<AnyImplBase, typename Any<Ts...>::AnyImplBase>::value, Tags::Derived, Tags::Unrelated>;
    };

    std::unique_ptr<AnyImplBase> impl_ptr;

public:

    BaseAny() = default;

    /// Dispatcher.
    template <typename T>
    BaseAny(T&& t) : BaseAny(std::forward<T>(t), GetTag_t<T>{})
    {}

    /// Default behavior for storing most types.
    template <typename T>
    BaseAny(T&& t, Tags::Default) : impl_ptr(std::make_unique<AnyImpl<std::decay_t<T>>>(std::forward<T>(t)))
    {}

    /// Stores a reference instead of a value.
    /// Use with caution; this Any must not outlive the given reference.
    template <typename T>
    BaseAny(const std::reference_wrapper<T>& t, Tags::Reference) : impl_ptr(std::make_unique<AnyImpl<T&>>(t.get()))
    {}

    /// Derived to base Any upcast.
    /// This is equivalent to a static_cast, no allocation is used.
    template <typename T>
    BaseAny(T&& t, Tags::Derived) : impl_ptr(std::forward<T>(t).impl_ptr)
    {}

    /// Stores an unrelated Any as if it were a normal type.
    /// Guarantees dynamic allocation, consider using Any inheritance instead.
    template <typename T>
    [[deprecated("Conversion between unrelated Anys causes unnecessary dynamic allocation.")]]
    BaseAny(T&& t, Tags::Unrelated) : BaseAny(std::forward<T>(t), Tags::Default{})
    {}

    AnyImplBase* get_ptr() {
        return impl_ptr.get();
    }

    const AnyImplBase* get_ptr() const {
        return impl_ptr.get();
    }

    explicit operator bool() const {
        return bool(impl_ptr);
    }
};

template <typename... Concepts>
struct ConceptFilter;

template <typename ConceptList, typename BaseList, typename... Concepts>
struct ConceptFilterImpl;

template <typename... PrevConcepts, typename... PrevBases, typename... Nested, typename... Concepts>
struct ConceptFilterImpl <TypeList<PrevConcepts...>, TypeList<PrevBases...>, Any<Nested...>, Concepts...> :
    ConceptFilterImpl<TypeList<PrevConcepts...>, TypeList<PrevBases..., BaseAny<ConceptFilter<Nested...>>>, Concepts...>
{};

template <typename... PrevConcepts, typename... PrevBases, typename HeadConcept, typename... Concepts>
struct ConceptFilterImpl <TypeList<PrevConcepts...>, TypeList<PrevBases...>, HeadConcept, Concepts...> :
    ConceptFilterImpl<TypeList<PrevConcepts..., HeadConcept>, TypeList<PrevBases...>, Concepts...>
{};

template <typename...>
struct TypeListCat;

template <typename... Ts>
using TypeListCat_t = typename TypeListCat<Ts...>::type;

template <typename... As, typename... Bs, typename... Tail>
struct TypeListCat<TypeList<As...>, TypeList<Bs...>, Tail...> {
    using type = TypeListCat_t<TypeList<As..., Bs...>, Tail...>;
};

template <typename T>
struct TypeListCat<T> {
    using type = T;
};

template <>
struct TypeListCat<> {
    using type = TypeList<>;
};

template <typename, typename>
struct CollapseConcepts;

template <typename Concepts, typename Bases>
using CollapseConcepts_t = typename CollapseConcepts<Concepts,Bases>::type;

template <typename Concepts, typename... BaseFilters>
struct CollapseConcepts<Concepts, TypeList<BaseAny<BaseFilters>...>> {
    using type = TypeListCat_t<Concepts, typename BaseFilters::AllConcepts...>;
};

template <typename PrevConcepts, typename PrevBases>
struct ConceptFilterImpl <PrevConcepts, PrevBases> {
    using Concepts = PrevConcepts;
    using Bases = PrevBases;
    using AllConcepts = CollapseConcepts_t<Concepts, Bases>;
};

/// Filters parameters into a list of concepts and a list of Any bases.
template <typename... Concepts>
struct ConceptFilter : ConceptFilterImpl<TypeList<>, TypeList<>, Concepts...> {};

template <typename... Concepts>
class Any : public BaseAny<ConceptFilter<Concepts...>> {
    using Base = BaseAny<ConceptFilter<Concepts...>>;
    using Base::BaseAny;
    using Base::get_ptr;
    using Base::operator bool;
};

}; // namespace _detail

using _detail::Any;

} // namespace Raspberry

#endif // RASPBERRY_HPP
