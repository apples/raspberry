Raspberry
===

Raspberry is a lightweight C++ type erasure library.

The particular method of type erasure used by Raspberry is very simple.

Take a look at this typical inheritance example:

```C++
struct Animal {
    virtual void speak() = 0;
    virtual ~Animal() = 0;
};
inline Animal::~Animal() = default;

struct Cat final : Animal {
    virtual void speak() override {
        std::cout << "Meow!" << std::endl;
    }
};

void listen() {
    std::unique_ptr<Animal> animal = std::make_unique<Cat>();
    animal->speak();
}
```

And the equivalent example using Raspberry:

```C++
RASPBERRY_DECL_METHOD(SpeakConcept, speak);

using Animal = raspberry::Any<SpeakConcept<void()>>;

struct Cat {
    void speak() {
        std::cout << "Meow!" << std::endl;
    }
};

void listen() {
    Animal animal = Cat{};
    animal.speak();
}
```

These two examples are of equal efficiency.
They perform the same number of allocations,
use the same amount of memory,
execute in the same amount of time,
and result in the same assembly code (tested with GCC 5.1).

Usage
===

Everything you need is in the single, 160-line header file, `raspberry/raspberry.hpp`.

Declaring Concepts
---

Right now, Raspberry only supports class method concepts.

They are declared like this:

```C++
RASPBERRY_DECL_METHOD(FuncConcept, func);
```

Declaring Erasure Types
---

Raspberry's erasure objects are called `Any`.
Any object can be stored in an `Any<>`.
This is, of course, useless.

We can use previously declared Raspberry concepts to add methods to our `Any`:

```C++
RASPBERRY_DECL_METHOD(FuncConcept, func);

using AnyFunc = raspberry::Any< FuncConcept<void()> >;
```

Notice how we give the concept the type of the method to create.
This allows an `Any` to have several overloads of the same method.

Creating Erasures from Objects
---

Any class type that contains the required methods can be converted into an `Any`.
If a class type is missing any of the required methods, compilation will fail.

```C++
RASPBERRY_DECL_METHOD(FuncConcept, func);

using AnyFunc = raspberry::Any< FuncConcept<void()> >;

struct SomeFunc {
    void func() {
        std::cout << "func() called!" << std::endl;
    }
};

void test(AnyFunc a) {
    a.func();
}

int main() {
    SomeFunc sf;
    test(sf);
}
```

Creating Erasures from References
---

`Any` has special semantics for `std::reference_wrapper` types.
The underlying reference is extracted from the wrapper,
and stored in the erasure.

This allows `Any` to safely store references.

```C++
void test(AnyFunc a) {
    a.func();
}

int main() {
    SomeFunc sf;
    test( std::ref(sf) );
}
```

Multiple Concepts, Const-qualifiers, and Overloading
---

`Any` supports any number of concepts, including zero.
Methods may be `const`, and may be overloaded.

As expected, `const` methods may be called on a non-const object.

```C++
RASPBERRY_DECL_METHOD(GetStringConcept, get_string);
RASPBERRY_DECL_METHOD(SetStringConcept, set_string);

using AnyStringHolder = raspberry::Any<
        GetStringConcept<const std::string&() const>,
        SetStringConcept<void(const std::string&)>,
        SetStringConcept<void(const char*)>
>;
```

Method Return Value Implicit Conversions
---

The return type specified in a concept may be different from the erased method's actual return type,
only if the actual return type can be implicitly converted to the concept's return type.

```C++
RASPBERRY_DECL_METHOD(IdentityConcept, identity);

using AnyIdentity = raspberry::Any< IdentityConcept<int(double)> >;

struct SomeIdentity {
    double identity(double d) { return d; }
};

void test() {
    SomeIdentity s;
    assert( s.identity(7.42) == 7.42 );
    
    AnyIdentity a = s;
    
    assert( a.identity(7.42) == 7 );
}
```

Please be careful with this feature, as it can lead to data loss (e.g. casting a `double` to an `int`).

License
===

MIT. See `LICENSE.txt`.
