<!-- markdownlint-disable MD024 -->
# OPC-UA Server Object Model Reference

This document summarizes the OPC-UA information model defined in:

- `Design/Design.xml`
- `bin/config.xml`
- `build/Design/Design.dot` (generated overview)

It is intended as a practical reference for integration work and for pruning monitored items in the OPC-UA client.

## 1) Normal operation scope

For normal operation, only methods under `IoLaserSystem` (`LS1`) should be used by the control client.

Methods under all other objects (`IoLMotor`, `IoLPowerMeter`, `IoLCIB`, `IoLLaserUnit`, `IoLAttenuator`) are considered low-level/debug interfaces and are typically used only for diagnostics, standalone checks, or subsystem-level troubleshooting.

### 1.1 Server diagram

- Packaged diagram (copied from build output): [OPCUA_Server_Diagram.pdf](OPCUA_Server_Diagram.pdf)

---

## 2) Server structure

### 2.1 Root hierarchy (design-level)

`ROOT`

- `IoLaserSystem` (configured object) - Wrapper object
  - `IoLLaserUnit` (exactly 1)
  - `IoLMotor` (exactly 3)
  - `IoLAttenuator` (exactly 1)
  - `IoLCIB` (exactly 1)
  - `IoLPowerMeter` (exactly 1)

### 2.2 Current configured instances (`bin/config.xml`)

- `IoLaserSystem`: `LS1`
  - `IoLMotor`: `RNN800`, `RNN600`, `LSTAGE`
  - `IoLAttenuator`: `A1`
  - `IoLLaserUnit`: `L1`
  - `IoLCIB`: `CIB1`
    - Calculated variable: `dac_mV = LS1.CIB1.dac_threshold*2500/4095`
  - `IoLPowerMeter`: `PM1`

---

## 3) Monitored items inventory (for OPC-UA client pruning)

This section lists the variables that one would like to monitor from the slow control client. The list is still being reviewed “flat” so you can delete lines as you decide what not to integrate.

### 3.1 Browse path patterns (by class)

- `LS1.state`
- `LS1.iols_active`
- `LS1.iols_allowed`

- `LS1.RNN600.target_position`
- `LS1.RNN600.current_position_cib`
- `LS1.RNN600.is_moving`
- `LS1.RNN600.torque`
- `LS1.RNN600.temperature_C`
- `LS1.RNN600.alarm_code`
- `LS1.RNN600.speed_readout`
- `LS1.RNN600.state`

- `LS1.LSTAGE.target_position`
- `LS1.LSTAGE.current_position_cib`
- `LS1.LSTAGE.is_moving`
- `LS1.LSTAGE.torque`
- `LS1.LSTAGE.temperature_C`
- `LS1.LSTAGE.alarm_code`
- `LS1.LSTAGE.speed_readout`
- `LS1.LSTAGE.state`

- `LS1.A1.position`
- `LS1.A1.offset`
- `LS1.A1.transmission`
- `LS1.A1.motor_state`
- `LS1.A1.state`

- `LS1.L1.qswitch_delay_us`
- `LS1.L1.qswitch_width_us`
- `LS1.L1.discharge_voltage_kV`
- `LS1.L1.flash_count`
- `LS1.L1.shot_count_cib`
- `LS1.L1.ext_shutter_open`
- `LS1.L1.laser_shutter_open`
- `LS1.L1.laser_status_code`
- `LS1.L1.state`
- `LS1.L1.standby_timer_s`
- `LS1.L1.pause_timer_s`
- `LS1.L1.warmup_timer_s`
- `LS1.L1.fire_active`
- `LS1.L1.qswitch_active`

- `LS1.CIB1.state`
- `LS1.CIB1.dac_threshold`
- `LS1.CIB1.dac_mV` *(calculated variable from config)*
- `LS1.CIB1.pdts_state`
- `LS1.CIB1.pdts_address`
- `LS1.CIB1.daq_queue_enabled`

