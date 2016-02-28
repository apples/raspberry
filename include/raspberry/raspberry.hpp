#ifndef RASPBERRY_HPP
#define RASPBERRY_HPP

#include <memory>
#include <utility>

/// Implementation detail for RASPBERRY_DECL_METHOD. Unstable.
#define RASPBERRY_DETAIL_QUALIFIED_METHOD(ConceptName, FuncName, CVQualifier, RefQualifier) \
template <typename R, typename... Args> \
struct ConceptName<R(Args...) CVQualifier RefQualifier> { \
private: \
    template <typename> \
    friend class raspberry::_detail::BaseAny; \
    template <typename> \
    friend class raspberry::_detail::Any_BeamConf; \
    template <typename> \
    friend class raspberry::_detail::AnyImplBase_BeamConf; \
    template <typename Next, typename Ancestor> \
    struct Virtual : Next { \
        using Next::FuncName; \
        virtual R FuncName(Args...) CVQualifier RefQualifier = 0; \
    }; \
    template <typename Next> \
    struct Virtual<Next,void> : Next { \
        virtual R FuncName(Args...) CVQualifier RefQualifier = 0; \
    }; \
    template <typename Impl, typename Base> \
    struct VirtualImpl : Base { \
        virtual R FuncName(Args... args) CVQualifier RefQualifier override final { \
            using Value = typename Impl::value_type; \
            using QValue = raspberry::_detail::MakeRef_t<CVQualifier Value RefQualifier>; \
            const auto& self = static_cast<const Impl&>(*this); \
            const auto& value = self.get_value(); \
            return const_cast<QValue>(value).FuncName(std::forward<Args>(args)...); \
        } \
    }; \
    template <typename Impl, typename Next, typename Ancestor> \
    struct NonVirtual : Next { \
        using Next::FuncName; \
        R FuncName(Args... args) CVQualifier RefQualifier { \
            using Base = raspberry::_detail::AnyImplBase<raspberry::_detail::GetConfig_t<Impl>>; \
            using QBase = raspberry::_detail::MakeRef_t<CVQualifier Base RefQualifier>; \
            auto& self = static_cast<const Impl&>(*this); \
            auto& base = self._raspberry_get_impl(); \
            return const_cast<QBase>(base).FuncName(std::forward<Args>(args)...); \
        } \
    }; \
    template <typename Impl, typename Next> \
    struct NonVirtual<Impl,Next,void> : Next { \
        R FuncName(Args... args) CVQualifier RefQualifier { \
            using Base = raspberry::_detail::AnyImplBase<raspberry::_detail::GetConfig_t<Impl>>; \
            using QBase = raspberry::_detail::MakeRef_t<CVQualifier Base RefQualifier>; \
            const auto& self = static_cast<const Impl&>(*this); \
            const auto& base = self._raspberry_get_impl(); \
            return const_cast<QBase>(base).FuncName(std::forward<Args>(args)...); \
        } \
    }; \
}

/// Declares a concept named ConceptName that implements the FuncName method.
#define RASPBERRY_DECL_METHOD(ConceptName, FuncName) \
template <typename Func> \
struct ConceptName; \
RASPBERRY_DETAIL_QUALIFIED_METHOD(ConceptName, FuncName, , ); \
RASPBERRY_DETAIL_QUALIFIED_METHOD(ConceptName, FuncName, , &); \
RASPBERRY_DETAIL_QUALIFIED_METHOD(ConceptName, FuncName, , &&); \
RASPBERRY_DETAIL_QUALIFIED_METHOD(ConceptName, FuncName, const, ); \
RASPBERRY_DETAIL_QUALIFIED_METHOD(ConceptName, FuncName, const, &); \
RASPBERRY_DETAIL_QUALIFIED_METHOD(ConceptName, FuncName, const, &&)

namespace raspberry {

namespace _detail {

/// A simple list of types.
template <typename...>
struct TypeList {};

// Forward declarations.
template <typename Config>
class BaseAny;
template <typename... Concepts>
class Any;

template <typename T>
struct MakeRef {
    using type = T&;
};

/// Given T with underlying type U, maps { U -> U&, U& -> U&, U&& -> U&& }.
template <typename T>
using MakeRef_t = typename MakeRef<T>::type;

template <typename T>
struct MakeRef<T&&> {
    using type = T&&;
};

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

template <typename Config>
struct AnyImplBase_BeamConf {
    struct BaseConfig {
        template <typename Any>
        using Base = typename Any::AnyImplBase;
    };
    using Base = InheritAll_t<BaseConfig, typename Config::Bases>;
    template <typename Concept, typename Next, typename... TailConcepts>
    using Link = typename Concept::template Virtual<Next,FindOverloadOrVoid_t<Concept,TailConcepts...>>;
};

template <typename Config>
struct AnyImplBase : BeamInheritance_t<AnyImplBase_BeamConf<Config>, typename Config::Concepts> {
    virtual ~AnyImplBase() = 0;
};

template <typename Config>
inline AnyImplBase<Config>::~AnyImplBase() {}

template <typename Any>
struct Any_BeamConf;

template <typename Config>
struct Any_BeamConf<BaseAny<Config>> {
    struct BaseConfig {
        template <typename SuperAny>
        using Base = BeamInheritance_t<Any_BeamConf, typename GetConfig_t<SuperAny>::Concepts, Any_BeamConf<SuperAny>>;
    };
    using Any = BaseAny<Config>;
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

