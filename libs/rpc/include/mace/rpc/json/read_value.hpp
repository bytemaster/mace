#ifndef _MACE_RPC_JSON_READ_VALUE_HPP_
#define _MACE_RPC_JSON_READ_VALUE_HPP_
#include <vector>
#include <mace/rpc/varint.hpp>

namespace mace { namespace rpc { namespace json {

  namespace errors {
    enum error_type {
      unknown_error       = 0x0001,  // Other errors not specified below
      warning             = 0x0002,  // Other warnigns not specified below
      sytnax_error        = 0x0004,  // fatal syntax errors unclosed brace
      sytnax_warning      = 0x0008,  // recoverable syntax error (missing, missing ':', unquoted string)
      missing_comma       = 0x0010,  // if the error was related to json syntax and not semantic
      string_to_int       = 0x0020,  // any time lexical cast from string to int is required
      double_to_int       = 0x0040,  // any time a double is received for an int
      int_overflow        = 0x0080,  // any time int value is greater than underlying type
      signed_to_unsigned  = 0x0100,  // any time a negative value is read for an unsigned field
      int_to_bool         = 0x0200,  // any time an int is read for a bool 
      string_to_bool      = 0x0400,  // any time a string is read for a bool
      bad_array_index     = 0x0800,  // atempt to read a vector field beyond end of sequence
      unexpected_key      = 0x1000,  // fields in object
      missing_key         = 0x2000,  // check required fields
      type_mismatch       = 0x4000,  // expected a fundamental, got object, expected object, got array, etc.
      type_conversion     = 0x8000,  // also set any time a conversion occurs
      all                 = 0xffff,
      none                = 0x0000
    };
  } // namespace errors

/**
 *  Stores information about errors that occurred durring the parse.
 *
 *  By default extra fields are 'ignored' as warning
 *  Loss of presision errors are warning.
 *  String to Int conversion warnings
 *  Double to Int
 *  Int to bool
 *
 */
struct parse_error {
  parse_error( int32_t ec, std::string msg, char* s = 0, char* e = 0 )
  :message(std::move(msg)),type(ec),start(s),end(e){}

  parse_error( parse_error&& m )
  :message(std::move(m.message)),type(m.type),start(m.start),end(m.end){}

  std::string message;
  int32_t     type;
  char*       start;
  char*       end;
};

/**
 *  Collects errors and manages how they are responded to.
 */
class error_collector : public boost::exception {
  public:
    error_collector( error_collector&& e )
    :m_errors(std::move(e.m_errors)){ 
      memcpy((char*)m_eclass,(char*)e.m_eclass, sizeof(m_eclass) );
    }
    /*
    error_collector( const error_collector&& e )
    :m_errors(e.m_errors){
      memcpy((char*)m_eclass,(char*)e.m_eclass, sizeof(m_eclass) );
    }
    */
    ~error_collector() throw() {
      try {
        m_errors.clear();
      }catch(...){}
    }

    enum error_defaults {
       default_report  = json::errors::all,
       default_recover = json::errors::all,
       default_throw   = json::errors::none,
       default_ignore  = ~(default_report|default_recover|default_throw)
    };

    error_collector(){
      m_eclass[report_error_t]  = default_report;
      m_eclass[recover_error_t] = default_recover;
      m_eclass[throw_error_t]   = default_throw;
      m_eclass[ignore_error_t]  = default_ignore;
    }

    inline bool report( int32_t e )const {
      return m_eclass[report_error_t] & e;
    }
    inline bool recover( int32_t e )const {
      return m_eclass[recover_error_t] & e;
    }
    inline bool ignore( int32_t e )const {
      return m_eclass[ignore_error_t] & e;
    }

    void report_error( int32_t e ) {
      m_eclass[report_error_t] |= e;
      m_eclass[ignore_error_t] &= ~e;
    }
    void recover_error( int32_t e ) {
      m_eclass[recover_error_t] |= e;
      m_eclass[ignore_error_t] &= ~e;
    }
    void throw_error( int32_t e ) {
      m_eclass[throw_error_t]  |= e;
      m_eclass[ignore_error_t] &= ~e;
    }
    void ignore_error( int32_t e ) {
      m_eclass[ignore_error_t]  |= e;
      m_eclass[report_error_t]  &= ~m_eclass[ignore_error_t];
      m_eclass[recover_error_t] &= ~m_eclass[ignore_error_t];
      m_eclass[throw_error_t]   &= ~m_eclass[ignore_error_t];
    }

