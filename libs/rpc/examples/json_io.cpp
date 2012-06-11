#include <mace/reflect/reflect.hpp>
#include<string>
#include<vector>
#include<map>

#include <mace/cmt/log/log.hpp>
#include <mace/rpc/json/json_io.hpp>
#include <boost/fusion/include/make_vector.hpp>

struct test {
  double a;
  std::string b;
  std::vector<std::string> c;
};

MACE_REFLECT( test, (a)(b)(c) )

struct nest {
  std::map<std::string,test> dict;
  test sub;
};

MACE_REFLECT( nest, (dict)(sub) )




void print_key_val( char* name, char* val, char* val_end, void* self ) {
  slog( "'%1%' => '%2%'", name, val );
}

void print_value( char* s, char* e, void* self ) {
  int* n = (int*)self;
  (*n)++;
  slog( "%2%] '%1%'", std::string(s,e), (*n) );
}


int main( int argc, char** argv ) {
  nest n;
  n.dict["hello"].a=3.14;
  n.dict["hello"].b=std::string("world\n hex:") + char(3);
  n.dict["he\tllo"].c.push_back( "\"Hi\"" );
  n.dict["hello"].c.push_back( "There" );
  n.dict["hello"].c.push_back( "Again" );

  mace::rpc::default_filter f;
  mace::rpc::json::to_json( boost::fusion::make_vector(std::string("vec"), 5.89, n), std::cout, f );

  slog(".....");
  slog( "%1% from \"hello world\"", mace::rpc::json::from_json<std::string>("\"hello world\"") ); 
  slog(".....");
  slog( "%1% from hello", mace::rpc::json::from_json<std::string>("hello") ); 
  slog(".....");
  slog( "int32_t %1% from 5", mace::rpc::json::from_json<int>("5") ); 
  slog(".....");
  slog( "int32_t %1% from -5", mace::rpc::json::from_json<int>("-5") ); 
  slog(".....");
  slog( "uint32_t %1% from 5", mace::rpc::json::from_json<uint32_t>("5") ); 
  slog(".....");
  slog( "uint32_t %1% from -5", mace::rpc::json::from_json<uint32_t>("-5") ); 
  slog(".....");
  slog( "int32_t %1% from \"5\"", mace::rpc::json::from_json<int32_t>("\"5\"") ); 
  slog(".....");
  slog( "uint32_t %1% from \"-5\"", mace::rpc::json::from_json<uint32_t>("\"-5\"") ); 
  slog(".....");
  slog( "float %1% from -5.5", mace::rpc::json::from_json<float>("-5.5") ); 
  slog(".....");
  slog( "float %1% from -5.5", mace::rpc::json::from_json<float>("-5.5") ); 
  slog(".....");
  slog( "double %1% from \"-5.5\"", mace::rpc::json::from_json<double>("\"-5.5\"") ); 
  slog(".....");
  slog( "double %1% from \"-5.5\"", mace::rpc::json::from_json<double>("\"-5.5\"") ); 


  std::string l;
  while( std::getline( std::cin, l) ) {
    int n = 0;
    mace::rpc::json::error_collector ec;
    /*
    std::vector<char> c = mace::rpc::json::read_value(l.begin(),l.end());
    */
    std::vector<char> v(l.begin(),l.end());
    v.push_back('\0');
    if( v.size() )
       //read_key_val( &v.front(), &v.front()+v.size()-1, ec, print_key_val, &n );
       //read_values( &v.front(), &v.front()+v.size()-1, ec, print_value, &n );
       read_key_vals( &v.front(), &v.front()+v.size()-1, ec, print_key_val, &n );
    else
        std::cout<<"... empty...\n";

    
  }


  return 0;
}
