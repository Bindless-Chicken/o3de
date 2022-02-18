#include <ProbeGlobalIllumination/EditorProbeGlobalIlluminationComponent.h>

namespace AZ
{
    namespace Render
    {
        void EditorProbeGlobalIlluminationComponent::Reflect(AZ::ReflectContext* context)
        {
            BaseClass::Reflect(context);

            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<EditorProbeGlobalIlluminationComponent, BaseClass>()->Version(0);

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext
                        ->Class<EditorProbeGlobalIlluminationComponent>(
                            "Probe Global Illumination", "Simple Probe based global illumination.")
                            ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                                ->Attribute(AZ::Edit::Attributes::Category, "Atom")
                                ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                            ->ClassElement(AZ::Edit::ClassElements::Group, "Probe Bake")
                                ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                                ->UIElement(AZ::Edit::UIHandlers::Button, "Bake GI Probe", "Trigger the baking of the global illumination probe")
                                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Bake")
                                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorProbeGlobalIlluminationComponent::OnBakeProbeClicked);
                }
            }
        }

        EditorProbeGlobalIlluminationComponent::EditorProbeGlobalIlluminationComponent(const ProbeGlobalIlluminationComponentConfig& config)
            : BaseClass(config)
        {
        }

        void EditorProbeGlobalIlluminationComponent::Activate()
        {
            BaseClass::Activate();
        }
        void EditorProbeGlobalIlluminationComponent::Deactivate()
        {
            BaseClass::Deactivate();
        }

        AZ::u32 EditorProbeGlobalIlluminationComponent::OnBakeProbeClicked()
        {
            m_controller.BakeProbe();
            return AZ::Edit::PropertyRefreshLevels::None;
        }
    } // namespace Render
}
