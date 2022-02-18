#include <ProbeGlobalIllumination/ProbeGlobalIllumination.h>

#include <Atom/RPI.Reflect/Pass/EnvironmentCubeMapPassData.h>
#include <Atom/RPI.Public/Pass/Specific/EnvironmentCubeMapPass.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RHI/Factory.h>
#include <Atom/RHI/ImagePool.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>
#include <Atom/RPI.Public/Image/AttachmentImagePool.h>

#include <AzFramework/Visibility/IVisibilitySystem.h>

#include <Atom/RPI.Reflect/Asset/AssetUtils.h>

namespace AZ
{
    namespace Render
    {
        ProbeGlobalIllumination::~ProbeGlobalIllumination()
        {
            Data::AssetBus::MultiHandler::BusDisconnect();
            m_meshFeatureProcessor->ReleaseMesh(m_visualizationMeshHandle);
            m_cubemapFeatureProcessor->ReleaseCubemap(m_cubemapHandle);
        }

        void ProbeGlobalIllumination::OnAssetReady(Data::Asset<Data::AssetData> asset)
        {
            if (m_visualizationMaterialAsset.GetId() == asset.GetId())
            {
                m_visualizationMaterialAsset = asset;
                Data::AssetBus::MultiHandler::BusDisconnect(asset.GetId());

                m_meshFeatureProcessor->SetMaterialAssignmentMap(
                    m_visualizationMeshHandle, AZ::RPI::Material::FindOrCreate(m_visualizationMaterialAsset));

                m_meshFeatureProcessor->SetVisible(m_visualizationMeshHandle, true);

                auto& srgs = m_meshFeatureProcessor->GetObjectSrgs(m_visualizationMeshHandle);
                auto nameIdx = RHI::ShaderInputNameIndex(AZ::Name("m_cubemap"));
                const bool result = srgs[0]->SetImage(nameIdx, m_image);

                AZ_Assert(result, "Could not bind image to the object srg.");

                m_meshFeatureProcessor->QueueObjectSrgForCompile(m_visualizationMeshHandle);
            }
        }

        void ProbeGlobalIllumination::OnAssetError(Data::Asset<Data::AssetData> asset)
        {
            AZ_Error(
                "ReflectionProbe", false, "Failed to load ReflectionProbe dependency asset %s", asset.ToString<AZStd::string>().c_str());
            Data::AssetBus::MultiHandler::BusDisconnect(asset.GetId());
        }

        void ProbeGlobalIllumination::Init(RPI::Scene* scene)
        {
            AZ_Assert(scene, "ReflectionProbe::Init called with a null Scene pointer");

            m_scene = scene;

            // Create the persistent image
            {
                Data::Instance<RPI::AttachmentImagePool> pool = RPI::ImageSystemInterface::Get()->GetSystemAttachmentPool();

                AZ::Uuid uuid = AZ::Uuid::CreateRandom();
                AZStd::string uuidStr;
                uuid.ToString(uuidStr);
                AZ::Name name = AZ::Name(AZStd::string::format("ProbeGICubemap_%s", uuidStr.c_str()));

                m_imageDescriptor = RHI::ImageDescriptor::CreateCubemap(
                    RHI::ImageBindFlags::ShaderReadWrite | RHI::ImageBindFlags::Color | RHI::ImageBindFlags::CopyRead |
                        RHI::ImageBindFlags::CopyWrite,
                    1024, RHI::Format::R16G16B16A16_FLOAT);

                m_imageViewDesc.m_isCubemap = true;
                m_image = RPI::AttachmentImage::Create(*pool.get(), m_imageDescriptor, name, nullptr, &m_imageViewDesc);
            }

            // Create the cubemap baker
            {
                m_cubemapFeatureProcessor = m_scene->GetFeatureProcessor<Render::CubemapFeatureProcessorInterface>();
                m_cubemapHandle = m_cubemapFeatureProcessor->AcquireCubemap();
                m_cubemapFeatureProcessor->SetShadowTransform(m_cubemapHandle, m_transform);
                m_cubemapFeatureProcessor->SetPersistentImage(m_cubemapHandle, m_image);
                // [IRC:TP 18-02-22] Baking the cubemap here produces a crash, something not ready yet?
                //m_cubemapFeatureProcessor->BakeCubemap(m_cubemapHandle);
            }


            // Create the debuig visualizer
            {
                m_meshFeatureProcessor = m_scene->GetFeatureProcessor<Render::MeshFeatureProcessorInterface>();
                m_visualizationModelAsset = AZ::RPI::AssetUtils::GetAssetByProductPath<AZ::RPI::ModelAsset>(
                    "Models/ReflectionProbeSphere.azmodel", AZ::RPI::AssetUtils::TraceLevel::Assert);

                m_visualizationMeshHandle = m_meshFeatureProcessor->AcquireMesh(MeshHandleDescriptor{ m_visualizationModelAsset });
                m_meshFeatureProcessor->SetExcludeFromReflectionCubeMaps(m_visualizationMeshHandle, true);
                m_meshFeatureProcessor->SetRayTracingEnabled(m_visualizationMeshHandle, false);
                m_meshFeatureProcessor->SetTransform(m_visualizationMeshHandle, m_transform);

                m_visualizationMaterialAsset = AZ::RPI::AssetUtils::GetAssetByProductPath<AZ::RPI::MaterialAsset>(
                    "Materials/CubemapVisualization/CubemapVisualization.azmaterial", AZ::RPI::AssetUtils::TraceLevel::Assert);
                m_visualizationMaterialAsset.QueueLoad();
                Data::AssetBus::MultiHandler::BusConnect(m_visualizationMaterialAsset.GetId());
            }
        }

        void ProbeGlobalIllumination::BakeProbe()
        {
            m_cubemapFeatureProcessor->BakeCubemap(m_cubemapHandle);
        }

        void ProbeGlobalIllumination::SetTransform(const AZ::Transform& transform)
        {
            m_transform = transform;
            m_meshFeatureProcessor->SetTransform(m_visualizationMeshHandle, m_transform);
            m_cubemapFeatureProcessor->SetShadowTransform(m_cubemapHandle, m_transform);
        }
    } // namespace Render
} // namespace AZ
