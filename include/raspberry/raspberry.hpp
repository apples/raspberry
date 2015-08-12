#ifndef RASPBERRY_HPP
#define RASPBERRY_HPP

#include <memory>
#include <utility>

#define DECL_ERASURE_MEMBER_CONCEPT(ConceptName, FuncName) \
template <typename Func> \
struct ConceptName; \
template <typename R, typename... Args> \
struct ConceptName<R(Args...)> { \
    template <typename Base> \
    struct Virtual : Base { \
        virtual R FuncName(Args...) = 0; \
    }; \
    template <typename Impl, typename Base> \
    struct VirtualImpl : Base { \
        virtual R FuncName(Args... args) override final { \
            return static_cast<Impl&>(*this).get_value().FuncName(args...); \
        } \
    }; \
    template <typename Impl> \
    struct NonVirtual { \
        template <typename... A> \
        R FuncName(A&&... args) { \
            return static_cast<Impl&>(*this).get_ptr()->FuncName(std::forward<A>(args)...); \
        } \
    }; \
}

namespace Raspberry {

template <typename, typename...>
struct BeamInheritance;

template <typename Conf, typename... Types>
using BeamInheritance_t = typename BeamInheritance<Conf, Types...>::type;

template <typename Conf>
struct BeamInheritance<Conf> {
    using type = typename Conf::Base;
};

template <typename Conf, typename HeadConcept, typename... TailConcepts>
struct BeamInheritance<Conf, HeadConcept, TailConcepts...> {
    using type = typename Conf::template Link<HeadConcept, BeamInheritance_t<Conf, TailConcepts...>>;
};

template <typename... Concepts>
class Any final : public Concepts::template NonVirtual<Any<Concepts...>>... {

    struct AnyImplBase_BeamConf {
        using Base = struct {};
        template <typename Concept, typename Next>
        using Link = typename Concept::template Virtual<Next>;
    };
    
    struct AnyImplBase : BeamInheritance_t<AnyImplBase_BeamConf, Concepts...> {
        virtual ~AnyImplBase() = default;
    };
    
    template <typename AnyImpl>
    struct AnyImpl_BeamConf {
        using Base = AnyImplBase;
        template <typename Concept, typename Next>
        using Link = typename Concept::template VirtualImpl<AnyImpl, Next>;
    };
	
    template <typename T, bool B = std::is_empty<T>::value>
    struct AnyImpl;
	
    template <typename T>
    struct AnyImpl<T,false> final : BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T>>, Concepts...> {
		T value;
        AnyImpl(const T& value) : value(value) {}
        AnyImpl(T&& value) : value(std::move(value)) {}
		T& get_value() { return value; }
    };
    
    template <typename T>
    struct AnyImpl<T,true> final : T, BeamInheritance_t<AnyImpl_BeamConf<AnyImpl<T>>, Concepts...> {
        AnyImpl(const T& value) : T(value) {}
        AnyImpl(T&& value) : T(std::move(value)) {}
		T& get_value() { return *this; }
    };
    
    std::unique_ptr<AnyImplBase> impl_ptr;
    
public:
    
    Any() = default;
    
    template <typename T>
    Any(T&& t) : impl_ptr(std::make_unique<AnyImpl<T>>(std::forward<T>(t)))
    {}
    
    const std::unique_ptr<AnyImplBase>& get_ptr() const {
        return impl_ptr;
    }
    
	template <typename T>
	using nope = AnyImpl<T>;
};

} // namespace Raspberry

#endif RASPBERRY_HPP
