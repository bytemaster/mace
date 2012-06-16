#include <string>
#include <mace/cmt/log/log.hpp>
#include <mace/rpc/json/read_value.hpp>
#include <boost/lexical_cast.hpp>

namespace mace { namespace rpc { namespace json {
  std::string escape_string( const std::string s ) {
    // calculate escape string size.
    uint32_t ecount = 0;
    for( auto i = s.begin(); i != s.end(); ++i ) {
      if( ' '<= *i && *i <= '~' &&  *i !='\\' && *i != '"' ) {
        ecount+=1;
      } else {
        switch( *i ) {
          case '\t' : 
          case '\n' : 
          case '\r' : 
          case '\\' : 
          case '"' : 
            ecount += 2; break;
          default: 
            ecount += 4;
        }
      }
    }
    // unless the size changed, just return it.
    if( ecount == s.size() ) { return s; }
    
    // reserve the bytes
    std::string out; out.reserve(ecount);
    // print it out.
    for( auto i = s.begin(); i != s.end(); ++i ) {
      if( ' '<= *i && *i <= '~' &&  *i !='\\' && *i != '"' ) {
        out += *i;
      } else {
        out += '\\';
        switch( *i ) {
          case '\t' : out += 't'; break;
          case '\n' : out += 'n'; break;
          case '\r' : out += 'r'; break;
          case '\\' : out += '\\'; break;
          case '"' :  out += '"'; break;
          default: 
            out += "x";
            const char* const hexdig = "0123456789abcdef";
            out += hexdig[*i >> 4];
            out += hexdig[*i & 0xF];
        }
      }
    }
    return out;
  }

