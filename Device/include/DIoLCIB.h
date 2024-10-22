
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


#ifndef __DIoLCIB__H__
#define __DIoLCIB__H__

#include <Base_DIoLCIB.h>
#include <json.hpp>
using json = nlohmann::json;
#include <atomic>
#include <AD5339.h>

namespace Device
{

class
    DIoLCIB
    : public Base_DIoLCIB
{

public:
    /* sample constructor */
    explicit DIoLCIB (
        const Configuration::IoLCIB& config,
        Parent_DIoLCIB* parent
    ) ;
    /* sample dtr */
    ~DIoLCIB ();

    /* delegators for
    cachevariables and sourcevariables */
    /* Note: never directly call this function. */
    UaStatus writeDac_threshold ( const OpcUa_UInt16& v);


    /* delegators for methods */
    UaStatus callSet_dac_threshold (
        OpcUa_UInt16 dac_level,
        UaString& response
    ) ;
    UaStatus callReset_pdts (
        UaString& response
    ) ;
    UaStatus callSet_trigger_pulser (
        OpcUa_Boolean enabled,
        UaString& response
    ) ;
    UaStatus callSet_trigger_external (
        OpcUa_Boolean enabled,
        UaString& response
    ) ;
    UaStatus callSet_lbls_width (
        OpcUa_UInt32 width,
        UaString& response
    ) ;
    UaStatus callSet_lbls_state (
        OpcUa_Boolean enabled,
        UaString& response
    ) ;
    UaStatus callSet_align_state (
        OpcUa_Boolean enabled,
        UaString& response
    ) ;
    UaStatus callSet_align_params (
        OpcUa_UInt32 width_clocks,
        OpcUa_UInt32 period_clocks,
        UaString& response
    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DIoLCIB( const DIoLCIB& other );
    DIoLCIB& operator=(const DIoLCIB& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    // structure describing memory maps
    typedef struct cib_gpio_t
    {
      uintptr_t paddr;
      uintptr_t vaddr;
      size_t size;
      int id;
    } cib_gpio_t;

    typedef struct conf_param_t
    {
      cib_gpio_t reg;
      uintptr_t offset;
      uint16_t bit_low;
      uint16_t bit_high;
      uintptr_t maddr; // maddr is the memory mapped address of this particular register
      uint32_t mask;
      size_t n_bits;
    } conf_param_t;
    //
    enum Status{sOffline=0x0,sReady=2,sError=3};
    //
    bool is_ready() {return (m_status == sReady);}
    void update();
    UaStatus terminate(json &resp);
    UaStatus set_id(const std::string &id);
    const std::string get_id() {return m_id;}
    void refresh_dac();
    int init_dac();
    UaStatus config(json &conf, json &resp);
private:
    void poll_cpu();
    void poll_mem();
    void refresh_pdts();
    UaStatus set_dac_threshold(uint16_t &val,json &resp);
    UaStatus init_cib_mem();
    UaStatus map_registers(json &conf,json &resp);
    UaStatus validate_registers(json &conf,json &resp);
    UaStatus validate_config_fragment(json &conf, json &resp);
    UaStatus reset_pdts(json &resp);
    // internal methods specific to the CIB
    UaStatus cib_pdts_status(uintptr_t &addr,uint8_t &pdts_stat, uint8_t &pdts_addr);
    UaStatus cib_pdts_reset(conf_param_t &reg, json&resp);
    UaStatus get_cib_trigger_pulser_state(conf_param_t &reg, uint32_t &state);
    UaStatus set_cib_trigger_pulser_state(conf_param_t &reg, uint32_t &state);
    UaStatus get_cib_trigger_external_state(conf_param_t &reg, uint32_t &state);
    UaStatus set_cib_trigger_external_state(conf_param_t &reg, uint32_t &state);
    UaStatus get_cib_daq_queue_state(conf_param_t &reg, bool &enabled)  ;
    UaStatus get_cib_lbls_queue_state(conf_param_t &reg, bool &enabled) ;
    UaStatus get_cib_lbls_width(conf_param_t &reg, uint32_t &value)     ;
    UaStatus set_cib_lbls_width(conf_param_t &reg, const uint32_t value);
    UaStatus get_cib_align_state(conf_param_t &reg, bool &enabled)      ;
    UaStatus set_cib_align_state(conf_param_t &reg, bool &enabled)      ;
    UaStatus set_cib_align_width(conf_param_t &reg, uint32_t &width)    ;
    UaStatus get_cib_align_width(conf_param_t &reg, uint32_t &width)    ;
    UaStatus set_cib_align_period(conf_param_t &reg, uint32_t &value)   ;
    UaStatus get_cib_align_period(conf_param_t &reg, uint32_t &value)   ;
    //
    UaStatus get_cib_state(conf_param_t &reg, bool &enabled);
    UaStatus get_cib_value(conf_param_t &reg, uint32_t &value);
    UaStatus set_cib_value(conf_param_t &reg, const uint32_t value);
    void refresh_registers();
    UaStatus check_cib_mem(json &resp);
    //
    void update_status(Status new_state);
    // -- internal variable cache
    uint32_t m_align_width;
    uint32_t m_align_period;
    bool m_align_enabled;
    uint32_t m_lbls_width;
    bool m_lbls_enabled;
    bool m_trigger_pulser_enabled;
    bool m_trigger_external_enabled;
    bool m_daq_queue_enabled;

    bool m_is_ready;
    float m_cpu_load;
    float m_used_mem;
    //
    //
    //
    long long unsigned int m_prev_tot_usr;
    long long unsigned int m_prev_tot_usr_low;
    long long unsigned int m_prev_tot_sys;
    long long unsigned int m_prev_tot_idle;
    long long unsigned int m_total;
    std::string m_id;
    int m_mmap_fd;
    std::map<int,cib_gpio_t> m_reg_map;
    std::map<std::string,conf_param_t> m_regs;
    std::map<Status,std::string> m_status_map;
    Status m_status;

    cib::i2c::AD5339 m_dac;

};

}

#endif // __DIoLCIB__H__
