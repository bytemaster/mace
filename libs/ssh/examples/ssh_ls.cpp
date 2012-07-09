#include <mace/ssh/client.hpp>
#include <mace/cmt/log/log.hpp>
#include <mace/cmt/thread.hpp>
#include <boost/bind.hpp>

bool progress( const std::string& file, size_t sent, size_t total ) {
  if( sent == total ) slog( "%3% sent %1% of %2%", sent, total, file ); 
  return true;
}

/**
 *  This program will connect to localhost, run 'ls' and print the
 *  result without 'blocking'.
 */
int main( int argc, char** argv ) {
  mace::cmt::thread::current().set_name("main");  
  try {
      mace::cmt::thread::current().debug("main");

      auto sshc = mace::ssh::client::create();
      sshc->connect( argv[1], argv[2], "localhost");

      auto ls   = sshc->exec( "/Users/dlarimer/projects/mace/libs/ssh/examples/test", "vt100");
      ls->in_stream()<<"ls\n";
      ls->in_stream().flush();
   //   ls->in_stream().close();

      //std::string str((std::istreambuf_iterator<char>(ls->out_stream())),
      //              std::istreambuf_iterator<char>());
      std::string t;
      while( ls->out_stream() ) {
      ls->out_stream() >> t;
      std::cout<<"stdout: "<<t<<std::endl;
      }
      std::cout<<"\nresult: "<<ls->result()<<std::endl;
      return 0;

      auto stat = sshc->stat("test_dir/subdir");
      wlog( "size: %1%", stat.size );
      //auto does_not_exist = sshc->stat("test_dir/subdir/sd");
      sshc->mkdir("test_dir/subdir");
      sshc->mkdir("test_dir/subdir");
      sshc->mkdir("test.a");


      // print out anything that is sent our way
   

      auto f = mace::cmt::async([=](){ return sshc->scp_send( "libmace_ssh.a", "test.a", boost::bind(progress,"test.a",_1,_2) );}, "test.a");
      auto f2 = mace::cmt::async([=](){ return sshc->scp_send( "libmace_ssh_debug.a", "test.b", boost::bind(progress,"test.b",_1,_2) );}, "test.b");
      slog( "wait for first" );
      f.wait();
      slog( "wait for second" );
      f2.wait();
      wlog( "------------------- DONE WITH TEST  CLEANUP AND DISCONNECT -------------------------\n" );
  } catch ( ... ) {
    elog( "%1%", boost::current_exception_diagnostic_information() );
  }

  return 0;
}