  uint8_t from_hex( char c ) {
    if( c >= '0' && c <= '9' )
      return c - '0';
    if( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return c;
  }
  
  std::string unescape_string( const std::string s ) {
    std::string out; out.reserve(s.size());
    for( auto i = s.begin(); i != s.end(); ++i ) {
      if( *i != '\\' ) {
        if( *i != '"' ) out += *i;
      }
      else {
        ++i; 
        if( i == out.end() ) return out;
        switch( *i ) {
          case 't' : out += '\t'; break;
          case 'n' : out += '\n'; break;
          case 'r' : out += '\r'; break;
          case '\\' : out += '\\'; break;
          case '"' :  out += '"'; break;
          case 'x' : { 
            ++i; if( i == out.end() ) return out;
            char c = from_hex(*i);           
            ++i; if( i == out.end() ) { out += c;  return out; }
            c = c<<4 | from_hex(*i);           
            out += c;
            break;
          }
          default:
            out += '\\';
            out += *i;
        }
      }
    }
    return out;
  }

  /**
   *  Any unescaped quotes are dropped. 
   *  Because unescaped strings are always shorter, we can simply reuse
   *  the memory of s.
   *  
   *  @param s a null terminated string that contains one or more escape chars
   */
  char* inplace_unescape_string( char* s ) {
    while( *s == '\"' ) ++s;
    char* out = s;
    for( auto i = s; *i != '\0'; ++i ) {
      if( *i != '\\' ) {
        if( *i != '"' ) {
            *out = *i;
            ++out; 
        }
      }
      else {
        ++i; 
        if( *i == '\0' ) { *out = '\0'; return s; }
        switch( *i ) {
          case 't' : *out = '\t'; ++out; break;
          case 'n' : *out = '\n'; ++out; break;
          case 'r' : *out = '\r'; ++out; break;
          case '\\': *out = '\\'; ++out; break;
          case '"' : *out = '"'; ++out; break;
          case 'x' : { 
            ++i; if( *i == '\0' ){ *out = '\0'; return s; }
            char c = from_hex(*i);           
            ++i; if( *i == '\0' ){ *out = c; ++out; *out = '\0'; return s; }
            c = c<<4 | from_hex(*i);           
            *out = c;
            ++out;
            break;
          }
          default:
            *out = '\\';
            ++out; 
            *out = *i;
            ++out;
        }
      }
    }
    *out = '\0';
    return s;
  }





  /**
   *  JSON Parsing Strategy 
   *  1) don't 'expand' a json value unless we need it.
   *  2) don't copy strings unless necessary
   * 
   *  Giving any stream, read 'one value' as defined by matching "", int, double, true, false,  
   *  matching [] or matching {}
   *
   *  Return a string and a value type.
   */



/**
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
char* read_value( char* in, char* end, char*& oend ) {
   char* start = in;
   // ignore leading whitespace
   while( (in < end) && ((*in == ' ') || (*in == '\t') || (*in == '\n') || (*in == '\r')) ) {
     ++in;
   }
   start = in;
   if( start == end ) {
    oend = end;
    return start;
   }

   bool found_dot = false;
   // check for literal vs object, array or string
   switch( *in ) {
     case ':':
     case ',':
     case '=':
        oend = in+1;
        return start;
     case '[':
     case '{':
     case '"':
       break;
     default: {  // literal
       // read until non-literal character
       // allow it to start with - 
       // allow only one '.' 
       while( in != end ) {
         switch( *in ) {
           case '[': case ']':
           case '{': case '}': 
           case ':': case '=':
           case ',': case '"': 
           case ' ': case '\t': case '\n': case '\r': { 
             oend = in;
             return start;
           }
           case '.':
             if( found_dot ) {
                oend = in;
                return start;
             }
             found_dot = true;
             break;
           case '-':
             if( in-start ){ oend = in; return start; }
         }
         ++in;
       }
       oend = in;
       return start;
     }
   } // end literal check

   int depth = 0;
   bool in_quote = false;
   bool in_escape = false;
   // read until closing ]} or " ignoring escaped "
   while( in != end ) {
     if( !in_quote ) {
       switch( *in ) {
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
       switch( *in ) {
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
     ++in;
     if( !depth ) { oend = in; return start; }
  }
  if( depth != 0 ) {
   // TODO: Throw Parse Error!
   elog("Parse Error!!");
  }
  oend = in; return start;
}

struct temp_set {
  temp_set( char* v, char t )
  :old(*v),val(v) {  *val = t; }
  ~temp_set() { *val = old; }
  char  old;
  char* val;
};

/**
 * A,B,C
 * Warn on extra ',' or missing ','
 */
void  read_values( char* in, char* end, mace::rpc::json::error_collector& ec, void (*on_value)(char*,char*,void*), void* self ) {
  char* ve = 0;
  char* v = mace::rpc::json::read_value( in, end, ve );
  while( *v == ',' ) {
    wlog( "unexpected ','");
    v = mace::rpc::json::read_value( ve, end, ve );
  }
  if( v == ve ) return; // no values

  { temp_set temp(ve,'\0'); on_value( v, ve, self ); }

  char* c;
  char* ce = 0;
  do { // expect comma + value | ''

    // expect comma or ''
    c = mace::rpc::json::read_value( ve, end, ce );

    // '' means we are done, no errors
    if( c == ce ) return;

    if( *c != ',' ) // we got a value when expecting ','
    {
       wlog( "missing ," );
       temp_set temp(ce,'\0'); on_value( c, ce, self );
       ve = ce;
       continue; // start back at start
    }
    
    // expect value
    v = mace::rpc::json::read_value( ce, end, ve );
    while ( *v == ',' ) { // but got comma
      // expect value
      wlog( "extra comma at c->ce" );
      c = v; ce = ve;
      v = mace::rpc::json::read_value( ve, end, ve );
    }
    if( v == ve ) {
      wlog( "trailing comma at c->ce" );
    } else { // got value
      temp_set temp(ve,'\0'); 
      on_value( v, ve, self ); 
    }
  } while( ve < end );// expect comma + value | ''
}


/**
 *  Reads an optional ',' followed by key : value, returning the next input position
 *  @param sc - start with ','
 */
char* read_key_val( bool sc, char* in, char* end, mace::rpc::json::error_collector& ec, void(*on_key)(char*,char*,char*,void*), void* self ) {
  char* name_end = 0;
  char* name = in;
  do {
    // read first char
    name = mace::rpc::json::read_value( name, end, name_end );
    if( sc ) { // if we expect a ,
      if( *name != ',' ) { // but didn't get one
        wlog( "expected ',' but got %1%", name ); // warn and accept name
      } else { // we got the exepcted , read the expected name 
        name = mace::rpc::json::read_value( name_end, end, name_end );
      }
    } else { // don't start with ','
      while( *name == ',' ) { //  while we don't have a name, keep looking
        wlog( "unexpected ',' " );
        name = mace::rpc::json::read_value( name_end, end, name_end );
      }
    }
  } while( *name == ',' );

  
  // now we should have a name.
  if( name_end >= end -1 ) {
    temp_set ntemp(name_end,'\0');
    elog( "early end after reading name %1%", name );
    return name_end;
  }
  if( *name != '"' ) {
    temp_set ntemp(name_end,'\0');
    wlog( "unquoted name '%1%'", name );
  }

  char* col_end = 0;
  char* col = mace::rpc::json::read_value( name_end, end, col_end );

  char* val_end = 0;
  char* val = 0;
  
  bool sep_error = false;
  if( col_end-col == 1 ) {
    switch(  *col ) {
      case ':': break;
      case ';': 
      case '=': 
        wlog( "found %1% instead of ':'", *col );
        break;
      default:
        sep_error = true;
    }
  } else {
    sep_error = true;
  }

  if( sep_error ) {
    temp_set ntemp(name_end,'\0');
    temp_set vtemp(col_end,'\0');
    wlog( "expected ':' but got %1%", col );
    wlog( "assuming this is the value... " );
    val = col;
    val_end = col_end;
  }  else {
    if( name_end >= end -1 ) {
      temp_set ntemp(name_end,'\0');
      temp_set vtemp(col_end,'\0');
      elog( "early end after reading name '%1%' and col '%2%'", name, col );
      return name_end;
    }
    val = mace::rpc::json::read_value( col_end, end, val_end );
    if( val == end ) {
      wlog( "no value specified" );
    }
  }
  temp_set ntemp(name_end,'\0');
  temp_set vtemp(val_end,'\0');
  on_key( name, val, val_end, self );
  return val_end;
}

// first_key =::  '' | "name" : VALUE  *list_key
// list_key       '' | ',' "name" : VALUE
void read_key_vals( char* in, char* end, mace::rpc::json::error_collector& ec, void(*on_key)(char*,char*,char*,void*), void* s) {
  bool ex_c = false;
  char* kv_end = in;
  do {
    //slog( "%1% bytes to read", (end-kv_end) );
    kv_end = read_key_val( ex_c, kv_end, end, ec, on_key, s );
    ex_c = true;
  } while( kv_end < end );
}




template<typename T>
void real_from_json( T& v, char* itr, char* end, error_collector& e ) {
  char* i = itr;
  char* ie = end;
  if( *itr == '"' ) { ++i; --ie; --ie;
    wlog( "quoted string to number '%1%'", std::string(itr,end) );
  }
  temp_set move_end(ie,'\0');
  auto bv = boost::lexical_cast<double>(i);
  if( bv < -std::numeric_limits<T>::max() || 
      bv > std::numeric_limits<T>::max() ) {
    wlog( "Loss of presision" );
  }
  v = static_cast<T>(bv);
}

template<typename T>
void int_from_json( T& v, char* itr, char* end, error_collector& e ) {
  char* i = itr;
  char* ie = end;
  if( *itr == '"' ) { ++i; --ie; --ie; 
    slog( "'%1%'", std::string(i,ie) );
    wlog( "quoted string to number" );
  }
  temp_set move_end(ie,'\0');

  // check to see if we have a float number
  char* t = i;
  while( t != ie ) { if( *t == '.' ) break; ++t; }

  if( !std::is_signed<T>::value ) {
    if( *i == '-' ) {  wlog( "signed value for unsigned field" ); }
      if( t == ie ) {
        uint64_t bv = boost::lexical_cast<uint64_t>(i);
        if( bv < std::numeric_limits<T>::min() || 
            bv > std::numeric_limits<T>::max() ) {
          wlog( "truncating value" );
        }
        v = static_cast<T>(bv);
      } else {
        wlog( "Converting real to int" ); 
        v =  static_cast<T>(boost::lexical_cast<double>(i));
      }
  } else {
    if( t == ie ) {
        int64_t bv = boost::lexical_cast<int64_t>(i);
        if( bv < std::numeric_limits<T>::min() || 
            bv > std::numeric_limits<T>::max() ) {
          wlog( "truncating value" );
        }
        v = static_cast<T>(bv);
    } else {
        wlog( "Converting real to int" ); 
        v =  static_cast<T>(boost::lexical_cast<double>(i));
    }
  }
}

void from_json( int32_t& v, char* itr, char* end, error_collector& e ) { 
  int_from_json(v,itr,end,e);
}
void from_json( uint32_t& v, char* itr, char* end, error_collector& e ) { 
  int_from_json(v,itr,end,e);
}
void from_json( int64_t& v, char* itr, char* end, error_collector& e ) { 
  int_from_json(v,itr,end,e);
}
void from_json( uint64_t& v, char* itr, char* end, error_collector& e ) { 
  int_from_json(v,itr,end,e);
}
void from_json( int16_t& v, char* itr, char* end, error_collector& e ) { 
  int_from_json(v,itr,end,e);
}
void from_json( uint16_t& v, char* itr, char* end, error_collector& e ) { 
  int_from_json(v,itr,end,e);
}
void from_json( int8_t& v, char* itr, char* end, error_collector& e ) { 
  int_from_json(v,itr,end,e);
}
void from_json( uint8_t& v, char* itr, char* end, error_collector& e ) { 
  int_from_json(v,itr,end,e);
}
void from_json( double& v, char* itr, char* end, error_collector& e ) { 
  real_from_json(v,itr,end,e);
}
void from_json( float& v, char* itr, char* end, error_collector& e ) { 
  real_from_json(v,itr,end,e);
}
void from_json( bool& v, char* itr, char* end, error_collector& e ) { 
  char* i = itr;
  char* se = end;
  if( *itr == '"' ) { 
    ++i;
    --se;
    wlog( "bool from string" );
  }
  temp_set move_end(se,'\0');

  if( strcmp( i, "true" ) == 0 ){
    v = true;
  } else if( strcmp( i, "false" ) == 0 ) {
    v = false;
  } else {
    wlog( "bool from number ?? " );
    double b=0;
    from_json(b,i,se,e);
    v = (b != 0);
  }
}
void from_json( std::string& v, char* itr, char* end, error_collector& e) {
  // TODO: implace unescape
  if( *itr == '"' ) { 
    temp_set move_end(end,'\0');
    v = inplace_unescape_string(itr);
  } else {
    wlog( "unescaped string! " );
    v = std::string(itr,end);
  }
}








} } }
