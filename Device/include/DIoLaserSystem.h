
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
#include <json.hpp>
using json = nlohmann::json;
#include <atomic>

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
        const UaString&  conf,
        UaString& response
    ) ;
    UaStatus callCheck_ready (
        OpcUa_Boolean& ready
    ) ;
    UaStatus callStop (
        UaString& response
    ) ;
    UaStatus callFire_at_position (
        const UaString&  arguments,
        UaString& answer
    ) ;
    UaStatus callFire_segment (
        const UaString&  arguments,
        UaString& answer
    ) ;
    UaStatus callExecute_scan (
        const UaString&  plan,
        UaString& answer
    ) ;
    UaStatus callExecute_grid_scan (
        const UaString&  plan,
        UaString& answer
    ) ;
    UaStatus callPause (
        UaString& answer
    ) ;
    UaStatus callStandby (
        UaString& answer
    ) ;
    UaStatus callResume (
        UaString& response
    ) ;
    UaStatus callWarmup_laser (
        UaString& response
    ) ;
    UaStatus callShutdown (
        UaString& response
    ) ;
    UaStatus callMove_to_pos (
        const UaString&  arguments,
        UaString& response
    ) ;
    UaStatus callClear_error (
        UaString& response
    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DIoLaserSystem( const DIoLaserSystem& other );
    DIoLaserSystem& operator=(const DIoLaserSystem& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *
    //enum State {sOffline, sReady, sWarmup, sPause, sStandby, sOperating, sError};
    enum State {sOffline, sReady,sBusy,sError};
    //
    UaStatus config(json &conf, json &resp);
    UaStatus check_ready(bool &ready);
    UaStatus stop(json &resp);
    bool process_fap_arguments(const UaString &arguments, std::vector<int32_t> &target_pos, std::string &approach, uint32_t &num_pulses, bool &lbls, json &response);
    void init_fire_point_task(const std::vector<int32_t> &target, const std::string &approach, const uint32_t &num_pulses);
    void fire_point_task(const std::vector<int32_t> &target, const std::string &approach, const uint32_t &num_pulses);

    UaStatus fire_at_position(const std::vector<int32_t> &target_pos, const std::string &approach, uint32_t num_pulses, json &resp);

    UaStatus fire_segment(
        const std::vector<OpcUa_Int32>&  spos,
        const std::vector<OpcUa_Int32>&  lpos,
        json& resp
    );
    UaStatus execute_scan(json &plan, json &resp);
    UaStatus pause(json &resp);
    UaStatus standby(json &resp);
    UaStatus resume(json &resp);
    UaStatus warmup(json &resp);
    UaStatus shutdown(json &resp);
    bool process_move_arguments(const UaString& arguments, std::vector<int32_t> &target_pos, std::string &approach, json &response);
    void move_task(const std::vector<int32_t> target_pos, const std::string approach);
    UaStatus move_to_pos(
        const std::vector<int32_t>&  position, const std::string approach,json &resp);
    // this is a stripped down version that does not make any checks
    // those are supposed to have been made before
    UaStatus move_motor(
        const std::vector<OpcUa_Int32>&  position,
        json &resp);
//    UaStatus set_dac_threshold(uint16_t &val,json &resp);

    inline void reset(std::ostringstream &s);
    bool validate_config_fragment(json &frag, json &resp);
    void update_state(State s);
    void refresh_state();
    void init_segment_task(const std::vector<OpcUa_Int32>&  spos,
                           const std::vector<OpcUa_Int32>&  lpos
                          );
    void segment_task(const std::vector<OpcUa_Int32>&  spos,
                           const std::vector<OpcUa_Int32>&  lpos
                          );
    void init_scan_task(json &segments);
    void get_message_queue(json &resp, bool clear);
    bool validate_scan_plan(json &plan, json &resp);
    // crappy method necessary because damn motor code is a freaking nightmare
    void wait_for_motor(DIoLMotor*motor, int32_t target);
    void wait_for_motors(const std::vector<int32_t> &target);
    void update_task_message_queue(json &resp);
    UaStatus validate_grid_parameters(const json &plan, json &resp);
    UaStatus execute_grid_scan(json &plan, json &resp);

public:

    UaStatus clear_error(json &resp);
    // makes a roll call for each system to update itself
    void update();
    // makes a status call over all subsystems
    bool is_ready();
    // really initializes the DAC readout
private:

    std::map<State,std::string> m_state_map;
    State m_state;
    json m_task_message_queue;
    std::map<size_t,size_t> m_map_motor_coordinates;


};

}

#endif // __DIoLaserSystem__H__
