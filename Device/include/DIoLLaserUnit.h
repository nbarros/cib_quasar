
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


#ifndef __DIoLLaserUnit__H__
#define __DIoLLaserUnit__H__

#include <Base_DIoLLaserUnit.h>
#include <json.hpp>
using json = nlohmann::json;
namespace device {
  class Laser;
}

namespace Device
{

  typedef struct
  {
    uint16_t offset;
    uint16_t bit_high;
    uint16_t bit_low;
    uintptr_t addr; // addr is the memory mapped address of this particular register
    uint32_t mask;
  } laser_regs_t;

  typedef struct laser_state_t
  {
    // lsb
    bool ext_shutter_closed    : 1;
    bool laser_shutter_closed  : 1;
    bool qswitch_en            : 1;
    bool laser_started         : 1;
    uint8_t padding            : 4;
    // msb
    laser_state_t() :
      ext_shutter_closed(0x0),
      laser_shutter_closed(0x0),
      qswitch_en(0x0),
      laser_started(0x0),
      padding(0x0) {}
  } laser_state_t;

  typedef struct laser_state_u
  {
    laser_state_t state;
    laser_state_u(laser_state_t s) : state(s) {state.padding = 0x0;}
    laser_state_u(uint8_t s) : state(*reinterpret_cast<laser_state_t*>(&s)) {state.padding = 0x0;}
    uint8_t get_byte() {return *reinterpret_cast<uint8_t*>(&state);}
    bool    is_set(uint16_t bit) {return (get_byte() & ((0x1U << bit) & 0xFF));}
    bool    bits_set(uint8_t mask) {return (get_byte() & mask);}
  } laser_state_u;

class
    DIoLLaserUnit
    : public Base_DIoLLaserUnit
{

public:
    /* sample constructor */
    explicit DIoLLaserUnit (
        const Configuration::IoLLaserUnit& config,
        Parent_DIoLLaserUnit* parent
    ) ;
    /* sample dtr */
    ~DIoLLaserUnit ();

    /* delegators for
    cachevariables and sourcevariables */
    /* Note: never directly call this function. */
    UaStatus writeDischarge_voltage_kV ( const OpcUa_Double& v);
    /* Note: never directly call this function. */
    UaStatus writeRep_rate_hz ( const OpcUa_Double& v);
    /* Note: never directly call this function. */
    UaStatus writeRep_rate_divider ( const OpcUa_UInt32& v);


    /* delegators for methods */
    UaStatus callSet_connection (
        const UaString&  device_port,
        OpcUa_UInt16 baud_rate,
        UaString& response
    ) ;
    UaStatus callConfig (
        const UaString&  conf,
        UaString& response
    ) ;
    UaStatus callInit (
        UaString& response
    ) ;
    UaStatus callStop (
        UaString& response
    ) ;
    UaStatus callCheck_laser_status (
        OpcUa_UInt16& status,
        UaString& description
    ) ;
    UaStatus callSingle_shot (
        UaString& response
    ) ;
    UaStatus callStart_standalone (
        OpcUa_Boolean fire,
        OpcUa_UInt32 num_shots,
        UaString& response
    ) ;
    UaStatus callStart_cib (
        UaString& response
    ) ;
    UaStatus callSwitch_laser_shutter (
        OpcUa_Boolean close,
        UaString& response
    ) ;
    UaStatus callForce_ext_shutter (
        OpcUa_Boolean close,
        UaString& response
    ) ;
    UaStatus callTerminate (
        UaString& response
    ) ;
    UaStatus callStop_cib (
        UaString& response
    ) ;
    UaStatus callPause (
        UaString& response
    ) ;
    UaStatus callStandby (
        UaString& response
    ) ;
    UaStatus callResume (
        UaString& response
    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DIoLLaserUnit( const DIoLLaserUnit& other );
    DIoLLaserUnit& operator=(const DIoLLaserUnit& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    // sError is a special case of error
    // when we reach this state the whole thing shuts down. We cannot really do anything
    // basically, sError means that manual intervention (and quite likely restart of the slow control server)
    // is necessary
    // this basically means that an irrecoverable error state was reached and manual intervention is necessary.
    enum Status{sOffline=0x0,sReady=1,sWarmup=2,sLasing=3,sPause = 4,sStandby=5, sError=6};
    enum ShutterState{sOpen=0x0,sClose=0x1};

    /**
      The transition diagram can be tricky here: ,--> sStandby
      sOffline -> sReady -> sWarmup ---> sLasing ---> sPause
                                    `
                                     `--> sStandby --> sLasing
                                      `
                                       `--> sPause  --> sLasing
                                                    `-> sStandby

     */

    // bitfield with the settings that are already configured
    typedef struct {
      uint8_t hv :1;
      uint8_t r_div :1;
      uint8_t rate : 1;
      uint8_t qsw : 1;
      void init(uint8_t v) {hv = v & 0x1; r_div = v & 0x2; rate = v & 0x4; qsw = v & 0x8;};
    } laser_word;
    //
    typedef union conf_word
    {
      laser_word word;
      bool is_ready() {return (*reinterpret_cast<uint8_t*>(&word) == 0xF);};
    } conf_word;
    //
    // the real worker methods
    //
    UaStatus set_conn(const std::string port, uint16_t baud, json &resp);
    UaStatus init(json &resp);
    UaStatus config(json &conf, json &resp);
    UaStatus stop(json &resp);
    UaStatus start_cib(json &resp);
    // these two have timeouts associated with them
    UaStatus pause(json &resp);
    UaStatus standby(json &resp);
    UaStatus single_shot(json &resp);
    UaStatus fire_standalone(uint32_t num_pulses,json &resp);
    UaStatus switch_laser_shutter(const ShutterState nstate, json &resp);
    Status get_state() const {return m_status;}
    bool good_for_operation() {return ((m_status == sPause) || (m_status == sStandby) || (m_status == sLasing));}
    UaStatus force_ext_shutter(const ShutterState nstate, json &resp);
    UaStatus terminate(json &resp);
    UaStatus resume(json &resp);
    UaStatus fire_discrete_shots(uint32_t num_pulses,json &resp );
    //
    // auxiliary methods that are called by other services
    //
    void update();
    //
    bool get_counting_flashes() {return m_count_flashes;}
    void set_counting_flashes(bool s) {m_count_flashes = s;}
    UaStatus refresh_shot_count();
    bool is_ready() {return (m_status == sReady);}
    //
private:
    // -- private methods
    void update_status(Status nst);
    void automatic_port_search();
    void refresh_status(json &resp);
    void refresh_status(void);
    bool validate_config_fragment(json &conf, json &resp);
    // methods that internally deal with the device writing logic
    UaStatus write_divider(const uint16_t v,json &resp);
    UaStatus write_rate(const double v,json &resp);
    UaStatus write_hv(const double v,json &resp);
    //UaStatus write_qswitch(const uint16_t v,json &resp);
    // to be implemented
    void set_pause_timeout(const uint32_t v,json &resp) {m_pause_timeout = v;}
    void set_standby_timeout(const uint32_t v,json &resp){m_standby_timeout = v;}
    UaStatus set_qswitch_delay(const uint32_t v,json &resp);
    UaStatus set_qswitch_width(const uint32_t v,json &resp);
    UaStatus set_fire_width(const uint32_t v,json &resp);


    //
    void start_lasing_timer();
    void start_warmup_timer();
    void start_pause_timer();
    void start_standby_timer();

    UaStatus map_registers(json &reginfo, json &resp);
    UaStatus unmap_registers();
    uint32_t get_cib_shot_count();
    //
    //
    bool m_is_ready;
    Status m_status;
    //
    device::Laser *m_laser;
    uint16_t m_divider;
    double m_pump_hv;
    double m_rate_hz;
    // cache variables for the present state of these parts
    laser_state_u m_part_state;
    // FIXME: Get rid of these
    //    bool m_laser_shutter_closed;
    //    bool m_ext_shutter_closed;
    //    bool m_qswitch_en;
    //    bool m_laser_started;

    uint32_t m_shot_count;
    //
    std::string m_comport;
    uint16_t m_baud_rate;
    bool m_count_flashes;
    //
    //
    std::map<Status,std::string> m_status_map;
    std::string m_name;
    //conf_word m_config;
    //
    // these timers are not yet set up
    // they should be set up at the IoLaserSystem level
    std::uint32_t m_idle_counter;
    std::uint32_t m_idle_timeout; // timeout after which the laser
    //
    // Variables necessary to map registers in the CIB
    //
    int m_mmap_fd;
    uintptr_t m_mapped_mem;
    // variables that are relevant for the memory mapped registers
    // we could actually make this absolutely generic, dependent on the configuration
    // of course the usage then would be different
    std::map<std::string,laser_regs_t> m_regs;
    //
    //
    uint32_t m_pause_timeout;
    uint32_t m_standby_timeout;
    uint32_t m_qswitch_delay;
    uint32_t m_qswitch_width;
    uint32_t m_fire_width;
    std::string m_serial_number;
    uint32_t m_warmup_timer;


};

}

#endif // __DIoLLaserUnit__H__
