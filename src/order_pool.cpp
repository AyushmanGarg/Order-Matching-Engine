#include "order_pool.h"
#include <limits>

namespace order_book {
    // the list prevents double initialisation of members of class and thus is faster 
    /*
    Phase 1 — Member initialization (BEFORE body):
    ┌─────────────────────────────────────────────────────┐
    │ If you wrote : member(value)  → construct with value │
    │ If you didn't                 → default construct    │
    └─────────────────────────────────────────────────────┘

    Phase 2 — Constructor body (AFTER all members exist):
    {
        // all members are already valid objects here
        // you can only REASSIGN them, not construct them
    } 
    */

    /*
    Why constexpr is Used
    Using constexpr provides distinct advantages over traditional keywords like const or macros (#define).1. Compilation-Time EvaluationThe compiler calculates the maximum value of the integer while building your application. Zero processing time is wasted on this calculation when your program actually runs.2. Performance OptimizationBecause the compiler knows the exact value at compile time, it can inject the raw number directly into the machine code wherever INVALID_INDEX is used. This eliminates the need to fetch the value from computer memory during runtime, speeding up execution.3. Use in Compile-Time ContextsC++ requires certain values to be known before the program runs. Because this variable is marked constexpr, you can safely use INVALID_INDEX in places where const variables are often rejected:Specifying the size of a static array: int my_array[INVALID_INDEX]; (conceptually)As a template argument: std::array<int, INVALID_INDEX>Inside switch case labelsWithin compile-time conditional checks (if constexpr)4. Type Safety and DebuggingUnlike an old-school macro like #define INVALID_INDEX 0xFFFFFFFF, constexpr enforces strict type checking. The compiler ensures the value perfectly matches a std::uint32_t. Furthermore, because it is an actual variable, the name INVALID_INDEX remains visible inside your debugger, making troubleshooting much easier.
    */

    static constexpr std::uint32_t INVALID_INDEX = std::numeric_limits<std::uint32_t>::max();

    OrderPool::OrderPool(std::size_t max_capacity): nodes(max_capacity), max_capacity(max_capacity), curr_size(0), size(0) {
        for(std::size_t i = 0 ; i<max_capacity ; i++) {
            nodes[i].next = (i+1 < max_capacity) ? (i+1) :  INVALID_INDEX;
        }
    }

    void OrderPool::reset() {
        free_node = 0;
        curr_size = 0;
        for(std::size_t i = 0 ; i<max_capacity ; i++) {
            nodes[i].next = (i+1 < max_capacity) ? (i+1) :  INVALID_INDEX;
        }
    }

    Order* OrderPool::allocate() {
        if(free_node == INVALID_INDEX)
            return nullptr;
        
        std::uint32_t idx = free_node;
        Node& node = nodes[idx];
        free_head = node.next;
        node.next = INVALID_INDEX;
        size++;

        return &node.order;
    }

    void OrderPool::deallocate(Order* order) {
        if(!order) return;
        std::uint32_t idx = get_idx_from_ptr(order);
        node[idx].next = free_node;
        free_node = idx;
        size--;
    }

    std::uint32_t get_idx_from_ptr(const Order* order) {
        const char* base_bit_address = reinterpret_cast<const char*>(nodes.data());
        const char* curr_ptr_bit_address = reinterpret_cast<const char*>(order);

        std::ptrdiff_t bytes_from_start = curr_ptr_bit_address - base_bit_address; 
        std::ptrdiff_t node_size = static_cast<std::ptrdiff_t>(sizeof(Node));
        std::uint32_t idx = static_cast<std::uint32_t>(bytes_from_start/node_size);

        return idx;
    }

}