    void post_error( int32_t ec, std::string msg, char* s = 0, char* e = 0 ) {
      m_errors.push_back( parse_error( ec, std::move(msg), s, e ) ); 
      if( ec & m_eclass[throw_error_t] ) {
        throw std::move(*this);
      }
    }
    const std::vector<parse_error>& get_errors()const {
      return m_errors;
    }

  private:
    enum error_class {
      ignore_error_t,
      report_error_t,
      recover_error_t,
      throw_error_t,
      num_error_classes
    };
    uint32_t m_eclass[num_error_classes];
    std::vector<parse_error>   m_errors;
};

/**
 *  @param val    reference to the value to be read
 *  @param input  the input buffer, may be temporarialy modified but will be restored.
 *  @param end    the end of the input
 *  @param e      collects errors and specifies how errors should be handled
 *  @param f      filters the unpack
 *
 *  @return the position after consumed bytes. If itr == return then an error occured.
 */
template<typename T, typename F>
void from_json( T& v, char* itr, char* end, error_collector& e, F& f );

void from_json( int64_t& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( int64_t& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}

void from_json( float& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( float& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}

void from_json( double& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( double& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}

void from_json( uint64_t& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( uint64_t& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}
void from_json( int32_t& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( int32_t& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}
void from_json( uint32_t& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( uint32_t& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}
void from_json( int16_t& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( int16_t& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}
void from_json( uint16_t& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( uint16_t& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}
void from_json( int8_t& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( int8_t& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}
void from_json( uint8_t& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( uint8_t& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}
void from_json( bool& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( bool& v, char* itr, char* end, error_collector& e, F& ){
  from_json(v,itr,end,e);
}
void from_json( std::string& v, char* itr, char* end, error_collector& e);
template<typename F> // strip of the filter, implement in cpp file
void from_json( std::string& v, char* itr, char* end, error_collector& e, F&  ){
  from_json(v,itr,end,e);
}
template<typename F> // strip of the filter, implement in cpp file
void from_json( unsigned_int& v, char* itr, char* end, error_collector& e, F&  ){
  from_json(v.value,itr,end,e);
}
template<typename F> // strip of the filter, implement in cpp file
void from_json( signed_int& v, char* itr, char* end, error_collector& e, F&  ){
  from_json(v.value,itr,end,e);
}


/**
 *  parses a sequence of values calling 'push_back(f(T()))' 
 */
template<typename Container, typename InType, typename OutType, typename F>
void from_json( Container& v, char* itr, char* end, error_collector& e, F& f ) {
/*
  if( *itr == '[' && *(end-1) == ']' ) {
    char* start = itr + 1; // array values start after [ 
    char* ae    = end - 1; // array values end before ]

    char* next;
    // read one value from the buffer
    start = read_value( start, ae, next );
    while( start != ae ) {
      InType tmp;
      char* r = from_json( tmp, start, next, e, f  );
      if( r != start ) { // we consumed something... success@
        v.push_back(OutType);
        f(tmp,v.back());
      }
      if( r != next ) {
        wlog( "unused bytes..." );
      }
      BOOST_ASSERT( next <= ae );
    }
  } else {
    using namespace mace::rpc::json::errors;
    if( e.ignore( type_mismatch ) ) {
       // if recover, try to parse it as a single T  
       if( e.recover( type_mismatch ) ) {
       }
       if( e.report( type_mismatch ) ) {
          e.post_error( type_mismatch, "Expected Array", itr, end );
       }
    }
  }
  */
 return 0;
}

template<typename T, typename F>
void from_json( std::vector<T>& v, char* itr, char* end, error_collector& e, F& f ) {
  typedef std::remove_reference<decltype( f(T()) )> filtered_type;
  char* post = from_json<std::vector<T>,filtered_type,F>( v, itr, end, e, f );

  return end;
}

template<typename T>
T from_json( const std::string& js ) {
  std::vector<char> d(js.begin(),js.end());
  d.push_back('\0');
  T v;
  error_collector e;
  from_json( v, &d.front(), &d.front()+d.size(), e );
  return v;
}


/**
 *   Returns the first non-whitespace char.
 */
template<typename Iterator>
bool skip_whitespace( Iterator& in, const Iterator& e ) {
   if( in == e ) 
     return false;
   char c = *in;
   // ignore leading whitespace
   while( c == ' ' || c == '\t' || c == '\n' || c == '\r' ) {
     ++in;
     if( in == e ) 
       return false;
     c = *in;
   }
   return true;
}



/**
 *   In-place reading of values.
 *
 *   Ignores leading white space. 
 *   If it starts with [,", or { reads until matching ],", or }
 *   If it starts with something else it reads until [{",}]: or whitespace only
 *        allowing a starting - or a single .
 *
 *   @note internal json syntax errors are not caught, only bracket errors 
 *         are caught by this method.  This makes it easy for error recovery
 *         when values are read recursivley.
 *   
 *   @param in    start of input
 *   @param end   end of input
 *   @param oend  output parameter to the end of the value
 *   
 *   @return a pointer to the start of the value
 */
char* read_value( char* in, char* end, char*& oend );

/**
 *  Calls on_value(start,end,self)
 *  Start is a null terminated string.
 */
void  read_values( char* in, char* end, error_collector& ec, void (*on_value)(char*,char*,void*), void* self );

/**
 *  Calls on_key(name,start,end,self) for each key found between in and end.
 *  key ::  "name" : VALUE
 *  Posts a warning if the values are not ',' separated.
 */
void read_key_vals( char* in, char* end, mace::rpc::json::error_collector& ec, void(*on_key)(char*,char*,char*,void*), void* s);

/**
 *   @tparam Iterator - an input iterator
 *
 *   Ignores leading white space.
 *   If it starts with [" or { reads until matching ]" or }
 *   If it starts with something else it reads until [{",}]: or whitespace only
 *        allowing a starting - or a single .
 *
 *   Once you have a valid range, use 'from_json' to load the json into
 *   an object.
 *
 *   @note internal json syntax errors are not caught, only bracket errors 
 *         are caught by this method.  This makes it easy for error recovery
 *         when values are read sequentially.
 *
 *   @return a null-terminated vector<char> containing the first 'valid' json value
 */
template<typename Iterator>
std::vector<char> read_value( Iterator itr, const Iterator& end ) {
   std::vector<char> buf;
   if( !skip_whitespace(itr,end) ) return buf;

   bool found_dot = false;
   // check for literal vs object, array or string
   switch( *itr ) {
     case '[':
     case '{':
     case '"':
       break;
     default: {  // literal
       // read until non-literal character
       // allow it to start with - 
       // allow only one '.' 
       while( itr != end ) {
         switch( *itr ) {
           case '[': 
           case '{': 
           case '}': 
           case ']':
           case '"': 
           case ',': 
           case ':': 
           case ' ': 
           case '\t': 
           case '\n': 
           case '\r': 
             return buf;
           case '.':
             if( found_dot ) 
                return buf;
             found_dot = true;
             break;
           case '-':
             if( buf.size() ) { buf.push_back('\0'); return buf; }
         }
         buf.push_back(*itr);
         ++itr;
       }
       buf.push_back('\0'); 
       return buf; 
     }
   } // end literal check

   int depth = 0;
   bool in_quote = false;
   bool in_escape = false;
   // read until closing ]} or " ignoring escaped "
   while( itr != end ) {
     if( !in_quote ) {
       switch( *itr ) {
         case '[':
         case '{': ++depth;         break;
         case ']':
         case '}': --depth;         break;
         case '"': 
           ++depth;
           in_quote = true; 
           break;
         default: // do nothing;
           break;
       }
     } else { // in quote
       switch( *itr ) {
         case '"': if( !in_escape ) {
           --depth;
           in_quote = false;
           break;
         }
         case '\\': 
           in_escape = !in_escape;
           break;
         default:
           in_escape = false;
       }
     }
     buf.push_back(*itr);
     ++itr;
     if( !depth )  {
       buf.push_back('\0'); 
       return buf; 
     }
  }
  if( depth != 0 ) {
   // TODO: Throw Parse Error!
   std::cerr<<"Parse Error!\n";
  }
  buf.push_back('\0'); 
  return buf;
}

} } } // mace::rpc::json

#endif 
