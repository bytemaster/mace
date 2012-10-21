#include <sigar.h>
#include <mace/sigar.hpp>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <mace/cmt/thread.hpp>

namespace mace {
  struct sigar_d {
    sigar_d():last(-1){}
    sigar_t *sigar;
    sigar_cpu_list_t cpulist;
    int total_sys;
    int total_user;
    int total_total;
    double last;
  };
  sigar::sigar() {
    my = new sigar_d();
    sigar_open(&my->sigar);
    sigar_cpu_list_get(my->sigar, &my->cpulist);

    my->total_sys = 0;
    my->total_user = 0;
    my->total_total = 0;
    for (uint32_t i=0; i<my->cpulist.number; i++) {
      my->total_sys += my->cpulist.data[i].sys;
      my->total_user += my->cpulist.data[i].user;
      my->total_total += my->cpulist.data[i].total;
    }
    my->last = -1;
  }

  double sigar::percent_cpu_usage() {
    sigar_cpu_list_t cpulist;
    sigar_cpu_list_get(my->sigar, &cpulist);

    int64_t total_sys = 0;
    int64_t total_user = 0;
    int64_t total_total = 0;

    for (uint32_t i=0; i<cpulist.number; i++) {
      total_sys += cpulist.data[i].sys;
      total_user += cpulist.data[i].user;
      total_total += cpulist.data[i].total;
    }

    int64_t dtotal = total_total - my->total_total;
    int64_t duser = total_user - my->total_user;
    int64_t dsys = total_sys - my->total_sys;

    if( dtotal == 0 ) 
      return my->last;

    my->last  = double(duser+dsys) / dtotal;

    my->total_total = total_total;
    my->total_user  = total_user;
    my->total_sys   = total_sys;

    sigar_cpu_list_destroy(my->sigar, &my->cpulist);
    memcpy( &my->cpulist, &cpulist, sizeof(cpulist) ); 

    return my->last;
  }

  double sigar::load_average() {
    sigar_loadavg_t load;
    if( SIGAR_OK == sigar_loadavg_get( my->sigar, &load ) )
        return load.loadavg[0];
    return -1;
  }


  sigar::~sigar() {
    sigar_cpu_list_destroy(my->sigar, &my->cpulist);
    sigar_close(my->sigar);
    delete my;
  }


