
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
#include <json.hpp>
using json = nlohmann::json;

extern "C" {
#include <curl/curl.h>
};

#include <chrono>
#include <thread>
#include <functional>

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
	,m_position(-99999)
	,m_position_setpoint(-99999)
	, m_is_ready(false)
	,is_moving_(false)
	,m_acceleration(0.0)
	,m_deceleration(0.0)
	,m_speed(0.0)
	,m_torque(0.0)
	,m_temperature(0.0)
	//,m_address("")
	,m_refresh_ms(0)
	,m_monitor(false)
	,m_monitor_status(OpcUa_BadResourceUnavailable)
{
    /* fill up constructor body here */
	// initialize cURL
	  curl_global_init(CURL_GLOBAL_ALL);

	  // everything else will be processed in contained form

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

UaStatus DIoLMotor::writePositionSetPoint ( const OpcUa_Double& v)
{

	// check ranges
	if ((v < 0) || (v > 50000))
	{
		LOG(Log::ERR) << "Value out of range for positionSetPoint. Offending value = " << v;
		return OpcUa_BadOutOfRange;
	}

	if (v != m_position_setpoint)
	{
		LOG(Log::INF) << "Updating motor ID=" << id() << " with positionSetPoint = (" << v << ")";
		m_position_setpoint = v;
	}

    return OpcUa_Good;
}

/* Note: never directly call this function. */

UaStatus DIoLMotor::writeRefresh_period_ms ( const OpcUa_UInt16& v)
{
	m_refresh_ms = v;
	LOG(Log::INF) << "Updating refresh rate for motor " << id() << " to " << v << " ms";
	if (v != 0 && !m_monitor)
	{
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
	std::string resp;

	if (!is_ready())
	{
		resp = "{\"status\":\"FAIL\",\"message\":\"Motor is not ready to operate\"}";
		response = UaString(resp.c_str());
		return OpcUa_BadInvalidState;
	}

	// check that position and position_set_point are not the same
	if (m_position == m_position_setpoint)
	{
		resp = "{\"status\":\"FAIL\",\"message\":\"Motor is already at the destination\"}";
		response = UaString(resp.c_str());
		return OpcUa_BadInvalidState;
	}

	// if it reaches this point we can initiate the movement
	return move_motor();


}

UaStatus DIoLMotor::callStop (
    UaString& response
)
{
	// stopping is a serious business. Should be called immediately
	return stop_motor();

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

//	if (get_motor_info() == OpcUa_Bad)
//	{
//		LOG(Log::ERR) << "Failed to query device for status. Setting read values to InvalidData";
//		status = OpcUa_BadResourceUnavailable;
//	}

	//LOG(Log::INF) << "Updating for IOLMotor::ID=" << id();

	// server updates
	// i.e., form the server to the client
	//position_ = {static_cast<double>(rand()),static_cast<double>(rand()),static_cast<double>(rand())};
	status |= getAddressSpaceLink()->setPosition(m_position,status);
	//getAddressSpaceLink()->setSpeed(m_speed, status);
	// -- the speed is obtained from the address space
	//m_speed = getAddressSpaceLink()->getSpeed();
	status |= getAddressSpaceLink()->setTorque(m_torque, status);
	status |= getAddressSpaceLink()->setTemperature_C(m_temperature, status);
	//getAddressSpaceLink()->setAcceleration(m_acceleration, OpcUa_Good);

	// now the getters
	status |= getAddressSpaceLink()->getSpeed(m_speed);





	/** The code below is valid for regular writing mechanism
	 *
	// from the client to the server
	//std::vector<double> pos2;
	std::vector<double> tmp_positionSetPoint_ = {0,0,0};
	OpcUa_StatusCode res = getAddressSpaceLink()->getPositionSetPoint(tmp_positionSetPoint_);
	if (res == OpcUa_Good)
	{
		//LOG(Log::ERR) << "Failed to acquire PositionSetPoint " << ;
		if (tmp_positionSetPoint_ != positionSetPoint_)
		{
			LOG(Log::INF) << "Updating motor ID=" << id() << " with positionSetPoint = (" << tmp_positionSetPoint_.at(0) << "," << tmp_positionSetPoint_.at(1) << "," << tmp_positionSetPoint_.at(2) << ")";
			positionSetPoint_ = tmp_positionSetPoint_;
		}
	} else
	{
		// for now do nothing
	}
	*/
}

bool DIoLMotor::is_ready()
{
	// actually, this should check a few more things:
	// 1. is moving (not ready)
	// 2. all parameters have reasonable values
	// 2.1. position_set_point
	// 2.2. speed
	// 2.3. acceleration
	if (is_moving_)
	{
		return false;
	}
	if (m_speed == 0.0)
	{
		return false;
	}
	if (m_acceleration == 0.0)
	{
		return false;
	}
//	if (m_deceleration == 0.0)
//	{
//		return false;
//	}

	LOG(Log::INF) << "Checking readiness for motor ID=" << id()
			<< " with positionSetPoint = (" << m_position_setpoint << ")";

	return true;

}


size_t DIoLMotor::curl_write_function(void* ptr, size_t size, size_t nmemb, std::string* data)
{
	data->append((char*)ptr, size * nmemb);
	    return size * nmemb;
}

//void DIoLMotor::timer_start(std::function<void(void)> func, uint16_t interval)
void DIoLMotor::timer_start(DIoLMotor *obj)
{
	if (m_monitor)
	{
		LOG(Log::WRN) << "Trying to set a timer that has already been set up";
		return;
	}
	m_monitor = true;

  std::thread([obj]()
  {
    while (obj->get_monitor())
    {
      auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(obj->get_refresh_ms());
	if (obj->get_motor_info() != OpcUa_Good)
	{
		LOG(Log::ERR) << "Failed to query device for status. Setting read values to InvalidData";
	}

      std::this_thread::sleep_until(x);
    }
  }).detach();
}


UaStatus DIoLMotor::get_motor_info()
{
	string addr = "http://" + server_address() + "/api/info";
	uint16_t lport = port();
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

//		char* url;
//		long response_code;
//		double elapsed;
//		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
//		curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
//		curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

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

//        cout << response_string;
		LOG(Log::INF) << "Received response [" << response_string << "]";
		// now we should parse the answer
		// it is meant to be a json object
		json answer = json::parse(response_string);
		/**
		 * Typical answer: {"cur_pos":25000,"cur_speed":0,"m_temp":" 38.8","tar_pos":25000,"torque":"  1.9"}
		 */
		m_position = answer["cur_pos"];
		m_speed = answer["cur_speed"];
		m_temperature = answer["m_temp"];
		m_torque = answer["torque"];

		curl_easy_cleanup(curl);
		curl = NULL;
	}
	m_monitor_status = OpcUa_Good;
	return OpcUa_Good;
}

UaStatus DIoLMotor::move_motor()
{
	string addr = "http://" + server_address() + "/api/move";

	static const uint16_t port = 5001;
	OpcUa_StatusCode status= OpcUa_Good;

	auto curl = curl_easy_init();
	if (curl) {

	    // NOTE: Zero values are not passed onto the query, and expects the server to assume defaults
		ostringstream query("");
		query << "pos=" << m_position_setpoint;
		if (m_speed != 0)
		{
			query << "&speed=" << m_speed;
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
		CURLcode ret;

		curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
		curl_easy_setopt(curl, CURLOPT_PORT, port);

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

//		char* url;
//		long response_code;
//		double elapsed;
//		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
//		curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);
//		curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &url);

		ret = curl_easy_perform(curl);
		// Check for errors
		if (ret != CURLE_OK)
		{
			LOG(Log::ERR) << "curl_easy_perform() failed: "
				<< curl_easy_strerror(ret);
			curl_easy_cleanup(curl);
			curl = NULL;

			return OpcUa_Bad;
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
			status = OpcUa_Good;
		}
		else
		{
			status =  OpcUa_Bad;
		}

		curl_easy_cleanup(curl);
		curl = NULL;
	} else
	{
		LOG(Log::ERR) << "Failed to get a connection handle";
		status =  OpcUa_Bad;
	}
	return status;
}

UaStatus DIoLMotor::stop_motor()
{
	string addr = "http://" + server_address() + "/api/stop";

	static const uint16_t port = 5001;
	OpcUa_StatusCode status= OpcUa_Good;

	auto curl = curl_easy_init();
	if (curl) {

		CURLcode ret;

		curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
		curl_easy_setopt(curl, CURLOPT_PORT, port);

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

		ret = curl_easy_perform(curl);
		// Check for errors
		if (ret != CURLE_OK)
		{
			LOG(Log::ERR) << "curl_easy_perform() failed: "
				<< curl_easy_strerror(ret);
			curl_easy_cleanup(curl);
			curl = NULL;

			return OpcUa_Bad;
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
			status =  OpcUa_Good;
		}
		else
		{
			status = OpcUa_Bad;
		}

		curl_easy_cleanup(curl);
		curl = NULL;
	} else
	{
		LOG(Log::ERR) << "Failed to get a connection handle";
		status = OpcUa_Bad;
	}
	return status;
}

}
