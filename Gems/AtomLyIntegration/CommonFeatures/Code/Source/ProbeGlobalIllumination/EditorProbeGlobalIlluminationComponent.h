#pragma once

#include <Atom/Feature/Utils/EditorRenderComponentAdapter.h>
#include <ProbeGlobalIllumination/ProbeGlobalIlluminationComponent.h>

namespace AZ
{
    namespace Render
    {
        class EditorProbeGlobalIlluminationComponent final
            : public EditorRenderComponentAdapter<ProbeGlobalIlluminationComponentController, ProbeGlobalIlluminationComponent, ProbeGlobalIlluminationComponentConfig>
        {
        public:
            using BaseClass = EditorRenderComponentAdapter<ProbeGlobalIlluminationComponentController, ProbeGlobalIlluminationComponent, ProbeGlobalIlluminationComponentConfig>;

            AZ_EDITOR_COMPONENT(AZ::Render::EditorProbeGlobalIlluminationComponent, "{038C1390-1311-4B99-AD43-F050E8B5FF76}", BaseClass);

            static void Reflect(AZ::ReflectContext* context);

            EditorProbeGlobalIlluminationComponent() = default;
            EditorProbeGlobalIlluminationComponent(const ProbeGlobalIlluminationComponentConfig& config);

            void Activate() override;
            void Deactivate() override;

        private:
            AZ::u32 OnBakeProbeClicked();
        };
    }
}
