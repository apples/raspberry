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
    friend struct raspberry::_detail::Any_BeamConf; \
    template <typename AnyImpl> \
    friend struct raspberry::_detail::anyimpl::AnyImpl_BeamConf; \
    template <typename> \
    friend struct raspberry::_detail::anyimplbase::AnyImplBase_BeamConf; \
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

namespace typelist {

    /// A simple list of types.
    template <typename...>
    struct TypeList {};

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

} // namespace typelist

using typelist::TypeList;
using typelist::TypeListCat_t;

namespace makeref {

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

} // namespace makeref

using makeref::MakeRef_t;

// Forward declarations.
template <typename Config>
class BaseAny;
template <typename... Concepts>
class Any;

namespace inheritance {

    template <typename, typename>
    struct InheritAll;

    /// Inherits from each `Config::Base<T>` for each `T` in `Types`.
    template <typename Config, typename Types>
    using InheritAll_t = typename InheritAll<Config,Types>::type;

    template <typename Config, typename... Ts>
    struct InheritAll<Config, TypeList<Ts...>> {
        struct type : Config::template Base<Ts>... {};
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

} // namespace inheritance

using inheritance::InheritAll_t;
using inheritance::BeamInheritance_t;

namespace find_overload {

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

} // namespace find_overload

using find_overload::FindOverloadOrVoid_t;

namespace get_config {

    template <typename Any>
    struct GetConfig;

    /// Given a `BaseAny<Config>`, returns `Config`.
    template <typename Any>
    using GetConfig_t = typename GetConfig<Any>::type;

    template <typename Config>
    struct GetConfig<BaseAny<Config>> {
        using type = Config;
    };

} // namespace get_config

using get_config::GetConfig_t;

namespace anyimplbase {

    template <typename Config>
    struct AnyImplBase_BeamConf {
        struct BaseConfig {
            template <typename Any>
            using Base = typename Any::_raspberry_impl_base;
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

} // namespace anyimplbase

using anyimplbase::AnyImplBase;

namespace anyimpl {

    template <typename T, typename Config>
    struct AnyImpl;

    template <typename AnyImpl>
    struct AnyImpl_BeamConf;

    template <typename T, typename Config>
    struct AnyImpl_BeamConf<AnyImpl<T,Config>> {
        using Base = AnyImplBase<Config>;
        template <typename Concept, typename Next, typename... TailConcepts>
        using Link = typename Concept::template VirtualImpl<AnyImpl<T,Config>, Next>;
    };

    /// Implementation that contains a value.
    template <typename T, typename Config>
    struct AnyImpl final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T,Config>>, typename Config::AllConcepts> {
        using value_type = T;
        value_type value;
        AnyImpl(const value_type& value) : value(value) {}
        AnyImpl(value_type&& value) : value(std::move(value)) {}
        const value_type& get_value() const { return value; }
    };

    /// Specialized implementation that contains a reference.
    /// Must not outlive the referred-to object.
    template <typename T, typename Config>
    struct AnyImpl<T&,Config> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T&,Config>>, typename Config::AllConcepts> {
        using value_type = T;
        value_type& value;
        AnyImpl(value_type& value) : value(value) {}
        const value_type& get_value() const { return value; }
    };

    /// Specialized implementation that contains a pointer.
    /// Must not outlive the pointed-to object.
    template <typename T, typename Config>
    struct AnyImpl<T*,Config> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T*,Config>>, typename Config::AllConcepts> {
        using value_type = T;
        value_type* value;
        AnyImpl(value_type* value) : value(value) {}
        const value_type& get_value() const { return *value; }
    };

    /// Specialized implementation that contains a std::unique_pointer.
    template <typename T, typename Config>
    struct AnyImpl<std::unique_ptr<T>,Config> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<std::unique_ptr<T>,Config>>, typename Config::AllConcepts> {
        using value_type = T;
        std::unique_ptr<T> value;
        AnyImpl(std::unique_ptr<T> value) : value(std::move(value)) {}
        const value_type& get_value() const { return *value; }
    };

