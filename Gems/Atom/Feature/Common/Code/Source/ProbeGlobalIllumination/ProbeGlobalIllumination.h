#pragma once
#include <Atom/RPI.Reflect/Base.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RHI.Reflect/ImageDescriptor.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/Feature/Cubemap/CubemapFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Model/Model.h>

namespace AZ
{
    namespace RPI
    {
        class Scene;
        class EnvironmentCubeMapPass;
    } // namespace RPI

    namespace RHI
    {
        class ImagePool;
    } // namespace RHI

    namespace Render
    {
        class ProbeGlobalIllumination : public AZ::Data::AssetBus::MultiHandler
        {
        public:
            ProbeGlobalIllumination() = default;
            ~ProbeGlobalIllumination();

            void Init(RPI::Scene* scene);

            void BakeProbe();

            void SetTransform(const AZ::Transform& transform);
        private:

            // AZ::Data::AssetBus::Handler overrides...
            void OnAssetReady(Data::Asset<Data::AssetData> asset) override;
            void OnAssetError(Data::Asset<Data::AssetData> asset) override;

        private:
            //AZ::RPI::RenderPipelineId m_environmentCubeMapPipelineId;
            //RPI::Ptr<RPI::EnvironmentCubeMapPass> m_environmentCubeMapPass = nullptr;

            AZ::Transform m_transform = AZ::Transform::CreateIdentity();

            Data::Instance<RPI::AttachmentImage> m_image;
            RHI::ImageDescriptor m_imageDescriptor;
            RHI::ImageViewDescriptor m_imageViewDesc;

            RPI::Scene* m_scene = nullptr;

            // Cubemap baking
            AZ::Render::CubemapFeatureProcessorInterface* m_cubemapFeatureProcessor = nullptr;
            AZ::Render::CubemapProcessHandle m_cubemapHandle;

            // Debug probe visualization
            AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor = nullptr;
            Data::Asset<RPI::ModelAsset> m_visualizationModelAsset;
            Data::Asset<RPI::MaterialAsset> m_visualizationMaterialAsset;
            AZ::Render::MeshFeatureProcessorInterface::MeshHandle m_visualizationMeshHandle;
        };
    } // namespace Render
} // namespace AZ
