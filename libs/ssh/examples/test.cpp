#include <iostream>
#include <fstream>
int main( int arc, char** argv ) {
  setvbuf( stdout, NULL, _IONBF, 0 );
  for( int i = 0; i < 1; ++i ) {
      std::cerr<<i<<"cerr ------------------------------------------------------------------writing....\n";
      std::cout<<i<<"cout -----------------------------------------------------------------------------------writing....\n";
      std::cerr.flush();
      std::cout.flush();
      fflush(stdout);
  }
  std::cerr.flush();
  std::string in;

  
  std::ofstream o("out.txt");
  while( std::cin )
  {
    std::cin >> in;
    o<<in;
  }
  

  return 0;
}
