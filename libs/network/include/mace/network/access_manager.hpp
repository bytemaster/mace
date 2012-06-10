#ifndef MACE_NETWORK_ACCESS_MANAGER_HPP_
#define MACE_NETWORK_ACCESS_MANAGER_HPP_

namespace mace { namespace network {

  class reply;
  class request;

  namespace detail {
    class access_manager;
  }

  /**
   *  @brief sends network requests and receives replies.
   *
   *  The access_manager maintains cache, cookies, proxies, and
   *  other 'global' configuration options. 
   *
   *  All requests should be initiated through an access_manager and
   *  one access_manager should be sufficient for the entire 
   *  application.
   *
   *  @code
   *  using namespace mace::network;
   *
   *  access_manager am;
   *  reply am( get("http://")
   *              .header("name","value")
   *              .header(...)
   *              .content(std::cin) );
   *
   *  @endcode
   *
   *  The access manager will open a configurable number of
   *  parallel connections to each host and multi-plex requests
   *  among those connections.
   */
  class access_manager : boost::noncopyable {
    public:
      access_manager( access_manager&& m );
      ~access_manager();

      reply operator()( request&& r );

    private:
      detail::access_manager* my;
  };


} }

#endif
