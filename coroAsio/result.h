#pragma once

#include <experimental/coroutine>
#include <variant>
#include <optional>
#include <system_error>

namespace ncoro
{

    namespace stde = std::experimental;

    namespace details
    {
        template<typename T, typename E, typename ExceptionHandler>
        class result
        {
        public:
            using value_type = std::remove_cvref_t<T>;
            using error_type = std::remove_cvref_t<E>;
            using exception_handler = std::remove_cvref_t<ExceptionHandler>;

            result(const value_type& t);
            result(value_type&& t);
            result(const error_type& e);
            result(error_type&& e);

            result(const result& r);
            result(result&& r);

            result& operator=(const value_type& v);
            result& operator=(value_type&& v);

            result& operator=(const error_type& v);
            result& operator=(error_type&& v);

            result& operator=(result&& r);
            result& operator=(result const& r);

            bool has_value() const;
            bool has_error() const;
            operator bool() const;

            value_type& operator->();
            const value_type& operator->() const;
            value_type& operator*();
            const value_type& operator*() const;

            value_type& value();
            const value_type& value() const;

            value_type& value_or(value_type& alt);
            const value_type& value_or(const value_type& alt) const;

            error_type& error();
            const error_type& error() const;

            bool operator==(result const& y) const;
            bool operator!=(result const& y) const;
            bool operator==(const value_type&) const;
            bool operator!=(const value_type&) const;

            friend bool operator==(const value_type&, const result&);
            friend bool operator!=(const value_type&, const result&);

            bool operator==(const error_type&) const;
            bool operator!=(const error_type&) const;
            friend bool operator==(const error_type&, const result&);
            friend bool operator!=(const error_type&, const result&);


            struct promise_type;
            template <class Promise>
            struct result_awaiter {

                using value_type = typename Promise::result_type::value_type;
                using exception_handler = typename  Promise::result_type::exception_handler;

                result<value_type, error_type, exception_handler>&& res;

                result_awaiter(result<value_type, error_type, exception_handler>&& r)
                    : res(std::move(r))
                {}

                value_type&& await_resume() noexcept {
                    return std::move(res.value());
                }
                bool await_ready() noexcept {
                    return res.has_value();
                }

                void await_suspend(std::experimental::coroutine_handle<promise_type> const& handle) noexcept {
                    handle.promise().var() = res.error();
                }
            };

            struct promise_type
            {
                using result_type = result;

                stde::suspend_never initial_suspend() { return {}; }
                stde::suspend_never final_suspend() noexcept { return {}; }
                result get_return_object() {
                    return stde::coroutine_handle<promise_type>::from_promise(*this);
                }

                void return_value(value_type&& v) noexcept {
                    var() = std::move(v);
                }
                void return_value(value_type const& v) {
                    var() = v;
                }
                void return_value(error_type&& e) noexcept {
                    var() = std::move(e);
                }
                void return_value(error_type const& e) noexcept {
                    var() = e;
                }

                void unhandled_exception() {
                    var() = ExceptionHandler{}.unhandled_exception();
                }

                template <class TT, class H>
                auto await_transform(result<TT, error_type, H>&& res) noexcept {
                    return result_awaiter<result<TT, error_type, H>::promise_type>(std::move(res));
                }
                std::variant<value_type, error_type>& var()
                {
                    return mRes->mVar;
                }

                result_type* mRes;
            };

        private:
            using coro_handle = std::experimental::coroutine_handle<promise_type>;
            std::variant<value_type, error_type> mVar;
            std::variant<value_type, error_type>& var() {
                return mVar;
            };
            const std::variant<value_type, error_type>& var() const {
                return mVar;
            };

            result(coro_handle handle)
            {
                handle.promise().mRes = this;
            }
        };

        template<typename T, typename E, typename H>
        result<T, E, H>::result(const value_type& t)
            :mVar(t)
        {}
        template<typename T, typename E, typename H>
        result<T, E, H>::result(value_type&& t)
            : mVar(std::forward<value_type>(t))
        {}
        template<typename T, typename E, typename H>
        result<T, E, H>::result(const error_type& e)
            : mVar(e)
        {}
        template<typename T, typename E, typename H>
        result<T, E, H>::result(error_type&& e)
            : mVar(std::forward<error_type>(e))
        {}
        template<typename T, typename E, typename H>
        result<T, E, H>::result(result const& r)
            : mVar(r.mVar)
        {}
        template<typename T, typename E, typename H>
        result<T, E, H>::result(result&& r)
            : mVar(std::move(r.mVar))
        {}

        template<typename T, typename E, typename H>
        result<T, E, H>& result<T, E, H>::operator=(const value_type& v) { var() = v; return *this; }
        template<typename T, typename E, typename H>
        result<T, E, H>& result<T, E, H>::operator=(value_type&& v) { var() = std::move(v); return *this; }
        template<typename T, typename E, typename H>
        result<T, E, H>& result<T, E, H>::operator=(const error_type& v) { var() = v; return *this; }
        template<typename T, typename E, typename H>
        result<T, E, H>& result<T, E, H>::operator=(error_type&& v) { var() = std::move(v); return *this; }
        template<typename T, typename E, typename H>
        result<T, E, H>& result<T, E, H>::operator=(result&& r) { var() = std::move(r.var()); return *this; }
        template<typename T, typename E, typename H>
        result<T, E, H>& result<T, E, H>::operator=(result const& r) { var() = r.var(); return *this; }

        template<typename T, typename E, typename H>
        bool result<T, E, H>::has_value() const { return std::holds_alternative<value_type>(var()); }
        template<typename T, typename E, typename H>
        bool result<T, E, H>::has_error() const { return !has_value(); }
        template<typename T, typename E, typename H>
        result<T, E, H>::operator bool() const { return has_value(); }

