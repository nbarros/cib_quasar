
/*  © Copyright CERN, 2015. All rights not expressly granted are reserved.

    The stub of this file was generated by quasar (https://github.com/quasar-team/quasar/)

    Quasar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public Licence as published by
    the Free Software Foundation, either version 3 of the Licence.
    Quasar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public Licence for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Quasar.  If not, see <http://www.gnu.org/licenses/>.


 */


#include <Configuration.hxx> // TODO; should go away, is already in Base class for ages

#include <DIoLAttenuator.h>
#include <ASIoLAttenuator.h>

#include <Attenuator.hh>
#include <json.hpp>

#include <utilities.hh>
#include <sstream>
#include <vector>

using std::ostringstream;
using std::string;
using std::vector;

using json = nlohmann::json;

#define log_msg(s,met,msg) "[" << s << "]::" << met << " : " << msg

#define log_e(m,s) log_msg("ERROR",m,s)
#define log_w(m,s) log_msg("WARN",m,s)
#define log_i(m,s) log_msg("INFO",m,s)

namespace Device
{
// 1111111111111111111111111111111111111111111111111111111111111111111111111
// 1     GENERATED CODE STARTS HERE AND FINISHES AT SECTION 2              1
// 1     Users don't modify this code!!!!                                  1
// 1     If you modify this code you may start a fire or a flood somewhere,1
// 1     and some human being may possible cease to exist. You don't want  1
// 1     to be charged with that!                                          1
// 1111111111111111111111111111111111111111111111111111111111111111111111111






// 2222222222222222222222222222222222222222222222222222222222222222222222222
// 2     SEMI CUSTOM CODE STARTS HERE AND FINISHES AT SECTION 3            2
// 2     (code for which only stubs were generated automatically)          2
// 2     You should add the implementation but dont alter the headers      2
// 2     (apart from constructor, in which you should complete initializati2
// 2     on list)                                                          2
// 2222222222222222222222222222222222222222222222222222222222222222222222222

/* sample ctr */
DIoLAttenuator::DIoLAttenuator (
    const Configuration::IoLAttenuator& config,
    Parent_DIoLAttenuator* parent
):
    Base_DIoLAttenuator( config, parent)

