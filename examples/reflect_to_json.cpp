#include <mace/reflect/reflect.hpp>
#include <iostream>

template<typename T>
void to_json( const T&,           std::ostream& os );
//! [Define base cases]
template<typename T>
void to_json( const std::vector<T>& v, std::ostream& os );
void to_json( const std::string& s,    std::ostream& os ) { os << '"'<<s<<'"';             }
void to_json( const int& i,            std::ostream& os ) { os << i;                       }
void to_json( const double& d,         std::ostream& os ) { os << d;                       }
void to_json( const float& f,          std::ostream& os ) { os << f;                       }
void to_json( const bool& b,           std::ostream& os ) { os << (b ?  "true" : "false"); }
//! [Define base cases]

//! [Define a visitor]
template<typename T>
struct to_json_visitor {
    to_json_visitor( const T& v, std::ostream& _os ):val(v),os(_os),i(0){}

    template<typename MemberPtr, MemberPtr m>
    void operator()( const char* name )const {
      if( i == 0 ) os << "{";    
      os<<"\""<<name<<"\":";
      to_json( val.*m, os);
      if( i != mace::reflect::reflector<T>::total_member_count-1 ) os << ",";
      if( i == mace::reflect::reflector<T>::total_member_count-1 ) os << "}";
      ++i;
    }
    mutable int i;
    const T& val;
    std::ostream& os;
};
//! [Define a visitor]

template<typename T>
void to_json( const std::vector<T>& v, std::ostream& os ) {
   os<<"[";
   for( uint32_t i = 0; i < v.size(); ++i ) {
     to_json( v[i], os );
     if( i != v.size() -1 ) os <<",";
   }
   os<<"]";
}

//! [Use a visitor]
template<typename T>
void to_json( const T& v, std::ostream& os ) {
    mace::reflect::reflector<T>::visit( to_json_visitor<T>( v, os ) );
}
//! [Use a visitor]

//! [Reflect your types]
struct contact {
    contact( const std::string& f, const std::string& l, int z )
    :first_name(f),last_name(l),zip(z){}

    std::string first_name;
    std::string last_name;
    int         zip;
};

struct address_book {
    std::string name;
    bool        is_locked;
    std::vector<contact> contacts;
};

MACE_REFLECT( contact, (first_name)(last_name)(zip) )
MACE_REFLECT( address_book, (name)(is_locked)(contacts) )
//! [Reflect your types]


//! [To JSON Example]
int main( int argc, char** argv ) {
    address_book ab;  
    ab.name      = "My Address Book";
    ab.is_locked = false;
    ab.contacts.push_back( contact( "Steve", "Jobs", 90210 ) );
    ab.contacts.push_back( contact( "Bill", "Gates", 10000 ) );
    to_json( ab, std::cout );
    return 0;
}
//! [To JSON Example]

