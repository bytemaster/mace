#include <boost/cmt/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <boost/cmt/log/log.hpp>
#include <boost/context/all.hpp>
#include <boost/cmt/signals.hpp>

using namespace boost::cmt;

namespace bc = boost::contexts;
typedef bc::context<> context_t;

context_t* a;
context_t* b;
context_t* c;

boost::signal<void(std::string)> test_signal;

int goodbye(const std::string& world ) {
//    slog( "goodbye %1%", world );
    return 5;//world.size();
}
void bench();

void_t delay()
{
    slog( "delay 3 sec, %1%", 3.13);
    boost::cmt::thread::current().usleep(2000000);
    slog( "test_signal");
    test_signal("hello world!");
}

int hello(const std::string& world ) {
    
    int test;
 //   slog("%1%  %2%", world, &test );
    /*
    if( n == 0 )
        a->suspend();
    if( n )
        n->resume();
        */
    //boost::cmt::thread::current().usleep(1000000);
    
    ////slog( "waiting for test_signal" );
    //std::string rtn = boost::cmt::wait<std::string>(test_signal, 3000000);
    //slog( "got test_signal %1%", rtn );

    //std::cerr<<world<< " " << async<int>(boost::bind(goodbye, "goodbye"), "goodbye_func" ).wait() <<std::endl;
    return world.size();
}

void main2() {
 //   slog( "%1%", *tmp );
    /*
    future<int> rtn  = async<int>( boost::bind(hello, "world"), "hello_func" );
    future<int> rtn2 = async<int>( boost::bind(hello, "world2"), "hello_func2" );
    std::cerr<<"length: "<<rtn.wait()<<std::endl;
    future<int> rtn3 = async<int>( boost::bind(hello, "world3"), "hello_func3" );
    future<int> rtn4 = async<int>( boost::bind(hello, "world4____"), "hello_func4" );
    std::cerr<<"length: "<<rtn4.wait()<<std::endl;
    future<int> rtn5 = async<int>( boost::bind(hello, "world5"), "hello_func5" );
    wlog( "main2 done" );
    */
}

void bench() {
    async<void_t>(delay).wait();
    uint64_t cnt = 20000000;

    boost::cmt::thread* t = boost::cmt::thread::create();

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
    std::string hellostr("hello");
    for( uint32_t i = 0; i < cnt; ++i ) {
        try {
            t->async<int>( boost::bind(hello, boost::ref(hellostr)), "hello_func" ).wait(1000000);
           // async<int>( boost::bind(hello, "world"), "hello_func" ).wait();//2000000);
        } catch( const boost::exception& e ) {
                elog( "%1%", boost::diagnostic_information(e) );
        }
    }
    boost::posix_time::ptime stop = boost::posix_time::microsec_clock::universal_time();
    slog( "%1% calls/sec", ((cnt*1.0)/((stop-start).total_microseconds()/1000000.0)) );
    start = boost::posix_time::microsec_clock::universal_time();
    for( uint32_t i = 0; i < cnt; ++i ) {
        try {
           // async<int>( boost::bind(hello, "world"), "hello_func" ).wait(1000000);
            t->async<int>( boost::bind(hello, boost::ref(hellostr)), "hello_func" ).wait();//2000000);
        } catch( const boost::exception& e ) {
                elog( "%1%", boost::diagnostic_information(e) );
        }
    }
    stop = boost::posix_time::microsec_clock::universal_time();
    slog( "%1% calls/sec", ((cnt*1.0)/((stop-start).total_microseconds()/1000000.0)) );
    start = boost::posix_time::microsec_clock::universal_time();
    for( uint32_t i = 0; i < cnt; ++i ) {
        try {
           // async<int>( boost::bind(hello, "world"), "hello_func" ).wait(1000000);
            t->sync<int>( boost::bind(hello, boost::ref(hellostr)), "hello_func" );//2000000);
        } catch( const boost::exception& e ) {
                elog( "%1%", boost::diagnostic_information(e) );
        }
    }
    stop = boost::posix_time::microsec_clock::universal_time();
    slog( "%1% calls/sec", ((cnt*1.0)/((stop-start).total_microseconds()/1000000.0)) );
}

int main( int argc, char** argv )
{
    slog( "%1%", 1.234 );
    /*
    a = new context_t( boost::bind(&hello,"A",(context_t*)NULL), 
                   bc::protected_stack( bc::stack_helper::default_stacksize() ), true, true );
    b = new context_t( boost::bind(&hello,"B",a), 
                   bc::protected_stack( bc::stack_helper::default_stacksize() ), true, true );
    c = new context_t( boost::bind(&hello,"C",b), 
                   bc::protected_stack( bc::stack_helper::default_stacksize() ), true, true );

    c->resume();
    slog( "done" );
    */
    async(bench);
   // async(main2);
    boost::cmt::exec();
}
