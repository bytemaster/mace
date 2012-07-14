#include <mace/rpc/connection_base.hpp>
#include <mace/rpc/detail/connection_base.hpp>
#include <utility>

namespace mace { namespace rpc {
  
  connection_base::connection_base( detail::connection_base* b ):my(b){}
  connection_base::~connection_base() {
    delete my;
  }

  void connection_base::close() { my->close(); }

  const method* connection_base::get_method( const std::string& n )const {
    detail::method_map::const_iterator i = my->methods.find(n);
    if( i != my->methods.end() ) return &i->second;
    return 0; 
  }

  void connection_base::add_method( const std::string& name, const method& m ) {
    my->methods[name] = m;
  }

  std::string connection_base::add_method( const method& m ) {
    std::string nn = create_method_id();
    add_method( nn, m );
    return nn;
  }

  std::string connection_base::create_method_id() {
    return "mace_rpc_cb" + boost::lexical_cast<std::string>(++my->next_method_id);
  }

  cmt::future<datavec> connection_base::raw_call( std::string&& mid, datavec&& param ) {
    mace::cmt::promise<datavec>::ptr prom(new mace::cmt::promise<datavec>() );
    detail::pending_result::ptr pr( new detail::raw_pending_result(prom) );
    raw_call( std::move(mid), std::move(param), pr );
    return prom;
  }

  void connection_base::raw_call( std::string&& meth, datavec&& param, const detail::pending_result::ptr& pr ) {
    if( pr ) {
        my->send( message( std::move(meth), std::move(param), ++my->next_req_id ) );
        my->results[my->next_req_id] = pr;
    } else 
        my->send( message( std::move(meth), std::move(param) ) );
  }

namespace detail {
  void connection_base::break_promises() {
    auto itr = results.begin();
    while( itr != results.end() ) {
      itr->second->handle_error( message::broken_promise, datavec() );
      ++itr;
    }
    results.clear();
  }

  void connection_base::handle( message&& m ) {
    if( m.meth.size() ) {
      auto itr = methods.find( m.meth );
      if( itr != methods.end() ) {
         send( itr->second( m ) );
      } else {
         handle_error( message::unknown_method, std::move(m) );
      }
    } else if( m.id ) {
      auto itr = results.find( *m.id );
      if( itr != results.end() ) {
        if( !m.err ) {
          itr->second->handle_value( std::move( m.data ) );
        } else {
          itr->second->handle_error( m.err, std::move(m.data) );
        }
        results.erase(itr);
      } else {
         handle_error( message::invalid_response, std::move(m) );
      }
    } else {
      handle_error( message::invalid_response, std::move(m) );
    }
  }

  void connection_base::handle_error( message::error_type e, message&& msg ) {
    elog( "%1%", mace::reflect::reflector<message::error_type>::to_string(e) );

    message reply;
    reply.id   = msg.id;
    reply.err  = e; 
    auto s = mace::reflect::reflector<message::error_type>::to_string(e);
    reply.data = std::vector<char>(strlen(s));//(s, s+ strlen(s));

    send( std::move(reply) );
  }

} // namespace detail

} } // namepace mace::rpc
