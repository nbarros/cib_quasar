
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

#include <DIoLPowerMeter.h>
#include <ASIoLPowerMeter.h>

#include <PowerMeter.hh>
#include <utilities.hh>
#include <sstream>


#include <chrono>
#include <thread>
#include <functional>
#include <string>
#include <random>
#include <json.hpp>

using json = nlohmann::json;

#define log_msg(s,met,msg) "[" << s << "]::" << met << " : " << msg

#define log_e(m,s) log_msg("ERROR",m,s)
#define log_w(m,s) log_msg("WARN",m,s)
#define log_i(m,s) log_msg("INFO",m,s)

using std::ostringstream;

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
DIoLPowerMeter::DIoLPowerMeter (
    const Configuration::IoLPowerMeter& config,
    Parent_DIoLPowerMeter* parent
):
    Base_DIoLPowerMeter( config, parent)

    /* fill up constructor initialization list here */
    ,m_pm(nullptr)
    ,m_comport("")
    ,m_status(sOffline)
    ,m_measurement_mode(1)
    ,m_sel_range(2)
    ,m_wavelength(266)
    ,m_e_threshold(1)
    ,m_ave_setting(1)
    ,m_pulse_width(1)
    ,m_energy_reading(0.0)
{
    /* fill up constructor body here */
    // first probe the for the port that this meter is attached to
    m_threshold_limits.first = 0;
    m_threshold_limits.second = 0;

    m_comport = port();
    LOG(Log::INF) << "DIoLPowerMeter::DIoLPowerMeter : Port set to [" << m_comport << "]";
    if (m_comport == "auto")
    {
      LOG(Log::WRN) << "DIoLPowerMeter::DIoLPowerMeter : Attempting automatic port detection based off serial number.";
      automatic_port_search();
    }

    // -- initialize the status map
    m_status_map.insert({sOffline,"offline"});
    m_status_map.insert({sUnconfigured,"unconfigured"});
    m_status_map.insert({sReady,"ready"});

//    // having picked up the values that have been passed down from the XML configuration
//    // attempt to initialize the system
//    if (m_comport.size() == 0)
//    {
//      LOG(Log::WRN) << "DIoLPowerMeter::DIoLPowerMeter : Couldn't find device port. Device will remain unintialized.";
//      m_status=sOffline;
//    }
//    else
//    {
//      json m;
//      init_device(m);
//    }


}

/* sample dtr */
DIoLPowerMeter::~DIoLPowerMeter ()
{
  if (m_pm) delete m_pm;
}

/* delegates for cachevariables */



/* delegators for methods */
UaStatus DIoLPowerMeter::callInit (
    UaString& response
)
{
  json resp;
  UaStatus st = OpcUa_Good;
  st = init_device(resp);
  response = UaString(resp.dump().c_str());
    return st;
}
UaStatus DIoLPowerMeter::callSet_port (
    const UaString&  device_port
)
{
  // if the port is already set and the connection is
  // established, do nothing.
  if (m_pm) {
    return OpcUa_BadInvalidState;
  }
  m_comport = device_port.toUtf8();

  UaString dp(m_comport.c_str());
  getAddressSpaceLink()->setDevice_port(dp,m_comport.size(),OpcUa_Good);

  // Note: should we init the connection at this stage?

    return OpcUa_Good;
}

