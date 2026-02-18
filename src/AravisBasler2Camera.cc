/*
 * Author: <parenti>
 *
 * Created on April 27, 2021,  6:00 PM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#include "AravisBasler2Camera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(Device, ImageSource, CameraImageSource, AravisCamera, ...)
    // XXX Work-around: do not register all parameters here, but call parent's expectedParameters in this class
    KARABO_REGISTER_FOR_CONFIGURATION(Device, ImageSource, CameraImageSource, AravisBasler2Camera)

    void AravisBasler2Camera::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisBaslerBase::expectedParameters(expected);

        OVERWRITE_ELEMENT(expected)
              .key("pixelFormat")
              .setNewOptions("Mono8,Mono10,Mono10p,Mono12,Mono12p")
              .setNewDefaultValue("Mono12p")
              .commit();

        // This class supports the following models: ace2 (Area Scan)
        const std::vector<std::string> supportedModels = {"a2A"};
        OVERWRITE_ELEMENT(expected).key("supportedModels").setNewDefaultValue(supportedModels).commit();

        FLOAT_ELEMENT(expected)
              .key("resultingFrameRate")
              .alias("ResultingFrameRate")
              .tags("poll")
              .displayedName("Resulting Frame Rate")
              .description(
                    "Maximum frame acquisition rate with current camera settings (in "
                    "frames per second).")
              .unit(Unit::HERTZ)
              .readOnly()
              .commit();

        STRING_ELEMENT(expected)
              .key("temperatureSelector")
              .alias("DeviceTemperatureSelector")
              .tags("genicam")
              .displayedName("Temperature Selector")
              .description("Lists the temperature sources available for readout.")
              .assignmentOptional()
              .defaultValue("Coreboard")
              .options("Coreboard")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        FLOAT_ELEMENT(expected)
              .key("temperature")
              .alias("DeviceTemperature")
              .tags("poll")
              .displayedName("Temperature")
              .description("Shows the current temperature of the selected target.")
              .unit(Unit::DEGREE_CELSIUS)
              .readOnly()
              .commit();

        STRING_ELEMENT(expected)
              .key("temperatureState")
              .alias("BslTemperatureStatus")
              .tags("poll")
              .displayedName("Temperature State")
              .description("Indicates the temperature state.")
              .readOnly()
              .commit();

        STRING_ELEMENT(expected)
              .key("shutterMode")
              .alias("SensorShutterMode")
              .tags("genicam")
              .displayedName("Shutter Mode")
              .description("Sets the shutter mode of the camera.")
              .assignmentOptional()
              .defaultValue("Global")
              .options("Global,Rolling,GlobalResetRelease")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();
    }

    AravisBasler2Camera::AravisBasler2Camera(const karabo::data::Hash& config) : AravisBaslerBase(config) {
        m_is_device_reset_available = true; // "DeviceReset" command is available
    }

    bool AravisBasler2Camera::synchronize_timestamp() {
        GError* error = nullptr;

        // XXX Possibly use PTP in the future
        m_ptp_enabled = false;

        // XXX The counter cannot be reset during operation on a2A cameras:
        // https://docs.baslerweb.com/timestamp#specifics
        // In case of synchronization loss, a camera reset could be needed.

        // Karabo current timestamp
        m_reference_karabo_time = this->getActualTimestamp();

        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Get current timestamp on the camera.
        // It has been verified on an a2A2590-22gmPRO that this takes 4 ms ca.,
        // thus this is the precision we can aim to in the synchronization.
        arv_camera_execute_command(m_camera, "TimestampLatch", &error);
        if (error == nullptr) {
            m_reference_camera_timestamp = arv_camera_get_integer(m_camera, "TimestampLatchValue", &error);
        }

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId()
                                       << ": Could not synchronize timestamp: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }

    bool AravisBasler2Camera::configure_timestamp_chunk() {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Enable chunk data
        arv_camera_set_chunk_mode(m_camera, true, &error);

        // Enable timestamp chunk
        if (error == nullptr) arv_camera_set_chunk_state(m_camera, "Timestamp", true, &error);

        // Refer the timestamp to acquisition start of current image
        if (error == nullptr) {
            arv_device_set_string_feature_value(m_device, "BslChunkTimestampSelector", "FrameStart", &error);
        }

        if (error != nullptr) {
            arv_camera_set_chunk_mode(m_camera, false, nullptr);
            m_chunk_mode = false;
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not enable timestamp chunk: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        m_chunk_mode = true;
        return true; // success
    }

    int AravisBasler2Camera::get_tick_frequency() {
        // The timestamp tick frequency is 1 GHz for all a2A cameras.
        return 1'000'000'000;
    }

    bool AravisBasler2Camera::get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts) {
        if (m_chunk_mode) {
            return AravisBaslerBase::get_timestamp(buffer, ts, "BslChunkTimestampValue");
        } else {
            return false;
        }
    }

} // namespace karabo