- `LS1.PM1.range_selected`
- `LS1.PM1.wavelength`
- `LS1.PM1.energy_reading`
- `LS1.PM1.trigger_threshold`
- `LS1.PM1.pulse_width`
- `LS1.PM1.state`

### 3.2 Expanded current motor list (as configured)

There are 3 configured motors (`RNN800`, `RNN600`, `LSTAGE`), but  __there is only need to monitor motors `RNN600` and `LSTAGE`__. Motor `RNN800` is not installed in NP02 (and will be dropped from the design for DUNE-FD).

---

---

## 4) Integration notes

- If you want an initial “lean” subscription profile, keep status/state/position/readout variables first and defer static/config-like fields (`*_options`, baud/port details, etc.).
- Write-enabled variables are limited (`delegated`/`regular`) and are explicitly listed in each class table.
- `reset()` in `IoLPowerMeter` is modeled with no return value (`void`).
- All methods return success with the real feedback being returned in the `response` string (that is actually a json object), which can be parsed by the client for more structured handling if desired. This is due to a limitation of the OPC-UA method call interface, which only allows a single return value and does not fill any output arguments if the method returns an error status code, making the client blind to any failure feedback.

---

## 5) Object reference (parameters and methods)

Notes:

- __Write policy__ maps from `addressSpaceWrite`: `forbidden`, `delegated`, `regular`.
- `id` is a key `configentry` for all object classes.
- “Monitored candidate” means cache variable suitable for OPC-UA client subscription.

### 5.1 `IoLaserSystem`

#### Cache variables

| Name | Type | Write policy | Initial value | Initial status | Monitored candidate |
| --- | --- | --- | --- | --- | --- |
| `state` | `UaString` | `forbidden` | `unknown` | `OpcUa_BadWaitingForInitialData` | Yes |
| `iols_active` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |
| `iols_allowed` | `OpcUa_Boolean` | `regular` | `OpcUa_False` | `OpcUa_Good` | Yes |

#### Methods

- `load_config(UaString conf) -> UaString response` — Loads and applies a full system configuration payload.
- `check_ready() -> OpcUa_Boolean ready` — Returns whether the system is in a ready-to-operate state.
- `stop() -> UaString response` — Issues an immediate coordinated stop to all subsystems.
- `fire_at_position(UaString arguments) -> UaString answer` — Executes a fire command at one specified position.
- `fire_segment(UaString arguments) -> UaString answer` — Executes a segment-based firing sequence.
- `execute_scan(UaString plan) -> UaString answer` — Runs a scan plan over configured points/paths.
- `execute_grid_scan(UaString plan) -> UaString answer` — Runs a 2D/grid-oriented scan plan.
- `pause() -> UaString answer` — Temporarily pauses active coordinated operations.
- `standby() -> UaString answer` — Transitions system to standby mode.
- `resume() -> UaString response` — Resumes operation after pause/standby when possible.
- `warmup_laser() -> UaString response` — Starts the laser warmup sequence.
- `shutdown() -> UaString response` — Performs an orderly system shutdown sequence.
- `move_to_pos(UaString arguments) -> UaString response` — Moves motion subsystem to a requested position setpoint.
- `clear_error() -> UaString response` — Clears recoverable faults/alarms in the coordinated controller.

### 5.2 `IoLMotor`

#### Cache variables