    /// Specialized implementation that contains a std::shared_pointer.
    template <typename T, typename Config>
    struct AnyImpl<std::shared_ptr<T>,Config> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<std::shared_ptr<T>,Config>>, typename Config::AllConcepts> {
        using value_type = T;
        std::shared_ptr<T> value;
        AnyImpl(std::shared_ptr<T> value) : value(std::move(value)) {}
        const value_type& get_value() const { return *value; }
    };

} // namespace anyimpl

using anyimpl::AnyImpl;

/// Tags used for BaseAny constructor dispatching.
struct Tags {
    using Default = struct{};
    using Reference = struct{};
    using Derived = struct{};
    using Unrelated = struct{};
    using Pointer = struct{};
};

namespace get_tag {

    template <typename T, typename Config>
    struct GetTag {
        using type = Tags::Default;
    };

    /// Selects the appropriate tag for constructor dispatching.
    template <typename T, typename Config>
    using GetTag_t = typename GetTag<std::decay_t<T>,Config>::type;

    template <typename T, typename Config>
    struct GetTag<std::reference_wrapper<T>, Config> {
        using type = Tags::Reference;
    };

    template <typename... Ts, typename Config>
    struct GetTag<Any<Ts...>, Config> {
        using type = std::conditional_t<std::is_base_of<AnyImplBase<Config>, typename Any<Ts...>::_raspberry_impl_base>::value, Tags::Derived, Tags::Unrelated>;
    };

    template <typename T, typename Config>
    struct GetTag<T*, Config> {
        using type = Tags::Pointer;
    };

    template <typename T, typename Config>
    struct GetTag<std::unique_ptr<T>, Config> {
        using type = Tags::Pointer;
    };

    template <typename T, typename Config>
    struct GetTag<std::shared_ptr<T>, Config> {
        using type = Tags::Pointer;
    };

} // namespace get_tag

using get_tag::GetTag_t;

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

/// The backbone of Any.
template <typename Config>
class BaseAny : public BeamInheritance_t<Any_BeamConf<BaseAny<Config>>, typename Config::Concepts> {
    template <typename>
    friend class BaseAny;

    /// Pointer to implementation.
    std::unique_ptr<AnyImplBase<Config>> _raspberry_impl_ptr;

public:
    using _raspberry_impl_base = AnyImplBase<Config>;

    /// Any must never be empty.
    BaseAny() = delete;

    /// Dispatcher.
    template <typename T>
    BaseAny(T&& t) : BaseAny(std::forward<T>(t), GetTag_t<T,Config>{})
    {}

    /// Default behavior for storing most types.
    template <typename T>
    BaseAny(T&& t, Tags::Default) : _raspberry_impl_ptr(std::make_unique<AnyImpl<std::decay_t<T>,Config>>(std::forward<T>(t)))
    {}

    /// Stores a reference instead of a value.
    /// Use with caution; this Any must not outlive the given reference.
    template <typename T>
    BaseAny(const std::reference_wrapper<T>& t, Tags::Reference) : _raspberry_impl_ptr(std::make_unique<AnyImpl<T&,Config>>(t.get()))
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

    /// Stores a pointer instead of a value.
    /// \note Raw pointers will not be deleted, and cannot be retrieved.
    template <typename T>
    BaseAny(T&& t, Tags::Pointer) : BaseAny(std::forward<T>(t), Tags::Default{})
    {}

    /// Gets a pointer to the implementation.
    const _raspberry_impl_base& _raspberry_get_impl() const {
        return *_raspberry_impl_ptr;
    }
};

namespace concept_filter {

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

} // namespace concept_filter

using concept_filter::ConceptFilter;

/// Erasure container.
/// All-natural replacement for inheritance.
template <typename... Concepts>
class Any : public BaseAny<ConceptFilter<Concepts...>> {
    using Base = BaseAny<ConceptFilter<Concepts...>>;
public:
    using Base::BaseAny;
};

} // namespace _detail

using _detail::Any;

} // namespace Raspberry

#endif // RASPBERRY_HPP