        template<typename T, typename E, typename H>
        typename result<T, E, H>::value_type& result<T, E, H>::operator->() { return value(); }
        template<typename T, typename E, typename H>
        const typename result<T, E, H>::value_type& result<T, E, H>::operator->() const { return value(); }
        template<typename T, typename E, typename H>
        typename  result<T, E, H>::value_type& result<T, E, H>::operator*() { return value(); }
        template<typename T, typename E, typename H>
        const typename result<T, E, H>::value_type& result<T, E, H>::operator*() const { return value(); }

        template<typename T, typename E, typename H>
        typename result<T, E, H>::value_type& result<T, E, H>::value() {
            if (has_error())
                exception_handler{}.throwErrorType(error());
            return std::get<value_type>(var());
        }
        template<typename T, typename E, typename H>
        const typename result<T, E, H>::value_type& result<T, E, H>::value() const {
            if (has_error())
                exception_handler{}.throwErrorType(error());
            return std::get<value_type>(var());
        }

        template<typename T, typename E, typename H>
        typename result<T, E, H>::value_type& result<T, E, H>::value_or(value_type& alt) {
            if (has_error())
                return alt;
            return std::get<value_type>(var());
        }
        template<typename T, typename E, typename H>
        const typename result<T, E, H>::value_type& result<T, E, H>::value_or(const value_type& alt) const {
            if (has_error())
                return alt;
            return std::get<value_type>(var());
        }

        class bad_result_access
            : public std::exception
        {
        public:
            bad_result_access(char const* const msg)
                : std::exception(msg)
            { }
        };

        template<typename T, typename E, typename H>
        typename result<T, E, H>::error_type& result<T, E, H>::error() {
            if (has_value())
                throw bad_result_access("error() was called on a Result<T,E> which stores an value_type");

            return std::get<error_type>(var());
        }
        template<typename T, typename E, typename H>
        const typename result<T, E, H>::error_type& result<T, E, H>::error() const {
            if (has_value())
                throw bad_result_access("error() was called on a Result<T,E> which stores an value_type");

            return std::get<error_type>(var());
        }

        template<typename T, typename E, typename H>
        bool result<T, E, H>::operator==(result const& y) const {
            return var() == y.var();
        }
        template<typename T, typename E, typename H>
        bool result<T, E, H>::operator!=(result const& y) const {
            return var() != y.var();
        }
        template<typename T, typename E, typename H>
        bool result<T, E, H>::operator==(const value_type& v) const {
            return var() == v;
        }
        template<typename T, typename E, typename H>
        bool result<T, E, H>::operator!=(const value_type& v) const {
            return var() != v;
        }
        template<typename T, typename E, typename H>
        bool operator==(const T& v, const result<T, E, H>& r) {
            return r.var() == v;
        }
        template<typename T, typename E, typename H>
        bool operator!=(const T& v, const result<T, E, H>& r) {
            return r.var() != v;
        }
        template<typename T, typename E, typename H>
        bool result<T, E, H>::operator==(const error_type& v) const {
            return var() == v;
        }
        template<typename T, typename E, typename H>
        bool result<T, E, H>::operator!=(const error_type& v) const {
            return var() != v;
        }
        template<typename T, typename E, typename H>
        bool operator==(const E& v, const result<T, E, H>& r) {
            return r.var() == v;
        }
        template<typename T, typename E, typename H>
        bool operator!=(const E& v, const result<T, E, H>& r) {
            return r.var() != v;
        }

        //template<typename T, typename E, typename EX>
        //std::ostream& operator<<(std::ostream& out, const result<T, E, EX>& r) {
        //    if (r.has_value())
        //        out << "{" << r.value() << "}";
        //    else
        //        out << "<" << r.error().message() << ">" << std::endl;
        //    return out;
        //}
    }

    enum class Errc
    {
        success = 0,
        uncaught_exception
    };
    using error_code = std::error_code;
    error_code make_error_code(Errc e);
}

namespace std {
    template <>
    struct is_error_code_enum<ncoro::Errc> : true_type {};
}

namespace ncoro {
    template<typename T, typename E>
    struct RethrowExceptionHandler
    {
        std::variant<T, E> unhandled_exception() {
            throw;
        }

        void throwErrorType(E& e) {
            throw e;
        }
    };

    template<typename T, typename E>
    class NothrowExceptionHandler;

    template<typename T>
    class NothrowExceptionHandler<T, std::error_code>
    {
    public:
        std::variant<T, std::error_code> unhandled_exception() {
            try {
                throw;
            }
            catch (std::error_code e) {
                return e;
            }
            catch (...) {
                return Errc::uncaught_exception;
            }
            std::terminate();
        }

        void throwErrorType(std::error_code e) {
            throw e;
        }
    };

    template<typename T>
    class NothrowExceptionHandler<T, std::exception_ptr>
    {
    public:
        std::variant<T, std::exception_ptr> unhandled_exception() {
            return std::current_exception();
        }

        void throwErrorType(std::exception_ptr e) {
            std::rethrow_exception(e);
        }
    };


    template<typename T, typename E>
    class NothrowExceptionHandler
    {
    public:
        std::variant<T, E> unhandled_exception() {
            try {
                throw;
            }
            catch (E & e) {
                return e;
            }
            catch (...) {
                return E{};
            }
            std::terminate();
        }

        void throwErrorType(E const& e) {
            throw e;
        }
    };

    template<typename T, typename Error = std::error_code, typename ExceptionHandler = NothrowExceptionHandler<T, Error>>
    using result = details::result<T, Error, ExceptionHandler>;
}

int resultMain();