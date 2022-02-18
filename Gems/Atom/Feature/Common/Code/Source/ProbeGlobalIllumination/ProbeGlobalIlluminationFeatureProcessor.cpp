#include <ProbeGlobalIllumination/ProbeGlobalIlluminationFeatureProcessor.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>

#include <Atom/RHI/Factory.h>
#include <Atom/RHI/RHISystemInterface.h>

#include <ProbeGlobalIllumination/ProbeGlobalIllumination.h>

namespace AZ
{
    namespace Render
    {
        void ProbeGlobalIlluminationFeatureProcessor::Reflect(ReflectContext* context)
        {
            if (SerializeContext* serializeContext = azrtti_cast<SerializeContext*>(context))
            {
                serializeContext->Class<ProbeGlobalIlluminationFeatureProcessor, FeatureProcessor>()->Version(0);
            }
        }

        void ProbeGlobalIlluminationFeatureProcessor::Activate()
        {
            RHI::RHISystemInterface* rhiSystem = RHI::RHISystemInterface::Get();
            RHI::Ptr<RHI::Device> device = rhiSystem->GetDevice();

            // [IRC:TP 18-02-22] TODO: Investigate if this is still needed
            EnableSceneNotification();
        }

        ProbeGlobalIlluminationHandle ProbeGlobalIlluminationFeatureProcessor::AddProbeGlobalIllumination()
        {
            ProbeGlobalIlluminationHandle probeGlobalIllumination = AZStd::make_shared<ProbeGlobalIllumination>();
            probeGlobalIllumination->Init(GetParentScene());
            m_probeGlobalIlluminationHandles.push_back(probeGlobalIllumination);
            return probeGlobalIllumination;
        }

        void ProbeGlobalIlluminationFeatureProcessor::RemoveProbeGlobalIllumination(ProbeGlobalIlluminationHandle& handle)
        {
            auto itEntry = AZStd::find_if(
                m_probeGlobalIlluminationHandles.begin(), m_probeGlobalIlluminationHandles.end(),
                [&](ProbeGlobalIlluminationHandle const& entry)
                {
                    return (entry == handle);
                }
            );
            m_probeGlobalIlluminationHandles.erase(itEntry);
            handle = nullptr;
        }

        void ProbeGlobalIlluminationFeatureProcessor::SetProbeTransform(
            const ProbeGlobalIlluminationHandle& handle, const AZ::Transform& transform)
        {
            handle->SetTransform(transform);
        }

        void ProbeGlobalIlluminationFeatureProcessor::BakeProbe(ProbeGlobalIlluminationHandle& handle)
        {
            handle->BakeProbe();
        }
    } // namespace Render
} // namespace AZ
