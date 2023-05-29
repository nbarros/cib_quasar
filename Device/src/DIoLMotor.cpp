
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

#include <DIoLMotor.h>
#include <ASIoLMotor.h>
#include <sstream>

extern "C" {
#include <curl/curl.h>
};

#include <chrono>
#include <thread>
#include <functional>
#include <string>
#include <random>

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
DIoLMotor::DIoLMotor (
		const Configuration::IoLMotor& config,
		Parent_DIoLMotor* parent
):
    				Base_DIoLMotor( config, parent)

					/* fill up constructor initialization list here */
					,m_sim_pos(0)
					,m_sim_speed(0)
					,m_sim_tpos(0)
					,m_sim_moving(false)
					,m_position(-99999)
					,m_position_setpoint(-99999)
					,m_position_setpoint_status(OpcUa_BadWaitingForInitialData)
					, m_is_ready(false)
					,is_moving_(false)
					,m_acceleration(0.0)
					,m_deceleration(0.0)
					,m_speed_setpoint(0.0)
					,m_speed_readout(0.0)
					,m_torque(0.0)
					,m_temperature(0.0)
					//,m_address("")
					,m_refresh_ms(0)
					,m_monitor(false)
					,m_monitor_status(OpcUa_BadResourceUnavailable)
					,m_server_host("")
					,m_server_port(0)
					{
	/* fill up constructor body here */
	// initialize cURL
	curl_global_init(CURL_GLOBAL_ALL);

	// Provide whatever clients a useful, crappy value for
	// everything else will be processed in contained form
	//getAddressSpaceLink()->setPositionSetPoint(m_position_setpoint, OpcUa_BadWaitingForInitialData);

#ifdef SIMULATION
	LOG(Log::WRN)<< "***!!! RUNNING IN SIMULATION MODE !!!***";
#endif
	//
	if (m_monitor)
	{
		if (m_refresh_ms != 0.0)
		{
			timer_start(this);
		}
	}
}

/* sample dtr */
DIoLMotor::~DIoLMotor ()
{
	curl_global_cleanup();

}

/* delegates for cachevariables */

/* Note: never directly call this function. */

UaStatus DIoLMotor::writePositionSetPoint ( const OpcUa_Int32& v)
{

	// check ranges
	if ((v < -50000) || (v > 50000))
	{
		LOG(Log::ERR) << "Value out of range for positionSetPoint. Offending value = " << v;
		return OpcUa_BadOutOfRange;
	}

	LOG(Log::INF) << "Updating motor ID=" << id() << " with positionSetPoint = (" << v << ")";
	m_position_setpoint = v;
	if (m_position_setpoint_status != OpcUa_Good)
	{
		m_position_setpoint_status = OpcUa_Good;
	}

	// TODO: Should one allow the position setpoint to be updated before the destination has been set?
	// note that the movement itself is not initiated until the move_motor method is called
	// on the other hand, if we allow this to be set we can no longer keep track of the current destination


	return OpcUa_Good;
}
/* Note: never directly call this function. */

UaStatus DIoLMotor::writeRefresh_period_ms ( const OpcUa_UInt32& v)
{
	m_refresh_ms = v;
	LOG(Log::INF) << "Updating refresh rate for motor " << id() << " to " << v << " ms";
	if (v != 0 && !m_monitor)
	{
		LOG(Log::INF) << "Starting a timer on motor " << id() << " to refresh every " << v << " ms";
		timer_start(this);
	}
	else if (v == 0)
	{
		LOG(Log::WRN) << "Stopping the monitor timer" ;
		m_monitor = false;
	}

	return OpcUa_Good;
}
/* delegators for methods */

