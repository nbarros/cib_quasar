
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

#include <DIoLaserSystem.h>
#include <ASIoLaserSystem.h>

#include <DIoLLaserUnit.h>
#include <DIoLMotor.h>
#include <DIoLPiezoController.h>
#include <DIoLAttenuator.h>
#include <DIoLPowerMeter.h>
#include <DIoLShutter.h>

#include <string>

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
DIoLaserSystem::DIoLaserSystem (
    const Configuration::IoLaserSystem& config,
    Parent_DIoLaserSystem* parent
):
    Base_DIoLaserSystem( config, parent)

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
}

/* sample dtr */
DIoLaserSystem::~DIoLaserSystem ()
{
}

/* delegates for cachevariables */



/* delegators for methods */
UaStatus DIoLaserSystem::callLoad_config (
    const UaString&  config,
    UaString& response
)
{
    // this is where the whole configuration json file is applied

    std::string msg = "{\"status\":\"Failed\", \"messages\":[\"Method not completely implemented yet\"]}";
    response = UaString(msg.c_str());
//    return OpcUa_Bad;
    return OpcUa_BadNotImplemented;
}
UaStatus DIoLaserSystem::callCheck_ready (
    OpcUa_Boolean& ready
)
{

    // check all associated subsystems if they're ready
  bool rdy = true;
  for (Device::DIoLLaserUnit* lunit : iollaserunits())
  {
      rdy |= lunit->is_ready();
  }
  for (Device::DIoLMotor* lmotor : iolmotors ())
  {
      rdy |= lmotor->is_ready();
  }
  for (Device::DIoLPiezoController* lctl : iolpiezocontrollers () )
  {
      rdy |= lctl->is_ready();
  }
  for (Device::DIoLAttenuator* latt : iolattenuators ())
  {
      rdy |= latt->is_ready();
  }
  return OpcUa_Good;
}
UaStatus DIoLaserSystem::callStop (

)
{
    return OpcUa_BadNotImplemented;
}
UaStatus DIoLaserSystem::callFire_at_position (
    const std::vector<OpcUa_Int32>&  target_pos,
    OpcUa_UInt16 num_pulses,
    UaString& answer
)
{
  //
  json resp;
  // first check that the whole system is ready
  if (!is_ready())
  {
    resp["status"] = "ERROR";
    std::ostringstream msg("");
    msg << log_e("fire_at_position","System is not ready to operate");
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidState;

    LOG(Log::ERR) << msg.str();
    answer = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }

  // once it is ready, pass on the information to the respective subsystems

  // in this case the laser is not going to just start firing, but simply fire a discrete number of pulses
  // there is a specific callfor that under the IOLaser device

  // -- the motors are identified, so each entry refers to a motor
  // do a check on the order of position and number of motors
  if (target_pos.size()!= iolmotors().size())
  {
    // different number of position coordinates and motors
    resp["status"] = "ERROR";
    std::ostringstream msg("");
    msg << log_e("fire_at_position","Different number of coordinates (")
        << target_pos.size() << ") and available motors ("
        << iolmotors().size() << "). Refusing to operate.";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidArgument;

    return OpcUa_Good;
  }

  if (num_pulses < 1)
  {
    // different number of position coordinates and motors
    resp["status"] = "ERROR";
    std::ostringstream msg("");
    msg << log_e("fire_at_position","Invalid number of pulses (")
        << num_pulses << "). Value must be at least 1.";
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidArgument;

    return OpcUa_Good;
  }

  // ready to operate
  // -- be sure to set the shutter closed until destination is reached
  //TODO: Add shutter device

  // the logic is the following
  // 1. Call each of the the motors to move to the desired position
  // 2. open the shutter
  // 3. fire a discrete number of shots
  // 4. close the shutter

  return OpcUa_Good;
}
UaStatus DIoLaserSystem::callFire_segment (
    const std::vector<OpcUa_Int32>&  start_pos,
    const std::vector<OpcUa_Int32>&  last_pos,
    UaString& answer
)
{
  json resp;
  // first check that the whole system is ready
  if (!is_ready())
  {
    resp["status"] = "ERROR";
    std::ostringstream msg("");
    msg << log_e("start_move","System is not ready to operate");
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidState;

    LOG(Log::ERR) << msg.str();
    answer = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }
  //
  // Operation sequence
  //
  // 1. fill FIFOs of red zones
  // 2. set target destination for all motors
  // 3. move motors into start point
  // 4. set inital fire position
  // 5. initiate movement
  // 6. open iris
  // 7. When reaching destination, close iris
    return OpcUa_Good;
}
UaStatus DIoLaserSystem::callExecute_scan (
    const UaString&  plan,
    UaString& answer
)
{
  json resp;
  // first check that the whole system is ready
  if (!is_ready())
  {
    resp["status"] = "ERROR";
    std::ostringstream msg("");
    msg << log_e("start_move","System is not ready to operate");
    resp["messages"].push_back(msg.str());
    resp["status_code"] = OpcUa_BadInvalidState;

    LOG(Log::ERR) << msg.str();
    answer = UaString(resp.dump().c_str());
    return OpcUa_Good;
  }

  // -- Parse the configuration JSON
  // -- it should be composed of sub-sequences


  //
  // Operation sequence
  //
  // 1. start a firing sequence
  // 2. close iris
  // 3. start another firing sequence
  // 4. repeat at will
  //

    return OpcUa_BadNotImplemented;
}

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333
void DIoLaserSystem::update()
{
	// call update over all daughters
    for (Device::DIoLLaserUnit* lunit : iollaserunits())
    {
    	lunit->update();
    }
    for (Device::DIoLMotor* lmotor : iolmotors ())
    {
    	lmotor->update();
    }
    for (Device::DIoLPiezoController* lctl : iolpiezocontrollers () )
    {
    	lctl->update();
    }
    for (Device::DIoLAttenuator* latt : iolattenuators ())
    {
    	latt->update();
    }
    for (Device::DIoLShutter *lshutter : iolshutters())
    {
      lshutter->update();
    }
    for (Device::DIoLPowerMeter* lmeter : iolpowermeters())
    {
      lmeter->update();
    }
}
bool DIoLaserSystem::is_ready()
{
  for (Device::DIoLLaserUnit* lunit : iollaserunits())
  {
    if (!lunit->is_ready())
    {
      return false;
    }
  }
  for (Device::DIoLMotor* lmotor : iolmotors ())
  {
    if (!lmotor->is_ready())
    {
      return false;
    }
  }
  for (Device::DIoLPiezoController* lctl : iolpiezocontrollers () )
  {
    if (!lctl->is_ready())
    {
      return false;
    }
  }
  for (Device::DIoLAttenuator* latt : iolattenuators ())
  {
    if (!latt->is_ready())
    {
      return false;
    }
  }
  for (Device::DIoLPowerMeter* lmeter : iolpowermeters())
  {
    if (!lmeter->is_ready())
    {
      return false;
    }
  }
  return true;
}
}
