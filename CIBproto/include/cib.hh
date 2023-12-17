/*
 * cib.hh
 *
 *  Created on: Apr 13, 2023
 *      Author: nbarros
 */

#ifndef CIBPROTO_INCLUDE_CIB_HH_
#define CIBPROTO_INCLUDE_CIB_HH_


#include "cib.pb.h"

#include <zmq.hpp>
#include <string>


class CIB_ZMQ
{
public:
  CIB_ZMQ ();
  virtual ~CIB_ZMQ ();

  void connect(const std::string &zmq_endpoint);

  template <class R, class C>
  bool send_command(const C &msg, R &repl, int timeout_ms = -1) {

      cib::Command command;
      command.mutable_cmd()->PackFrom(msg);

      std::string cmd_str;
      command.SerializeToString(&cmd_str);

      zmq::message_t request(cmd_str.size());
      memcpy((void*)request.data(), cmd_str.c_str(), cmd_str.size());
      socket.send(request,zmq::send_flags::none);

      if (zmq_poll(&poller, 1, timeout_ms) <= 0) {
          return false;
      }

      zmq::message_t reply;
      socket.recv(reply,zmq::recv_flags::none);

      std::string reply_str(static_cast<char*>(reply.data()), reply.size());
      repl.ParseFromString(reply_str);

      return true;
  }

protected:

  zmq::context_t context;
  zmq::socket_t socket;
  zmq::pollitem_t poller;

};

#endif /* CIBPROTO_INCLUDE_CIB_HH_ */

