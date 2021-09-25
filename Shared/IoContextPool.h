#pragma once

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>

/// A pool of io_context objects.
class IoContextPool
    : private boost::noncopyable {
public:
    /// Construct the io_context pool.
    explicit IoContextPool(std::size_t pool_size);

    /// Run all io_context objects in the pool.
    void run();

    /// Stop all io_context objects in the pool.
    void stop();

    /// Get an io_context to use.
    boost::asio::io_context& get_io_context();

private:
    typedef boost::shared_ptr<boost::asio::io_context> io_context_ptr;
    typedef boost::asio::executor_work_guard<
        boost::asio::io_context::executor_type>
        io_context_work;

    /// The pool of io_contexts.
    std::vector<io_context_ptr> io_contexts_;

    /// The work that keeps the io_contexts running.
    std::list<io_context_work> work_;

    /// The next io_context to use for a connection.
    std::size_t next_io_context_;
};
