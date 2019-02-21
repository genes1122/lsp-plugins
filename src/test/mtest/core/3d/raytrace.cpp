/*
 * raytrace.cpp
 *
 *  Created on: 21 февр. 2019 г.
 *      Author: sadko
 */


#include <test/mtest.h>
#include <core/3d/rt_context.h>
#include <core/3d/RayTrace3D.h>
#include <core/files/Model3DFile.h>

using namespace lsp;

//-----------------------------------------------------------------------------
// Performance test for equalizer module
MTEST_BEGIN("core.3d", raytrace)

    MTEST_MAIN
    {
        Scene3D scene;
        RayTrace3D trace;
        ray3d_t source, capture;

        // Initialize position of source and capture
        dsp::init_point_xyz(&source.z, -1.0f, 0.0f, 0.0f);
        dsp::init_vector_dxyz(&source.v, 0.0f, 0.0f, 0.3048f); // 12" speaker source

        dsp::init_point_xyz(&capture.z, 1.0f, 0.0f, 0.0f);
        dsp::init_vector_dxyz(&capture.v, 0.0f, 0.0f, 0.0508f); // 2" microphone diaphragm

        // Load scene
        MTEST_ASSERT(Model3DFile::load(&scene, "res/test/3d/empty-room-4x4x3.obj", true) == STATUS_OK);

        // Prepare raytracing
        MTEST_ASSERT(trace.init() == STATUS_OK);
        trace.set_sample_rate(48000);
        trace.set_energy_threshold(1e-6f);
        trace.set_tolerance(1e-4f);
        MTEST_ASSERT(trace.set_scene(&scene, true) == STATUS_OK);
        MTEST_ASSERT(trace.add_source(&source, RT_AS_OMNI, 1.0f) == STATUS_OK);
        MTEST_ASSERT(trace.add_capture(&capture, RT_AC_OMNIDIRECTIONAL, NULL, 0, 1.0f) == STATUS_OK);

        // Perform raytracing
        MTEST_ASSERT(trace.process(1.0f) == STATUS_OK);

        trace.destroy(false);
        scene.destroy();
    }
MTEST_END