| Name | Type | Write policy | Initial value | Initial status | Monitored candidate |
| --- | --- | --- | --- | --- | --- |
| `server_addr` | `UaString` | `forbidden` | `localhost` | `OpcUa_Bad` | Yes |
| `server_port` | `OpcUa_UInt16` | `forbidden` | `5000` | `OpcUa_Bad` | Yes |
| `range_min` | `OpcUa_Int32` | `delegated` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `range_max` | `OpcUa_Int32` | `delegated` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `target_position` | `OpcUa_Int32` | `forbidden` | `-99999` | `OpcUa_BadWaitingForInitialData` | Yes |
| `current_position_motor` | `OpcUa_Int32` | `forbidden` | *(none)* | `OpcUa_BadWaitingForInitialData` | Yes |
| `current_position_cib` | `OpcUa_Int32` | `forbidden` | *(none)* | `OpcUa_BadWaitingForInitialData` | Yes |
| `refresh_period_ms` | `OpcUa_UInt16` | `delegated` | `5` | `OpcUa_Good` | Yes |
| `is_moving` | `OpcUa_Boolean` | `forbidden` | *(none)* | `OpcUa_BadWaitingForInitialData` | Yes |
| `torque` | `OpcUa_Double` | `forbidden` | *(none)* | `OpcUa_BadWaitingForInitialData` | Yes |
| `temperature_C` | `OpcUa_Double` | `forbidden` | *(none)* | `OpcUa_BadWaitingForInitialData` | Yes |
| `alarm_code` | `OpcUa_Int32` | `forbidden` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `acceleration` | `OpcUa_UInt32` | `delegated` | `100` | `OpcUa_BadWaitingForInitialData` | Yes |
| `deceleration` | `OpcUa_UInt32` | `delegated` | `100` | `OpcUa_BadWaitingForInitialData` | Yes |
| `speed_readout` | `OpcUa_Int32` | `forbidden` | *(none)* | `OpcUa_BadWaitingForInitialData` | Yes |
| `speed` | `OpcUa_UInt32` | `delegated` | `200` | `OpcUa_BadWaitingForInitialData` | Yes |
| `state` | `UaString` | `forbidden` | `unknown` | `OpcUa_BadWaitingForInitialData` | Yes |
| `enabled` | `OpcUa_Boolean` | `forbidden` | `OpcUa_True` | `OpcUa_BadWaitingForInitialData` | Yes |

#### Methods

- `config(UaString config_json) -> UaString response` — Applies motor-specific configuration parameters from JSON.
- `move_absolute(OpcUa_Int32 destination) -> UaString response` — Commands an absolute move to target position.
- `move_relative(OpcUa_Int32 num_steps) -> UaString response` — Commands a relative move from current position.
- `stop() -> UaString response` — Stops motor motion as quickly as allowed by controller.
- `reset() -> UaString response` — Resets motor/controller state machine.
- `clear_alarm() -> UaString response` — Clears active motor alarm condition when permitted.

### 5.3 `IoLPowerMeter`

#### Cache variables

| Name | Type | Write policy | Initial value | Initial status | Monitored candidate |
| --- | --- | --- | --- | --- | --- |
| `port` | `UaString` | `forbidden` | `auto` | `OpcUa_BadWaitingForInitialData` | Yes |
| `baud_rate` | `OpcUa_UInt16` | `forbidden` | `9800` | `OpcUa_Good` | Yes |
| `range_selected` | `OpcUa_Int16` | `forbidden` | `2` | `OpcUa_Good` | Yes |
| `measurement_mode` | `OpcUa_UInt16` | `forbidden` | `0` | `OpcUa_Good` | Yes |
| `wavelength` | `OpcUa_UInt16` | `forbidden` | `266` | `OpcUa_Good` | Yes |
| `average_window` | `OpcUa_UInt16` | `forbidden` | `1` | `OpcUa_Good` | Yes |
| `energy_reading` | `OpcUa_Double` | `forbidden` | `0.0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `average_reading` | `OpcUa_Double` | `forbidden` | `0.0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `trigger_threshold` | `OpcUa_UInt16` | `forbidden` | `1` | `OpcUa_Good` | Yes |
| `pulse_width` | `OpcUa_UInt16` | `forbidden` | `1` | `OpcUa_Good` | Yes |
| `state` | `UaString` | `forbidden` | `unknown` | `OpcUa_BadWaitingForInitialData` | Yes |
| `range_options` | `UaString` | `forbidden` | `invalid` | `OpcUa_BadWaitingForInitialData` | Yes |
| `measurement_options` | `UaString` | `forbidden` | `invalid` | `OpcUa_BadWaitingForInitialData` | Yes |
| `average_options` | `UaString` | `forbidden` | `invalid` | `OpcUa_BadWaitingForInitialData` | Yes |
| `pulse_length_options` | `UaString` | `forbidden` | `invalid` | `OpcUa_BadWaitingForInitialData` | Yes |
| `measurement_interval_ms` | `OpcUa_UInt16` | `regular` | `500` | `OpcUa_Good` | Yes |

