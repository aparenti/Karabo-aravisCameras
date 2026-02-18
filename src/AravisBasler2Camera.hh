/*
 * Author: <parenti>
 *
 * Created on April 27, 2021,  6:00 PM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#ifndef KARABO_ARAVISBASLER2CAMERA_HH
#define KARABO_ARAVISBASLER2CAMERA_HH

#include <karabo/karabo.hpp>

#include "AravisBaslerBase.hh"
#include "version.hh" // provides ARAVISCAMERAS_PACKAGE_VERSION

/**
 * The main Karabo namespace
 */
namespace karabo {

    class AravisBasler2Camera final : public AravisBaslerBase {
       public:
        KARABO_CLASSINFO(AravisBasler2Camera, "AravisBasler2Camera", ARAVISCAMERAS_PACKAGE_VERSION)

        static void expectedParameters(karabo::data::Schema& expected);

        explicit AravisBasler2Camera(const karabo::data::Hash& config);

        virtual ~AravisBasler2Camera() = default;

        bool synchronize_timestamp() override;

        bool configure_timestamp_chunk() override;

        int get_tick_frequency() override;

        bool get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts) override;
    };

} // namespace karabo

#endif // KARABO_ARAVISBASLER2CAMERA_HH
