#include <mace/rpc/value.hpp>
#include <string.h>

namespace mace { namespace rpc {

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
value::value( value& m ){
  gh(m.holder)->copy_helper(holder);
}
value::~value() {
  gh(holder)->~value_holder();
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
value&       value::operator[]( uint32_t idx ) {
  return gh(holder)->at(idx);  
}
const value& value::operator[]( uint32_t idx )const {
  return gh(holder)->at(idx);  
}

const char* value::type()const { return gh(holder)->type(); }

} } // namepace mace::rpc
