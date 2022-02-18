#pragma once

#include <Atom/RPI.Public/FeatureProcessor.h>
#include <AzCore/Math/Transform.h>
#include <Atom/RPI.Public/Image/AttachmentImage.h>

namespace AZ
{
    namespace Render
    {
        class CubemapProcess;
        using CubemapProcessHandle = AZStd::shared_ptr<CubemapProcess>;

        //! This feature processor handles request to build cubemaps
        class CubemapFeatureProcessorInterface : public RPI::FeatureProcessor
        {
        public:
            AZ_RTTI(AZ::Render::CubemapFeatureProcessorInterface, "{897E1AAE-5564-47EB-A8CD-D20A379D6C20}", AZ::RPI::FeatureProcessor);

            //! Creates a new cubemap and returns a handle that can be used to reference it later.
            virtual CubemapProcessHandle AcquireCubemap() = 0;
            //! Releases a projected shadow given its ID.
            virtual void ReleaseCubemap(CubemapProcessHandle handle) = 0;

            //! Sets the world space transform of the cubemap
            virtual void SetShadowTransform(CubemapProcessHandle handle, const AZ::Transform& transform) = 0;
            //! Sets the persitent image for that cubemap
            virtual void SetPersistentImage(CubemapProcessHandle handle, Data::Instance<RPI::AttachmentImage> image) = 0;

            //! Bake the cubemap
            virtual void BakeCubemap(CubemapProcessHandle handle) = 0;
        };
    }
}