#### Methods

- `init() -> UaString response` — Initializes the power meter and validates communication.
- `set_connection(UaString port, OpcUa_UInt16 baud_rate) -> UaString response` — Sets serial connection parameters.
- `set_average(OpcUa_UInt16 target_value) -> UaString response` — Updates averaging window setting.
- `set_range(OpcUa_Int16 target_value) -> UaString response` — Selects measurement range.
- `set_pulse_width(OpcUa_UInt16 target_value) -> UaString response` — Sets pulse width parameter for trigger logic.
- `set_threshold(OpcUa_UInt16 target_value) -> UaString response` — Sets trigger threshold for detection.
- `set_wavelength(OpcUa_UInt16 target_value) -> UaString response` — Sets operating wavelength parameter.
- `set_measurement_mode(OpcUa_UInt16 target_value) -> UaString response` — Selects measurement mode.
- `reset() -> void` — Resets instrument state using device reset routine.
- `config(UaString conf) -> UaString response` — Applies grouped configuration from a serialized config payload.
- `stop_measurements() -> UaString response` — Stops periodic/continuous measurements.
- `start_measurements() -> UaString response` — Starts periodic/continuous measurements.
- `terminate() -> UaString response` — Closes/tears down active power-meter session.

### 5.4 `IoLCIB`

#### Cache variables

