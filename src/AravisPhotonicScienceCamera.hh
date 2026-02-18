/*
 * Author: <parenti>
 *
 * Created on March 17, 2020, 12:20 PM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#ifndef KARABO_ARAVISPHOTONICSCIENCECAMERA_HH
#define KARABO_ARAVISPHOTONICSCIENCECAMERA_HH

#include <karabo/karabo.hpp>

#include "AravisCamera.hh"
#include "version.hh" // provides ARAVISCAMERAS_PACKAGE_VERSION

/**
 * The main Karabo namespace
 */
namespace karabo {

    class AravisPhotonicScienceCamera final : public AravisCamera {
       public:
        KARABO_CLASSINFO(AravisPhotonicScienceCamera, "AravisPhotonicScienceCamera", ARAVISCAMERAS_PACKAGE_VERSION)

        static void expectedParameters(karabo::data::Schema& expected);

        explicit AravisPhotonicScienceCamera(const karabo::data::Hash& config);

        virtual ~AravisPhotonicScienceCamera() = default;

        bool synchronize_timestamp() override;

        bool configure_timestamp_chunk() override;

        bool get_region(gint& x, gint& y, gint& width, gint& height) override;

        int get_tick_frequency() override;

        bool get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts) override;

        bool is_flip_y_available() const override;

       private:
        void configure(karabo::data::Hash& configuration) override;
        void trigger() override;

        double m_reference_camera_timestamp;
    };

} // namespace karabo

#endif // KARABO_ARAVISPHOTONICSCIENCECAMERA_HH