UaStatus DIoLPowerMeter::callSet_average (
    OpcUa_UInt16 target_value,
    UaString& response
)
{
    LOG(Log::INF) << "Setting average";
    json resp;
    std::ostringstream msg("");

    // we need the system to be at least in unconfigured state
    // as we need the device connection to be established
    if (m_status == sOffline)
    {
      msg.clear(); msg.str("");
      msg << log_e("set_ave","Device is not connected yet. Only the port can be set in this state.");
      resp["messages"].push_back(msg.str());
      resp["status"] = "ERROR";
      resp["status_code"] = OpcUa_BadInvalidState;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;
    }

    if (m_ave_windows.size() == 0)
    {
      refresh_average_ranges();
    }
    // check if the setting is in the map
    if (m_ave_windows.count(target_value) == 0)
    {
      msg.clear(); msg.str("");
      msg << log_e("set_ave","Invalid value. Check available options");
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_BadInvalidArgument;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;
    }
    if (target_value == m_ave_setting)
    {
      msg.clear(); msg.str("");
      msg << log_w("set_ave","Value is already current setting. ");
      resp["messages"].push_back(msg.str());
    }
    // change the current value
    try {
      m_pm->average_query(target_value, m_ave_setting);
      if (m_ave_setting != target_value)
      {
        msg.clear(); msg.str("");
        msg << log_e("set_ave","Failed to update average window.");
        resp["status"] = "ERROR";
        resp["messages"].push_back(msg.str());
        resp["status_code"] = OpcUa_BadInvalidArgument;
        response = UaString(resp.dump().c_str());
        return OpcUa_Good;

      }
    }
    catch(serial::SerialException &e)
    {
      msg.clear(); msg.str("");
      msg << log_e("set_ave","Failed with a Serial exception :") << e.what();
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;

    }
    catch(std::exception &e)
    {
      msg.clear(); msg.str("");
      msg << log_e("set_ave","Failed with an STL exception :") << e.what();
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;

    }
    catch(...)
    {
      msg.clear(); msg.str("");
      msg << log_e("set_ave","Failed with an unknown exception.");
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;

    }

    // all went good
    resp["status"] = "OK";
    msg.clear(); msg.str("");
    msg << log_e("set_ave","Average setting updated.");
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Good;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

}
UaStatus DIoLPowerMeter::callSet_range (
    OpcUa_Int16 target_value,
    UaString& response
)
{
  LOG(Log::INF) << "Setting range to " << target_value;
  json resp;
  std::ostringstream msg("");


  // we need the system to be at least in unconfigured state
  // as we need the device connection to be established
  if (m_status == sOffline)
  {
    msg.clear(); msg.str("");
    msg << log_e("range","Device is not connected yet. Only the port can be set in this state.");
    resp["messages"].push_back(msg.str());
    resp["status"] = "ERROR";
    resp["status_code"] = OpcUa_BadInvalidState;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }


  if (m_ranges.size() == 0)
  {
    refresh_measurement_ranges();
  }
  // check if the setting is in the map
  if (m_ranges.count(target_value) == 0)
  {
    msg.clear(); msg.str("");
    msg << log_e("range","Invalid value. Check available options");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidArgument;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  if (target_value == m_sel_range)
  {
    msg.clear(); msg.str("");
    msg << log_w("range","Value is already current setting. ");
    resp["messages"].push_back(msg.str());
  }
  // change the current value
  try {
    bool success;
    m_pm->write_range(target_value, success);
    if (!success)
    {
      msg << log_e("range"," ") << "Failed to select range";
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      LOG(Log::ERR) << msg.str();
      response = UaString(resp.dump().c_str());

      return OpcUa_Good;
    }
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("range"," ") << "Port not open [" << e.what() << "]";
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("range","Failed with a Serial exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("range","Failed with an STL exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e("range","Failed with an unknown exception.");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }

  // all went good
  resp["status"] = "OK";
  msg.clear(); msg.str("");
  msg << log_e("range","Range setting updated.");
  resp["messages"].push_back(msg.str());
  resp["status_code"] = OpcUa_Good;
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLPowerMeter::callSet_pulse_width (
    OpcUa_UInt16 target_value,
    UaString& response
)
{
  LOG(Log::INF) << "Setting pulse width to " << target_value;
  json resp;
  std::ostringstream msg("");

  // we need the system to be at least in unconfigured state
  // as we need the device connection to be established
  if (m_status == sOffline)
  {
    msg.clear(); msg.str("");
    msg << log_e("pulse_width","Device is not connected yet. Only the port can be set in this state.");
    resp["messages"].push_back(msg.str());
    resp["status"] = "ERROR";
    resp["status_code"] = OpcUa_BadInvalidState;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }

  if (m_pulse_widths.size() == 0)
  {
    refresh_measurement_ranges();
  }
  // check if the setting is in the map
  if (m_pulse_widths.count(target_value) == 0)
  {
    msg.clear(); msg.str("");
    msg << log_e("pulse_width","Invalid value. Check available options");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidArgument;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  if (target_value == m_pulse_width)
  {
    msg.clear(); msg.str("");
    msg << log_w("pulse_width","Value is already current setting. ");
    resp["messages"].push_back(msg.str());

  }
  // change the current value
  try {
    m_pm->pulse_length(target_value, m_pulse_width);
    if (m_pulse_width != target_value)
    {
      msg.clear(); msg.str("");
      msg << log_e("pulse_width","Failed to update pulse width") << " (req " << target_value << " cur " << m_pulse_width << ")";
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_BadInvalidArgument;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;

    }
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("pulse_width"," ") << "Port not open [" << e.what() << "]";
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("pulse_width","Failed with a Serial exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("pulse_width","Failed with an STL exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e("pulse_width","Failed with an unknown exception.");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }

  // all went good
  resp["status"] = "OK";
  msg.clear(); msg.str("");
  msg << log_e("pulse_width","Pulse width updated.");
  resp["messages"].push_back(msg.str());
  resp["status_code"] = OpcUa_Good;
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}


UaStatus DIoLPowerMeter::callSet_threshold (
    OpcUa_UInt16 target_value,
    UaString& response
)
{
  LOG(Log::INF) << "Setting threshold to " << target_value;
  json resp;
  std::ostringstream msg("");

  // we need the system to be at least in unconfigured state
  // as we need the device connection to be established
  if (m_status == sOffline)
  {
    msg.clear(); msg.str("");
    msg << log_e("threshold","Device is not connected yet. Only the port can be set in this state.");
    resp["messages"].push_back(msg.str());
    resp["status"] = "ERROR";
    resp["status_code"] = OpcUa_BadInvalidState;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }

  // check of the threshold ranges have been set
  if (m_threshold_limits.second == 0)
  {
    refresh_threshold_limits();
  }

  // check if the setting is within range
  if ((target_value <  m_threshold_limits.first) || (target_value > m_threshold_limits.second))
  {
    msg.clear(); msg.str("");
    msg << log_e("threshold","Invalid value. Threshold should be between ["
                 << m_threshold_limits.first << " , "
                 << m_threshold_limits.second << "]");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidArgument;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  if (target_value == m_e_threshold)
  {
    msg.clear(); msg.str("");
    msg << log_w("threshold","Value is already current setting. ");
    resp["messages"].push_back(msg.str());

  }
  // change the current value
  try {
    m_pm->user_threshold(target_value, m_e_threshold);
    if (m_e_threshold != target_value)
    {
      msg.clear(); msg.str("");
      msg << log_e("threshold","Failed to update threshold") << " (req " << target_value << " cur " << m_e_threshold << ")";
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;
    }
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("threshold"," ") << "Port not open [" << e.what() << "]";
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("threshold","Failed with a Serial exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("threshold","Failed with an STL exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e("pulse_width","Failed with an unknown exception.");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }

  // all went good
  resp["status"] = "OK";
  msg.clear(); msg.str("");
  msg << log_e("set_ave","Threshold updated.");
  resp["messages"].push_back(msg.str());
  resp["status_code"] = OpcUa_Good;
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;
}
UaStatus DIoLPowerMeter::callSet_wavelength (
    OpcUa_UInt16 target_value,
    UaString& response
)
{
  LOG(Log::INF) << "Setting wavelength to " << target_value;
  json resp;
  std::ostringstream msg("");

  // we need the system to be at least in unconfigured state
  // as we need the device connection to be established
  if (m_status == sOffline)
  {
    msg.clear(); msg.str("");
    msg << log_e("wavelength","Device is not connected yet. Only the port can be set in this state.");
    resp["messages"].push_back(msg.str());
    resp["status"] = "ERROR";
    resp["status_code"] = OpcUa_BadInvalidState;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }


  if (target_value == m_wavelength)
  {
    msg.clear(); msg.str("");
    msg << log_w("wavelength","Value is already current setting. ");
    resp["messages"].push_back(msg.str());

  }
  // change the current value
  try {
    bool success;
    m_pm->wavelength(target_value, success);
    if (!success)
    {
      // query what is the current wavelength setting
      std::string wls;
      m_pm->get_all_wavelengths(wls);

      msg.clear(); msg.str("");
      msg << log_e("wavelength","Failed to update wavelength")
          << " (req " << target_value << " available [" << wls << "])";
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;
    }
  }
  catch(serial::PortNotOpenedException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("wavelength"," ") << "Port not open [" << e.what() << "]";
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("wavelength","Failed with a Serial exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("wavelength","Failed with an STL exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e("wavelength","Failed with an unknown exception.");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }

  // all went good
  resp["status"] = "OK";
  msg.clear(); msg.str("");
  msg << log_e("wavelength","Wavelength updated.");
  resp["messages"].push_back(msg.str());
  resp["status_code"] = OpcUa_Good;
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;

}
UaStatus DIoLPowerMeter::callSet_measurement_mode (
    OpcUa_UInt16 target_value,
    UaString& response
)
{
  LOG(Log::INF) << "Changing measurement mode";
  json resp;
  std::ostringstream msg("");

  // we need the system to be at least in unconfigured state
  // as we need the device connection to be established
  if (m_status == sOffline)
  {
    msg.clear(); msg.str("");
    msg << log_e("measurement","Device is not connected yet. Only the port can be set in this state.");
    resp["messages"].push_back(msg.str());
    resp["status"] = "ERROR";
    resp["status_code"] = OpcUa_BadInvalidState;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }

  if (m_measurement_modes.size() == 0)
  {
    refresh_measurement_modes();
  }
  // check if the setting is in the map
  if (m_measurement_modes.count(target_value) == 0)
  {
    msg.clear(); msg.str("");
    msg << log_e("measurement","Invalid setting. Check available options");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidArgument;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  if (target_value == m_measurement_mode)
  {
    msg.clear(); msg.str("");
    msg << log_w("measurement","Value is already current setting. ");
    resp["messages"].push_back(msg.str());
  }
  // change the current value
  try {
    m_pm->measurement_mode(target_value, m_measurement_mode);
    if (m_measurement_mode != target_value)
    {
      msg.clear(); msg.str("");
      msg << log_e("measurement","Failed to update measurement mode.");
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_BadInvalidArgument;
      response = UaString(resp.dump().c_str());
      return OpcUa_Good;

    }
  }
  catch(serial::SerialException &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("measurement","Failed with a Serial exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(std::exception &e)
  {
    msg.clear(); msg.str("");
    msg << log_e("measurement","Failed with an STL exception :") << e.what();
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }
  catch(...)
  {
    msg.clear(); msg.str("");
    msg << log_e("measurement","Failed with an unknown exception.");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    response = UaString(resp.dump().c_str());
    return OpcUa_Good;

  }

  // all went good
  // refresh the ranges, since those now have changed
  refresh_measurement_ranges();

  resp["status"] = "OK";
  msg.clear(); msg.str("");
  msg << log_e("measurement","Measurement mode updated.");
  resp["messages"].push_back(msg.str());
  resp["status_code"] = OpcUa_Good;
  response = UaString(resp.dump().c_str());
  return OpcUa_Good;

}

UaStatus DIoLPowerMeter::callReset (

)
{
  // just issues a reset to the system
  // also, clear all settings and disconnect
  if (m_pm)
  {
    m_pm->reset();
  }
  
  // clear all ranges
  // set the status to offline
  m_status = sOffline;
  m_ranges.clear();
  m_pulse_widths.clear();
  m_ave_windows.clear();
  m_measurement_modes.clear();
  m_status_map.clear();
  return OpcUa_Good;
}

UaStatus DIoLPowerMeter::callStop (
    UaString& response
)
{
    return OpcUa_BadNotImplemented;
}

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333


void DIoLPowerMeter::update()
{
  OpcUa_StatusCode status= OpcUa_Good;
  /** We shouldn't need to update this
    m_pm->get_range_fast(m_sel_range);
    getAddressSpaceLink()->setRange_selected(m_sel_range, status);
   */

  static bool first = true;
  if (first)
  {
    // now initialize all relevant variables passed through configuration
    // even though we initialize the class with what we believe to be those initial values,
    // nothing prevents them from changing in the future

    getAddressSpaceLink()->getRange_selected(m_sel_range);
    getAddressSpaceLink()->getMeasurement_mode(m_measurement_mode);
    getAddressSpaceLink()->getWavelength(m_wavelength);
    getAddressSpaceLink()->getTrigger_threshold(m_e_threshold);
    getAddressSpaceLink()->getAverage_window(m_ave_setting);
    getAddressSpaceLink()->getPulse_width(m_pulse_width);
    UaString dp(m_comport.c_str());
    getAddressSpaceLink()->setDevice_port(dp,m_comport.size(),OpcUa_Good);
    first = false;
  }

  // get an energy reading
  refresh_energy_reading();
  getAddressSpaceLink()->setEnergy_reading(m_energy_reading, OpcUa_Good);
  // get also an average reading, if available

  if (m_ave_setting == 1)
  {
    status = OpcUa_BadDataUnavailable;
  }
  else {
    refresh_average_reading();
  }
  getAddressSpaceLink()->setAverage_reading(m_average_reading, status);

  std::string str_st = m_status_map.at(m_status).c_str();
  UaString ua_str = str_st.c_str();
  getAddressSpaceLink()->setStatus(ua_str,str_st.size(),OpcUa_Good);

  // TODO: Is there anything else to update?
}

void DIoLPowerMeter::refresh_energy_reading()
{
  if (m_status != sReady)
  {
    // do nothing, return
    getAddressSpaceLink()->setEnergy_reading(0.0, OpcUa_BadInvalidState);
  }


}

void DIoLPowerMeter::refresh_average_reading()
{
  if (m_status != sReady)
  {
    // do nothing, return
    getAddressSpaceLink()->setAverage_reading(0.0, OpcUa_BadInvalidState);
  }


}


void DIoLPowerMeter::automatic_port_search()
{
 try
 {
  // the automatic port search queries all available ports, their descriptions and IDs for an occurrence of the device
  // serial number
  // this has been shown to work for most cases, but there is always a chance that something fails
  // for example, the wrong serial number was provided
  m_comport = util::find_port(serial_number());
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

void DIoLPowerMeter::refresh_all_ranges()
{
  refresh_measurement_ranges();
  refresh_pulse_width_ranges();
  refresh_average_ranges();
  refresh_threshold_limits();
}


void DIoLPowerMeter::refresh_measurement_modes()
{
  std::string type,sn,name;
  bool power,energy,freq;
  m_pm->head_info(type, sn, name, power, energy, freq);

  // now build the map with the capabilities of the power meter
  m_measurement_modes.clear();
  // insert passive (all have passive)
  m_measurement_modes.insert({1, "passive"});
  if (energy)
  {
    m_measurement_modes.insert({2, "energy"});
  }
  if (power)
  {
    m_measurement_modes.insert({3, "power"});
  }
  if (freq)
  {
    m_measurement_modes.insert({4, "exposure"});
  }
  std::string s = util::serialize_map(m_measurement_modes);
  UaString ss(s.c_str());
  getAddressSpaceLink()->setMeasurement_options(ss,s.size(), OpcUa_Good);

}
void DIoLPowerMeter::refresh_threshold_limits()
{
  uint16_t current, min, max;
  m_pm->query_user_threshold(current, min, max);
  m_threshold_limits.first = min;
  m_threshold_limits.second = max;
  if (current != m_e_threshold)
  {
    LOG(Log::WRN) << "DIoLPowerMeter::refresh_threshold_limits : Mismatch between cached threshold and device reported ("
        << m_e_threshold << " <> " << current << ")";
  }
}

void DIoLPowerMeter::refresh_measurement_ranges()
{
  //
  int16_t v;
  m_pm->get_all_ranges(v);
  if (v != m_sel_range)
  {
    LOG(Log::WRN) << "DIoLPowerMeter::refresh_measurement_ranges : Mismatch between cached range and device reported (" << m_sel_range << " <> " << v << ")";
  }
   m_pm->get_range_map(m_ranges);
   // force address space update
   std::string s = util::serialize_map(m_ranges);
   UaString ss(s.c_str());
   getAddressSpaceLink()->setRange_options(ss,s.size(), OpcUa_Good);

}

void DIoLPowerMeter::refresh_pulse_width_ranges()
{
  uint16_t a;
  m_pm->pulse_length(0, a); // this guarantees that the map is filled
  m_pm->get_pulse_map(m_pulse_widths);

  // force address space update
  std::string s = util::serialize_map(m_pulse_widths);
  UaString ss(s.c_str());
  getAddressSpaceLink()->setPulse_length_options(ss,s.size(), OpcUa_Good);

}

void DIoLPowerMeter::refresh_average_ranges()
{
  // -- check the average window options
  uint16_t a;
  m_pm->average_query(0, a); // this guarantees that the map is filled
  m_pm->get_averages_map(m_ave_windows);

  // force address space update
  std::string s = util::serialize_map(m_ave_windows);
  UaString ss(s.c_str());
  getAddressSpaceLink()->setAverage_options(ss,s.size(), OpcUa_Good);

}

UaStatus DIoLPowerMeter::init_device(json &resp)
{
  // if the port does not start by '/'
  std::ostringstream msg("");
  if (m_comport.size() == 0)
  {
    msg << log_e("init","There is no port");
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    LOG(Log::ERR) << msg.str();
    return OpcUa_Good;
  }

  if (m_comport.at(0)!= '/')
  {
    msg << log_e("init"," ") << "Malformed port [" << m_comport << "]. Expected something like [/dev/ttyXXXXX]";
    resp["status"] = "ERROR";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_Bad;
    return OpcUa_Bad;
  }

  // all seems good
  // try to establish the connection
  try
  {
    if (!m_pm)
    {
      m_pm = new device::PowerMeter(m_comport.c_str(),static_cast<uint32_t>(baud_rate()));
    }
    // set the device to the measurement mode we have set in configuration
    // query instrument for the measurement modes it can do
    std::string type, sn, name;
    bool power,energy,freq;
    m_pm->head_info(type, sn, name, power, energy, freq);
    // crosscheck that the serial number is the same as the configuration
    if (sn != serial_number())
    {
      msg << log_e("init"," ") << "Device mismatch. Device serial number vs. configured : ["
          << sn << "] != [" << serial_number()<<"]";
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      LOG(Log::ERR) << msg.str();

      delete m_pm;
      m_status = sOffline;
      return OpcUa_Bad;
    }

    // update port in the address space
    UaString dport(m_comport.c_str());
    getAddressSpaceLink()->setDevice_port(dport,m_comport.size(),OpcUa_Good);

    // now build the map with the capabilities of the power meter
    m_measurement_modes.clear();
    // insert passive (all have passive)
    m_measurement_modes.insert({1, "passive"});
    if (energy)
    {
      m_measurement_modes.insert({2, "energy"});
    }
    if (power)
    {
      m_measurement_modes.insert({3, "power"});
    }
    if (freq)
    {
      m_measurement_modes.insert({4, "exposure"});
    }
    std::string s = util::serialize_map(m_measurement_modes);
    UaString ss(s.c_str());
    getAddressSpaceLink()->setMeasurement_options(ss,s.size(), OpcUa_Good);

    // check if the configured measurement mode is valid
    // for the device, otherwise set to passive
    if (m_measurement_modes.count(m_measurement_mode))
    {
      // it is a valid mode
      // set it as the operating mode
      uint16_t mm;
      m_pm->measurement_mode(m_measurement_mode,mm);
      // --
      getAddressSpaceLink()->setMeasurement_mode(mm, OpcUa_Good);


      // refresh the m_pulse_lengths

    } else
    {
      // remain online, but unconfigured
      m_status = sUnconfigured;
      return OpcUa_Bad;
    }

    //
    // -- Select the measurement range
    //

    // refresh the ranges (for this measurement mode
    refresh_measurement_ranges();

    // -- so far so good, so set the requested range
    bool success;
    m_pm->write_range(m_sel_range, success);
    if (!success)
    {
      msg << log_e("init"," ") << "Failed to select range";
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      LOG(Log::ERR) << msg.str();

      m_status = sUnconfigured;
      return OpcUa_Bad;
    }

    //
    // -- Select the pulse widths/lengths
    //
    refresh_pulse_width_ranges();
    uint16_t u16;
    m_pm->pulse_length(m_pulse_width, u16);
    if (u16 != m_pulse_width)
    {

      msg << log_e("init"," ") << "Failed to select pulse width.";
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      LOG(Log::ERR) << msg.str();

      m_status = sUnconfigured;
      return OpcUa_Bad;
    }

    //
    //-- Select the average setting
    //
    refresh_average_ranges();
    m_pm->average_query(m_ave_setting, u16); // this ensures that the map is filled
    if (u16 != m_ave_setting)
    {
      LOG(Log::WRN) << "DIoLPowerMeter::init_device : Failed to select average setting.";
      m_status = sUnconfigured;
      return OpcUa_Bad;
    }

    //
    // -- Select the wavelength
    //
    m_pm->wavelength(m_wavelength, success);
    if (!success)
    {
      LOG(Log::WRN) << "DIoLPowerMeter::init_device : Failed to set wavelength to " << m_wavelength;

      msg << log_e("init"," ") << "Failed to set wavelength to " << m_wavelength;
      resp["status"] = "ERROR";
      resp["messages"].push_back(msg.str());
      resp["status_code"] = OpcUa_Bad;
      LOG(Log::ERR) << msg.str();

      m_status = sUnconfigured;
      return OpcUa_Bad;
    }

    //
    // -- Select the threshold
    //
    uint16_t current, min, max;
    m_pm->query_user_threshold(current, min, max);
    m_threshold_limits.first = min;
    m_threshold_limits.second = max;

    if (current != m_e_threshold)
    {
      // if the current setting is the same as the set threshold, do nothing
      m_pm->user_threshold(m_e_threshold, u16);
      if (u16 != m_e_threshold)
      {
        msg << log_e("init"," ") << " Failed to set energy threshold to " << m_e_threshold;
        resp["status"] = "ERROR";
        resp["messages"].push_back(msg.str());
        resp["status_code"] = OpcUa_Bad;
        LOG(Log::ERR) << msg.str();
        m_status = sUnconfigured;
        return OpcUa_Bad;
      }
    }
    // -- if it reached this point, the whole thing is initialized
    m_status = sReady;

  }
  catch(serial::PortNotOpenedException &e)
  {
    // port is not open. Keep status as offline
    // don't commit any assignments
    LOG(Log::ERR) << "DIoLPowerMeter::init_device : Port not open [" << e.what() << "].";

    m_status = sOffline;
    return OpcUa_Bad;
  }
  catch(serial::SerialException &e)
  {
    LOG(Log::ERR) << "DIoLPowerMeter::init_device : Caught a serial exception : [" << e.what() << "].";
    m_status = sUnconfigured;
    return OpcUa_Bad;

  }
  catch(std::exception &e)
  {
    LOG(Log::ERR) << "DIoLPowerMeter::init_device : Caught an STL exception : [" << e.what() << "].";
    m_status = sUnconfigured;
    return OpcUa_Bad;

  }
  catch(...)
  {
    // caught something completely unexpected. Just treat it as something went really wrong.
    // Assume one is offline
    LOG(Log::ERR) << "DIoLPowerMeter::init_device : Caught an unknown exception.";
    m_status = sUnconfigured;
    return OpcUa_Bad;

  }
  return OpcUa_Good;

}

} // namespace Device
