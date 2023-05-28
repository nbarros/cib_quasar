
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

#include <DIoLLaserUnit.h>
#include <ASIoLLaserUnit.h>

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
DIoLLaserUnit::DIoLLaserUnit (
    const Configuration::IoLLaserUnit& config,
    Parent_DIoLLaserUnit* parent
):
    Base_DIoLLaserUnit( config, parent)

    /* fill up constructor initialization list here */
	, m_is_ready(false)

{
    /* fill up constructor body here */
}

/* sample dtr */
DIoLLaserUnit::~DIoLLaserUnit ()
{
}

/* delegates for cachevariables */

/* Note: never directly call this function. */

UaStatus DIoLLaserUnit::writeQswitch_us ( const OpcUa_UInt32& v)
{
    return OpcUa_BadNotImplemented;
}
/* Note: never directly call this function. */

UaStatus DIoLLaserUnit::writeDischarge_voltage_kV ( const OpcUa_Float& v)
{
    return OpcUa_BadNotImplemented;
}
/* Note: never directly call this function. */

UaStatus DIoLLaserUnit::writeRep_rate_hz ( const OpcUa_Double& v)
{
    return OpcUa_BadNotImplemented;
}
/* Note: never directly call this function. */

UaStatus DIoLLaserUnit::writeRep_rate_divider ( const OpcUa_UInt16& v)
{
    return OpcUa_BadNotImplemented;
}
/* Note: never directly call this function. */

UaStatus DIoLLaserUnit::writeExt_shutter_time_pre_shot ( const OpcUa_UInt32& v)
{
    return OpcUa_BadNotImplemented;
}
/* Note: never directly call this function. */

UaStatus DIoLLaserUnit::writeExt_shutter_open_time_us ( const OpcUa_UInt32& v)
{
    return OpcUa_BadNotImplemented;
}


/* delegators for methods */
UaStatus DIoLLaserUnit::callStop (

)
{
    return OpcUa_BadNotImplemented;
}
UaStatus DIoLLaserUnit::callCheck_status (
    OpcUa_UInt16& status
)
{
    return OpcUa_BadNotImplemented;
}
UaStatus DIoLLaserUnit::callConfigure_laser (
    const UaString&  config,
    UaString& response
)
{
    return OpcUa_BadNotImplemented;
}
UaStatus DIoLLaserUnit::callSingle_shot (
    UaString& response
)
{
    return OpcUa_BadNotImplemented;
}
UaStatus DIoLLaserUnit::callSwitch_shutter (
    OpcUa_Boolean open,
    UaString& response
)
{
    return OpcUa_BadNotImplemented;
}
UaStatus DIoLLaserUnit::callFire_standalone (
    OpcUa_Boolean fire,
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

}