UaStatus DIoLMotor::callStart_move (UaString& response)
{
	json resp;

	// the returned status is always OpcUa_Good
	// but the real execution status is passed through the json response
	UaStatus st = OpcUa_Good;

	if (!is_ready())
	{
		resp["status"] = "ERROR";

		std::ostringstream msg("");
		msg << log_e("start_move","Motor is not ready to operate");
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_BadInvalidState;

		LOG(Log::ERR) << msg.str();
		response = UaString(resp.dump().c_str());
		return OpcUa_Good;
	}

	// -- if we don't have a good monitoring status we cannot be sure of
	// what is the status of the motor downstream.
	// therefore, refuse any movement operation
	if (m_monitor_status != OpcUa_Good)
	{
		resp["status"] = "ERROR";
		std::ostringstream msg("");
		msg << log_e("start_move","Motor is in an unknown state");
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_BadInvalidState;
		LOG(Log::ERR) << msg.str();
		response = UaString(resp.dump().c_str());
		return OpcUa_Good;

	}

	if (m_position_setpoint_status != OpcUa_Good)
	{
		resp["status"] = "ERROR";
		std::ostringstream msg("");
		msg << log_e("start_move","Target position in invalid state") << UA_StatusCode_name(m_position_setpoint_status);
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_BadInvalidState;

		LOG(Log::ERR) << msg.str();
		response = UaString(resp.dump().c_str());
		return OpcUa_Good;
	}

	// check that position and position_set_point are not the same
	if (m_position == m_position_setpoint)
	{
		resp["status"] = "OK";
		std::ostringstream msg("");
		msg << log_w("start_move","Motor is already at destination") << " (" << m_position << " vs " << m_position_setpoint << ")";
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_Good;

		LOG(Log::ERR) << msg.str();
		response = UaString(resp.dump().c_str());
		return OpcUa_Good;
	}

	//
	//	int32_t psp;
	//	UaStatus ss = getAddressSpaceLink()->getPositionSetPoint(psp);
	//	LOG(Log::INF) << "Current state of positionSetPoint ---> " << ss << " " << ss.toString().toUtf8();
	//	if (ss != OpcUa_Good)
	//	{
	//		resp["status"] = "ERROR";
	//		std::string msg = "Invalid target position";
	//		resp["messages"].push_back(msg);
	//		LOG(Log::ERR) << msg;
	//
	//		response = UaString(resp.dump().c_str());
	//		return OpcUa_BadInvalidState;
	//	}
	// if it reaches this point we can initiate the movement
#ifdef SIMULATION
	st = sim_move_motor(resp);
#else
	st = move_motor(resp);
#endif

	response = UaString(resp.dump().c_str());
	return st;

}

UaStatus DIoLMotor::callStop (
		UaString& response
)
{
	// stopping is a serious business. Should be called immediately
	json resp;
	UaStatus st = OpcUa_Good;
#ifdef SIMULATION
	st = sim_stop_motor(resp);
#else
	st = stop_motor(resp);
#endif

	if (st != OpcUa_Good)
	{
		LOG(Log::ERR) << "Remote command execution failed.";
	}
	response = UaString(resp.dump().c_str());
	return OpcUa_Good;

}

