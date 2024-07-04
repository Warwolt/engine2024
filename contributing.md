# Style guide
(This style guide is just me writing down my intentions so I can remember why
the code looks a certain way when returning to it later).

## Passing by reference
When calling a function, values can be passed either by-value or by-reference.
In order to make it clear when a value is used for an out-parameter, the
following rules apply:

| Pass                 | Type     | Example declaration                   | Example usage                  |
| -------------------- | -------- | ------------------------------------- | ------------------------------ |
| by value             | T        | print_number(int x);                  | print_number(123);             |
| by mutable reference | T*       | initialize_system(System* system);    | initialize_system(&my_system); |
| by const reference   | const T& | print_string(const std::string& str); | print_string("Hello world!");  |

Using `const T&` allows for both passing an existing variable by const
reference, as well as passing a literal value. With `const T*` parameters,
literals cannot be used as arguments as easily.

```C++
void greet_person(const Person& person);

// pass variable
Person alice = Person { .name = "Alice", .age = 23 };
greet_person(alice);

// pass literal
greet_person(Person { .name = "Bob", .age = 34 });
```
