#ifndef IOLS_CLIENT_FUNCTIONS_H
#define IOLS_CLIENT_FUNCTIONS_H

#ifdef UA_LOGLEVEL
#undef UA_LOGLEVEL
#endif
#define UA_LOGLEVEL 601

extern "C"
{
#include <open62541.h>
}

#include <spdlog/spdlog.h>
#include <json.hpp>
#include <vector>
#include <string>
#include <cstdint>

extern UA_Client *g_client;

using json = nlohmann::json;
//FIXME: These structures should be moved to a common header file under cib_utils
/**
 * Structures to help manage the system
 */
typedef struct motor_position_t
{
  int32_t motor;
  int32_t cib;
} motor_position_t;

typedef struct iols_motor_mon_t
{
  std::string state;
  int32_t position_motor;
  int32_t position_cib;
} iols_motor_mon_t;
typedef struct iols_laser_mon_t
{
  std::string state;
  int32_t power;
} iols_laser_mon_t;
typedef struct iols_att_mon_t
{
  std::string state;
  int32_t position;
} iols_att_mon_t;
typedef struct iols_pm_mon_t
{
  std::string state;
  double latest_reading;
  double average_reading;
} iols_pm_mon_t;
typedef struct iols_monitor_t
{
  iols_motor_mon_t m1;    // rnn800
  iols_motor_mon_t m2;    // rnn600
  iols_motor_mon_t m3;    // lstage
  iols_laser_mon_t laser; // laser
  iols_att_mon_t att;     // attenuator
  iols_pm_mon_t pm;       // power meter
  std::string iols_state;
} iols_monitor_t;

#endif