
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


#ifndef __DIoLPowerMeter__H__
#define __DIoLPowerMeter__H__

#include <Base_DIoLPowerMeter.h>

class device::PowerMeter;

namespace Device
{

class
    DIoLPowerMeter
    : public Base_DIoLPowerMeter
{

public:
    /* sample constructor */
    explicit DIoLPowerMeter (
        const Configuration::IoLPowerMeter& config,
        Parent_DIoLPowerMeter* parent
    ) ;
    /* sample dtr */
    ~DIoLPowerMeter ();

    /* delegators for
    cachevariables and sourcevariables */


    /* delegators for methods */
    UaStatus callSet_average (
        OpcUa_UInt16 target_value,
        UaString& response
    ) ;
    UaStatus callSet_range (
        OpcUa_Int16 target_value,
        UaString& response
    ) ;
    UaStatus callSet_pulse_width (
        OpcUa_UInt16 target_value,
        UaString& response
    ) ;
    UaStatus callSet_threshold (
        OpcUa_UInt16 target_value,
        UaString& response
    ) ;
    UaStatus callSet_wavelength (
        OpcUa_UInt16 target_value,
        UaString& response
    ) ;
    UaStatus callSet_measurement_mode (
        OpcUa_UInt16 target_value,
        UaString& response
    ) ;
    UaStatus callReset (

    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DIoLPowerMeter( const DIoLPowerMeter& other );
    DIoLPowerMeter& operator=(const DIoLPowerMeter& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    void update();

private:
    device::PowerMeter *m_pm;
    std::string m_comport;


};

}

#endif // __DIoLPowerMeter__H__