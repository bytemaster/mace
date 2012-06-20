#include <mace/rpc/value.hpp>
#include <string.h>

namespace mace { namespace rpc {

  namespace detail {
      value_holder::~value_holder(){}
      const char* value_holder::type()const                  { return "null"; }
      void value_holder::visit( const_visitor&& v )const     { v();           }
      void value_holder::visit( visitor&& v )                { v();           }

      void value_holder::clear()                             {}
      size_t value_holder::size()const                       { return 0; }
      void value_holder::resize( size_t )                    { throw std::runtime_error( std::string("value not an array") ); }
      void value_holder::reserve( size_t )                   { throw std::runtime_error( std::string("value not an array or object") ); }
      value& value_holder::at( size_t )                      { throw std::runtime_error( std::string("value not an array") ); }
      const value& value_holder::at( size_t )const           { throw std::runtime_error( std::string("value not an array") ); }
      void value_holder::push_back( value&& v )              { throw std::runtime_error( std::string("value not an array") ); }

      value_holder* value_holder::move_helper( char* c )     { return new(c) value_holder(); }
      value_holder* value_holder::copy_helper( char* c )const{ return new(c) value_holder(); }

      void value_holder_impl<array>::resize( size_t s )               { val.fields.resize(s);  }
      void value_holder_impl<array>::reserve( size_t s )              { val.fields.reserve(s); }
      value& value_holder_impl<array>::at( size_t i)                  { return val.fields[i]; }
      const value& value_holder_impl<array>::at( size_t i)const       { return val.fields[i]; }
      value_holder* value_holder_impl<array>::move_helper( char* c ){ return new(c) value_holder_impl( std::move(val) ); }
      value_holder* value_holder_impl<array>::copy_helper( char* c )const{ return new(c) value_holder_impl(val);              }

      void value_holder_impl<array>::clear()                        { val.fields.clear();        }
      size_t value_holder_impl<array>::size()const                  { return val.fields.size();  }
      void value_holder_impl<array>::visit( const_visitor&& v )const { v(val); }
      void value_holder_impl<array>::visit( visitor&& v )            { v(val); }
      void value_holder_impl<array>::push_back( value&& v )          { val.fields.push_back( std::move(v) ); }


      void value_holder_impl<object>::visit( const_visitor&& v )const { v(val); }
      void value_holder_impl<object>::visit( visitor&& v )            { v(val); }
      value_holder* value_holder_impl<object>::move_helper( char* c ) { return new(c) value_holder_impl( std::move(val) ); }
      value_holder* value_holder_impl<object>::copy_helper( char* c )const { return new(c) value_holder_impl(val);              }
      void value_holder_impl<object>::reserve( size_t s )             { val.fields.reserve(s); }

