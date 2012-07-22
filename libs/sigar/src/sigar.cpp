#include <sigar.h>
#include <mace/sigar.hpp>
#include <boost/asio.hpp>
#include <boost/chrono.hpp>

namespace mace {
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
	std::cerr<<"cpu user "<<cpu.user<<std::endl;
	std::cerr<<"cpu sys "<<cpu.sys<<std::endl;
	std::cerr<<"cpu total "<<cpu.total<<std::endl;
	std::cerr<<"cpu per "<<double(cpu.user+cpu.sys)/cpu.total<<std::endl;
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
    std::cerr<<"load: "<<load.loadavg[0]<< " "<<load.loadavg[1] << "  " <<load.loadavg[2]<<std::endl;

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
    sigar_t *sigar;
    sigar_open(&sigar);


    sigar_cpu_list_t cpulist;
    sigar_cpu_list_get(sigar, &cpulist);

    sys_stat stat;
    stat.cpu.mhz = 0;
    stat.time = boost::chrono::system_clock::now().time_since_epoch().count();

    stat.cpu.count = cpulist.number;
    for (uint32_t i=0; i<cpulist.number; i++) {
        sigar_cpu_t cpu = cpulist.data[i];
        stat.cpu.percent += (uint64_t(cpu.user)+cpu.sys)/double(cpu.total);
    }
    sigar_cpu_info_list_t cpui;
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
