#include <mace/cmt/actor.hpp>
#include <mace/cmt/log/log.hpp>


class calculator {
  public:
    calculator():r(0){};
    int add( int v ) { slog( "%1%", v ); return r += v; }

  private:
    int r;
};

MACE_STUB( calculator, (add) )


void test( mace::cmt::actor<calculator> act ) {
  slog( "start act test" );
  slog( "+5] %1%", (int)act->add( 5 ));
  slog( "+6] %1%", (int)act->add( 6 ).wait() );
  slog( "+7] %1%", (int)act->add( 7 ).wait() );
}

int main( int argc, char** argv ) {

  mace::cmt::thread* at = mace::cmt::thread::create( "actor_thread" );
  mace::stub::ptr<calculator> ap( boost::make_shared<calculator>() );
  mace::cmt::actor<calculator> act( boost::make_shared<calculator>(), at );
  mace::cmt::actor<calculator> act2(ap, at );

  mace::cmt::thread* tt = mace::cmt::thread::create( "test_thread" );
  tt->async<void>( boost::bind(test, act) ).wait();
  tt->async<void>( boost::bind(test, act2) ).wait();
  at->async<void>( boost::bind(test, act2) ).wait();

  //test(act);
  test(act2);

  tt->quit();
  at->quit();

  return 0;
}
