/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/base.h>
#include <Atom/RPI.Public/FeatureProcessor.h>

namespace AZ
{
    namespace Render
    {
        class ProbeGlobalIllumination;

        using ProbeGlobalIlluminationHandle = AZStd::shared_ptr<ProbeGlobalIllumination>;

        //! This class provides general features and configuration for the probe based global illumination system.
        class ProbeGlobalIlluminationFeatureProcessorInterface
            : public RPI::FeatureProcessor
        {
        public:
            AZ_RTTI(AZ::Render::ProbeGlobalIlluminationFeatureProcessorInterface, "{EE2425D9-C136-41B0-9EB3-83EF99087EFC}");

            virtual ProbeGlobalIlluminationHandle AddProbeGlobalIllumination() = 0;
            virtual void RemoveProbeGlobalIllumination(ProbeGlobalIlluminationHandle& handle) = 0;

            // Set the location of the probe
            virtual void SetProbeTransform(const ProbeGlobalIlluminationHandle& handle, const AZ::Transform& transform) = 0;

            // Bake the probe
            virtual void BakeProbe(ProbeGlobalIlluminationHandle& handle) = 0;
        };
    } // namespace Render
} // namespace AZ
