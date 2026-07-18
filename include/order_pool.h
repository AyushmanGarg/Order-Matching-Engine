#pragma once

#include <vector>
#include <cstdint>
#include "order.h"

namespace order_book {

    class OrderPool {
        private:
            struct Node {
                Order order;
                std::uint32_t next;
            }; 

            std::vector<Node> nodes;
            std::uint32_t free_node;
            std::size_t max_capacity;
            std::size_t curr_size;
            std::uint32_t get_idx_from_ptr(const Order* order) const;
        
        public:
            explicit OrderPool(std::size_t max_capacity);
            std::size_t get_capacity() const { return max_capacity; }
            std::size_t get_curr_size() const { return curr_size; }
            Order* allocate();
            void deallocate(Order* order);
            void reset();
    };

}

/*
In C++, the const keyword at the end of a member function, the const on parameters, and the const on the return type serve three completely different purposes.Here is a breakdown of what they mean and why they are necessary.1. The Trailing const (After the Function)std::uint32_t get_idx_from_ptr(const Order* order) constWhat it means: The trailing const signifies that this is a constant member function. It guarantees that the function will not modify any member variables of the class.Why we need it:It allows the function to be called on const instances of your class. If this keyword were missing, the compiler would throw an error if you tried to call get_idx_from_ptr() on a read-only or constant OrderManager object.It tells the compiler that the hidden this pointer passed to this function should be treated as a pointer to a const object (e.g., const ClassName* const this).2. The Parameter constget_idx_from_ptr(const Order* order)What it means: The const on the parameter means the function promises it will not modify the Order object pointed to by order.Why we need it (and why it is independent of the trailing const):You asked if the trailing const converts the parameter to const. No, it does not. The trailing const only applies to the object that owns the function (the this pointer). It has absolutely no effect on the function's arguments.You must explicitly put const on the order parameter so the compiler knows you are passing a read-only pointer. This allows the function to accept both regular Order pointers and const Order pointers safely. It acts as a strict contract that protects the Order data from accidental changes inside this function.Summary of const in your function signatureconst Order* — Protects the Order object being passed as an argument from being modified.Trailing const — Protects the class calling the function (its own member variables) from being modified.std::uint32_t — The return type, ensuring an unsigned 32-bit integer is returned.
*/

/*
why explicit is used

Why explicit Here
explicit prevents the compiler from silently converting a number into an OrderPool object.

Without explicit — Dangerous Implicit Conversion
OrderPool(std::size_t capacity);   // no explicit

void process(OrderPool& pool);

process(1000);   // COMPILES! Compiler silently creates: OrderPool(1000)
                 // then passes it to process()
                 // You just allocated 1M × 128 bytes = 128 MB by accident
With explicit — Compiler Catches the Mistake
explicit OrderPool(std::size_t capacity);

process(1000);              // ❌ ERROR: no implicit conversion allowed
process(OrderPool(1000));   // ✅ OK: you clearly intended to create a pool
The rule: any constructor that takes one argument (or can be called with one argument) should be explicit unless you want implicit conversion (which is almost never).

Is explicit Only for Constructors?
No — since C++11 it can also be used on conversion operators:
class OrderPool {
    // conversion operator — lets you write: bool b = pool;
    operator bool() const { return size > 0; }

    // explicit version — prevents: bool b = pool;  but allows: if (pool) {...}
    explicit operator bool() const { return size > 0; }
};
You'd think so — but C++ doesn't give a type error. That's exactly the problem.

C++ Has "Implicit Conversion via Constructor"
In C++, a constructor that takes one argument doubles as a conversion function — the compiler uses it automatically to convert types:
OrderPool(std::size_t capacity);   // without explicit

// The compiler sees this constructor and learns:
// "I can convert any size_t → OrderPool by calling OrderPool(value)"
void process(OrderPool& pool);
 void process(OrderPool& pool);

process(1000);
process(1000);

1. process() expects OrderPool
2. You gave it 1000 (int)
3. Can I convert int → OrderPool?
4. int → size_t? Yes (implicit numeric conversion) ✅
5. size_t → OrderPool? Yes! There's a constructor OrderPool(size_t) ✅
6. So I'll create a temporary: process(OrderPool(1000))
7. Compiles fine. No error.

This is a deliberate C++ language feature, not a bug. The language designers wanted things like std::string s = "hello" to work (implicit conversion from const char* to std::string via its constructor).

Proof — This Actually Compiles
class Foo {
public:
    Foo(int x) {}   // single-argument constructor
};

void bar(Foo f) {}

int main() {
    bar(42);   // ✅ COMPILES — compiler calls Foo(42) silently
}
    class Foo {
public:
    explicit Foo(int x) {}
};

void bar(Foo f) {}

int main() {
    bar(42);       // ❌ ERROR: no matching function
    bar(Foo(42));  // ✅ OK: explicit construction
}
    Why This Rule Exists
C++ was designed to let user-defined types behave like built-in types. Just as double d = 5; implicitly converts int to double, the language allows MyClass obj = 5; if you provide a matching constructor.

This was useful for things like:
std::string name = "Alice";         // const char* → string (implicit, convenient)
std::complex<double> c = 3.14;      // double → complex (implicit, makes sense)

But it's dangerous for types where construction has side effects:

OrderPool pool = 1000000;   // Oops — just allocated 128 MB of memory
std::vector<int> v = 10;    // Creates a vector of size 10 (not a vector containing 10!)

No, a constructor in C++ does not always have to be public. You can declare constructors as private or protected.When a constructor is not public, you cannot initialize an object using standard instantiation (like MyClass obj;). Instead, the object is initialized through specific architectural patterns.How Objects Are Initialized with Non-Public Constructors1. Via Static Factory Methods (Private Constructor)The class provides a public static member function. This function has access to the private constructor, creates the object, and returns it. This is common in the Singleton Pattern or Factory Pattern.cppclass Admin {
private:
    Admin() {} // Private constructor

public:
    static Admin createInstance() {
        return Admin(); // Allowed: static method can access private constructor
    }
};

int main() {
    // Admin emp; // Error: Constructor is private
    Admin emp = Admin::createInstance(); // Correct
}
Use code with caution.2. Via Friend Classes or Functions (Private Constructor)You can designate another class or function as a friend. This grants them permission to access private members, including the private constructor.cppclass Database {
private:
    Database() {} // Private constructor
    friend class DatabaseManager; // Grants access
};

class DatabaseManager {
public:
    Database createConnection() {
        return Database(); // Allowed because DatabaseManager is a friend
    }
};
Use code with caution.3. Via Derived Classes (Protected Constructor)A protected constructor cannot be called by the outside world, but it can be called by derived (child) classes. Object initialization happens when you instantiate the child class.cppclass Parent {
protected:
    Parent() {} // Protected constructor
};

class Child : public Parent {
public:
    Child() : Parent() {} // Allowed: Derived class calls parent constructor
};

int main() {
    // Parent p; // Error: Protected constructor
    Child c; // Correct: Initializes Parent components through Child
}
Use code with caution.Why Use Non-Public Constructors?Control Instance Count: Restricts creation to exactly one instance (Singleton).Enforce Validation: Forces users to use factory methods that check inputs before creating an object.Abstract Base Classes: Prevents instantiation of a generic parent class that should only exist through its children.To help tailor this to your code, let me know:What design pattern or goal are you trying to implement?Do you want to restrict instantiation to one instance or specific classes?I can provide a complete code template based on your specific use case.
*/