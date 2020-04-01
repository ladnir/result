#include "Result.h"


namespace { // anonymous namespace

    struct ncoroErrCategory : std::error_category
    {
        const char* name() const noexcept override
        {
            return "ncoro";
        }

        std::string message(int ev) const override
        {
            switch (static_cast<ncoro::Errc>(ev))
            {
            case ncoro::Errc::success:
                return "Success";
            case ncoro::Errc::uncaught_exception:
                return "an uncaught exception that was converted to std::error_code by Result<T,...>";
            default:
                return "(unrecognized error)";
            }
        }
    };

    const ncoroErrCategory theNcoroCategory{};
} // anonymous namespace

namespace ncoro
{
    error_code ncoro::make_error_code(Errc e)
    {
        return { static_cast<int>(e), theNcoroCategory };
    }
}

// An std::error_code based expected/result type which catches
// all exceptions via coroutines.
template<typename T>
using result = ncoro::result<T, std::error_code, ncoro::NothrowExceptionHandler<T, std::error_code>>;

// An std::exception_ptr based expected/result type which catches
// all exceptions via coroutines. Execeptions are stored internally
// and can be rethrown later.
template<typename T>
using result_e = ncoro::result<T, std::exception_ptr, ncoro::NothrowExceptionHandler<T,std::exception_ptr>>;


int g_make_empty = 10;
const auto generic_error = std::error_code{ 1, std::system_category() };

// Classic style
result<int> poll_value_old() {
    if (g_make_empty-- == 0) 
        return generic_error;
    return 1;
}

result<long> sum_values_old(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        auto res = poll_value_old();
        if (res) 
            return res.error();
        sum += *res;
    }
    return sum;
}

// Coro style
result<int> poll_value_co() {
    if (g_make_empty-- == 0)
    {
        co_return generic_error;
    }
    co_return 1;
}

result<long> sum_values_co(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        sum += co_await poll_value_co();
    }
    co_return sum;
}

// exception based coro
result<int> poll_value_ex() {
    if (g_make_empty-- == 0)
    {
        // effectively co_return generic_error
        throw generic_error;

        // effectively co_return ncoro::Errc::unchaught_exception;
        throw std::exception();
    }
    co_return 1;
}

result<long> sum_values_ex(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) 
    {
        // effectively co_await poll_value_ex(); 
        sum += *poll_value_ex();
    }
    co_return sum;
}

// lossless exception based coro
result_e<int> poll_value_ex2() {
    if (g_make_empty-- == 0)
    {
        throw std::runtime_error("test rt");
    }
    co_return 1;
}

result_e<long> sum_values_ex2(int nb_sum) {

    int sum = 0;
    while (nb_sum-- > 0) {
        sum += poll_value_ex().value();
    }
    co_return sum;
}

template<typename R>
void print_opt(const R& r)
{
    std::cout << "res = " << r << '\n';
}

int resultMain() {

    g_make_empty = 10;
    print_opt(sum_values_old(5));
    print_opt(sum_values_old(15));
    g_make_empty = 10;
    print_opt(sum_values_co(5));
    print_opt(sum_values_co(15));
    g_make_empty = 10;
    print_opt(sum_values_ex(5));
    print_opt(sum_values_ex(15));
    g_make_empty = 10;
    print_opt(sum_values_ex2(5));
    print_opt(sum_values_ex2(15));
    return 0;
}

