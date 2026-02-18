/*
 * Author: <parenti>
 *
 * Created on January 18, 2022,  3:28 PM
 *
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#ifndef KARABO_ARAVISBASLERBASE_HH
#define KARABO_ARAVISBASLERBASE_HH

#include <karabo/karabo.hpp>

#include "AravisCamera.hh"


/**
 * The main Karabo namespace
 */
namespace karabo {

    class AravisBaslerBase : public AravisCamera {
       public:
        static void expectedParameters(karabo::data::Schema& expected);

        explicit AravisBaslerBase(const karabo::data::Hash& config);

        virtual ~AravisBaslerBase() = default;

        bool get_timestamp(ArvBuffer* buffer, karabo::data::Timestamp& ts, const std::string& tsFeature);

        virtual bool is_flip_x_available() const override;

        virtual bool is_flip_y_available() const override;

        std::string aravisBaslerScene();

       protected:
        bool m_ptp_enabled;
        gint64 m_reference_camera_timestamp;

       private:
        void resetCamera() override;
    };

} // namespace karabo

#endif // KARABO_ARAVISBASLERBASE_HH
