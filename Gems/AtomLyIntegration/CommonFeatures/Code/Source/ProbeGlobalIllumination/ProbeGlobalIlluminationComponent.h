#pragma once

#include <ProbeGlobalIllumination/ProbeGlobalIlluminationComponentController.h>
#include <AzFramework/Components/ComponentAdapter.h>

namespace AZ
{
    namespace Render
    {
        class ProbeGlobalIlluminationComponent final
            : public AzFramework::Components::
                  ComponentAdapter<ProbeGlobalIlluminationComponentController, ProbeGlobalIlluminationComponentConfig>
        {
        public:
            using BaseClass = AzFramework::Components::
                ComponentAdapter<ProbeGlobalIlluminationComponentController, ProbeGlobalIlluminationComponentConfig>;
            AZ_COMPONENT(AZ::Render::ProbeGlobalIlluminationComponent, "{E3E1343E-238B-46BA-873B-C5BA0762968A}", BaseClass);

            ProbeGlobalIlluminationComponent() = default;
            ProbeGlobalIlluminationComponent(const ProbeGlobalIlluminationComponentConfig& config);

            static void Reflect(AZ::ReflectContext* context);
        };
    }
}