UaStatus DIoLMotor::callConfigure_motor (
		const UaString&  config_json,
		UaString& response
)
{
	LOG(Log::INF) << "Received JSON configuration file ";

	LOG(Log::INF) << "Raw content : " << config_json.toUtf8();
	json resp;
	std::ostringstream msg("");

	try
	{
		// have to be careful about the escapes
		// \n, \". etc. These seem to cause trouble
		//json config(config_json.toUtf8());
		//std::string tmp(reinterpret_cast<char*>(config_json.toOpcUaString()->data),config_json.toOpcUaString()->length);
		json config = json::parse(config_json.toUtf8());

		// first confirm that this configuration is for the correct motor
		LOG(Log::INF) << "Dumping : " << config.dump();
		if (config.at("id").get<uint16_t>() != id())
		{
			msg << log_e("config"," ") << "Mismatch in motor id on configuration token (" << id() << "!=" << config.at("id").get<uint16_t>() << ")";
			LOG(Log::ERR) << msg.str();
			resp["status"] = "ERROR";
			resp["messages"].push_back(msg.str());
			resp["status_code"] = OpcUa_BadInvalidArgument;

			response = UaString(resp.dump().c_str());
			return OpcUa_BadInvalidArgument;
		}

		// if the ids match, lets set the parameters
		// special iterator member functions for objects
		for (json::iterator it = config.begin(); it != config.end(); ++it)
		{
			LOG(Log::INF) << "Processing " << it.key() << " : " << it.value() << "\n";
			if (it.key() == "server_address")
			{
				m_server_host = it.value();
				if (m_server_host != server_address())
				{
					msg.clear(); msg.str("");
					msg << log_w("config","Server host is being updated. This should be done with care as we cannot update address space configuration.");
					resp["messages"].push_back(msg.str());
					LOG(Log::WRN) << msg.str();
				}
			}
			if (it.key() == "port")
			{
				m_server_port = it.value();
				if (m_server_port != port())
				{
					msg.clear(); msg.str("");
					msg << log_w("config","Server port is being updated. This should be done in the initial configuration.");
					LOG(Log::WRN) << msg.str();
					resp["messages"].push_back(msg.str());

				}
			}
			if (it.key() == "speed")
			{
				m_speed_setpoint = it.value();
				getAddressSpaceLink()->setSpeed_setpoint(m_speed_setpoint,OpcUa_Good);
			}
			if (it.key() == "acceleration")
			{
				m_acceleration = it.value();
				getAddressSpaceLink()->setAcceleration(m_acceleration,OpcUa_Good);
			}
			if (it.key() == "deceleration")
			{
				m_deceleration = it.value();
				getAddressSpaceLink()->setDeceleration(m_deceleration,OpcUa_Good);
			}
			if (it.key() == "refresh_period_ms")
			{
				m_refresh_ms = it.value();
				writeRefresh_period_ms(m_refresh_ms);
				getAddressSpaceLink()->setRefresh_period_ms(m_refresh_ms, OpcUa_Good);
			}
		}
		resp["status"] = "OK";
		msg.clear(); msg.str("");
		msg << log_i("config","Motor configuration updated");
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_Good;

		response = UaString(resp.dump().c_str());
	}
	catch(json::exception &e)
	{
		std::ostringstream msg("");
		msg << log_e("config","Caught a JSON parsing exception : ") << e.what();
		resp["status"] = "ERROR";
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_Bad;

		response = UaString(resp.dump().c_str());

		return OpcUa_Good;

	}
	catch(std::exception &e)
	{
		msg << log_e("config","Caught an STL exception : ") << e.what();
		resp["status"] = "ERROR";
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_Bad;

		response = UaString(resp.dump().c_str());

		return OpcUa_Good;
	}
	catch(...)
	{
		msg << log_e("config","Caught an unknown exception");
		resp["status"] = "ERROR";
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_Bad;

		response = UaString(resp.dump().c_str());

		return OpcUa_Good;
	}
	// force an update after a config
	update();

	return OpcUa_Good;
}

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333
//TODO: Add an internal method on a timer to query the motor (if the connection is valid)
void DIoLMotor::update()
{

	// method should be periodically poking the motors for their status
	OpcUa_StatusCode status= OpcUa_Good;
	// this should no longer be called

	//	bool msg_printed = false;
	if (m_monitor_status != OpcUa_Good)
	{
		//		if (!msg_printed)
		//		{
		//			LOG(Log::ERR) << "Failed to query device for status. Setting read values to InvalidData";
		//			msg_printed = true;
		//		}
		status = OpcUa_BadResourceUnavailable;
		//	} else
		//	{
		//		// reset the state until connection is lost again
		//		msg_printed = false;
	}

	//LOG(Log::INF) << "Updating for IOLMotor::ID=" << id();

	// server updates
	// i.e., form the server to the client
	//position_ = {static_cast<double>(rand()),static_cast<double>(rand()),static_cast<double>(rand())};
	getAddressSpaceLink()->setPosition(m_position,status);
	//getAddressSpaceLink()->setSpeed(m_speed, status);
	// -- the speed is obtained from the address space
	//m_speed = getAddressSpaceLink()->getSpeed();
	getAddressSpaceLink()->setTorque(m_torque, status);
	getAddressSpaceLink()->setTemperature_C(m_temperature, status);
	getAddressSpaceLink()->setSpeed_readout(m_speed_readout, status);
	getAddressSpaceLink()->setIs_moving(is_moving_, status);
	//getAddressSpaceLink()->setAcceleration(m_acceleration, OpcUa_Good);

	// now the getters -- for now allow all to be updated, but eventually set limitations
	// these getters are for variables with "regular" writing policy. We can change that
	status |= getAddressSpaceLink()->getSpeed_setpoint(m_speed_setpoint);
	status |= getAddressSpaceLink()->getAcceleration(m_acceleration);
	status |= getAddressSpaceLink()->getDeceleration(m_deceleration);

}

