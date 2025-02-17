#include <print>

import Crafter.Thread;
using namespace Crafter::Thread;

int main() {
    ThreadPool::Start(3);
    ThreadPool::Enqueue({[](){std::println("1");}, [](){std::println("2");}, [](){std::println("3");}, [](){std::println("4");}, [](){std::println("5");},[](){std::println("6");}, [](){std::println("7");}, [](){std::println("8");}, [](){std::println("9");}, [](){std::println("10");}});
    ThreadPool::Stop();
}