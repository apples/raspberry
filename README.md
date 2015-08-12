Raspberry
===

Raspberry is a lightweight C++ type erasure library.

The particular method of type erasure used by Raspberry is very simple.

Take a look at this basic inheritance example:

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

struct Dog final : Animal {
    virtual void speak() override {
        std::cout << "Arf!" << std::endl;
    }
};

void listen() {
    std::unique_ptr<Animal> animal = std::make_unique<Cat>();
    animal->speak();
}
```

And the equivalent type erasure using Raspberry:

```C++
DECL_ERASURE_MEMBER_CONCEPT(SpeakConcept, speak);

using Animal = Raspberry::Any<SpeakConcept<void()>>;

struct Cat {
    void speak() {
        std::cout << "Meow!" << std::endl;
    }
};

struct Dog {
    void speak() {
        std::cout << "Arf!" << std::endl;
    }
};

void listen() {
    Animal animal = Dog{};
    animal.speak();
}
```

Those two examples will produce almost the exact same assembly code (tested with GCC 5.2).

License
===

MIT. See `LICENSE.txt`.