bool DIoLMotor::is_ready()
{
	// actually, this should check a few more things:
	// 1. is moving (not ready)
	// 2. all parameters have reasonable values
	// 2.1. position_set_point
	// 2.2. speed
	// 2.3. acceleration
	LOG(Log::INF) << "Checking readiness for motor ID=" << id()
							<< " with positionSetPoint = (" << m_position_setpoint << ")";

	if (is_moving_)
	{
		return false;
	}
	if (m_speed_setpoint == 0.0)
	{
		return false;
	}

	LOG(Log::INF) << "It is ready";

	return true;

}


size_t DIoLMotor::curl_write_function(void* ptr, size_t size, size_t nmemb, std::string* data)
{
	data->append((char*)ptr, size * nmemb);
	return size * nmemb;
}

void DIoLMotor::timer_start(DIoLMotor *obj)
{
	if (m_monitor)
	{
		LOG(Log::WRN) << "Trying to set a timer that has already been set up. Skipping.";
		return;
	}
	m_monitor = true;

	std::thread([obj]()
			{
		while (obj->get_monitor())
		{
			auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(obj->get_refresh_ms());
#ifdef SIMULATION
			if (obj->sim_get_motor_info() != OpcUa_Good)
#else
				if (obj->get_motor_info() != OpcUa_Good)
#endif
				{
					LOG(Log::ERR) << "Failed to query device for status. Setting read values to InvalidData";
				}

			std::this_thread::sleep_until(x);
		}
			}).detach();
}


UaStatus DIoLMotor::get_motor_info()
{
	string addr = "http://";
	if (m_server_host.length())
	{
		addr+= m_server_host;
	} else
	{
		addr+= server_address();
	}
	addr += "/api/info";

	uint16_t lport = m_server_port;
	if (lport == 0)
	{
		lport = port();
	}
	//OpcUa_StatusCode status= OpcUa_Good;
	auto curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
		curl_easy_setopt(curl, CURLOPT_PORT, lport);

		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		//curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

		std::string response_string;
		std::string header_string;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

		CURLcode res = curl_easy_perform(curl);
		// Check for errors
		if (res != CURLE_OK)
		{
			LOG(Log::ERR) << "curl_easy_perform() failed: "
					<< curl_easy_strerror(res);
			curl_easy_cleanup(curl);
			curl = NULL;
			m_monitor_status = OpcUa_BadResourceUnavailable;
			return OpcUa_BadResourceUnavailable;
		}

		//LOG(Log::INF) << "Received response [" << response_string << "]";
		// now we should parse the answer
		// it is meant to be a json object
		json answer = json::parse(response_string);
		/**
		 * Typical answer: {"cur_pos":25000,"cur_speed":0,"m_temp":" 38.8","tar_pos":25000,"torque":"  1.9"}
		 */
		m_position = answer["cur_pos"];
		m_speed_readout = answer["cur_speed"];
		m_temperature = std::stod(answer["m_temp"].get<std::string>());
		m_torque = std::stod(answer["torque"].get<std::string>());

		curl_easy_cleanup(curl);
		curl = NULL;
	}
	m_monitor_status = OpcUa_Good;

	return OpcUa_Good;
}

