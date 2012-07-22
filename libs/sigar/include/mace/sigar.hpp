#ifndef _MACE_SIGAR_HPP_
#define _MACE_SIGAR_HPP_
#include <stdint.h>
#include <string>
#include <map>

namespace mace {
  struct disk_stat {
    uint64_t size;
    uint64_t used;
    uint64_t avail;
    uint64_t bytes_written;
    uint64_t bytes_read;
  };

  struct cpu_stat {
    uint64_t  mhz;
    double    percent;
    float     load;
    uint16_t  count;
  };

  struct net_if {
    std::string name;
    std::string type;
    std::string address;
    uint64_t tx_bytes;
    uint64_t rx_bytes;
    uint64_t speed;
  };

  struct net_stat {
    std::string host_name;
    std::string domain_name;
    std::string default_gateway;
    std::string default_gateway_interface;
    std::map<std::string,net_if> interfaces;
  };

  struct mem_stat {
    uint64_t size;
    uint64_t used;
    uint64_t free;
  };

  struct sys_stat {
    uint64_t  time;
    double    uptime;
    cpu_stat  cpu;
    net_stat  net;
    mem_stat  mem;
    disk_stat disk;
  };

  disk_stat disk_status(const std::string& dir = "/");
  cpu_stat  cpu_status();
  mem_stat  mem_status();
  sys_stat  system_status(const std::string& dir = "/");

  /**
   *  @brief used to calculate cpu usage over time.
   *
   *  There is no such thing as an instantanious CPU
   *  usage percent.  Each time you call percent_cpu_usage()
   *  it will return average since the last time it was 
   *  called. The first time it is called takes longer
   *  to run.
   */
  class sigar {
    public:
      sigar();
      ~sigar();
      double percent_cpu_usage();
      double load_average();
    private:
      struct sigar_d* my;
  };
}

#include <mace/reflect/reflect.hpp>
MACE_REFLECT( mace::disk_stat, (size)(used)(avail)(bytes_written)(bytes_read) );
MACE_REFLECT( mace::cpu_stat, (count)(mhz)(percent)(load) );
MACE_REFLECT( mace::net_stat, (host_name)(domain_name)(default_gateway)(default_gateway_interface)(interfaces) );
MACE_REFLECT( mace::mem_stat, (size)(used)(free) );
MACE_REFLECT( mace::sys_stat, (time)(uptime)(cpu)(net)(mem)(disk) )
MACE_REFLECT( mace::net_if,   (name)(type)(address)(tx_bytes)(rx_bytes)(speed)  )

#endif // _MACE_SIGAR_HPP_
