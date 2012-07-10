#ifndef _MACE_RPC_JSON_CLI_HPP_
#define _MACE_RPC_JSON_CLI_HPP_
#include <sstream>
#include <iomanip>
#include <iostream>
#include <boost/fusion/support/deduce_sequence.hpp>
#include <boost/fusion/sequence/io.hpp>
#include <mace/reflect/reflect.hpp>
#include <mace/rpc/json/io.hpp>
#include <mace/cmt/thread.hpp>
#include <mace/stub/ptr.hpp>
#include <sstream>

namespace mace { namespace rpc { namespace json {

/**
 *  Takes any interface object and provides a command line interface for it.
 */
class cli {
    public:
       template<typename AnyPtr>
       cli( AnyPtr aptr):my_ptr(aptr) { 
          // AnyPtr will create a copy which will go out of scope unless we
          // are smart about keeping a reference to it.
          mace::stub::visit( aptr, visitor<typename AnyPtr::vtable_type>( *this, **boost::any_cast<AnyPtr>(&my_ptr), m_api) ); 
       }
       ~cli() {
         try {
           read_done.wait();
         } catch ( ... ) { }
       }

       const std::vector<std::string>& api()const {
        return m_api;
       }

       boost::function<std::string(const std::string&)>& operator[]( const std::string& name ) 
       { return methods[name]; }

       void start() {  
          read_done = mace::cmt::async( [=](){ this->read_loop(); } );
       }

   private:
       boost::any my_ptr;
       std::vector<std::string> m_api;

       mace::cmt::future<void> read_done;
       void read_loop() {
         mace::cmt::thread* getline_thread = mace::cmt::thread::create();
         while( true ) {
           std::string cmd, line, args;
           line = getline_thread->async( [](){ std::string s; std::getline(std::cin,s); return s; } );
           cmd = line.substr( 0, line.find(' ') );
           args = line.substr( cmd.size(), line.size() );
           try {
             std::cerr << methods[cmd](args) << std::endl;
           } catch ( ... ) {
             std::cerr << boost::current_exception_diagnostic_information() << std::endl;
           }
         }
       }

       template<typename VTableType> 
       struct visitor {
         visitor( cli& c, VTableType& vtbl, std::vector<std::string>& api ):m_cli(c),m_vtbl(vtbl),m_api(api){}

         template<typename MemberPtr, MemberPtr m>
         void operator()( const char* name ) const {
              typedef typename boost::function_types::result_type<MemberPtr>::type member_ref;
              typedef typename boost::remove_reference<member_ref>::type member;
              std::stringstream ss; 
              ss<<std::left<<std::setw(30)<<name<<"     "<<mace::reflect::get_typename<typename member::signature>();
              m_api.push_back( ss.str() );

              m_cli.methods[name] = cli_functor<typename member::fused_params,member_ref>(m_vtbl.*m);
          /*
              typedef typename boost::remove_reference<typename boost::function_types::result_type<M>::type>::type R;
              std::cerr << std::setw(10) << std::setiosflags(std::ios::left) << std::string("'")+name+std::string("'")
                        << "         " << mace::reflect::get_typename<typename R::signature>()
                        << (R::is_const ? "const" : "") <<std::endl;
              m_cli.methods[name] = cli_functor<typename R::fused_params, R&>(m_vtbl.*m);
              */
         }
         cli&                      m_cli;
         VTableType&               m_vtbl;
         std::vector<std::string>& m_api;
       };

       template<typename Seq, typename Functor>
       struct cli_functor {
           cli_functor( Functor f )
           :m_func(f){}

           typedef typename boost::remove_reference<Functor>::type functor_type;

           template<typename T>
           const T& wait_future( const mace::cmt::future<T>& f ) { return f.wait(); }
           template<typename T>
           const T& wait_future( const T& f ) { return f; }

           std::string operator()( const std::string& cli )
           {
              typedef typename boost::fusion::traits::deduce_sequence<Seq>::type param_type;
              mace::rpc::function_filter<void> f;
              try {
                return json::to_pretty_string( m_func.call_fused(json::io::unpack<param_type>(f, std::vector<char>(cli.begin(),cli.end()))) );
              } catch ( ... ) {
                return boost::current_exception_diagnostic_information(); 
              }
          //    auto v = json::io::pack( f, wait_future( 
           //   if( v.size() )
            //      return std::string( &v.front(), v.size() );
           //   return std::string();
           }
           Functor m_func;
       };
       std::map<std::string, boost::function<std::string(const std::string&)> > methods;
};

} } } // mace::rpc::json

#endif
