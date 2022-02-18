#include "ProbeGlobalIlluminationComponentController.h"
#include <ProbeGlobalIllumination/ProbeGlobalIlluminationComponentController.h>


#include <Atom/RPI.Public/Scene.h>

#include <AzCore/Serialization/SerializeContext.h>

namespace AZ
{
    namespace Render
    {
        void ProbeGlobalIlluminationComponentConfig::Reflect(AZ::ReflectContext* context)
        {
            if (auto* serializeContext = azrtti_cast<SerializeContext*>(context))
            {
                serializeContext->Class<ProbeGlobalIlluminationComponentConfig>()->Version(0);
            }
        }

        ProbeGlobalIlluminationComponentController::ProbeGlobalIlluminationComponentController(
            const ProbeGlobalIlluminationComponentConfig& config)
            : m_configuration(config)
        {
        }

        void ProbeGlobalIlluminationComponentController::Reflect(AZ::ReflectContext* context)
        {
            if (auto* serializeContext = azrtti_cast<SerializeContext*>(context))
            {
                serializeContext->Class<ProbeGlobalIlluminationComponentController>()->Version(0);
            }
        }

        void ProbeGlobalIlluminationComponentController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
        {
            AZ_UNUSED(dependent);
        }

        void ProbeGlobalIlluminationComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC("ProbeGlobalIlluminationService"));
        }

        void ProbeGlobalIlluminationComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC("ProbeGlobalIlluminationService"));
        }

        void ProbeGlobalIlluminationComponentController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
        {
            required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
        }

        void ProbeGlobalIlluminationComponentController::Activate(const AZ::EntityId entityId)
        {
            m_entityId = entityId;
            TransformNotificationBus::Handler::BusConnect(m_entityId);

            m_featureProcessor = RPI::Scene::GetFeatureProcessorForEntity<ProbeGlobalIlluminationFeatureProcessorInterface>(entityId);
            m_handle = m_featureProcessor->AddProbeGlobalIllumination();

            const auto transform = TransformBus::FindFirstHandler(entityId);
            m_featureProcessor->SetProbeTransform(m_handle, transform->GetWorldTM());
        }

        void ProbeGlobalIlluminationComponentController::Deactivate()
        {
            TransformNotificationBus::Handler::BusDisconnect();

            m_featureProcessor->RemoveProbeGlobalIllumination(m_handle);
            m_featureProcessor = nullptr;
        }

        void ProbeGlobalIlluminationComponentController::SetConfiguration(const ProbeGlobalIlluminationComponentConfig& config)
        {
            m_configuration = config;
        }

        const ProbeGlobalIlluminationComponentConfig& ProbeGlobalIlluminationComponentController::GetConfiguration() const
        {
            return m_configuration;
        }

        void ProbeGlobalIlluminationComponentController::BakeProbe()
        {
            m_featureProcessor->BakeProbe(m_handle);
        }

        void ProbeGlobalIlluminationComponentController::OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world)
        {
            AZ_UNUSED(local);
            m_featureProcessor->SetProbeTransform(m_handle, world);
        }
    } // namespace Render
}