/*
Member Initializer List vs Assignment Inside Body
What That Syntax Is
OrderPool::OrderPool(std::size_t capacity)
    : nodes(capacity),      // member initializer list
      free_head(0),         // ← these run BEFORE the { body }
      capacity(capacity),
      size(0)
{
    // constructor body — runs AFTER all members are already initialized
}
The : member(value), member(value) after the constructor signature is called the member initializer list. It directly constructs each member with the given value.

The Alternative (Assignment Inside Body)

OrderPool::OrderPool(std::size_t capacity) {
    // by the time we get here, all members are ALREADY constructed
    // (with default constructors), then we OVERWRITE them:
    this->nodes = std::vector<Node>(capacity);  // construct default → then assign new one
    this->free_head = 0;
    this->capacity = capacity;
    this->size = 0;
}

he Difference
Initializer List : member(value)	Assignment Inside Body member = value
When it runs	During construction	After default construction
What happens	Directly constructed with value	Default-constructed first, then overwritten
Steps for nodes	1 step: vector(capacity)	2 steps: vector() then = vector(capacity)
For simple types (int, uint32_t)	Same performance	Same performance
For complex types (vector, string)	Faster — avoids double work	Slower — default-construct then reassign
For const or & members	Required — only way	❌ Impossible — can't reassign const/reference
Concrete Example — Why It Matters for nodes

     // Initializer list (what the code does):
: nodes(capacity)
// 1. vector is directly constructed with size=capacity
// ONE allocation, done.

// Assignment in body (the worse alternative):
{
    nodes = std::vector<Node>(capacity);
}
// 1. vector default-constructed (size=0, no allocation)
// 2. NEW vector constructed with size=capacity (allocates memory)
// 3. Old empty vector destroyed
// 4. New vector moved into member
// TWO operations instead of one.

or free_head = 0 vs : free_head(0) — there's zero performance difference because uint32_t is a primitive. The compiler generates identical code. But the initializer list is preferred for consistency and because:

Cases Where You MUST Use Initializer List

class Example {
    const int max_size;         // const — can't reassign
    int& reference;             // reference — can't rebind
    OrderPool pool;             // no default constructor (explicit only)

    Example(int n, int& r)
        : max_size(n),          // MUST — const can't be assigned later
          reference(r),         // MUST — reference can't be rebound
          pool(1000)            // MUST — OrderPool has no default constructor
    {}
};
If you tried max_size = n; inside the body → ❌ compile error. const can only be set at construction time.

capacity(capacity) — Why Same Name Works
: capacity(capacity)
//    ↑         ↑
//  member    parameter
This says: "initialize the member this->capacity with the constructor parameter capacity." C++ resolves the ambiguity: inside the initializer list, the name outside parentheses is always the member, and inside parentheses it looks up the parameter first. So it works correctly despite the identical names.

Why a char* Pointer is UsedIn C++, a memory address will always be 8 bytes on a 64-bit system (or 4 bytes on a 32-bit system), regardless of what it points to. A pointer's data type tells the compiler how much memory to read/write at that address and how to step through memory during pointer arithmetic.Casting to const char* is standard practice to treat the memory as an array of raw bytes rather than a structured object. This is because:The sizeof(char) is exactly \(1\) byte. By pointing to a byte, you can inspect or manipulate data at the lowest possible granularity.It bypasses strict aliasing rules. The C++ standard allows accessing the underlying bytes of any object through a char pointer without causing Undefined Behavior.Pointer Arithmetic on Custom TypesIf you create a custom type Order that is 72 bytes, you can do pointer arithmetic, but it will behave entirely differently than char*:Example: char* Pointer Arithmeticcppchar* ptr = ...;
ptr = ptr + 1; // Moves forward exactly 1 byte
Use code with caution.Example: Order* Pointer ArithmeticcppOrder* orderPtr = ...;
orderPtr = orderPtr + 1; // Moves forward exactly 72 bytes!
Use code with caution.When you perform arithmetic on a typed pointer (like Order*), the compiler automatically multiplies your offset by the size of the type (\(72\)).Summary: Byte Arithmetic vs. Object ArithmeticUse char* arithmetic when you want to advance pointer positions byte by byte (e.g., iterating through raw memory buffers or binary files).Use Order* arithmetic when you want to advance object by object (e.g., iterating to the next Order in an array of Order structs).If you need to adjust an Order* pointer by a specific number of bytes, you must first cast it to a char*.If you are writing custom byte buffers, are you trying to serialize/deserialize the Order struct, or perform binary parsing? I can help you with specific implementation steps or safety checks for those tasks.
*/