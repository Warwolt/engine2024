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