UaStatus DIoLMotor::move_motor(json &resp)
{

	string addr = "http://";
	if (m_server_host.length())
	{
		addr+= m_server_host;
	} else
	{
		addr+= server_address();
	}
	addr += "/api/move";

	uint16_t lport = m_server_port;
	if (lport == 0)
	{
		lport = port();
	}

	OpcUa_StatusCode status= OpcUa_Good;

	auto curl = curl_easy_init();
	if (curl) {

		// NOTE: Zero values are not passed onto the query, and expects the server to assume some defaults
		ostringstream query("");
		query << "pos=" << m_position_setpoint;
		if (m_speed_setpoint != 0)
		{
			query << "&speed=" << m_speed_setpoint;
		}
		if (m_acceleration != 0)
		{
			query << "&accel=" << m_acceleration;
		}
		if (m_deceleration != 0)
		{
			query << "&decel=" << m_deceleration;
		}
		addr += "?";
		addr += query.str();
		int ret = 0;

		ret |= curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
		ret |= curl_easy_setopt(curl, CURLOPT_PORT, lport);

		ret |= curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		//curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
		ret |= curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
		ret |= curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		ret |= curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

		std::string response_string;
		std::string header_string;
		ret |= curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
		ret |= curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
		ret |= curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

		ret |= curl_easy_perform(curl);
		// Check for errors
		if (ret != CURLE_OK)
		{
			ostringstream msg("");
			msg << log_e("move_motor"," ") << "curl_easy_perform() failed: " << curl_easy_strerror(static_cast<CURLcode>(ret));
			resp["status"] = "ERROR";
			resp["messages"].push_back(msg.str());
			resp["status_code"] = OpcUa_BadNoCommunication;

			LOG(Log::ERR) << msg.str();

			curl_easy_cleanup(curl);
			curl = NULL;

			return OpcUa_BadNoCommunication;
		}

		//        cout << response_string;
		LOG(Log::INF) << "Received response [" << response_string << "]";
		// now we should parse the answer
		// it is meant to be a json object
		json answer = json::parse(response_string);
		/**
		 * Typical answer: {"cur_pos":25000,"cur_speed":0,"m_temp":" 38.8","tar_pos":25000,"torque":"  1.9"}
		 */
		if (answer["status"] == string("OK"))
		{
			std::ostringstream msg("");
			status = OpcUa_Good;
			msg << log_i("move_motor","Remote command successful");
			resp["messages"].push_back(msg.str());
			resp["status_code"] = OpcUa_Good;

		}
		else
		{
			resp["status"] = answer["status"];
			std::ostringstream msg("");
			msg << log_e("move_motor","Failed to execute remote command");
			resp["messages"].push_back(msg.str());
			resp["status_code"] = OpcUa_Bad;

			status =  OpcUa_Bad;
		}
		//resp = answer;
		//resp["messages"].push_back("move_motor called");

		curl_easy_cleanup(curl);
		curl = NULL;
	} else
	{
		resp["status"] = "ERROR";
		std::ostringstream msg("");
		msg << log_e("move_motor","Failed to get a connection handle");
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_BadNoCommunication;

		LOG(Log::ERR) << msg.str();
		status =  OpcUa_BadNoCommunication;
	}
	return status;
}

UaStatus DIoLMotor::stop_motor(json &resp)
{
	string addr = "http://";
	if (m_server_host.length())
	{
		addr+= m_server_host;
	} else
	{
		addr+= server_address();
	}
	addr += "/api/stop";

	uint16_t lport = m_server_port;
	if (lport == 0)
	{
		lport = port();
	}
	//	string addr = "http://" + server_address() + "/api/stop";
	//
	//	static const uint16_t port = 5001;
	OpcUa_StatusCode status= OpcUa_Good;

	auto curl = curl_easy_init();
	if (curl) {

		int ret = 0;

		ret |= curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
		ret |= curl_easy_setopt(curl, CURLOPT_PORT, lport);

		ret |= curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		//curl_easy_setopt(curl, CURLOPT_USERPWD, "user:pass");
		ret |= curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
		ret |= curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		ret |= curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

		std::string response_string;
		std::string header_string;
		ret |= curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
		ret |= curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
		ret |= curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
		// Check for errors
		if (ret != CURLE_OK)
		{
			ostringstream msg("");
			msg << log_w("stop_motor","Failed to set connection options : ") <<  curl_easy_strerror(static_cast<CURLcode>(ret));
			resp["messages"].push_back(msg.str());

			LOG(Log::WRN) << msg.str();

		}

		ret = curl_easy_perform(curl);
		// Check for errors
		if (ret != CURLE_OK)
		{
			ostringstream msg("");
			msg << log_e("stop_motor","curl_easy_perform() failed : ") << curl_easy_strerror(static_cast<CURLcode>(ret));
			resp["status"] = "ERROR";
			resp["messages"].push_back(msg.str());
			resp["status_code"] = OpcUa_BadCommunicationError;

			LOG(Log::ERR) << msg.str();

			curl_easy_cleanup(curl);
			curl = NULL;

			return OpcUa_BadCommunicationError;
		}

		// cout << response_string;
		LOG(Log::INF) << "Received response [" << response_string << "]";
		// now we should parse the answer
		// it is meant to be a json object
		json answer = json::parse(response_string);
		if (answer["status"] == string("OK"))
		{
			std::ostringstream msg("");
			msg << log_i("move_motor","Remote command successful");
			resp["status"] = "OK";
			resp["messages"].push_back(msg.str());
			resp["status_code"] = OpcUa_Good;

			LOG(Log::INF) << msg.str();
			status =  OpcUa_Good;
		}
		else
		{

			std::ostringstream msg("");
			msg << log_e("move_motor","Failed to execute remote command successful");
			resp["status"] = "ERROR";
			resp["messages"].push_back(msg.str());
			resp["status_code"] = OpcUa_Bad;

			LOG(Log::ERR) << msg.str();
			status = OpcUa_Bad;
		}

		curl_easy_cleanup(curl);
		curl = NULL;
	} else
	{
		std::ostringstream msg("");
		msg << log_e("move_motor","Failed to get a connection handle");

		resp["status"] = "ERROR";
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_BadNoCommunication;

		LOG(Log::ERR) << msg.str();
		status =  OpcUa_BadNoCommunication;
	}
	return status;
}

