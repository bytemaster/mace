#include <boost/iostreams/stream.hpp>
#include <mace/cmt/thread.hpp>
#include <iostream>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
#include <mace/cmt/signals.hpp>
#include <mace/cmt/mutex.hpp>

namespace mace { namespace cmt {
    namespace io = boost::iostreams;

    
    struct impl {
        impl() {
           buffer.resize(0xffff+1);
           cin_thread = mace::cmt::thread::create("cin");
          // cin_thread->async([this](){read_loop();},"read_loop");
           read_pos = write_pos = 0;
           p  = new promise<void_t>();
        }

        void read_loop() {
          while( true ) {
              while( read_pos + 0xffff == write_pos ) {
                  boost::unique_lock<boost::mutex> lock(sa_mutex);
                  space_avail.wait( lock );
              }
           //   dlog( "wait on cin" );
              auto wpos = write_pos & 0xffff;
              auto rpos = read_pos & 0xffff;
              if( rpos < wpos ) rpos = 0xffff;
              int r = std::cin.readsome(&buffer[wpos], rpos - wpos);
              if( r == 0 && !std::cin.eof() ) {
                 std::cin.get(buffer[wpos] );
                 r = 1;
              }
              if( !std::cin.eof() ) {
                 write_pos += r;
                if( write_pos == read_pos +1 ) p->set_value(void_t());
              } else {
                 elog( "!std::cin.good!" );
                 p->set_value(void_t());
                return;
              }
          }
        }

        char                         cache_line_spacer[64];
        volatile uint64_t            read_pos;      
        char                         cache_line_spacer1[64];
        std::vector<char>            buffer;
        boost::mutex                 sa_mutex;
        boost::condition_variable    space_avail;
        boost::signal<void()>        data_avail;

        promise<void_t>*             p;

        mace::cmt::thread*           cin_thread;
        char                         cache_line_spacer2[64];
        volatile uint64_t            write_pos;      

        std::string                  left;
    };




    class cin_source : public io::source {
      public:
        typedef char      type;

        template<typename T>
        cin_source(T) { my = new impl(); }
        cin_source(){ my = new impl(); }
        ~cin_source() { }

        std::streamsize read( char* s, std::streamsize n ) {
          if( my->left.size() ) {
              int m = std::min(int(n),int(my->left.size()) );
              memcpy( s, my->left.c_str(), m );
              if( my->left.size() == m ) my->left = std::string();
              else my->left = my->left.substr( m, my->left.size() );
              return m;
          }
          my->cin_thread->async([=]() {
                                std::getline( std::cin, my->left );
                                my->left += '\n';
                                } ).wait();
          if( !std::cin.good() ) return -1;
          return this->read(s,n);
          /* 
          while( my->read_pos == my->write_pos && (!std::cin.eof() || std::cin.good())) {
             mace::cmt::usleep(10);
             //my->p->reset();
             //std::swap(my->p, my->p2);
             if( my->read_pos == my->write_pos ) my->p->wait();
          }
          if( my->read_pos == my->write_pos && (std::cin.eof() || !std::cin.good()) ) return -1;

          auto rpos = my->read_pos & 0xffff;
          auto wpos = my->write_pos & 0xffff;
          //slog( "read n: %1% r %2% w %3% %4% %5%", n, my->read_pos, my->write_pos, rpos, wpos );
          if( wpos < rpos ) wpos = (0xffff+1);
          auto m = std::min( size_t(n), size_t(wpos - rpos) );

          memcpy( s, &my->buffer[rpos], m );
          my->read_pos += m;
          //slog( "read n: %2%  m %3% '%1%'  %4%  %5% %6% %7%", std::string( s, s+m ), n, m, my->read_pos, my->write_pos, wpos, rpos);

          boost::unique_lock<boost::mutex> lock(my->sa_mutex);
          my->space_avail.notify_one();
          */

          //return m;
        }

      private:
          impl* my;
    };

    std::istream& get_cin_stream() {
      static io::stream<cin_source> cin_stream;// =   cin_source(); 
      cin_stream.open(NULL);
      return cin_stream;
    }
    
} }
