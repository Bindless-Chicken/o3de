#include <ProbeGlobalIllumination/ProbeGlobalIlluminationComponent.h>

namespace AZ
{
    namespace Render
    {
        ProbeGlobalIlluminationComponent::ProbeGlobalIlluminationComponent(const ProbeGlobalIlluminationComponentConfig& config)
            : BaseClass(config)
        {
        }

        void ProbeGlobalIlluminationComponent::Reflect(AZ::ReflectContext* context)
        {
            BaseClass::Reflect(context);

            if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<ProbeGlobalIlluminationComponent, BaseClass>()->Version(0);
            }
        }
    }
}
