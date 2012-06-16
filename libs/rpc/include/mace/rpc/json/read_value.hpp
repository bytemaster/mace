#ifndef _MACE_RPC_JSON_READ_VALUE_HPP_
#define _MACE_RPC_JSON_READ_VALUE_HPP_
#include <vector>
#include <mace/rpc/varint.hpp>
#include <mace/rpc/filter.hpp>
#include <mace/reflect/value.hpp>

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
 std::string unescape_string( const std::string s );
 char* inplace_unescape_string( char* );

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


template<typename Container, typename Filter>
struct callback_data{
  callback_data( Container& c, Filter& f, error_collector& e )
  :container(c),filter(f),ec(e),count(0){}

  Container&       container;
  Filter&          filter;
  error_collector& ec;
  int              count;
};

template<typename Container,typename Filter, typename In, typename Out>
void push_back( char* start, char* end, void* data ) {
  callback_data<Container,Filter>& d = *((callback_data<Container,Filter>*)data);
  Out o;
  if( !std::is_same<In,Out>::value ) {
    In tmp;
    from_json( tmp, start, end, d.ec, d.filter );
    d.filter(tmp,o);
  } else {
    from_json( o, start, end, d.ec, d.filter );
  }
  d.container.push_back(std::move(o));
}
template<typename Container,typename Filter, typename In, typename Out>
void insert( char* start, char* end, void* data ) {
  callback_data<Container,Filter>& d = *((callback_data<Container,Filter>*)data);
  Out o;
  if( !std::is_same<In,Out>::value ) {
      In tmp;
      from_json( tmp, start, end, d.ec, d.filter );
      d.filter(tmp,o);
  } else {
      from_json( o, start, end, d.ec, d.filter );
  }
  d.container.insert(std::move(o));
}
template<typename K, typename V,typename Filter, typename In, typename Out>
void set_pair( char* start, char* end, void* data ) {
  callback_data<std::pair<K,V>,Filter>& d = *((callback_data<std::pair<K,V>,Filter>*)data);
  typedef decltype( d.filter(V()) ) second_filter;
  slog( "%1%  '%2%'", d.count, std::string(start,end) );
  typedef decltype( d.filter(K()) ) first_filter;
  switch( d.count ) {
    case 0: 
      if( std::is_same<K,first_filter>::value ) { 
        from_json( d.container.first, start, end, d.ec, d.filter ); 
      } else {
        first_filter tmp; 
        from_json( tmp, start, end, d.ec, d.filter ); 
        d.filter(tmp,d.container.first);
      }
      break;
    case 1: 
      if( std::is_same<V,second_filter>::value ){ 
        from_json( d.container.second, start, end, d.ec, d.filter );
      } else {
        second_filter tmp; 
        from_json( tmp, start, end, d.ec, d.filter ); 
        //d.filter(tmp,d.container.second);
      }
      break;
    default:
      wlog( "Extra values in pair" );
  }
  ++d.count;
}

template<typename Container,typename Filter, typename In, typename Out>
void insert_key( char* name, char* start, char* end, void* data ) {
  callback_data<Container,Filter>& d = *((callback_data<Container,Filter>*)data);
  In tmp;
  from_json( tmp, start, end, d.ec, d.filter );
  Out o;
  d.filter(tmp,o);
  d.container[inplace_unescape_string(name)] = std::move(o);
}

/**
 *  parses a sequence of values calling 'push_back(f(T()))' 
 */
template<typename Container, typename InType, typename OutType, typename F>
void from_json( Container& v, char* itr, char* end, error_collector& ec, F& f ) {
  if( *itr == '[' && *(end-2) == ']' ) {
    slog( "Yep, its an array" );
    callback_data<Container,F> d(v,f,ec);
    void (*callback)(char*,char*,void*) = &push_back<Container,F,InType,OutType>;
    read_values( itr+1, end -2, ec, callback, &d );
  }
  else {
    elog( "'%1%' is not an array", itr );
  }
}

template<typename T, typename F>
void from_json( std::vector<T>& v, char* itr, char* end, error_collector& e, F& f ) {
  v.clear();
  typedef typename std::remove_reference<decltype( f(T()) )>::type filtered_type;
  from_json<std::vector<T>,filtered_type,T,F>( v, itr, end, e, f );
}
template<typename T, typename F>
void from_json( std::list<T>& v, char* itr, char* end, error_collector& e, F& f ) {
  v.clear();
  typedef typename std::remove_reference<decltype( f(T()) )>::type filtered_type;
  from_json<std::list<T>,filtered_type,T,F>( v, itr, end, e, f );
}

template<typename T, typename F>
void from_json( std::set<T>& v, char* itr, char* end, error_collector& ec, F& f ) {
  v.clear();
  typedef typename std::remove_reference<decltype( f(T()) )>::type filtered_type;
  if( *itr == '[' && *(end-2) == ']' ) {
    slog( "Yep, its an array..set" );
    callback_data<std::set<T>,F> d(v,f,ec);
    void (*callback)(char*,char*,void*) = &insert<std::set<T>,F,T,filtered_type>;
    read_values( itr+1, end -2, ec, callback, &d );
  }
  else {
    elog( "'%1%' is not an array..set", itr );
  }
}

