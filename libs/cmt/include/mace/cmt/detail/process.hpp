#ifndef _MACE_CMT_DETAIL_PROCESS_HPP_
#define _MACE_CMT_DETAIL_PROCESS_HPP_
#include <boost/iostreams/stream.hpp>

namespace mace { namespace cmt { namespace detail {

    class process_d; 
    namespace io = boost::iostreams;

    class process_sink : public io::sink {
      public:
        struct category : io::sink::category, io::flushable_tag {};
        typedef char      type;

        process_sink( process_d& p ):m_process(p){}
    
        std::streamsize write( const char* s, std::streamsize n );
        void close();
        bool flush();
    
      private:
        process_d&      m_process;
    };

} } } 

#endif // _MACE_CMT_DETAIL_PROCESS_HPP_
