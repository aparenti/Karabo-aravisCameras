/*
 * Author: <parenti>
 *
 * Created on October 20, 2020,  4:34 PM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#include "AravisBaslerCamera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(Device, ImageSource, CameraImageSource, AravisCamera, ...)
    // XXX Work-around: do not register all parameters here, but call parent's expectedParameters in this class
    KARABO_REGISTER_FOR_CONFIGURATION(Device, ImageSource, CameraImageSource, AravisBaslerCamera)

    void AravisBaslerCamera::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisBaslerBase::expectedParameters(expected);

        // This class supports the following models: ace, aviator, pilot (Area Scan) and racer (Line Scan)
        const std::vector<std::string> supportedModels = {"acA", "avA", "piA", "raL"};
        OVERWRITE_ELEMENT(expected).key("supportedModels").setNewDefaultValue(supportedModels).commit();

        FLOAT_ELEMENT(expected)
              .key("resultingFramePeriodAbs")
              .alias("ResultingFramePeriodAbs")
              .tags("poll")
              .displayedName("Resulting Frame Period (Abs)")
              .description(
                    "Indicates the 'absolute' value of the minimum allowed acquisition frame period. "
                    "The 'absolute' value is a float value that indicates the minimum allowed acquisition frame "
                    "period in microseconds given the current settings for the area of interest, exposure time, "
                    "and bandwidth.")
              .unit(Unit::SECOND)
              .metricPrefix(MetricPrefix::MICRO)
              .readOnly()
              .commit();

        FLOAT_ELEMENT(expected)
              .key("resultingFrameRateAbs")
              .alias("ResultingFrameRateAbs")
              .tags("poll")
              .displayedName("Resulting Frame Rate (Abs)")
              .description(
                    "Indicates the 'absolute' value of the maximum allowed acquisition frame rate. "
                    "The 'absolute' value is a float value that indicates the maximum allowed acquisition frame "
                    "rate in frames per second given the current settings for the area of interest, exposure time, "
                    "and bandwidth.")
              .unit(Unit::HERTZ)
              .readOnly()
              .commit();

        STRING_ELEMENT(expected)
              .key("temperatureSelector")
              .alias("TemperatureSelector")
              .tags("genicam")
              .displayedName("Temperature Selector")
              .description("Lists the temperature sources available for readout.")
              .assignmentOptional()
              .defaultValue("Sensorboard")
              .options("Sensorboard,Coreboard")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        FLOAT_ELEMENT(expected)
              .key("temperature")
              .alias("TemperatureAbs")
              .tags("poll")
              .displayedName("Temperature")
              .description("Shows the current temperature of the selected target.")
              .unit(Unit::DEGREE_CELSIUS)
              .readOnly()
              .commit();

        STRING_ELEMENT(expected)
              .key("temperatureState")
              .alias("TemperatureState")
              .tags("poll")
              .displayedName("Temperature State")
              .description("Indicates the temperature state.")
              .readOnly()
              .commit();

        STRING_ELEMENT(expected)
              .key("shutterMode")
              .alias("ShutterMode")
              .tags("genicam")
              .displayedName("Shutter Mode")
              .description("Sets the shutter mode.")
              .assignmentOptional()
              .defaultValue("Global")
              .options("Global,Rolling,GlobalResetRelease")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("globalResetReleaseModeEnable")
              .alias("GlobalResetReleaseModeEnable")
              .tags("genicam")
              .displayedName("Global Reset Release Mode Enable")
              .description("Enable the global reset release mode.")
              .assignmentOptional()
              .defaultValue(false)
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();
    }

    AravisBaslerCamera::AravisBaslerCamera(const karabo::data::Hash& config) : AravisBaslerBase(config) {
        m_is_device_reset_available = true; // "DeviceReset" command is available
        m_last_clock_reset.now();
    }

    bool AravisBaslerCamera::synchronize_timestamp() {
        GError* error = nullptr;
        const std::string& deviceId = this->getInstanceId();
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // XXX Possibly use PTP in the future
        m_ptp_enabled = false;

        const karabo::data::Epochstamp epoch;
        if (epoch.elapsed(m_last_clock_reset).getTotalSeconds() > 60.) {
            // Verify synchronization once every minute
            bool resetNeeded = false;
            if (m_min_latency > 0. && m_max_latency / m_min_latency > 5.) {
                // When min and max latency differ too much, the clock could need to be reset
                KARABO_LOG_FRAMEWORK_INFO << deviceId
                                          << ": max_latency / min_latency = " << m_max_latency / m_min_latency;
                resetNeeded = true;
            } else if (m_max_latency > 3.) {
                // Max latency higher than 3 s
                KARABO_LOG_FRAMEWORK_INFO << deviceId << ": max_latency = " << m_max_latency << " s";
                resetNeeded = true;
            }

            if (resetNeeded) {
                const std::string message("Timestamp synchronization loss -> reset timestamp");
                KARABO_LOG_WARN << message;
                this->set("status", message);
                if (m_is_gv_device) { // GEV camera
                    arv_camera_execute_command(m_camera, "GevTimestampControlReset", nullptr);
                    arv_camera_execute_command(m_camera, "GevTimestampControlLatchReset", nullptr);
                } else { // USB3V camera
                    arv_camera_execute_command(m_camera, "TimestampReset", nullptr);
                }
                m_last_clock_reset.now();
            }
        }

        // Karabo current timestamp
        m_reference_karabo_time = this->getActualTimestamp();

        // Get current timestamp on the camera.
        // It has been verified on an acA640-120gm that this takes 1 ms ca.,
        // thus this is the precision we can aim to in the synchronization.
        if (m_is_gv_device) { // GEV camera
            arv_camera_execute_command(m_camera, "GevTimestampControlLatch", &error);
            if (error == nullptr) {
                m_reference_camera_timestamp = arv_camera_get_integer(m_camera, "GevTimestampValue", &error);
            }
        } else { // USB3V camera
            arv_camera_execute_command(m_camera, "TimestampLatch", &error);
            if (error == nullptr) {
                m_reference_camera_timestamp = arv_camera_get_integer(m_camera, "TimestampLatchValue", &error);
            }
        }

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << deviceId << ": Could not synchronize timestamp: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }

    bool AravisBaslerCamera::configure_timestamp_chunk() {
        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Enable chunk data
        arv_camera_set_chunk_mode(m_camera, true, &error);

        // Enable timestamp chunk
        if (error == nullptr) arv_camera_set_chunk_state(m_camera, "Timestamp", true, &error);

        if (error != nullptr) {
            arv_camera_set_chunk_mode(m_camera, false, nullptr);
            m_chunk_mode = false;
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId()
                                       << ": Could not enable timestamp chunk: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        m_chunk_mode = true;
        return true; // success
    }

    int AravisBaslerCamera::get_tick_frequency() {
        if (m_is_gv_device) {
            // On GEV cameras the value of the tick frequency is 125 MHz with PTP disabled,
            // 1 GHz with PTP enabled, see https://docs.baslerweb.com/timestamp#how-it-works
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const int tickFrequency =
                  arv_device_get_integer_feature_value(m_device, "GevTimestampTickFrequency", nullptr);
            return tickFrequency;
        } else if (m_is_uv_device) {
            // Tick frequency for USB models is always 1 GHz
            return 1'000'000'000;
        }

        return 0;
    }

    bool AravisBaslerCamera::get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts) {
        if (m_chunk_mode) {
            return AravisBaslerBase::get_timestamp(buffer, ts, "ChunkTimestamp");
        } else {
            return false;
        }
    }

} // namespace karabo
