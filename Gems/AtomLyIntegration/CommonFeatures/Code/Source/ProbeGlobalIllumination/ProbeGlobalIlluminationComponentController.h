#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Component/EntityId.h>
#include <Atom/Feature/ProbeGlobalIllumination/ProbeGlobalIlluminationFeatureProcessorInterface.h>
#include <AzCore/Component/TransformBus.h>

namespace AZ
{
    namespace Render
    {

        class ProbeGlobalIlluminationComponentConfig final
            : public AZ::ComponentConfig
        {
        public:
            AZ_RTTI(AZ::Render::ProbeGlobalIlluminationComponentConfig, "{3767548F-0BB3-48FD-980E-472E106D0675}", ComponentConfig);
            AZ_CLASS_ALLOCATOR(ProbeGlobalIlluminationComponentConfig, SystemAllocator, 0);
            static void Reflect(AZ::ReflectContext* context);

            ProbeGlobalIlluminationComponentConfig() = default;

            AZ::u64 m_entityId{ EntityId::InvalidEntityId };
        };

        class ProbeGlobalIlluminationComponentController final
            : private TransformNotificationBus::Handler
        {
        public:
            friend class EditorProbeGlobalIlluminationComponent;

            //AZ_CLASS_ALLOCATOR(ProbeGlobalIlluminationComponentController, AZ::SystemAllocator, 0);
            //AZ_RTTI(AZ::Render::ProbeGlobalIlluminationComponentController, "{1C71F110-44E3-4422-BE89-85D6C48C960B}");

            AZ_TYPE_INFO(AZ::Render::ProbeGlobalIlluminationComponentController, "{1C71F110-44E3-4422-BE89-85D6C48C960B}");

            ProbeGlobalIlluminationComponentController() = default;
            ProbeGlobalIlluminationComponentController(const ProbeGlobalIlluminationComponentConfig& config);

            static void Reflect(AZ::ReflectContext* context);

            static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
            static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
            static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
            static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

            void Activate(const AZ::EntityId entityID);
            void Deactivate();

            void SetConfiguration(const ProbeGlobalIlluminationComponentConfig& config);
            const ProbeGlobalIlluminationComponentConfig& GetConfiguration() const;

            void BakeProbe();

        private:
            AZ_DISABLE_COPY(ProbeGlobalIlluminationComponentController);

            ProbeGlobalIlluminationComponentConfig m_configuration;
            AZ::EntityId m_entityId;

            
            ProbeGlobalIlluminationFeatureProcessorInterface* m_featureProcessor = nullptr;
            ProbeGlobalIlluminationHandle m_handle = nullptr;

            void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;
        };
    }
}