      void value_holder_impl<object>::clear()                         { val = object(); }
      size_t value_holder_impl<object>::size()const                   { return val.fields.size();  }
  } // namespace detail

static detail::value_holder* gh( char* h ) {
  return (detail::value_holder*)h;
}
static const detail::value_holder* gh( const char* h ) {
  return (const detail::value_holder*)h;
}
  
value::value() {
  new (holder) detail::value_holder(); 
}
value::value( value&& m ) {
  gh(m.holder)->move_helper(holder);
}
value::value( const value& m ){
  gh(m.holder)->copy_helper(holder);
}
value::value( char* c ) {
   new (holder) detail::value_holder_impl<std::string>( c );
}
value::value( value& m ){
  gh(m.holder)->copy_helper(holder);
}
value::~value() {
  gh(holder)->~value_holder();
}
value::value( int8_t v){
  new (holder) detail::value_holder_impl<int8_t>(v);
}
value::value( int16_t v){
  new (holder) detail::value_holder_impl<int16_t>(v);
}
value::value( int32_t v){
  new (holder) detail::value_holder_impl<int32_t>(v);
}
value::value( int64_t v){
  new (holder) detail::value_holder_impl<int64_t>(v);
}
value::value( uint8_t v){
  new (holder) detail::value_holder_impl<uint8_t>(v);
}
value::value( uint16_t v){
  new (holder) detail::value_holder_impl<uint16_t>(v);
}
value::value( uint32_t v){
  new (holder) detail::value_holder_impl<uint32_t>(v);
}
value::value( uint64_t v){
  new (holder) detail::value_holder_impl<uint64_t>(v);
}
value::value( double v){
  new (holder) detail::value_holder_impl<double>(v);
}
value::value( float v){
  new (holder) detail::value_holder_impl<float>(v);
}
value::value( bool v){
  new (holder) detail::value_holder_impl<bool>(v);
}
value::value( std::string&& v){
  new (holder) detail::value_holder_impl<std::string>(std::move(v));
}
value::value( std::string& v){
  new (holder) detail::value_holder_impl<std::string>(v);
}
value::value( const std::string& v){
  new (holder) detail::value_holder_impl<std::string>(v);
}
value::value( object&& o ){
  new (holder) detail::value_holder_impl<object>(std::move(o));
}
value::value( array&& a ){
  new (holder) detail::value_holder_impl<array>(std::move(a));
}
value::value( const array& a ){
  new (holder) detail::value_holder_impl<array>(a);
}
value::value( array& a ){
  new (holder) detail::value_holder_impl<array>(a);
}
value::value( const object& a ){
  new (holder) detail::value_holder_impl<object>(a);
}
value::value( object& a ){
  new (holder) detail::value_holder_impl<object>(a);
}

value& value::operator=( value&& v ){
  char tmp[sizeof(holder)];
  gh(holder)->move_helper(tmp);
  gh(v.holder)->move_helper(holder);
  gh(tmp)->move_helper(v.holder);
  return *this;
}
value& value::operator=( const value& v ){
  if( this == &v ) return *this;
  gh(holder)->~value_holder();
  gh(v.holder)->copy_helper(holder);
  return *this;
}

object::const_iterator value::find( const char* key )const {
  if( strcmp(gh(holder)->type(), "object") == 0) {
    const detail::value_holder_impl<object>* o = static_cast<const detail::value_holder_impl<object>*>(gh(holder));
    for( auto i  = o->val.fields.begin();
              i != o->val.fields.end(); ++i ) {
      if( strcmp( i->key.c_str(), key ) == 0 )
        return i;
    }
    return o->val.fields.end();
  }
  throw std::bad_cast();
}
object::const_iterator value::begin()const {
  if( strcmp(gh(holder)->type(), "object") == 0 ) {
    const detail::value_holder_impl<object>* o = static_cast<const detail::value_holder_impl<object>*>(gh(holder));
    return o->val.fields.begin();
  }
  throw std::bad_cast();
}
object::const_iterator value::end()const {
  if( strcmp(gh(holder)->type(), "object" ) == 0 ) {
    const detail::value_holder_impl<object>* o = static_cast<const detail::value_holder_impl<object>*>(gh(holder));
    return o->val.fields.end();
  }
  throw std::bad_cast();
}
value&       value::operator[]( const char* key ) {
  if( strcmp(gh(holder)->type(), "object") == 0) {
    detail::value_holder_impl<object>* o = static_cast<detail::value_holder_impl<object>*>(gh(holder));
    for( auto i  = o->val.fields.begin();
              i != o->val.fields.end(); ++i ) {
      if( strcmp( i->key.c_str(), key ) == 0 )
        return i->val;
    }
    o->val.fields.push_back( key_val(key) );
    return o->val.fields.back().val;
  }
  throw std::bad_cast();
}
value&       value::operator[]( const std::string& key )      { return (*this)[key.c_str()]; }
const value& value::operator[]( const std::string& key )const { return (*this)[key.c_str()]; }
const value& value::operator[]( const char* key )const {
  auto i = find(key);
  if( i == end() ) throw std::out_of_range(key);
  return i->val;
}

void    value::clear() {
  gh(holder)->clear();  
}
size_t  value::size()const {
  return gh(holder)->size();  
}
void         value::resize( size_t s ) {
  gh(holder)->resize(s);  
}
void         value::reserve( size_t s ) {
  gh(holder)->reserve(s);  
}
void         value::push_back( value&& v ) {
  gh(holder)->push_back(std::move(v));  
}
value&       value::operator[]( int32_t idx ) {
  return gh(holder)->at(idx);  
}
const value& value::operator[]( int32_t idx )const {
  return gh(holder)->at(idx);  
}

const char* value::type()const { return gh(holder)->type(); }

    void  value::visit( const_visitor&& v )const {
      auto h = ((detail::value_holder*)&holder[0]);
      h->visit( std::move(v) );
    }

} } // namepace mace::rpc
