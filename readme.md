# About

This is a threadpool library for C++ 20

# How to use

```cpp
#include <print>

import Crafter.Thread;
using namespace Crafter::Thread;

int main() {
    ThreadPool::Start(3);
    ThreadPool::Enqueue({[](){std::println("1");}, [](){std::println("2");}, [](){std::println("3");}, [](){std::println("4");}, [](){std::println("5");},[](){std::println("6");}, [](){std::println("7");}, [](){std::println("8");}, [](){std::println("9");}, [](){std::println("10");}});
    ThreadPool::Stop();
}
```

When running this above code the prints will be spread across 2 threads. This can be seen in the ``main.cpp`` sample by running ``crafter-build -c test -r``

