#pragma once

#include <Atom/Feature/Cubemap/CubemapFeatureProcessorInterface.h>

namespace AZ
{
    namespace Render
    {
        class CubemapFeatureProcessor final : public CubemapFeatureProcessorInterface
        {
        public:
            AZ_RTTI(
                AZ::Render::CubemapFeatureProcessor,
                "{225E9CC0-F369-4242-AB31-7D74B97B0267}",
                AZ::Render::CubemapFeatureProcessorInterface);

            static void Reflect(AZ::ReflectContext* context);

            CubemapFeatureProcessor() = default;
            virtual ~CubemapFeatureProcessor() = default;

            // FeatureProcessor overrides ...
            void Activate() override;
            void Deactivate() override;

            // CubemapFeatureProcessorInterface overrides ...
            CubemapProcessHandle AcquireCubemap() override;
            void ReleaseCubemap(CubemapProcessHandle handle) override;
            void SetShadowTransform(CubemapProcessHandle handle, const AZ::Transform& transform) override;
            void SetPersistentImage(CubemapProcessHandle handle, Data::Instance<RPI::AttachmentImage> image) override;
            void BakeCubemap(CubemapProcessHandle handle) override;

        private:
            AZStd::vector<CubemapProcessHandle> m_cubemapHandles;
        };
    } // namespace Render
} // namespace AZ
