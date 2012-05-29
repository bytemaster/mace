/**
 *  @file mace/cmt/log/log.hpp
 *  @brief Defines helpful console logging methods.
 */
#ifndef _BOOST_RPC_LOG_HPP_
#define _BOOST_RPC_LOG_HPP_
#include <boost/format.hpp>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <boost/exception/diagnostic_information.hpp>

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#ifndef WIN32
#define  COLOR_CONSOLE 1
#endif
#include <mace/cmt/log/console_defines.h>
#include <boost/thread/mutex.hpp>

namespace mace { namespace cmt { 
    const char* thread_name();

    namespace detail {
    inline boost::mutex& log_mutex() {
      static boost::mutex m; return m;
    }
    inline std::string short_name( const std::string& file_name ) { return file_name.substr( file_name.rfind( '/' ) + 1 ); }

    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const char* text ) {
        boost::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<<text<< CONSOLE_DEFAULT << std::endl; 
    }
    template<typename P1>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1 ) {
        boost::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2 ) {
        boost::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< boost::format(format) %p1 %p2 << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3 ) {
        boost::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3, typename P4>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3, const P4& p4 ) {
        boost::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3 %p4) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3, typename P4, typename P5>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5 ) {
        boost::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3 %p4 %p5) << CONSOLE_DEFAULT << std::endl;
    }
    template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    inline void log( std::ostream& os, const char* color, const char* file, uint32_t line, const char* method, const std::string& format, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5, const P6& p6 ) {
        boost::unique_lock<boost::mutex> lock(log_mutex());
        os<<color<<std::left<<std::setw(10)<<thread_name()<<" "<<short_name(file)<<":"<<line<<" "<<method<<"] "<< (boost::format(format) %p1 %p2 %p3 %p4 %p5 %p6) << CONSOLE_DEFAULT << std::endl;
    }

} } } // mace::cmt::detail
/**
 *  @def dlog
 */
#define dlog(...) do {try { mace::cmt::detail::log( std::cerr, CONSOLE_DEFAULT, __FILE__, __LINE__, __func__, __VA_ARGS__ ); } \
                catch (boost::exception&e){ mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", boost::diagnostic_information(e) ); }  \
                catch (std::exception&e){ mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", boost::diagnostic_information(e) ); }  \
                catch (...) {mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs - exception while formating args" ); }  \
                }while(false)

/**
 *  @def slog
 */
#define slog(...) do {try {mace::cmt::detail::log( std::cerr, CONSOLE_DEFAULT, __FILE__, __LINE__, __func__, __VA_ARGS__ ); }\
                catch (boost::exception&e){ mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", boost::diagnostic_information(e) ); }  \
                catch (std::exception&e){ mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", boost::diagnostic_information(e) ); }  \
                catch (...) {mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs - exception while formating args" ); }  \
                }while(false)

/**
 *  @def elog
 */
#define elog(...) do {try {mace::cmt::detail::log( std::cerr, CONSOLE_RED,     __FILE__, __LINE__, __func__, __VA_ARGS__ ); }\
                catch (boost::exception&e){ mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", boost::diagnostic_information(e) ); }  \
                catch (std::exception&e){ mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", boost::diagnostic_information(e) ); }  \
                catch (...) {mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs - exception while formating args" ); }  \
                }while(false)

/**
 *  @def wlog
 */
#define wlog(...) do {try {mace::cmt::detail::log( std::cerr, CONSOLE_BROWN,   __FILE__, __LINE__, __func__, __VA_ARGS__ ); }\
                catch (boost::exception&e){ mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", boost::diagnostic_information(e) ); }  \
                catch (std::exception&e){ mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs: %1%", boost::diagnostic_information(e) ); }  \
                catch (...) {mace::cmt::detail::log( std::cerr, CONSOLE_RED, __FILE__, __LINE__, __func__, "Invalid logs - exception while formating args" ); }  \
                }while(false)

#endif 