| Name | Type | Write policy | Initial value | Initial status | Monitored candidate |
| --- | --- | --- | --- | --- | --- |
| `mem_load` | `OpcUa_Float` | `forbidden` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `cpu_load` | `OpcUa_Float` | `forbidden` | `-1.0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `state` | `UaString` | `forbidden` | `unknown` | `OpcUa_BadWaitingForInitialData` | Yes |
| `external_interlock` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |
| `dac_threshold` | `OpcUa_UInt16` | `delegated` | `4095` | `OpcUa_BadWaitingForInitialData` | Yes |
| `pdts_state` | `OpcUa_Byte` | `forbidden` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `pdts_address` | `OpcUa_Byte` | `forbidden` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `trigger_pulser_enabled` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |
| `trigger_external_enabled` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |
| `daq_queue_enabled` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |
| `lbls_queue_enabled` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |
| `lbls_pulse_width_clocks` | `OpcUa_UInt32` | `forbidden` | `2` | `OpcUa_BadWaitingForInitialData` | Yes |
| `align_laser_enabled` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |
| `align_laser_width` | `OpcUa_UInt32` | `forbidden` | `10` | `OpcUa_BadWaitingForInitialData` | Yes |
| `align_laser_period` | `OpcUa_UInt32` | `forbidden` | `6250000` | `OpcUa_BadWaitingForInitialData` | Yes |

Additional calculated variable (from config):

- `dac_mV` (expression-backed in `config.xml`, under `LS1.CIB1`)

#### Methods

- `set_dac_threshold(OpcUa_UInt16 dac_level) -> UaString response` — Sets CIB DAC threshold value.
- `reset_pdts() -> UaString response` — Resets PDTS interface/state on the CIB side.
- `set_trigger_pulser(OpcUa_Boolean enabled) -> UaString response` — Enables/disables internal trigger pulser.
- `set_trigger_external(OpcUa_Boolean enabled) -> UaString response` — Enables/disables external trigger path.
- `set_lbls_width(OpcUa_UInt32 width) -> UaString response` — Sets LBLS pulse width in clock units.
- `set_lbls_state(OpcUa_Boolean enabled) -> UaString response` — Enables/disables LBLS queue/trigger activity.
- `set_align_state(OpcUa_Boolean enabled) -> UaString response` — Enables/disables alignment laser mode.
- `set_align_params(OpcUa_UInt32 width_clocks, OpcUa_UInt32 period_clocks) -> UaString response` — Configures alignment pulse width/period.

### 5.5 `IoLLaserUnit`

#### Cache variables

| Name | Type | Write policy | Initial value | Initial status | Monitored candidate |
| --- | --- | --- | --- | --- | --- |
| `port` | `UaString` | `forbidden` | `auto` | `OpcUa_Good` | Yes |
| `baud_rate` | `OpcUa_UInt16` | `forbidden` | `9600` | `OpcUa_Good` | Yes |
| `qswitch_delay_us` | `OpcUa_UInt32` | `forbidden` | `170` | `OpcUa_BadWaitingForInitialData` | Yes |
| `qswitch_width_us` | `OpcUa_UInt32` | `forbidden` | `10` | `OpcUa_BadWaitingForInitialData` | Yes |
| `fire_width_us` | `OpcUa_UInt32` | `forbidden` | `10` | `OpcUa_BadWaitingForInitialData` | Yes |
| `discharge_voltage_kV` | `OpcUa_Double` | `delegated` | `1.1` | `OpcUa_BadWaitingForInitialData` | Yes |
| `rep_rate_hz` | `OpcUa_Double` | `delegated` | `10.0` | `OpcUa_Good` | Yes |
| `rep_rate_divider` | `OpcUa_UInt32` | `delegated` | `0` | `OpcUa_Good` | Yes |
| `flash_count` | `OpcUa_UInt32` | `forbidden` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `shot_count_cib` | `OpcUa_UInt32` | `forbidden` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `ext_shutter_open` | `OpcUa_Boolean` | `forbidden` | `OpcUa_True` | `OpcUa_BadWaitingForInitialData` | Yes |
| `laser_shutter_open` | `OpcUa_Boolean` | `forbidden` | *(none)* | `OpcUa_BadWaitingForInitialData` | Yes |
| `laser_status_code` | `OpcUa_UInt16` | `forbidden` | `999` | `OpcUa_BadWaitingForInitialData` | Yes |
| `state` | `UaString` | `forbidden` | `unknown` | `OpcUa_BadWaitingForInitialData` | Yes |
| `standby_timer_s` | `OpcUa_UInt32` | `forbidden` | `0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `pause_timer_s` | `OpcUa_UInt32` | `forbidden` | `0` | `OpcUa_Good` | Yes |
| `warmup_target_min` | `OpcUa_UInt32` | `delegated` | `20` | `OpcUa_Good` | Yes |
| `warmup_timer_s` | `OpcUa_UInt32` | `forbidden` | `0` | `OpcUa_Good` | Yes |
| `fire_active` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |
| `qswitch_active` | `OpcUa_Boolean` | `forbidden` | `OpcUa_False` | `OpcUa_BadWaitingForInitialData` | Yes |

#### Methods

- `set_connection(UaString device_port, OpcUa_UInt16 baud_rate) -> UaString response` — Sets serial connection parameters for the laser controller.
- `config(UaString conf) -> UaString response` — Applies grouped laser-unit configuration settings.
- `init() -> UaString response` — Initializes laser communication and base operating state.
- `stop() -> UaString response` — Stops active laser operation sequence.
- `check_laser_status() -> (OpcUa_UInt16 status, UaString description)` — Reads status code plus textual interpretation.
- `single_shot() -> UaString response` — Requests one laser shot.
- `start_standalone(OpcUa_Boolean fire, OpcUa_UInt32 num_shots) -> UaString response` — Starts standalone firing mode with optional shot count.
- `start_cib() -> UaString response` — Starts laser operation synchronized with CIB workflow.
- `switch_laser_shutter(OpcUa_Boolean close) -> UaString response` — Opens/closes the laser internal shutter.
- `force_ext_shutter(OpcUa_Boolean close) -> UaString response` — Forces external shutter state.
- `terminate() -> UaString response` — Terminates laser-unit session and releases resources.
- `stop_cib() -> UaString response` — Stops CIB-synchronized laser operation.
- `pause() -> UaString response` — Pauses laser-unit operation timers/actions.
- `standby() -> UaString response` — Places laser unit in standby state.
- `resume() -> UaString response` — Resumes from paused/standby laser-unit state.

