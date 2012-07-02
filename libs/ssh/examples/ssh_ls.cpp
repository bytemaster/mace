#include <mace/ssh/client.hpp>

/**
 *  This program will connect to localhost, run 'ls' and print the
 *  result without 'blocking'.
 */
int main( int argc, char** argv ) {
  
  try {
      auto sshc = mace::ssh::client::create();
      sshc->connect( "dlarimer", "zap.local");//10.211.55.2" );
      auto ls   = sshc->exec( "cat" );
      ls->in_stream()<<"Hello World\n";
      ls->in_stream().flush();
      ls->in_stream().close();

      // print out anything that is sent our way
   
      std::string str((std::istreambuf_iterator<char>(ls->out_stream())),
                    std::istreambuf_iterator<char>());
      std::cout<<"stdout: "<<str<<std::endl;
      std::cout<<"\nresult: "<<ls->result()<<std::endl;
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
  }

  return 0;
}
