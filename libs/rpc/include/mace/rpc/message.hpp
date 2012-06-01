#ifndef _MACE_RPC_MESSAGE_HPP_
#define _MACE_RPC_MESSAGE_HPP_

namespace mace { namespace rpc {

 class call_msg {
   public:
     int                         id;
     std::string                 name;
     boost::reflect::value_cref  params;
 };
 class result_msg {
    int id;
    boost::reflect::value_cref  result;
 };
 class error_msg {
    int         id
    int         code;
    std::string message;
    std::string data;
 };

} }