  sys_stat system_status( const std::string& dir ) {
    sigar_t *sigar;
    sigar_open(&sigar);
    sigar_cpu_list_t cpulist;
    sigar_cpu_list_get(sigar, &cpulist);

    sys_stat stat;
    stat.cpu.mhz = 0;
    stat.time = boost::chrono::system_clock::now().time_since_epoch().count();

    stat.cpu.count = cpulist.number;
    stat.cpu.percent = 0;
    stat.cpu.load = 0;
    for (uint32_t i=0; i<cpulist.number; i++) {
        sigar_cpu_t cpu = cpulist.data[i];
//	std::cerr<<"cpu user "<<cpu.user<<std::endl;
//	std::cerr<<"cpu sys "<<cpu.sys<<std::endl;
//	std::cerr<<"cpu total "<<cpu.total<<std::endl;
//	std::cerr<<"cpu per "<<double(cpu.user+cpu.sys)/cpu.total<<std::endl;
        stat.cpu.percent += (uint64_t(cpu.user)+cpu.sys)/double(cpu.total);
    }
    sigar_cpu_list_destroy(sigar, &cpulist);

    sigar_cpu_info_list_t cpui;
    sigar_cpu_info_list_get(sigar,&cpui);
    if( cpui.number )
    stat.cpu.mhz = cpui.data[0].mhz;
    sigar_cpu_info_list_destroy( sigar, &cpui );

    sigar_loadavg_t load;
    if( SIGAR_OK == sigar_loadavg_get( sigar, &load ) )
	    stat.cpu.load = load.loadavg[0];
 //   std::cerr<<"load: "<<load.loadavg[0]<< " "<<load.loadavg[1] << "  " <<load.loadavg[2]<<std::endl;

    sigar_mem_t mem;
    sigar_mem_get( sigar, &mem );
    stat.mem.size = mem.total;
    stat.mem.used = mem.actual_used;
    stat.mem.free = mem.actual_free;

    sigar_uptime_t up;
    sigar_uptime_get( sigar, &up );
    stat.uptime = up.uptime;

    sigar_file_system_usage_t fsu;
    sigar_file_system_usage_get(sigar, dir.c_str(), &fsu );
    stat.disk.size = fsu.total;
    stat.disk.used = fsu.used;
    stat.disk.avail = fsu.avail;

    sigar_disk_usage_t du;
    sigar_disk_usage_get(sigar, dir.c_str(), &du );
    stat.disk.bytes_written = du.write_bytes;
    stat.disk.bytes_read = du.read_bytes;

    sigar_net_info_t netinfo;
    sigar_net_info_get(sigar,&netinfo);
    stat.net.host_name = netinfo.host_name;
    stat.net.domain_name = netinfo.domain_name;
    stat.net.default_gateway = netinfo.default_gateway;
    stat.net.default_gateway_interface = netinfo.default_gateway_interface;
                       


    sigar_net_interface_list_t ifl;
    sigar_net_interface_list_get(sigar, &ifl);
    for( uint32_t i = 0; i < ifl.number; ++i ) {
        sigar_net_interface_stat_t ifstat;
        sigar_net_interface_stat_get(sigar, ifl.data[i], &ifstat );
        stat.net.interfaces[ifl.data[i]].tx_bytes = ifstat.tx_bytes;
        stat.net.interfaces[ifl.data[i]].rx_bytes = ifstat.rx_bytes;
        stat.net.interfaces[ifl.data[i]].speed = ifstat.speed;

        sigar_net_interface_config_t ifc;

        sigar_net_interface_config_get(sigar,ifl.data[i],&ifc);
        stat.net.interfaces[ifl.data[i]].type = ifc.type;
        stat.net.interfaces[ifl.data[i]].name = ifc.name;
        stat.net.interfaces[ifl.data[i]].address = boost::asio::ip::address_v4(htonl(ifc.address.addr.in)).to_string();

    }
    sigar_net_interface_list_destroy(sigar,&ifl);

    sigar_close(sigar);

    return stat;
  }

  disk_stat disk_status(const std::string& dir) {
    sigar_t *sigar;
    sigar_open(&sigar);
    sys_stat stat;

    sigar_file_system_usage_t fsu;
    sigar_file_system_usage_get(sigar, dir.c_str(), &fsu );
    stat.disk.size = fsu.total;
    stat.disk.used = fsu.used;
    stat.disk.avail = fsu.avail;

    sigar_disk_usage_t du;
    sigar_disk_usage_get(sigar, dir.c_str(), &du );
    stat.disk.bytes_written = du.write_bytes;
    stat.disk.bytes_read = du.read_bytes;

    sigar_close(sigar);
    return stat.disk;
  }
  cpu_stat  cpu_status() {
    static mace::sigar sig;
    sigar_t *sigar;
    sigar_open(&sigar);

    sigar_cpu_info_list_t cpui;

    sigar_cpu_list_t cpulist;
    sigar_cpu_list_get(sigar, &cpulist);

    sys_stat stat;
    stat.cpu.mhz = 0;
    stat.time = boost::chrono::system_clock::now().time_since_epoch().count();

    stat.cpu.count = cpulist.number;
    stat.cpu.percent = sig.percent_cpu_usage();
    sigar_cpu_info_list_get(sigar,&cpui);
    if( cpui.number )
    stat.cpu.mhz = cpui.data[0].mhz;
    sigar_cpu_info_list_destroy( sigar, &cpui );

    sigar_loadavg_t load;
    sigar_loadavg_get( sigar, &load );
    stat.cpu.load = load.loadavg[0];

    sigar_close(sigar);
    return stat.cpu;
  }
  mem_stat   mem_status() {
    sigar_t *sigar;
    sigar_open(&sigar);
    sys_stat stat;

    sigar_mem_t mem;
    sigar_mem_get( sigar, &mem );
    stat.mem.size = mem.total;
    stat.mem.used = mem.actual_used;
    stat.mem.free = mem.actual_free;

    sigar_close(sigar);
    return stat.mem;
  }
}