    /* fill up constructor initialization list here */
    // perhaps it would make sense to set up sensible defaults here
    ,m_is_ready(false)
    ,m_att(nullptr)
    ,m_comport("auto")
    ,m_baud_rate(38400)
    ,m_offset(0)
    ,m_max_speed(0)
    ,m_motor_state(0)
    ,m_acceleration(0)
    ,m_deceleration(0)
    ,m_resolution_setting(1)
    ,m_position(0)
    ,m_current_idle(10)
    ,m_current_moving(10)
    ,m_transmission(0.0)
    ,m_status(sOffline)
{
    /* fill up constructor body here */
  m_name = config.name();
  m_sn = serial_number();
  // check the port
  // at this stage do nothing else
}

/* sample dtr */
DIoLAttenuator::~DIoLAttenuator ()
{
  if (m_att) delete m_att;
}

/* delegates for cachevariables */



/* delegators for methods */
UaStatus DIoLAttenuator::callConfigure_attenuator (
    const UaString&  json_config,
    UaString& response
)
{
  ostringstream msg("");
  bool got_exception = false;
  json resp;
  UaStatus ret;
  LOG(Log::INF) << log_i("set_config","Configuring device");
  try
  {
    json conf = json::parse(json_config.toUtf8());
    ret = config(conf,resp);
  }
  catch(json::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_config","Got JSON exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_config","Got STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_config","Got Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    LOG(Log::ERR) << msg.str();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else if (ret != OpcUa_Good)
  {
    resp["status"] = "ERROR";
    resp["messages"].push_back("Failed to process configuration");
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("set_config","Configuration done!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callSet_transmission (
    OpcUa_Double transmission,
    UaString& response
)
{
  LOG(Log::INF) << log_i("set_transmission","Setting transmission to ") << transmission;
  json resp;
  ostringstream msg("");
  UaStatus ret = set_transmission(transmission,resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_transmission","Operation failed!");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("set_config","Operation successful!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callSet_conn_details (
    const UaString&  port,
    OpcUa_UInt16 baud_rate,
    UaString& response
)
{
  LOG(Log::INF) << log_i("set_connection","Setting connection to [") << port.toUtf8() << "::" << baud_rate << "]";
  json resp;
  ostringstream msg("");
  std::string pPort = port.toUtf8();
  UaStatus ret = set_connection(pPort, baud_rate,resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_connection","Operation failed!");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("set_connection","Operation successful!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callStop (
    UaString& response
)
{
  LOG(Log::INF) << log_i("stop","Stopping any operation");
  json resp;
  ostringstream msg("");
  UaStatus ret = stop(resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e("stop","Operation failed!");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("stop","Operation successful!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callSet_resolution (
    OpcUa_UInt16 resolution_setting,
    UaString& response
)
{
  LOG(Log::INF) << log_i("set_resolution","Setting resolution setting to [") << resolution_setting << "]";
  json resp;
  ostringstream msg("");
  UaStatus ret = set_resolution(resolution_setting,resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_resolution","Operation failed!");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("set_resolution","Operation successful!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callSet_current (
    OpcUa_UInt16 idle_setting,
    OpcUa_UInt16 moving_setting,
    UaString& response
)
{
  LOG(Log::INF) << log_i("set_current","Setting [idle,move] current settings to [") << idle_setting << ";" << moving_setting << "]";
  json resp;
  ostringstream msg("");
  UaStatus ret = set_current_idle(idle_setting,resp);
  ret = ret | set_current_moving(moving_setting,resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_current","Operation failed!");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("set_current","Operation successful!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callSet_acceleration (
    OpcUa_UInt16 acceleration,
    UaString& response
)
{
  LOG(Log::INF) << log_i("set_acceleration","Setting acceleration to ") << acceleration;
  json resp;
  ostringstream msg("");
  UaStatus ret = set_acceleration(acceleration,resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_acceleration","Operation failed!");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("set_acceleration","Operation successful!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callSet_deceleration (
    OpcUa_UInt16 deceleration,
    UaString& response
)
{
  LOG(Log::INF) << log_i("set_deceleration","Setting deceleration to ") << deceleration;
  json resp;
  ostringstream msg("");
  UaStatus ret = set_deceleration(deceleration,resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_deceleration","Operation failed!");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("set_deceleration","Operation successful!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callSet_max_speed (
    OpcUa_UInt16 max_speed,
    UaString& response
)
{
  LOG(Log::INF) << log_i("set_speed","Setting speed to ") << max_speed;
  json resp;
  ostringstream msg("");
  UaStatus ret = set_max_speed(max_speed,resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e("set_speed","Operation failed!");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Bad;
  }
  else
  {
    msg.clear(); msg.str("");
    msg << log_i("set_speed","Operation successful!");
    resp["status"] = "OK";
    resp["messages"].push_back(msg.str());
    resp["statuscode"] = OpcUa_Good;
  }
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLAttenuator::callGet_status (
    UaString& response
)
{
  // should this just return the motor state (we can get when querying the position)
  // or should this actually query all the settings?
  return OpcUa_BadNotImplemented;
}

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DIoLAttenuator::automatic_port_search()
{
 try
 {
  // the automatic port search queries all available ports, their descriptions and IDs for an occurrence of the device
  // serial number
  // this has been shown to work for most cases, but there is always a chance that something fails
  // for example, the wrong serial number was provided
  m_comport = util::find_port(m_sn);
  if (m_comport.size() == 0)
  {
    LOG(Log::ERR) << "DIoLPowerMeter::automatic_port_search : Couldn't find device port";
  }

  m_status = sOffline;
 }
 catch(...)
 {
   m_status = sOffline;
   LOG(Log::ERR) << "DIoLPowerMeter::automatic_port_search : Caught an exception searching for the port";
   m_comport = "";
 }
}

void DIoLAttenuator::refresh_status(json &resp)
{
  // -- this method connects to the device and loads all registers from there,
  // setting the local cache
  // and the address space equivalent
  ostringstream msg("");
  bool got_exception = false;
  if (m_status != sReady)
  {
    msg.clear();msg.str("");
    msg << log_e("refresh","device is not initialized");
    resp["messages"].push_back(msg.str());
    return;
  }
  //
  try
  {
    m_att->refresh_status();
    // now fetch all local variables
    m_att->get_offset(m_offset);
    getAddressSpaceLink()->setOffset(m_offset, OpcUa_Good);
    m_att->get_max_speed(m_max_speed);
    getAddressSpaceLink()->setMax_speed(m_max_speed, OpcUa_Good);
    m_att->get_acceleration(m_acceleration);
    getAddressSpaceLink()->setAcceleration(m_acceleration, OpcUa_Good);
    m_att->get_deceleration(m_deceleration);
    getAddressSpaceLink()->setDeceleration(m_deceleration, OpcUa_Good);
    m_att->get_resolution(m_resolution_setting);
    getAddressSpaceLink()->setResolution_setting(m_resolution_setting, OpcUa_Good);
    // refresh also the resolution setting map
    std::string p_rmap_str = util::serialize_map(m_resolution_states);
    getAddressSpaceLink()->setResolution_options(UaString(p_rmap_str.c_str()),p_rmap_str.size(),OpcUa_Good);
    m_att->get_current_idle(m_current_idle);
    getAddressSpaceLink()->setIdle_current_setting(m_current_idle, OpcUa_Good);
    m_att->get_current_move(m_current_moving);
    getAddressSpaceLink()->setMoving_current_setting(m_current_moving, OpcUa_Good);
    //
    // this one actually queries the device again
    m_att->get_position(m_position, m_motor_state, false);
    getAddressSpaceLink()->setPosition(m_position, OpcUa_Good);
    getAddressSpaceLink()->setMotor_state(m_motor_state, OpcUa_Good);
    std::string p_smap_str = util::serialize_map(m_motor_states);
    getAddressSpaceLink()->setMotor_state_options(UaString(p_smap_str.c_str()),p_smap_str.size(),OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("init","Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("init","Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("init","STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e("init","Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
  }
}

void DIoLAttenuator::refresh_position()
{
  if (!m_att)
  {
    // set the current address space values to invalid
    getAddressSpaceLink()->setPosition(m_position, OpcUa_BadDataUnavailable);
    getAddressSpaceLink()->setMotor_state(m_motor_state, OpcUa_BadDataUnavailable);
  }
  else {
    m_att->get_position(m_position, m_motor_state, false);
    getAddressSpaceLink()->setPosition(m_position, OpcUa_Good);
    getAddressSpaceLink()->setMotor_state(m_motor_state, OpcUa_Good);
  }
}


// init device just establishes the connection
// everything else is done elsewhere
UaStatus DIoLAttenuator::init_device(json &resp)
{
  UaStatus ret = OpcUa_Good;
  ostringstream msg("");
  bool got_exception = false;
  // if status is not init, then try to establish the connection
  if (!m_att)
  {
    // check that both port and baud rate are set
    if (m_comport == "auto")
    {
      msg.clear(); msg.str("");
      msg << log_w("init","port set to automatic. Searching for port using serial number");
      resp.push_back(msg.str());
      automatic_port_search();
    }
    // at this stage, it either has a good value or not
    try
    {
      m_att = new device::Attenuator(m_comport.c_str(),m_baud_rate);
      m_status = sReady;
      refresh_position();
      //NOTE: Do we need to refresh the status
      // this is a rather philosophical question:should the local cache be loaded
      // from the device, or from the configuration (and then passed to the device)?
      //refresh_status(resp);
    }
    catch(serial::PortNotOpenedException &e)
    {
      msg.clear(); msg.str("");
      msg << log_e("init","Port not open : ") << e.what();
      got_exception = true;
    }
    catch(serial::SerialException &e)
    {
      msg.clear(); msg.str("");
      msg << log_e("init","Serial exception :") << e.what();
      got_exception = true;
    }
    catch(std::exception &e)
    {
      msg.clear(); msg.str("");
      msg << log_e("init","STL exception :") << e.what();
      got_exception = true;
    }
    catch(...)
    {
      msg.clear(); msg.str("");
      msg << log_e("init","Unknown exception");
      got_exception = true;
    }
    if (got_exception)
    {
      resp["messages"].push_back(msg.str());
      ret = OpcUa_Bad;
    }
  } else
  {
    // a connection already exists
    //FIXME: Should a new connection be made, or just send out a warning?
    msg.clear(); msg.str("");
    msg << log_w("init","Device already initialized. Doing nothing.");
    resp["messages"].push_back(msg.str());
    ret = OpcUa_Good;
  }
  return ret;
}

UaStatus DIoLAttenuator::config(json config, json &resp)
{
  // -- should this be wrapped in exception catch or not?
  // at this stage we can accept not having a connected device

  // first look up for "port" and "baud_rate"
  // since those are mandatory
  bool got_exception = true;
  ostringstream msg("");
  UaStatus ret = OpcUa_Good;
  // if a connection is already established, throw a warning, but reset the device
  if (m_att)
  {
    msg.clear(); msg.str("");
    msg << log_w("config","There is already a connected device. Closing and resetting connection.");
    resp["messages"].push_back(msg.str());
    delete m_att;
    m_status = sOffline;
  }
  try
  {
    // -- the very first check is to confirm that this configuration fragment
    // is indeed for this device (use name *and* serial number match)
    std::string name, sn;
    name = config.at("name").get<std::string>();
    sn = config.at("serial_number").get<std::string>();
    if (!((name == m_name) && (sn == m_sn)))
    {
      msg.clear(); msg.str("");
      msg << log_e("config","Device mismatch (name,sn) = (") << name << "," << sn
          << ") expected (" << m_name << "," << m_sn << ")";
      resp["messages"].push_back(msg.str());
      return OpcUa_BadInvalidArgument;
    }

    std::string comport = config.at("port").get<std::string>();
    uint32_t baud_rate = config.at("baud_rate").get<std::uint32_t>();

    // now that we have this, attempt to init the device
    UaStatus s = set_connection(comport,baud_rate,resp);
    if (s != OpcUa_Good)
    {
      msg.clear(); msg.str("");
      msg << log_e("config","Failed to initialize device. Correct your configuration.");
      resp["messages"].push_back(msg.str());
      m_status = sOffline;
      return OpcUa_BadInvalidArgument;
    }
    m_status = sReady;
    // everything good so far... loop over the whole configuration

    for (json::iterator it = config.begin(); it != config.end(); ++it)
    {
      LOG(Log::INF) << "Processing " << it.key() << " : " << it.value() << "\n";
      if (it.key() == "port")
      {
        // already processed
        continue;
      }
      if (it.key() == "baud_rate")
      {
        // already processed
        continue;
      }
      if (it.key() == "acceleration")
      {
        uint16_t v = it.value();
        ret = ret | set_acceleration(v,resp);
      }
      if (it.key() == "deceleration")
      {
        uint16_t v = it.value();
        ret = ret | set_deceleration(v,resp);
      }
      if (it.key() == "max_speed")
      {
        uint32_t v = it.value();
        ret = ret | set_max_speed(v,resp);
      }
      if (it.key() == "current_idle")
      {
        uint16_t v = it.value();
        ret = ret | set_current_idle(v,resp);
      }
      if (it.key() == "current_moving")
      {
        uint16_t v = it.value();
        ret = ret | set_current_moving(v,resp);
      }
      if (it.key() == "resolution")
      {
        uint16_t v = it.value();
        ret = ret | set_resolution(v,resp);
      }
    }
    // if at this point ret is not good, something failed to set
    // however, this in itself is not necessarily a problem, as the deaults still hold
    // so keep it as a warning
    if (ret != OpcUa_Good)
    {
      msg.clear(); msg.str("");
      msg << log_w("config","Failed setting the whole configuration.Check previous messages.");
      resp["messages"].push_back(msg.str());
      return OpcUa_Bad;
    }
  }
 catch(json::exception &e)
 {
   msg.clear(); msg.str("");
   msg << log_e("config","Got JSON exception :") << e.what();
   got_exception = true;
 }
 catch(std::exception &e)
 {
   msg.clear(); msg.str("");
   msg << log_e("set_config","Got STL exception :") << e.what();
   got_exception = true;
 }
 catch(...)
 {
   msg.clear(); msg.str("");
   msg << log_e("set_config","Got Unknown exception");
   got_exception = true;
 }
 if (got_exception)
 {
   LOG(Log::ERR) << msg.str();
   resp["status"] = "ERROR";
   resp["messages"].push_back(msg.str());
   resp["statuscode"] = OpcUa_Bad;
   ret = OpcUa_Bad;
 }
 return ret;
}

UaStatus DIoLAttenuator::set_transmission(double t, json &resp)
{
  // we need to catch the exceptions here.
  // there is no guarantee that upper level calling methods are
  // performing exception handling
  ostringstream msg("");
  bool got_exception = false;
  const char* label = "transmission";
  if (t < 0 || t  > 1.0)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e("transmission","Value out of range :") << t << " <> [0,1]";
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidArgument;
  }
  if (!m_att)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e("transmission","Device is not connected yet.");
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidState;
  }
  // all should be good now
  try
  {
    // wait for the value to be set?
    m_att->set_transmission(t, true);
    m_transmission = t;
    getAddressSpaceLink()->setTransmission(m_transmission,OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
    getAddressSpaceLink()->setTransmission(m_transmission,OpcUa_Bad);
    return OpcUa_Bad;
  }
  return OpcUa_Good;
}

UaStatus DIoLAttenuator::set_connection(const std::string port, const uint32_t baud_rate, json &resp)
{
  // we need to catch the exceptions here.
  // there is no guarantee that upper level calling methods are
  // performing exception handling
  ostringstream msg("");
  //bool got_exception = false;
  const char* label = "connection";

  // if there is already a connection, set a warning and reset
  UaStatus ret = OpcUa_Good;
  // if a connection is already established, throw a warning, but reset the device
  if (m_att)
  {
    msg.clear(); msg.str("");
    msg << log_w(label,"There is already a connected device. Closing and resetting connection.");
    resp["messages"].push_back(msg.str());
    delete m_att;
    m_status = sOffline;
  }

  m_comport = port;
  m_baud_rate = baud_rate;
  ret = init_device(resp);
  if (ret != OpcUa_Good)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Failed to initialize device. Check previous messages.");
    resp["messages"].push_back(msg.str());
    m_status = sOffline;
    return OpcUa_BadInvalidArgument;
  }

  return OpcUa_Good;
}

UaStatus DIoLAttenuator::stop(json &resp)
{
  const char* label = "transmission";
  ostringstream msg("");
  bool got_exception = false;
  if (!m_att)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e(label,"Device is not connected yet.");
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidState;
  }
  // all should be good now
  try
  {
    // wait for the value to be set?
    m_att->stop(false);
    m_att->get_position(m_position, m_motor_state, false);
    getAddressSpaceLink()->setPosition(m_position, OpcUa_Good);
    getAddressSpaceLink()->setMotor_state(m_motor_state, OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
    getAddressSpaceLink()->setPosition(m_position, OpcUa_Bad);
    getAddressSpaceLink()->setMotor_state(m_motor_state, OpcUa_Bad);
    return OpcUa_Bad;
  }
  return OpcUa_Good;}

UaStatus DIoLAttenuator::set_resolution(const uint16_t v,json &resp)
{
  const char* label = "resolution";
  ostringstream msg("");
  bool got_exception = false;
  if (!m_att)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e(label,"Device is not connected yet.");
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidState;
  }
  // check that the setting is within the map
  if (m_resolution_states.count(v) == 0)
  {
    // invalid argument
     msg.clear(); msg.str("");
     msg << log_e(label,"Invalid argument. Check valid states.");
     resp["messages"].push_back(msg.str());
     return OpcUa_BadInvalidArgument;
  }
  // all should be good now
  try
  {
    m_att->set_resolution(v);
    m_resolution_setting = v;
    getAddressSpaceLink()->setResolution_setting(m_resolution_setting, OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
    getAddressSpaceLink()->setResolution_setting(m_resolution_setting, OpcUa_Bad);
    return OpcUa_Bad;
  }
  return OpcUa_Good;
}

UaStatus DIoLAttenuator::set_current_idle(const uint16_t v,json &resp)
{
  const char* label = "cur_idle";
  ostringstream msg("");
  bool got_exception = false;
  if (!m_att)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e(label,"Device is not connected yet.");
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidState;
  }
  // check the range
  if (v > 0xFF)
  {
    // invalid argument
     msg.clear(); msg.str("");
     msg << log_e(label,"Invalid argument. Valid range ]0,255].");
     resp["messages"].push_back(msg.str());
     return OpcUa_BadInvalidArgument;
  }
  // all should be good now
  try
  {
    m_att->set_idle_current(v);
    m_current_idle = v;
    getAddressSpaceLink()->setIdle_current_setting(m_current_idle, OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
    getAddressSpaceLink()->setIdle_current_setting(m_current_idle, OpcUa_Bad);
    return OpcUa_Bad;
  }
  return OpcUa_Good;
}

UaStatus DIoLAttenuator::set_current_moving(const uint16_t v,json &resp)
{
  const char* label = "cur_move";
  ostringstream msg("");
  bool got_exception = false;
  if (!m_att)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e(label,"Device is not connected yet.");
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidState;
  }
  // check the range
  if (v > 0xFF)
  {
    // invalid argument
     msg.clear(); msg.str("");
     msg << log_e(label,"Invalid argument. Valid range ]0,255].");
     resp["messages"].push_back(msg.str());
     return OpcUa_BadInvalidArgument;
  }
  // all should be good now
  try
  {
    m_att->set_moving_current(v);
    m_current_moving = v;
    getAddressSpaceLink()->setMoving_current_setting(m_current_moving, OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
    getAddressSpaceLink()->setMoving_current_setting(m_current_moving, OpcUa_Bad);
    return OpcUa_Bad;
  }
  return OpcUa_Good;
}


UaStatus DIoLAttenuator::set_acceleration(const uint16_t v,json &resp)
{
  const char* label = "acceleration";
  ostringstream msg("");
  bool got_exception = false;
  if (!m_att)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e(label,"Device is not connected yet.");
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidState;
  }
  // check the range
  if (v > 0xFF)
  {
    // invalid argument
     msg.clear(); msg.str("");
     msg << log_e(label,"Invalid argument. Valid range ]0,255].");
     resp["messages"].push_back(msg.str());
     return OpcUa_BadInvalidArgument;
  }
  // all should be good now
  try
  {
    m_att->set_acceleration(v);
    m_acceleration = v;
    getAddressSpaceLink()->setAcceleration(m_acceleration, OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
    getAddressSpaceLink()->setAcceleration(m_acceleration, OpcUa_Bad);
    return OpcUa_Bad;
  }
  return OpcUa_Good;
}

UaStatus DIoLAttenuator::set_deceleration(const uint16_t v,json &resp)
{
  const char* label = "deceleration";
  ostringstream msg("");
  bool got_exception = false;
  if (!m_att)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e(label,"Device is not connected yet.");
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidState;
  }
  // check the range
  if (v > 0xFF)
  {
    // invalid argument
     msg.clear(); msg.str("");
     msg << log_e(label,"Invalid argument. Valid range ]0,255].");
     resp["messages"].push_back(msg.str());
     return OpcUa_BadInvalidArgument;
  }
  // all should be good now
  try
  {
    m_att->set_deceleration(v);
    m_deceleration = v;
    getAddressSpaceLink()->setDeceleration(m_deceleration, OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
    getAddressSpaceLink()->setDeceleration(m_deceleration, OpcUa_Bad);
    return OpcUa_Bad;
  }
  return OpcUa_Good;
}

UaStatus DIoLAttenuator::set_max_speed(const uint32_t v,json &resp)
{
  const char* label = "speed";
  ostringstream msg("");
  bool got_exception = false;
  if (!m_att)
  {
    // bad range
    msg.clear(); msg.str("");
    msg << log_e(label,"Device is not connected yet.");
    resp["messages"].push_back(msg.str());
    return OpcUa_BadInvalidState;
  }
  // check the range
  // FIXME: Check what is the max range
//  if (v > 0xFF)
//  {
//    // invalid argument
//     msg.clear(); msg.str("");
//     msg << log_e(label,"Invalid argument. Valid range ]0,255].");
//     resp["messages"].push_back(msg.str());
//     return OpcUa_BadInvalidArgument;
//  }
  // all should be good now
  try
  {
    m_att->set_max_speed(v);
    m_max_speed = v;
    getAddressSpaceLink()->setMax_speed(m_max_speed, OpcUa_Good);
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Port not open : ") << e.what();
    got_exception = true;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Serial exception :") << e.what();
    got_exception = true;
  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"STL exception :") << e.what();
    got_exception = true;
  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e(label,"Unknown exception");
    got_exception = true;
  }
  if (got_exception)
  {
    resp["messages"].push_back(msg.str());
    getAddressSpaceLink()->setMax_speed(m_max_speed, OpcUa_Bad);
    return OpcUa_Bad;
  }
  return OpcUa_Good;
}

void DIoLAttenuator::update()
{
  refresh_position();
}

}
