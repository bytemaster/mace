#include <mace/cmt/asio.hpp>
#include <mace/cmt/thread.hpp>

namespace mace { namespace cmt { namespace asio {
    namespace detail {
        using namespace mace::cmt;

        void read_write_handler( const promise<size_t>::ptr& p, const boost::system::error_code& ec, size_t bytes_transferred ) {
            if( !ec ) p->set_value(bytes_transferred);
            else p->set_exception( boost::copy_exception( boost::system::system_error(ec) ) );
        }
        void read_write_handler_ec( promise<size_t>* p, boost::system::error_code* oec, const boost::system::error_code& ec, size_t bytes_transferred ) {
            p->set_value(bytes_transferred);
            *oec = ec;
        }
        void error_handler( const promise<boost::system::error_code>::ptr& p, 
                              const boost::system::error_code& ec ) {
            p->set_value(ec);
        }

        void error_handler_ec( promise<boost::system::error_code>* p, 
                              const boost::system::error_code& ec ) {
            p->set_value(ec);
        }

        template<typename EndpointType, typename IteratorType>
        void resolve_handler( 
                             const typename promise<std::vector<EndpointType> >::ptr& p,
                             const boost::system::error_code& ec, 
                             IteratorType itr) {
            if( !ec ) {
                std::vector<EndpointType> eps;
                while( itr != IteratorType() ) {
                    eps.push_back(*itr);
                    ++itr;
                }
                p->set_value( eps );
            } else {
                p->set_exception( boost::copy_exception( boost::system::system_error(ec) ) );
            }
        }
    }
    boost::asio::io_service& default_io_service() {
        static boost::asio::io_service*      io = new boost::asio::io_service();
        static boost::asio::io_service::work the_work(*io);
        static boost::thread                 io_t([=] { mace::cmt::thread::current().set_name("asio1"); io->run(); });
        static boost::thread                 io_t2([=]{ mace::cmt::thread::current().set_name("asio2"); io->run(); });
        static boost::thread                 io_t3([=]{ mace::cmt::thread::current().set_name("asio3"); io->run(); });
        return *io;
    }

    namespace tcp {
        std::vector<endpoint> resolve( const std::string& hostname, const std::string& port) {
            resolver res( mace::cmt::asio::default_io_service() );
            promise<std::vector<endpoint> >::ptr p( new promise<std::vector<endpoint> >() );
            res.async_resolve( resolver::query(hostname,port), 
                             boost::bind( detail::resolve_handler<endpoint,resolver_iterator>, p, _1, _2 ) );
            return p->wait();;
        }
    }
    namespace udp {
        std::vector<endpoint> resolve( resolver& r, const std::string& hostname, const std::string& port) {
                resolver res( mace::cmt::asio::default_io_service() );
                promise<std::vector<endpoint> >::ptr p( new promise<std::vector<endpoint> >() );
                res.async_resolve( resolver::query(hostname,port), 
                                    boost::bind( detail::resolve_handler<endpoint,resolver_iterator>, p, _1, _2 ) );
                return p->wait();
        }
    }

}}} // namespace mace::cmt::asio