template<typename K, typename T, typename F>
void from_json( std::pair<K,T>& v, char* itr, char* end, error_collector& ec, F& f ) {
  if( *itr == '[' ) {
    slog( "Yep, its an pair..set" );
    callback_data<std::pair<K,T>,F> d(v,f,ec);
    void (*callback)(char*,char*,void*) = &set_pair<K,T,F,T,void>;
    read_values( itr+1, end, ec, callback, &d );
    if( d.count < 2 ) {
      wlog( "missing %1% element%2% from pair", 2-d.count, (d.count == 0 ? "'s" : "") );
    }
  }
  else {
    elog( "'%1%' is not an array..pair  '%2%'", itr, *(end-2) );
  }
}



template<typename T, typename F>
void from_json( std::map<std::string,T>& v, char* itr, char* end, error_collector& ec, F& f ) {
  v.clear();
  typedef typename std::remove_reference<decltype( f(T()) )>::type filtered_type;
  if( *itr == '{' && *(end-2) == '}' ) {
    slog( "Yep, its an object..map" );
    callback_data<std::map<std::string,T>,F> d(v,f,ec);
    void (*callback)(char*,char*,char*,void*) = &insert_key<std::map<std::string,T>,F,T,filtered_type>;
    read_key_vals( itr+1, end -2, ec, callback, &d );
  }
  else {
    elog( "'%1%' is not an array..map", itr );
  }
}

template<typename K, typename T, typename F>
void from_json( std::map<K,T>& v, char* itr, char* end, error_collector& ec, F& f ) {
  v.clear();
  typedef typename std::remove_reference<decltype( f(std::pair<K,T>()) )>::type filtered_type;
  if( *itr == '[' && *(end-2) == ']' ) {
    slog( "Yep, its an array..map" );
    callback_data<std::map<K,T>,F> d(v,f,ec);
    void (*callback)(char*,char*,void*) = &insert<std::map<K,T>,F,std::pair<K,T>,filtered_type>;
    read_values( itr+1, end -2, ec, callback, &d );
  }
  else {
    elog( "'%1%' is not an array..map", itr );
  }
}

template<typename Filter>
struct write_from_json_visitor : mace::reflect::write_value_visitor {
  char* s; 
  char* e;
  Filter& f;
  error_collector& ec;
  write_from_json_visitor( char* _s, char* _e, error_collector& _ec, Filter& _f )
  :s(_s),e(_e),ec(_ec),f(_f){}

  virtual void operator()( mace::reflect::value_ref& v ){
    slog("...SUB OBJ........" );
    from_json_reflected( v, s, e, ec, f );
  }
  virtual void operator()( std::string& str ){
    from_json( str, s, e, ec, f );
  }
  virtual void operator()( uint64_t& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( int64_t& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( uint32_t& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( int32_t& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( uint16_t& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  };
  virtual void operator()( int16_t& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( uint8_t& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( int8_t& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( double& d ){
    slog( "got double!" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( float& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
  virtual void operator()( bool& d ){
    slog("............" );
    from_json( d, s, e, ec, f );
  }
};

template<typename Filter>
void set_key( char* name, char* start, char* end, void* data ) {
  callback_data<mace::reflect::value_ref,Filter>& d = *((callback_data<mace::reflect::value_ref,Filter>*)data);
  slog( "key %1%   from %2%", name, start );
  if( !d.container.has_field(name) ) {
    wlog( "type does not have field %1%", name );
  } else {
    slog( "Type has field" );
    d.container[name].visit(write_from_json_visitor<Filter>( start, end, d.ec, d.filter ));
  }
  /*
  In tmp;
  from_json( tmp, start, end, d.ec, d.filter );
  Out o;
  d.filter(tmp,o);
  d.container[inplace_unescape_string(name)] = std::move(o);
  */
}
template<typename Filter, typename In, typename Out>
void push_back( char* start, char* end, void* data ) {
  callback_data<Container,Filter>& d = *((callback_data<Container,Filter>*)data);
  Out o;
  if( !std::is_same<In,Out>::value ) {
    In tmp;
    from_json( tmp, start, end, d.ec, d.filter );
    d.filter(tmp,o);
  } else {
    from_json( o, start, end, d.ec, d.filter );
  }
  d.container.push_back(std::move(o));
}


template<typename Filter>
void from_json_reflected( mace::reflect::value_ref v, char* itr, char* end, error_collector& ec, Filter& f ) {
  if( *itr == '{' && *(end-2) == '}' ) {
    slog( "Yep, its an object. " );
    callback_data<mace::reflect::value_ref,Filter> d(v,f,ec);
    void (*callback)(char*,char*,char*,void*) = &set_key<Filter>;
    read_key_vals( itr+1, end -2, ec, callback, &d );
  } else if( *itr == '[' ) {
    if( !v.is_array() ) {
      elog( "value %1% is not an array", v.type_name() );
    }
    wlog( "Its an array??.. go into push back mode " );
  }
  else {
    elog( "'%1%' is not an array..map", itr );
  }
}


template<typename T>
T from_json( const std::string& js ) {
  std::vector<char> d(js.begin(),js.end());
  d.push_back('\0');
  T v;
  error_collector e;
  default_filter f;
  from_json( v, &d.front(), &d.front()+d.size(), e, f );
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
