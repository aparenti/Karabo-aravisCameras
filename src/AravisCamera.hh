/*
 * Author: <parenti>
 *
 * Created on September, 2019, 10:46 AM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#ifndef KARABO_ARAVISCAMERA_HH
#define KARABO_ARAVISCAMERA_HH

#include <unordered_map>

extern "C" {
#include <arv.h>
}

#include <image_source/CameraImageSource.hh>
#include <karabo/karabo.hpp>

#include "version.hh" // provides ARAVISCAMERAS_PACKAGE_VERSION

/**
 * The main Karabo namespace
 */
namespace karabo {

    enum class Result { SUCCESS, FAIL, NOT_AVAILABLE };

    class AravisCamera : public CameraImageSource {
       public:
        // Add reflection information and Karabo framework compatibility to this class
        KARABO_CLASSINFO(AravisCamera, "AravisCamera", ARAVISCAMERAS_PACKAGE_VERSION)

        /**
         * Necessary method as part of the factory/configuration system
         * @param expected Will contain a description of expected parameters for this device
         */
        static void expectedParameters(karabo::data::Schema& expected);

        /**
         * Constructor providing the initial configuration in form of a Hash object.
         * If this class is constructed using the configuration system the Hash object will
         * already be validated using the information of the expectedParameters function.
         * The configuration is provided in a key/value fashion.
         */
        explicit AravisCamera(const karabo::data::Hash& config);

        /**
         * The destructor will be called in case the device gets killed
         */
        virtual ~AravisCamera();

        /**
         * This function acts as a hook and is called after an reconfiguration request was received,
         * but BEFORE this reconfiguration request is actually merged into this device's state.
         *
         * The reconfiguration information is contained in the Hash object provided as an argument.
         * You have a chance to change the content of this Hash before it is merged into the device's current state.
         *
         * NOTE: (a) The incomingReconfiguration was validated before
         *       (b) If you do not need to handle the reconfigured data, there is no need to implement this function.
         *           The reconfiguration will automatically be applied to the current state.
         * @param incomingReconfiguration The reconfiguration information as was triggered externally
         */
        virtual void preReconfigure(karabo::data::Hash& incomingReconfiguration) override;


        /**
         * This function acts as a hook and is called after an reconfiguration request was received,
         * and AFTER this reconfiguration request got merged into this device's current state.
         * You may access any (updated or not) parameters using the usual getters and setters.
         * @code
         * int i = get<int>("myParam");
         * @endcode
         */
        virtual void postReconfigure() override;

       protected:
        bool m_is_base_class;      // False for derived classes
        bool m_is_gv_device;       // True for GEV cameras
        bool m_is_uv_device;       // True for USB3V cameras
        bool m_arv_camera_trigger; // Use arv_camera to access trigger
        bool m_is_device_reset_available;
        bool m_is_frame_count_available;

        mutable boost::mutex m_camera_mtx; // Object lock is needed for ArvCamera etc.
        ArvCamera* m_camera;
        ArvDevice* m_device;
        ArvChunkParser* m_parser;

        bool isFeatureAvailable(const std::string& feature) const;
        virtual void configure(karabo::data::Hash& configuration);

        virtual bool synchronize_timestamp();
        virtual bool configure_timestamp_chunk();
        bool m_chunk_mode;
        karabo::data::Timestamp m_reference_karabo_time;

        gint m_width;
        gint m_height;
        guint m_buffer_size;
        ArvPixelFormat m_format;
        virtual bool get_region(gint& x, gint& y, gint& width, gint& height);
        virtual int get_tick_frequency();
        virtual bool get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts);

        bool is_frame_count_available() const;
        virtual bool is_flip_x_available() const;
        virtual bool is_flip_y_available() const;

        bool set_region(int& x, int& y, int& width, int& height);
        bool set_binning(int& bin_x, int& bin_y);

        unsigned int m_max_correction_time;
        double m_min_latency;
        double m_max_latency;

       private:
        karabo::net::Strand::Pointer m_outputStrand;

        bool m_need_schema_update;
        void initialize();

        bool m_connect; // Set to false to quit connection loop
        bool m_is_connected;
        boost::asio::deadline_timer m_reconnect_timer;
        unsigned short m_failed_connections;
        static boost::mutex m_connect_mtx; // Class lock is needed for ArvInterface

        void connect(const boost::system::error_code& ec);
        void connection_failed_helper(const std::string& message, const std::string& detailed_msg = "");

        bool verify_vendor_and_model(const std::string& vendor, const std::string& model);

        bool set_auto_packet_size();
        bool set_exposure_time(double& exposure_time);
        bool set_frame_rate(bool enable, double frame_rate = 0.);
        bool get_gain(double& absGain, double& normGain);
        bool set_gain(double& absGain, double& normGain, bool normalized);
        bool set_frame_count(gint64& frame_count);

        boost::asio::deadline_timer m_poll_timer;

        bool m_is_acquiring;
        void acquire();
        void acquire_failed_helper(const std::string& detailed_msg);
        void stop();
        virtual void trigger();
        void refresh();
        void reset();
        virtual void resetCamera();

        void getPathsByTag(std::vector<std::string>& paths, const std::string& tags);

        void disableElement(const std::string& key, karabo::data::Schema& schemaUpdate);

        Result getBoolFeature(const std::string& feature, bool& value);
        Result getStringFeature(const std::string& feature, std::string& value);
        Result getIntFeature(const std::string& feature, long long& value);
        Result getFloatFeature(const std::string& feature, double& value);

        Result setBoolFeature(const std::string& feature, bool& value);
        Result setStringFeature(const std::string& feature, std::string& value);
        Result setIntFeature(const std::string& feature, long long& value);
        Result setFloatFeature(const std::string& feature, double& value);

        void clear_camera();
        void clear_stream();

        static void stream_cb(void* context, ArvStreamCallbackType type, ArvBuffer* buffer);
        void process_buffer(ArvBuffer* buffer);
        static void control_lost_cb(ArvGvDevice* gv_device, void* context);

        void pollOnce(karabo::data::Hash& h);
        void pollCamera(const boost::system::error_code& ec);
        void pollGenicamFeatures(const std::vector<std::string>& paths, karabo::data::Hash& h);
        bool updateOutputSchema();
        template <class T>
        void writeOutputChannels(const void* data, gint width, gint height, const karabo::data::Timestamp& ts);
        void updateFrameRate();

        bool resolveHostname(const std::string& hostname, std::string& ip_address, std::string& message);

        mutable boost::mutex m_stream_mtx; // Object lock for ArvStream
        ArvStream* m_stream;

        bool m_is_binning_available;
        bool m_is_exposure_time_available;
        bool m_is_flip_x_available;
        bool m_is_flip_y_available;
        bool m_is_frame_rate_available;
        bool m_is_gain_available;
        bool m_is_gain_auto_available;

        std::string m_exposure_time_feature;

        static const std::set<ArvPixelFormat> m_supportedPixelFormats;
        std::unordered_map<ArvPixelFormat, std::string> m_pixelFormatOptions;

        unsigned long long m_errorCount;
        ArvBufferStatus m_lastError;
        std::unordered_map<ArvBufferStatus, std::string> m_bufferStatus;

        // Image latency
        karabo::data::Epochstamp m_timer;
        unsigned long m_counter;
        double m_mean_latency;

        bool m_isContinuousMode;
        // Images to be acquired
        unsigned long long m_imgsToBeAcquired;

        karabo::xms::Encoding m_encoding;
        std::vector<unsigned long long> m_shape;

        std::vector<uint16_t> m_unpackedData;
    };
} // namespace karabo

#endif
