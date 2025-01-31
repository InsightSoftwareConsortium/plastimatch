// Copyright (C) 2016  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_AsYNC_Hh_
#define DLIB_AsYNC_Hh_ 

// C++11 things don't work in old versions of visual studio 
#if __cplusplus >= 201103L ||  _MSC_VER >= 1900

#include "async_abstract.h"
#include "thread_pool_extension.h"
#include <future>
#include <functional>

namespace dlib
{

// ----------------------------------------------------------------------------------------

    namespace impl
    {
        template <typename T> struct selector {};

        template <typename T, typename U, typename V>
        void call_prom_set_value(
            T& prom,
            U& fun,
            selector<V> 
        )
        {
            prom.set_value(fun());
        }

        template <typename T, typename U>
        void call_prom_set_value(
            T& prom,
            U& fun,
            selector<void>
        )
        {
            fun();
            prom.set_value();
        }
    }

// ----------------------------------------------------------------------------------------

    thread_pool& default_thread_pool();

// ----------------------------------------------------------------------------------------

    template < 
        typename Function, 
        typename ...Args
        >
#if __cplusplus >= 201703L
    std::future<std::invoke_result_t<Function, Args...>> async(
#else
    std::future<typename std::result_of<Function(Args...)>::type> async(
#endif
        thread_pool& tp, 
        Function&& f, 
        Args&&... args 
    )
    {
#if __cplusplus >= 201703L
        auto prom = std::make_shared<std::promise<std::invoke_result_t<Function, Args...>>>();
        std::future<std::invoke_result_t<Function, Args...>> ret = prom->get_future();
#else
        auto prom = std::make_shared<std::promise<typename std::result_of<Function(Args...)>::type>>();
        std::future<typename std::result_of<Function(Args...)>::type> ret = prom->get_future();
#endif
        using bind_t = decltype(std::bind(std::forward<Function>(f), std::forward<Args>(args)...));
        auto fun = std::make_shared<bind_t>(std::bind(std::forward<Function>(f), std::forward<Args>(args)...));
        tp.add_task_by_value([fun, prom]()
        { 
            try
            {
#if __cplusplus >= 201703L
                impl::call_prom_set_value(*prom, *fun, impl::selector<std::invoke_result_t<Function, Args...>>());
#else
                impl::call_prom_set_value(*prom, *fun, impl::selector<typename std::result_of<Function(Args...)>::type>());
#endif
            }
            catch(...)
            {
                prom->set_exception(std::current_exception());
            }
        });
        return std::move(ret);
    }

// ----------------------------------------------------------------------------------------

    template < 
        typename Function, 
        typename ...Args
        >
#if __cplusplus >= 201703L
    std::future<std::invoke_result_t<Function, Args...>> async(
#else
    std::future<typename std::result_of<Function(Args...)>::type> async(
#endif
        Function&& f, 
        Args&&... args 
    )
    {
        return async(default_thread_pool(), std::forward<Function>(f), std::forward<Args>(args)...);
    }

}

// ----------------------------------------------------------------------------------------

#ifdef NO_MAKEFILE
#include "async.cpp"
#endif

#endif
#endif // DLIB_AsYNC_Hh_



