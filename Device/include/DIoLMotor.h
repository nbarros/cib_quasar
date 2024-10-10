
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


#ifndef __DIoLMotor__H__
#define __DIoLMotor__H__

#include <Base_DIoLMotor.h>
#include <vector>
#include <string>
using std::vector;
using string = std::string;

#include <json.hpp>
using json = nlohmann::json;
#include <mutex>

namespace Device
{

  typedef struct
  {
    uint16_t offset;
    uint16_t bit_high;
    uint16_t bit_low;
    uintptr_t addr; // addr is the memory mapped address of this particular register
    uint32_t mask;
  } motor_regs_t;

class
    DIoLMotor
    : public Base_DIoLMotor
{

public:
    /* sample constructor */
    explicit DIoLMotor (
        const Configuration::IoLMotor& config,
        Parent_DIoLMotor* parent
    ) ;
    /* sample dtr */
    ~DIoLMotor ();

    /* delegators for
    cachevariables and sourcevariables */
    /* Note: never directly call this function. */
    UaStatus writeRange_min ( const OpcUa_Int32& v);
    /* Note: never directly call this function. */
    UaStatus writeRange_max ( const OpcUa_Int32& v);
    /* Note: never directly call this function. */
    UaStatus writeRefresh_period_ms ( const OpcUa_UInt16& v);
    /* Note: never directly call this function. */
    UaStatus writeAcceleration ( const OpcUa_UInt32& v);
    /* Note: never directly call this function. */
    UaStatus writeDeceleration ( const OpcUa_UInt32& v);
    /* Note: never directly call this function. */
    UaStatus writeSpeed ( const OpcUa_UInt32& v);


    /* delegators for methods */
    UaStatus callConfig (
        const UaString&  config_json,
        UaString& response
    ) ;
    UaStatus callMove_absolute (
        OpcUa_Int32 destination,
        UaString& response
    ) ;
    UaStatus callMove_relative (
        OpcUa_Int32 num_steps,
        UaString& response
    ) ;
    UaStatus callStop (
        UaString& response
    ) ;
    UaStatus callReset (
        UaString& response
    ) ;
    UaStatus callClear_alarm (
        UaString& response
    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DIoLMotor( const DIoLMotor& other );
    DIoLMotor& operator=(const DIoLMotor& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

    static size_t curl_write_function(void* ptr, size_t size, size_t nmemb, std::string* data);
    //void timer_start(std::function<void(void)> func, unsigned int interval);
    // -- this one monitors the motor itself
    void timer_start(DIoLMotor *obj);

    void refresh_server_info();
    UaStatus map_registers();
    UaStatus unmap_registers();

public:

    typedef struct cib_gpio_t
    {
      uintptr_t paddr;
      uintptr_t vaddr;
      size_t size;
      int id;
    } cib_gpio_t;
    //
    typedef struct cib_param_t
    {
      cib_gpio_t reg;
      uintptr_t offset;
      uint16_t bit_low;
      uint16_t bit_high;
      uintptr_t maddr; // maddr is the memory mapped address of this particular register
      uint32_t mask;
      size_t n_bits;
    } cib_param_t;
    //
    void update();
    bool is_ready();
    //
    // Simulation functions
    //
    UaStatus sim_get_motor_info();
    UaStatus sim_move_motor(json &resp);
    UaStatus sim_stop_motor(json &resp);

    // -- wrapper for interface commands commands
    UaStatus config_wrapper(json &conf, json &resp);
    UaStatus move_wrapper(int32_t dest, json &resp);
    UaStatus get_motor_info();

    UaStatus move_motor(json &resp);
    UaStatus stop(json &resp);
    UaStatus config(json &conf, json &resp);
    UaStatus terminate(json &resp);
    UaStatus reset(json &resp);
    UaStatus get_alarm_state(json &resp);
    UaStatus clear_alarm(json &resp);


    //
    const bool get_monitor() {return m_monitor;}
    void set_monitor(bool m) {m_monitor = m;}
    //
    UaStatus set_range_min(const int32_t &v);
    UaStatus set_range_max(const int32_t &v);
    UaStatus set_refresh_period(const uint16_t &v);
    UaStatus set_acceleration(const uint32_t &v);
    UaStatus set_deceleration(const uint32_t &v);
    UaStatus set_speed(const uint32_t &v);
    const size_t get_coordinate_index() {return m_coordinate_index;}
    UaStatus set_position_setpoint(const int32_t target);
    // Ua
    UaStatus set_id(const std::string &id);
    const std::string get_id() {return m_id;}
    //
    void set_server_addr(const std::string &s);
    const std::string get_server_addr() {return m_server_host;}
    bool is_moving() const {return m_is_moving;}
    bool is_in_range(const int32_t &v);
    int32_t get_range_min() const {return m_range_min;}
    int32_t get_range_max() const {return m_range_max;}
private:


    // aux methods
    // -- basic communication function. All communication goes through here
    UaStatus query_motor(const std::string request, json &reply, json &resp);
    UaStatus query_cib_position(int32_t &pos);
    UaStatus set_cib_init_position(int32_t &pos);
    UaStatus set_cib_direction(int32_t &dir);

    // basic fundamental tests
    UaStatus check_motor_ready(json &resp);
    UaStatus check_motor_moving(json &resp);

    // other aux functions
    UaStatus init_cib_mem();
    UaStatus clear_cib_mem();
    void refresh_registers();
    UaStatus validate_config_fragment(json &conf, json &resp);
    UaStatus map_registers(json &conf,json &resp);
    UaStatus validate_registers(json &conf,json &resp);
    UaStatus check_cib_mem(json &resp);
    UaStatus cib_set_init_position(int32_t &pos);
    UaStatus cib_set_direction(uint32_t &dir);
    UaStatus cib_get_position(int32_t &pos);
    // -- this one monitors the movement register
    void cib_movement_monitor(DIoLMotor *obj);

    const uint16_t get_refresh_ms() {return m_refresh_ms;}
    // these are internal variables to simulate the movement itself
    void sim_mv();
    int32_t m_sim_pos;
    int32_t m_sim_speed;
    int32_t m_sim_tpos;
    bool m_sim_moving;
    //
    //
    int32_t m_position_motor;
    int32_t m_position_cib;
    int32_t m_position_setpoint;
//    OpcUa_StatusCode m_position_setpoint_status;
    //
    bool m_is_ready; // declares where it is ready for operation
    // this essentially means that all settings are in a reasonable state
    bool m_is_moving; // -- this variable will be set by a GPIO bit. That code is not ready yet
    uint32_t m_acceleration;
    uint32_t m_deceleration;
    uint32_t m_speed_setpoint;
    uint32_t m_speed_readout; // current speed reported by the motor
    int32_t m_alarm_code_motor;
    double m_torque;
    double m_temperature;
    uint32_t m_refresh_ms;
    bool m_monitor;
    OpcUa_StatusCode m_monitor_status;
    std::string m_server_host;
    uint16_t m_server_port;
    int32_t m_range_min;
    int32_t m_range_max;
    std::string m_id;
    size_t m_coordinate_index;
    //
    // Variables necessary to map registers in the CIB
    //
    int m_mmap_fd;
    std::map<int,cib_gpio_t> m_reg_map;
    std::map<std::string,cib_param_t> m_regs;
    std::mutex m_motor_mtx;
};

}

#endif // __DIoLMotor__H__