UaStatus DIoLMotor::sim_get_motor_info()
{

	// -- simulate that we're just getting the current settings from the motor itself

	// now we should parse the answer
	// it is meant to be a json object
	/**
	 * Typical answer: {"cur_pos":25000,"cur_speed":0,"m_temp":" 38.8","tar_pos":25000,"torque":"  1.9"}
	 */
	// grab this from the
	m_monitor_status = OpcUa_Good;
	is_moving_ = m_sim_moving;
	m_position = m_sim_pos;
	// just set the speed equal to the setting
	m_speed_readout = m_sim_speed;
	// the temperature is just a random number between 36 and 37 degrees
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist_temp(36.0, 37.0);
	std::uniform_real_distribution<double> dist_torque(0.0, 2.0);
	m_temperature = dist_temp(mt);
	m_torque = dist_torque(mt);

	return OpcUa_Good;
}

UaStatus DIoLMotor::sim_move_motor(json &resp)
{

	OpcUa_StatusCode status= OpcUa_Good;



	// initiate the thread to start the movement simulator
	// note that the movement is *not* realistically simulated, i.e.,
	// only the position is being monotonically updated a rate of
	// <speed> steps per second


	if (m_sim_moving)
	{
		std::ostringstream msg("");
		msg << log_e("sim_move_motor","Motor is already moving");
		resp["status"] = "ERROR";
		resp["messages"].push_back(msg.str());
		resp["status_code"] = OpcUa_Bad;
		LOG(Log::ERR) << msg.str();
		return OpcUa_Bad;
	}

	// if it is not yet moving start the thread

	// set the simulation conditions
	m_sim_speed = m_speed_setpoint;
	// this permits that one changes the position set point
	// without affecting the ongoing movement
	m_sim_tpos = m_position_setpoint;

	std::thread([this]()
			{

		this->m_sim_moving = true;
		while (this->m_sim_moving)
		{
			if (!this->m_sim_moving || (this->m_sim_tpos == this->m_sim_pos))
			{
				// stop the thread
				break;
			}

			// update every 5 ms
			auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000/this->m_sim_speed);
			if (this->m_sim_pos < this->m_sim_tpos)
			{
				this->m_sim_pos += 1;

			} else
			{
				this->m_sim_pos -= 1;
			}

			std::this_thread::sleep_until(x);
		}
		LOG(Log::WRN) << "Dropping out of the sim_move thread";
			}).detach();

	std::ostringstream msg("");
	msg << log_i("sim_move_motor","Motor movement initiated");
	resp["status"] = "OK";
	resp["messages"].push_back(msg.str());
	resp["status_code"] = status;
	return status;
}

UaStatus DIoLMotor::sim_stop_motor(json &resp)
{
	OpcUa_StatusCode status= OpcUa_Good;
	std::ostringstream msg("");

	m_sim_moving = false;

	resp["status"] = "OK";
	msg << log_i("sim_stop_motor","Motor stopped successfuly");
	resp["messages"].push_back(msg.str());
	resp["status_code"] = OpcUa_Good;
	//resp["message"] = "Failed to get a connection handle";

	return status;
}



}
