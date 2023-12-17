/*
 * cib.cpp
 *
 *  Created on: Apr 13, 2023
 *      Author: nbarros
 */

#include "/home/nbarros/work/dune/cib/sw/slow_control/quasar/opcua-server/CIBproto/include/cib.hh"

CIB_ZMQ::CIB_ZMQ()
    : context(1)
    , socket(context, ZMQ_REQ)
{

}

CIB_ZMQ::~CIB_ZMQ()
{

}

void CIB_ZMQ::connect(const std::string &zmq_endpoint)
{
    socket.connect(zmq_endpoint);
    poller.socket = socket;
    poller.events = ZMQ_POLLIN;
}

