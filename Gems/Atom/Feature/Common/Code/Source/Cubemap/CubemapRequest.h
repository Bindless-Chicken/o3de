#pragma once

#include <Atom/Feature/Cubemap/CubemapFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Pass/Specific/EnvironmentCubeMapPass.h>
#include <AzCore/Preprocessor/Enum.h>

namespace AZ
{
    namespace Render
    {
        AZ_ENUM_CLASS_WITH_UNDERLYING_TYPE(CubemapFace, uint8_t, X_Positive, X_Negative, Y_Positive, Y_Negative, Z_Positive, Z_Negative);

        class CubemapProcess
        {
        public:
            CubemapProcess();
            ~CubemapProcess() = default;

            // Create the pipeline and schedule them for processing
            void CreatePipelines(RPI::Scene* scene);

            // Set the location of the cubemap
            void SetTransform(const AZ::Transform& transform);
            // Set the persitent image used as support
            void SetPersistentImage(AZ::Data::Instance<RPI::AttachmentImage> image);

        private:
            RPI::RenderPipelinePtr CreatePipelineForFace(const CubemapFace face) const;
            RPI::ViewPtr GetViewForFace(const CubemapFace face) const;

            AZStd::string GetPipelineNameForFace(const CubemapFace face) const;

        private:
            u32 m_faceSize = 1024;
            AZ::Name m_name;

            AZ::Transform m_transform;
            AZ::Data::Instance<RPI::AttachmentImage> m_image;

            AZStd::array<RPI::ViewPtr, 6> m_view;
            RHI::Ptr<RPI::EnvironmentCubeMapPass> m_pass;
        };
    } // namespace Render
} // namespace AZ