### 5.6 `IoLAttenuator`

#### Cache variables

| Name | Type | Write policy | Initial value | Initial status | Monitored candidate |
| --- | --- | --- | --- | --- | --- |
| `resolution_setting` | `OpcUa_UInt16` | `forbidden` | `65535` | `OpcUa_BadWaitingForInitialData` | Yes |
| `device_port` | `UaString` | `forbidden` | `auto` | `OpcUa_BadWaitingForInitialData` | Yes |
| `baud_rate` | `OpcUa_UInt16` | `forbidden` | `38400` | `OpcUa_BadWaitingForInitialData` | Yes |
| `position` | `OpcUa_Int32` | `forbidden` | `2147483647` | `OpcUa_BadWaitingForInitialData` | Yes |
| `acceleration` | `OpcUa_UInt16` | `forbidden` | `65535` | `OpcUa_BadWaitingForInitialData` | Yes |
| `deceleration` | `OpcUa_UInt16` | `forbidden` | `65535` | `OpcUa_BadWaitingForInitialData` | Yes |
| `max_speed` | `OpcUa_UInt16` | `forbidden` | `65535` | `OpcUa_BadWaitingForInitialData` | Yes |
| `offset` | `OpcUa_UInt16` | `forbidden` | `65535` | `OpcUa_BadWaitingForInitialData` | Yes |
| `idle_current_setting` | `OpcUa_UInt16` | `forbidden` | `65535` | `OpcUa_BadWaitingForInitialData` | Yes |
| `moving_current_setting` | `OpcUa_UInt16` | `forbidden` | `65535` | `OpcUa_BadWaitingForInitialData` | Yes |
| `transmission` | `OpcUa_Double` | `forbidden` | `-1.0` | `OpcUa_BadWaitingForInitialData` | Yes |
| `motor_state` | `OpcUa_UInt16` | `forbidden` | `65535` | `OpcUa_BadWaitingForInitialData` | Yes |
| `state` | `UaString` | `forbidden` | `unknown` | `OpcUa_BadWaitingForInitialData` | Yes |
| `resolution_options` | `UaString` | `forbidden` | `N/A` | `OpcUa_BadWaitingForInitialData` | Yes |
| `motor_state_options` | `UaString` | `forbidden` | `N/A` | `OpcUa_BadWaitingForInitialData` | Yes |

#### Methods

- `configure_attenuator(UaString json_config) -> UaString response` — Applies attenuator configuration from JSON payload.
- `set_transmission(OpcUa_Double transmission) -> UaString response` — Sets optical transmission target.
- `set_position(OpcUa_UInt32 position) -> UaString response` — Commands attenuator to an absolute position.
- `set_conn_details(UaString port, OpcUa_UInt16 baud_rate) -> UaString response` — Sets attenuator serial connection details.
- `stop() -> UaString response` — Stops attenuator motion.
- `set_resolution(OpcUa_UInt16 resolution_setting) -> UaString response` — Sets attenuator resolution mode.
- `set_current(OpcUa_UInt16 idle_setting, OpcUa_UInt16 moving_setting) -> UaString response` — Sets idle/moving current settings.
- `set_acceleration(OpcUa_UInt16 acceleration) -> UaString response` — Sets acceleration parameter.
- `set_deceleration(OpcUa_UInt16 deceleration) -> UaString response` — Sets deceleration parameter.
- `set_max_speed(OpcUa_UInt16 max_speed) -> UaString response` — Sets maximum speed parameter.
- `get_status() -> UaString response` — Requests current attenuator status summary.
- `set_calibration_parameters(OpcUa_Double offset, OpcUa_Double scale) -> UaString response` — Sets calibration offset and scale factors.

---
