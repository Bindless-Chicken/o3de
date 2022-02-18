#pragma once

#include <Atom/Feature/ProbeGlobalIllumination/ProbeGlobalIlluminationFeatureProcessorInterface.h>
#include <Atom/RPI.Reflect/Base.h>

namespace AZ
{
    namespace RHI
    {
        class ImagePool;
    }

    namespace Render
    {
        class ProbeGlobalIlluminationFeatureProcessor final
            : public ProbeGlobalIlluminationFeatureProcessorInterface
        {
        public:
            AZ_RTTI(
                AZ::Render::ProbeGlobalIlluminationFeatureProcessor,
                "{35CACBB2-9D73-4DC7-A24F-28CD719A4CBE}",
                ProbeGlobalIlluminationFeatureProcessorInterface);

            static void Reflect(AZ::ReflectContext* context);

            ProbeGlobalIlluminationFeatureProcessor() = default;
            virtual ~ProbeGlobalIlluminationFeatureProcessor() = default;

            void Activate() override;
            
            ProbeGlobalIlluminationHandle AddProbeGlobalIllumination() override;
            void RemoveProbeGlobalIllumination(ProbeGlobalIlluminationHandle& handle) override;

            void SetProbeTransform(const ProbeGlobalIlluminationHandle& handle, const AZ::Transform& transform) override;

            void BakeProbe(ProbeGlobalIlluminationHandle& handle) override;

        private:
            AZ_DISABLE_COPY_MOVE(ProbeGlobalIlluminationFeatureProcessor);

            AZStd::vector<ProbeGlobalIlluminationHandle> m_probeGlobalIlluminationHandles;
        };
    } // namespace Render
} // namespace AZ
