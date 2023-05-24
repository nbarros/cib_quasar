
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


#ifndef __DIoLaserSystem__H__
#define __DIoLaserSystem__H__

#include <Base_DIoLaserSystem.h>

namespace Device
{

class
    DIoLaserSystem
    : public Base_DIoLaserSystem
{

public:
    /* sample constructor */
    explicit DIoLaserSystem (
        const Configuration::IoLaserSystem& config,
        Parent_DIoLaserSystem* parent
    ) ;
    /* sample dtr */
    ~DIoLaserSystem ();

    /* delegators for
    cachevariables and sourcevariables */


    /* delegators for methods */
    UaStatus callLoad_config (
        const UaString&  config,
        UaString& response
    ) ;
    UaStatus callCheck_ready (
        OpcUa_Boolean& ready
    ) ;
    UaStatus callStop (

    ) ;
    UaStatus callFire_at_position (
        const std::vector<OpcUa_Int32>&  target_pos,
        OpcUa_UInt16 num_pulses,
        UaString& answer
    ) ;
    UaStatus callFire_segment (
        const std::vector<OpcUa_Int32>&  start_pos,
        const std::vector<OpcUa_Int32>&  last_pos,
        UaString& answer
    ) ;
    UaStatus callExecute_scan (
        const UaString&  plan,
        UaString& answer
    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DIoLaserSystem( const DIoLaserSystem& other );
    DIoLaserSystem& operator=(const DIoLaserSystem& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:

    void update();

private:



};

}

#endif // __DIoLaserSystem__H__