#include <Cubemap/CubemapFeatureProcessor.h>
#include <Cubemap/CubemapRequest.h>

#include <Atom/RPI.Public/Pass/Specific/EnvironmentCubeMapPass.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Reflect/Pass/RasterPassData.h>
#include <AzCore/Math/MatrixUtils.h>
#include <AzCore/std/smart_ptr/make_shared.h>

namespace AZ
{
    namespace Render
    {
        void CubemapFeatureProcessor::Reflect(ReflectContext* context)
        {
            if (auto* serializeContext = azrtti_cast<SerializeContext*>(context))
            {
                serializeContext->Class<CubemapFeatureProcessor, FeatureProcessor>()->Version(0);
            }
        }

        void CubemapFeatureProcessor::Activate()
        {
        }

        void CubemapFeatureProcessor::Deactivate()
        {
        }

        CubemapProcessHandle CubemapFeatureProcessor::AcquireCubemap()
        {
            return AZStd::make_shared<CubemapProcess>();
        }

        void CubemapFeatureProcessor::ReleaseCubemap(CubemapProcessHandle handle)
        {
            AZ_UNUSED(handle);
        }

        void CubemapFeatureProcessor::SetShadowTransform(CubemapProcessHandle handle, const AZ::Transform& transform)
        {
            handle->SetTransform(transform);
        }

        void CubemapFeatureProcessor::SetPersistentImage(CubemapProcessHandle handle, Data::Instance<RPI::AttachmentImage> image)
        {
            handle->SetPersistentImage(image);
        }

        void CubemapFeatureProcessor::BakeCubemap(CubemapProcessHandle handle)
        {
            handle->CreatePipelines(GetParentScene());
        }
    } // namespace Render
} // namespace AZ
