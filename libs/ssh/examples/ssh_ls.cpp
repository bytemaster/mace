#include <mace/ssh/client.hpp>

/**
 *  This program will connect to localhost, run 'ls' and print the
 *  result without 'blocking'.
 */
int main( int argc, char** argv ) {
  
  try {
      auto sshc = mace::ssh::client::create();
      sshc->connect( "dlarimer", "zap.local");//10.211.55.2" );
      auto ls   = sshc->exec( "ls" );

      // print out anything that is sent our way
   /*
      int r = 0;
      while( r >= 0 ) {
        char out[1024];
        int r = ls->read_some( out, sizeof(out) );
        std::cout.write( out, r );
      }
      */

      std::cout<<"\nresult: "<<ls->result()<<std::endl;
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
  }

  return 0;
}
