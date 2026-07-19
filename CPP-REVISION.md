# C++ Revision Notes

A living document of C++ concepts, errors, and explanations encountered during this project.

---

## 1. Returning by Value vs. by Reference from a Getter

### The Problem

```cpp
std::string node::get_name()
{
    return this->stEntry.name;  // returns a COPY
}

// At the call site:
std::string node_name = n.get_name();  // another COPY
```

`stEntry.name` already exists in memory. Returning by value means:
- `get_name()` creates a new `std::string` (copy #1) to return
- The caller stores it in `node_name` (copy #2, though often elided by NRVO)

Each copy allocates heap memory to duplicate the string's character data.

### The Fix — Return by Reference

```cpp
std::string& node::get_name()
{
    return this->stEntry.name;  // returns an ALIAS (memory address)
}

// At the call site:
std::string &node_name = n.get_name();  // zero copies
```

A reference is just a pointer-sized address. No new `std::string` object is created, no heap allocation, no character data duplicated. `node_name` is simply another name for the exact same memory as `stEntry.name`.

- **Reads** through `node_name` read the original.
- **Writes** through `node_name` mutate the original directly.

### When to Use Each

| Scenario | Return type |
|---|---|
| Caller should not modify original | `const std::string&` |
| Caller may read or modify original | `std::string&` |
| Caller needs an independent copy | `std::string` (by value) |

---

## 2. Abstract Classes and the "Cannot Be Instantiated" Error

### Error

```
parameter of abstract class type "node" is not allowed: C/C++(603)
function "node::accept" is a pure virtual function
```

### What Makes a Class Abstract

A class becomes abstract the moment it declares **at least one pure virtual function**:

```cpp
class node {
    virtual void accept(visitor &v) = 0;  // pure virtual — makes node abstract
};
```

The `= 0` signals "subclasses MUST provide this implementation." The base class is intentionally incomplete. C++ forbids creating an instance of it because it would be missing that method body.

### Why Passing by Value Triggers It

```cpp
bool remove_node(node n)  // ❌ ERROR
```

Passing by value means C++ must **copy-construct** a `node` object on the stack for the call. That requires instantiating `node` directly — which is illegal for an abstract class.

There is also a second hidden problem: **object slicing**. Even if `node` weren't abstract, copying a `arithexprNode` into a `node` parameter would strip away all the subclass-specific data and virtual function overrides, leaving a broken truncated object.

### The Fix — Pass by Reference

```cpp
bool remove_node(node &n)  // ✅ CORRECT
```

A reference is just an alias (a pointer under the hood). It does **not** create a new object — it points to the existing concrete subclass instance that was already constructed elsewhere. No instantiation of `node` occurs, so the abstract class restriction is never triggered.

### Polymorphism is Preserved

Through a base-class reference, **virtual dispatch still works correctly**:

```cpp
bool remove_node(node &n) {
    // If n is actually an arithexprNode, virtual calls dispatch to arithexprNode's overrides
    std::string &node_name = n.get_name();
}
```

Every concrete node type (`arithexprNode`, `assignNode`, `implNode`, etc.) can be passed in — they all inherit from `node`.

### Summary Table

| Parameter type | Creates object? | Slicing risk? | Polymorphism? | Works with abstract? |
|---|---|---|---|---|
| `node n` | ✅ yes (copy) | ✅ yes | ❌ no | ❌ no |
| `node &n` | ❌ no | ❌ no | ✅ yes | ✅ yes |
| `node *n` | ❌ no | ❌ no | ✅ yes | ✅ yes |

---

*Update this document whenever a theoretical C++ concept or error is explained during the session.*
