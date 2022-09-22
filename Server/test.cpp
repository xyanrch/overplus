#include <algorithm>
#include <exception>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <new>
#include <numeric>
#include <string>
#include <type_traits>
#include <unistd.h>
#include <vector>
class bar : public std::enable_shared_from_this<bar> {
public:
    bar()
    {
    }
    std::future<int> run()
    {
        auto self(shared_from_this());
        auto f = std::async(std::launch::async, [this]() {
            sleep(2);

            try {
                auto p = shared_from_this();
                std::cout << "p->count" << p.use_count() << std::endl;
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
            }
            return 0;
        });
        return f;
    }

    void ff()
    {
        auto self = shared_from_this();
        std::cout << "before 111  " << self.use_count() << "\n";
        auto f = [this, self]() {
            std::cout << "f" << self.use_count() << "\n";
        };
        std::cout << "before" << self.use_count() << "\n";

        f();
        // std::cout<<"before"<<self.use_count()<<"\n";
        f();
    }
    ~bar()
    {
        std::cout << "--- bar destroyed ---" << std::endl;
    }
};
class Foo {
public:
    Foo(int v = 0)
        : val(v)
    {
    }
    ~Foo()
    {
        std::cout << "--- Foo destroyed ---" << std::endl;
    }

    void test()
    {
        // bint_ptr(new bar);
        int_ptr.reset(new bar);

        int_ptr->ff();
    }
    std::shared_ptr<bar> int_ptr;
    int val = 0;
};

int main()
{
    try {
        Foo f;
        f.test();
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    return 0;
}
