/*
 * Author: <parenti>
 *
 * Created on March 17, 2020, 12:20 PM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#include "AravisPhotonicScienceCamera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(Device, ImageSource, CameraImageSource, AravisCamera,
    // AravisPhotonicScienceCamera)
    // XXX Work-around: do not register all parameters here, but call parent's expectedParameters in this class
    KARABO_REGISTER_FOR_CONFIGURATION(Device, ImageSource, CameraImageSource, AravisPhotonicScienceCamera)

    void AravisPhotonicScienceCamera::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisCamera::expectedParameters(expected);

        // Description of not available properties
        const std::string notAvailable("Not available for this camera.");

        // **************************************************************************************************************
        //                                   READ/WRITE HARDWARE PARAMETERS
        // **************************************************************************************************************

        OVERWRITE_ELEMENT(expected).key("flip.Y").setNewAlias("ReverseY").setNewTags({"genicam"}).commit();

        // This class supports camreas from Photonic Science
        OVERWRITE_ELEMENT(expected).key("supportedVendor").setNewDefaultValue("Photonic Science").commit();

        // This class supports the following models: 'SCOMS Camera'
        const std::vector<std::string> supportedModels = {"SCMOS"};
        OVERWRITE_ELEMENT(expected).key("supportedModels").setNewDefaultValue(supportedModels).commit();

        STRING_ELEMENT(expected)
              .key("pixelSize")
              .alias("PixelSize")
              .tags("genicam")
              .displayedName("Pixel Size")
              .description("This feature indicates the total size in bits of a pixel of the image.")
              .readOnly()
              .defaultValue("")
              .commit();

        OVERWRITE_ELEMENT(expected)
              .key("roi.width")
              .setNewDescription(
                    "This value sets the width of the area of interest in pixels. "
                    "It must be a multiple of 16. Use '0' for the whole sensor width.")
              .setNewDefaultValue(1920)
              .setNewMinInc(16)
              .commit();

        OVERWRITE_ELEMENT(expected).key("roi.height").setNewDefaultValue(1080).commit();

        OVERWRITE_ELEMENT(expected)
              .key("roi.x")
              .setNewDescription(notAvailable + " Use 'xOffset' instead.")
              .setNowReadOnly()
              .commit();

        OVERWRITE_ELEMENT(expected)
              .key("roi.y")
              .setNewDescription(notAvailable + " Use 'yOffset' instead.")
              .setNowReadOnly()
              .commit();

        INT32_ELEMENT(expected)
              .key("xOffset")
              .alias("OffsetX_in_camera")
              .tags("genicam")
              .displayedName("X Offset")
              .description(
                    "This value sets the X offset (left offset) for the area of interest in pixels, "
                    "i.e., the distance in pixels between the left side of the sensor and the left side of the image "
                    "area")
              .assignmentOptional()
              .defaultValue(0)
              .unit(Unit::PIXEL)
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        INT32_ELEMENT(expected)
              .key("yOffset")
              .alias("OffsetY_in_camera")
              .tags("genicam")
              .displayedName("Y Offset")
              .description(
                    "This value sets the Y offset (top offset) for the area of interest in pixels, "
                    "i.e., the distance in pixels between the top side of the sensor and the top side of the image "
                    "area")
              .assignmentOptional()
              .defaultValue(0)
              .unit(Unit::PIXEL)
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        //// XXX Apparently binning cannot be set neither by arv_camera_set_binning
        //////   nor by accessing BinningHorizontal/BinningVertical GenICam parameters.
        ////     No error is reported, but the read-back shows no change.
        ////     The same is anyway true for eBUSPlayer too, thus I assume binning
        ////     does not work on firmware.
        //        INT32_ELEMENT(expected).key("xBinning")
        //                .alias("BinningHorizontal")
        //                .tags("genicam")
        //                .displayedName("X Binning")
        //                .description("This feature represents the number of horizontal photo-sensitive cells "
        //                "that must be combined (added) together.")
        //                .assignmentOptional().defaultValue(1)
        //                .unit(Unit::PIXEL)
        //                .reconfigurable()
        //                .allowedStates(State::UNKNOWN, State::ON)
        //                .commit();
        //
        //        INT32_ELEMENT(expected).key("yBinning")
        //                .alias("BinningVertical")
        //                .tags("genicam")
        //                .displayedName("Y Binning")
        //                .description("This feature represents the number of vertical photo-sensitive cells "
        //                "that must be combined (added) together.")
        //                .assignmentOptional().defaultValue(1)
        //                .unit(Unit::PIXEL)
        //                .reconfigurable()
        //                .allowedStates(State::UNKNOWN, State::ON)
        //                .commit();

        OVERWRITE_ELEMENT(expected)
              .key("pixelFormat")
              // Fill-up with some commonly available options. They will be updated on connection.
              .setNewOptions("Mono8,Mono12,Mono12Packed,Mono16")
              .setNewDefaultValue("Mono16")
              .commit();

        STRING_ELEMENT(expected)
              .key("displayMode")
              .alias("Display_mode")
              .tags("genicam")
              .displayedName("Display Mode")
              .description("Select nonstandard display modes. Use to select between low gain, high gain or combined.")
              .assignmentOptional()
              .defaultValue("Combined_low_and_high_gains")
              .options("Low_Gain,High_Gain,Combined_low_and_high_gains")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        STRING_ELEMENT(expected)
              .key("exposureMode")
              .alias("ExposureMode")
              .tags("genicam")
              .displayedName("Exposure Mode")
              .description("This feature is used to set the operation mode of the Exposure (or shutter).")
              .assignmentOptional()
              .defaultValue("Timed")
              .options("Timed")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        INT32_ELEMENT(expected)
              .key("delayBetweenImages")
              .alias("DELAY_BETWEEN_IMAGES")
              .tags("genicam")
              .displayedName("Delay Between Images")
              .description(
                    "Adds additional delay in us between images in free running mode. Use to slow down the frame rate.")
              .assignmentOptional()
              .noDefaultValue()
              .minInc(1)
              .maxInc(100000)
              .unit(Unit::SECOND)
              .metricPrefix(MetricPrefix::MICRO)
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        STRING_ELEMENT(expected)
              .key("triggerMode")
              .alias("PSL_TRIGGER_MODE")
              .tags("genicam")
              .displayedName("Trigger Mode")
              .description("Trigger mode selector.")
              .assignmentOptional()
              .defaultValue("Hardware_rising_edge")
              .options("freerun,Hardware_falling_edge,Hardware_rising_edge,SW_Trigger,Pipeline_Marser,Pipeline_slave")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        STRING_ELEMENT(expected)
              .key("triggerSource")
              .displayedName("Trigger Source")
              .description(notAvailable)
              .readOnly()
              .commit();

        STRING_ELEMENT(expected)
              .key("triggerActivation")
              .displayedName("Trigger Activation")
              .description(notAvailable)
              .readOnly()
              .commit();

        STRING_ELEMENT(expected)
              .key("powerSaving")
              .alias("Power_saving")
              .tags("genicam")
              .displayedName("Power Saving")
              .description("The power saving mode.")
              .assignmentOptional()
              .defaultValue("Camera_On")
              .options("Camera_On,Cooling_Off,Camera_Off")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        //        INT32_ELEMENT(expected).key("coolerPower")
        //                .alias("Cooler_Power")
        //                .tags("genicam")
        //                .displayedName("Cooler Power")
        //                .description("This feature sets cooling power.")
        //                .assignmentOptional().noDefaultValue()
        //                .minInc(0).maxInc(255)
        //                .reconfigurable()
        //                .allowedStates(State::UNKNOWN, State::ON)
        //                .commit();

        STRING_ELEMENT(expected)
              .key("flickerReduction")
              .alias("flicker_reduction")
              .tags("genicam")
              .displayedName("Flicker Reduction")
              .description(
                    "This feature helps to reduce flicker caused by AC lighting. "
                    "Only works with exposure times greater than 10ms.")
              .assignmentOptional()
              .defaultValue("OFF")
              .options("OFF,FIFTY_HERTZ,SIXTY_HERTZ")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        INT32_ELEMENT(expected)
              .key("gammaGain")
              .alias("Gamma_gain")
              .tags("genicam")
              .displayedName("Gamma Gain")
              .description("Sets the gain to be applied to the darker parts of the image.")
              .assignmentOptional()
              .defaultValue(1)
              .minInc(1)
              .maxInc(8000)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("bestFit")
              .alias("Best_Fit")
              .tags("genicam")
              .displayedName("Best Fit")
              .description(
                    "Stretches the image to use the displays full dynamic range. If gamma = gammy gain then "
                    "the dark parts of the image are stretched most. if gamma = one then the image is stretched "
                    "linearly.")
              .assignmentOptional()
              .defaultValue(false)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("alcAllowAutoBin")
              .alias("ALC_Allow_auto_bin")
              .tags("genicam")
              .displayedName("ALC Allow Auto Bin")
              .description(
                    "If true then binning will be used to improve the image at very low light levels. The "
                    "image size is not changed by mode. If the Bin filter Feature is Set to True then binning will "
                    "remain "
                    "on all the time regardless of this setting.")
              .assignmentOptional()
              .defaultValue(false)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("columnReduction")
              .alias("Column_reduction")
              .tags("genicam")
              .displayedName("Column reduction")
              .description(
                    "To remove remaining column structure from image best applied in darkness. Must be off "
                    "to image.")
              .assignmentOptional()
              .defaultValue(false)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        STRING_ELEMENT(expected)
              .key("offsetCorrection")
              .alias("Offset_corection")
              .tags("genicam")
              .displayedName("Offset Correction")
              .description("Corrects the image for offset variations.")
              .assignmentOptional()
              .defaultValue("OFFSET_AND_CLAMP_CORRECTED")
              .options("OFF,OFFSET_CORRECTED,OFFSET_AND_CLAMP_CORRECTED")
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        STRING_ELEMENT(expected)
              .key("gammaSel")
              .alias("Gamma_sel")
              .tags("genicam")
              .displayedName("Gamma Sel")
              .description(
                    "Selects gamma 1 or gamma gain which is controlled by the Gamma_gain_dark control or the best fit.")
              .assignmentOptional()
              .defaultValue("one")
              .options("one,Gamma_gain,Gamma_gain_low")
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("brightPixelCorrection")
              .alias("Bright_Pixel_Corection")
              .tags("genicam")
              .displayedName("Bright Pixel Correction")
              .description("Turns on Bright pixel correction.")
              .assignmentOptional()
              .defaultValue(true)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("binFilter")
              .alias("Bin_Filter")
              .tags("genicam")
              .displayedName("Bin Filter")
              .description("If true image is binned 2x2 then rescaled to original size.")
              .assignmentOptional()
              .defaultValue(false)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("bpRemovalStrong")
              .alias("BP_REMOVEL_STRONG")
              .tags("genicam")
              .displayedName("BP Removal Strong")
              .description("Will correct more low level bright pixels when set But also erode edges.")
              .assignmentOptional()
              .defaultValue(true)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("preampGainMode")
              .alias("Preamp_Gain_Mode")
              .tags("genicam")
              .displayedName("Preamp Gain Mode")
              .assignmentOptional()
              .defaultValue(false)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        STRING_ELEMENT(expected)
              .key("pixelSpeed")
              .alias("Pixel_Speed")
              .tags("genicam")
              .displayedName("Pixel Speed")
              .description("Sets the pixel speed in MHz. Only preset values are supported.")
              .assignmentOptional()
              .defaultValue("MHz50")
              .options("MHz50,MHz100")
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        BOOL_ELEMENT(expected)
              .key("testChart")
              .alias("Test_Chart")
              .tags("genicam")
              .displayedName("Display Test Chart")
              .assignmentOptional()
              .defaultValue(false)
              .expertAccess()
              .reconfigurable()
              .allowedStates(State::UNKNOWN, State::ON)
              .commit();

        // **************************************************************************************************************
        //                                   READ ONLY HARDWARE PARAMETERS
        // **************************************************************************************************************

        STRING_ELEMENT(expected)
              .key("cameraVersion")
              .alias("DeviceVersion")
              .tags("genicam")
              .displayedName("Camera Version")
              .description("This feature provides the version of the device.")
              .readOnly()
              .commit();

        INT32_ELEMENT(expected)
              .key("sensorWidth")
              .alias("SensorWidth")
              .tags("genicam")
              .displayedName("Sensor Width")
              .description("This feature indicates the effective width of the sensor in pixels.")
              .unit(Unit::PIXEL)
              .readOnly()
              .commit();

        INT32_ELEMENT(expected)
              .key("sensorHeight")
              .alias("SensorHeight")
              .tags("genicam")
              .displayedName("Sensor Height")
              .description("This feature indicates the effective height of the sensor in pixels.")
              .unit(Unit::PIXEL)
              .readOnly()
              .commit();

        INT32_ELEMENT(expected)
              .key("maxWidth")
              .alias("WidthMax")
              .tags("genicam")
              .displayedName("Max Width")
              .description(
                    "This feature represents the maximum width (in pixels) of the image after "
                    "horizontal binning, decimation or any other function changing the horizontal dimensions "
                    "of the image.")
              .unit(Unit::PIXEL)
              .readOnly()
              .commit();

        INT32_ELEMENT(expected)
              .key("maxHeight")
              .alias("HeightMax")
              .tags("genicam")
              .displayedName("Max Height")
              .description(
                    "This feature represents the maximum height (in pixels) of the image after "
                    "vertical binning, decimation or any other function changing the vertical dimensions "
                    "of the image.")
              .unit(Unit::PIXEL)
              .readOnly()
              .commit();

        INT32_ELEMENT(expected)
              .key("ccdTemperature")
              .alias("CCD_TEMPERATURE")
              .tags("poll")
              .displayedName("CCD Temperature")
              .description("This feature represents the CCD temperature.")
              .readOnly()
              .commit();

        INT32_ELEMENT(expected)
              .key("pldTemperature")
              .alias("PLD_TEMPERATURE")
              .tags("poll")
              .displayedName("PCB Temperature")
              .description("This feature represents the PCB temperature.")
              .readOnly()
              .commit();

        INT32_ELEMENT(expected)
              .key("gevTimestampTickFrequency")
              .alias("GevTimestampTickFrequency")
              .tags("genicam")
              .displayedName("Tick Frequency")
              .description("This value indicates the number of clock ticks per second.")
              .unit(Unit::HERTZ)
              .readOnly()
              .commit();
    }

    AravisPhotonicScienceCamera::AravisPhotonicScienceCamera(const karabo::data::Hash& config)
        : AravisCamera(config), m_reference_camera_timestamp(0.) {
        m_is_base_class = false;
        m_arv_camera_trigger = false; // Trigger properties to be accessed from non-standard paths
    }

    bool AravisPhotonicScienceCamera::synchronize_timestamp() {
        GError* error = nullptr;
        double camera_timestamp;

        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        // Get current timestamp on the camera (GevTimestampValue).
        // GevTimestampValue counts the number of ticks since the last reset of the counter.
        // It has been verified on sCMOS camera that reading the counter takes < 1 ms,
        // thus this is the precision we can aim to in the synchronization.
        arv_camera_execute_command(m_camera, "GevTimestampControlLatch", &error);
        if (error == nullptr) camera_timestamp = arv_camera_get_integer(m_camera, "GevTimestampValue", &error);

        if (error != nullptr) {
            const std::string message("Could not synchronize timestamp");
            KARABO_LOG_ERROR << message << ": " << error->message;
            g_clear_error(&error);
            this->set("status", message);
            return false; // failure
        }

        // Karabo current timestamp
        m_reference_karabo_time = this->getActualTimestamp();

        // Camera current timestamp (s)
        const int tick_frequency = this->get<int>("tickFrequency");
        if (tick_frequency == 0) {
            KARABO_LOG_ERROR << "Could not synchronize timestamp: tick_frequency is 0";
            return false; // failure
        }

        m_reference_camera_timestamp = camera_timestamp / tick_frequency;

        return true; // success
    }

    bool AravisPhotonicScienceCamera::configure_timestamp_chunk() {
        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        arv_device_set_string_feature_value(m_device, "GevTimestampCounterSelector", "GevTimestamp", &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId()
                                       << ": Could not configure timestamp: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        return true; // success
    }

    bool AravisPhotonicScienceCamera::get_region(gint& x, gint& y, gint& width, gint& height) {
        GError* error = nullptr;
        boost::mutex::scoped_lock camera_lock(m_camera_mtx);

        x = arv_device_get_integer_feature_value(m_device, "OffsetX_in_camera", &error);
        if (error == nullptr) y = arv_device_get_integer_feature_value(m_device, "OffsetY_in_camera", &error);
        if (error == nullptr) width = arv_device_get_integer_feature_value(m_device, "Width", &error);
        if (error == nullptr) height = arv_device_get_integer_feature_value(m_device, "Height", &error);

        if (error != nullptr) {
            KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId() << ": Could not get region: " << error->message;
            g_clear_error(&error);
            return false; // failure
        }

        m_width = width;
        m_height = height;
        return true; // success
    }

    bool AravisPhotonicScienceCamera::get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts) {
        // Get timestamp from buffer.
        // The timestamp is provided in ns, thus convert it to s.
        const double timestamp = arv_buffer_get_timestamp(buffer) / 1e+9;

        // Elapsed time since last synchronization.
        // NB This can be negative, if the image acquisition started before
        //    synchronization, but finished after.
        const double elapsed_t = timestamp - m_reference_camera_timestamp;

        // Split elapsed time in seconds and attoseconds, then convert to TimeDuration.
        // elapsed_t is in seconds and TimeDuration expects fractions in attoseconds,
        // hence we multiply with 1e18.
        // A TimeDuration can only be positive, thus save sign and invert if negative.
        double intpart;
        const char sign = (elapsed_t >= 0.) ? 1 : -1;
        const unsigned long long fractions = 1.e+18 * modf(sign * elapsed_t, &intpart);
        const unsigned long long seconds = rint(intpart);
        const TimeDuration duration(seconds, fractions);

        // Calculate frame epochstamp from reference time and elapsed time
        Epochstamp epoch(m_reference_karabo_time.getEpochstamp());
        if (seconds <= m_max_correction_time) {
            if (this->get<bool>("wouldCorrectAboveMaxTime")) {
                this->set("wouldCorrectAboveMaxTime", false);
            }
            if (sign >= 0) {
                epoch += duration;
            } else {
                epoch -= duration;
            }
        } else if (!this->get<bool>("wouldCorrectAboveMaxTime")) {
            this->set("wouldCorrectAboveMaxTime", true);
            return false;
        }

        // Calculate timestamp from epochstamp
        ts = this->getTimestamp(epoch);
        return true;
    }

    void AravisPhotonicScienceCamera::configure(karabo::data::Hash& configuration) {
        if (configuration.has("roi.width")) {
            // width must be multiple of 16
            int width = configuration.get<int>("roi.width");
            if (width % 16 != 0) {
                width = 16 * std::round(width / 16.);
                configuration.set("roi.width", width);
            }
        }

        // Call parent's function
        AravisCamera::configure(configuration);
    }

    void AravisPhotonicScienceCamera::trigger() {
        const std::string& triggerMode = this->get<std::string>("triggerMode");
        if (triggerMode == "SW_Trigger") {
            GError* error = nullptr;
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            arv_camera_software_trigger(m_camera, &error);

            if (error != nullptr) {
                KARABO_LOG_FRAMEWORK_ERROR << this->getInstanceId()
                                           << ": arv_camera_software_trigger failed: " << error->message;
                g_clear_error(&error);
            }
        }
    }

    int AravisPhotonicScienceCamera::get_tick_frequency() {
        if (m_is_gv_device) {
            boost::mutex::scoped_lock camera_lock(m_camera_mtx);
            const int tickFrequency =
                  arv_device_get_integer_feature_value(m_device, "GevTimestampTickFrequency", nullptr);
            return tickFrequency;
        }

        return 0;
    }

    bool AravisPhotonicScienceCamera::is_flip_y_available() const {
        return true;
    }

} // namespace karabo