    template <typename>
    friend struct AnyImplBase_BeamConf;

    using AnyImplBase = AnyImplBase<Config>;

    template <typename AnyImpl>
    struct AnyImpl_BeamConf {
        using Base = AnyImplBase;
        template <typename Concept, typename Next, typename... TailConcepts>
        using Link = typename Concept::template VirtualImpl<AnyImpl, Next>;
    };

    /// Implementation that contains a value.
    template <typename T, bool B = std::is_empty<T>::value>
    struct AnyImpl;

    /// Specialization that contains a value type.
    template <typename T>
    struct AnyImpl<T,false> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T>>, typename Config::AllConcepts> {
        using value_type = T;
        value_type value;
        AnyImpl(const value_type& value) : value(value) {}
        AnyImpl(value_type&& value) : value(std::move(value)) {}
        const value_type& get_value() const { return value; }
    };

    /// Specialization that contains an empty value type.
    /// Might cause problems? Needs more testing.
    template <typename T>
    struct AnyImpl<T,true> final : T, BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T>>, typename Config::AllConcepts> {
        using value_type = T;
        AnyImpl(const value_type& value) : T(value) {}
        AnyImpl(value_type&& value) : T(std::move(value)) {}
        const value_type& get_value() const { return *this; }
    };

    /// Specialization that contains a reference.
    /// Must not outlive the referred-to object.
    template <typename T>
    struct AnyImpl<T&,false> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T&>>, typename Config::AllConcepts> {
        using value_type = T;
        value_type& value;
        AnyImpl(value_type& value) : value(value) {}
        const value_type& get_value() const { return value; }
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

    /// Pointer to implementation.
    std::unique_ptr<AnyImplBase> _raspberry_impl_ptr;

public:
    /// Any must never be empty.
    BaseAny() = delete;

    /// Dispatcher.
    template <typename T>
    BaseAny(T&& t) : BaseAny(std::forward<T>(t), GetTag_t<T>{})
    {}

    /// Default behavior for storing most types.
    template <typename T>
    BaseAny(T&& t, Tags::Default) : _raspberry_impl_ptr(std::make_unique<AnyImpl<std::decay_t<T>>>(std::forward<T>(t)))
    {}

    /// Stores a reference instead of a value.
    /// Use with caution; this Any must not outlive the given reference.
    template <typename T>
    BaseAny(const std::reference_wrapper<T>& t, Tags::Reference) : _raspberry_impl_ptr(std::make_unique<AnyImpl<T&>>(t.get()))
    {}

    /// Derived to base Any upcast.
    /// This is equivalent to a static_cast, no allocation is used.
    template <typename T>
    BaseAny(T&& t, Tags::Derived) : _raspberry_impl_ptr(std::forward<T>(t)._raspberry_impl_ptr)
    {}

    /// Stores an unrelated Any as if it were a normal type.
    /// Guarantees dynamic allocation; consider using Any concept inheritance instead.
    template <typename T>
    [[deprecated("Conversion between unrelated Anys causes unnecessary dynamic allocation.")]]
    BaseAny(T&& t, Tags::Unrelated) : BaseAny(std::forward<T>(t), Tags::Default{})
    {}

    /// Gets a pointer to the implementation.
    const AnyImplBase& _raspberry_get_impl() const {
        return *_raspberry_impl_ptr;
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

/// Concatenates any number of TypeLists into a single TypeList.
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

/// Recursively unwraps Bases into their concepts and returns a flat list of all the concepts, including Concepts.
/// Example: `CollapseConcepts_t<{C0, C4}, {Any<C1>, Any<Any<C2>,C3>}> => {C0, C4, C1, C2, C3}`
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

/// Erasure container.
/// All-natural replacement for inheritance.
template <typename... Concepts>
class Any : public BaseAny<ConceptFilter<Concepts...>> {
    using Base = BaseAny<ConceptFilter<Concepts...>>;
public:
    using Base::BaseAny;
};

}; // namespace _detail

using _detail::Any;

} // namespace Raspberry

#endif // RASPBERRY_HPP
