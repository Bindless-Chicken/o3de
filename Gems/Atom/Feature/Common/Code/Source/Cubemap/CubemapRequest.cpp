#include <Cubemap/CubemapRequest.h>

#include <AzCore/std/smart_ptr/make_shared.h>
#include <Atom/RPI.Public/Pass/Specific/EnvironmentCubeMapPass.h>
#include <Atom/RPI.Public/View.h>
#include <AzCore/Math/MatrixUtils.h>
#include <Atom/RPI.Reflect/Pass/RasterPassData.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/RenderPipeline.h>

namespace AZ
{
    namespace Render
    {
        namespace CubemapProcess_private
        {
            static AZStd::string GetStringForFace(const CubemapFace face)
            {
                switch (face)
                {
                case CubemapFace::X_Positive:
                    return "X+";
                case CubemapFace::X_Negative:
                    return "X-";
                case CubemapFace::Y_Positive:
                    return "Y+";
                case CubemapFace::Y_Negative:
                    return "Y-";
                case CubemapFace::Z_Positive:
                    return "Z+";
                case CubemapFace::Z_Negative:
                    return "Z-";
                default:
                    return "invalid";
                }
            }

            static const Vector3 CameraBasis[6][3] =
            {
                { Vector3(0.0f, 1.0f, 0.0f), Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f) },
                { Vector3(0.0f, -1.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f) },
                { Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 1.0f, 0.0f) },
                { Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, -1.0f, 0.0f) },
                { Vector3(-1.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f) },
                { Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f) },
            };
        } // namespace CubemapProcess_private

        AZ::Render::CubemapProcess::CubemapProcess()
        {
            AZ::Uuid uuid = AZ::Uuid::CreateRandom();
            AZStd::string uuidString;
            uuid.ToString(uuidString);
            m_name = AZ::Name(uuidString);
        }

        void AZ::Render::CubemapProcess::CreatePipelines(RPI::Scene* scene)
        {
            for (const CubemapFaceEnumeratorValueAndString face : CubemapFaceMembers)
            {
                auto pipeline = CreatePipelineForFace(face.m_value);
                scene->AddRenderPipeline(pipeline);
                pipeline->SetDefaultView(GetViewForFace(face.m_value));
            }
        }

        void CubemapProcess::SetTransform(const AZ::Transform& transform)
        {
            m_transform = transform;

            // [IRC:TP 18-02-22] TODO: Trigger a reupload of the SRG
        }

        void CubemapProcess::SetPersistentImage(AZ::Data::Instance<RPI::AttachmentImage> image)
        {
            m_image = image;

            // [IRC:TP 18-02-22] TODO: Re wire the connection to the image in the pass
        }

        RPI::ViewPtr AZ::Render::CubemapProcess::GetViewForFace(const CubemapFace face) const
        {
            RPI::ViewPtr view = RPI::View::CreateView(AZ::Name("MainCamera"), RPI::View::UsageCamera);
            
            AZ::Matrix3x4 viewTransform;
            const Vector3* basis = &CubemapProcess_private::CameraBasis[static_cast<size_t>(face)][0];
            viewTransform.SetBasisAndTranslation(basis[0], basis[1], basis[2], m_transform.GetTranslation());
            view->SetCameraTransform(viewTransform);
            
            AZ::Matrix4x4 viewToClipMatrix;
            MakePerspectiveFovMatrixRH(viewToClipMatrix, AZ::Constants::HalfPi, 1.0f, 0.1f, 100.0f, true);
            view->SetViewToClipMatrix(viewToClipMatrix);
            return view;
        }

        RPI::RenderPipelinePtr CubemapProcess::CreatePipelineForFace(const CubemapFace face) const
        {
            AZ::RPI::RenderPipelineDescriptor environmentCubeMapPipelineDesc;
            environmentCubeMapPipelineDesc.m_name = GetPipelineNameForFace(face);
            environmentCubeMapPipelineDesc.m_mainViewTagName = "MainCamera";
            environmentCubeMapPipelineDesc.m_renderSettings.m_multisampleState =
                RPI::RPISystemInterface::Get()->GetApplicationMultisampleState();
            environmentCubeMapPipelineDesc.m_renderSettings.m_size.m_width = m_faceSize;
            environmentCubeMapPipelineDesc.m_renderSettings.m_size.m_height = m_faceSize;
            environmentCubeMapPipelineDesc.m_executeOnce = true;

            RPI::RenderPipelinePtr renderpipeline = AZ::RPI::RenderPipeline::CreateRenderPipeline(environmentCubeMapPipelineDesc);

            AZStd::shared_ptr<RPI::EnvironmentCubeMapPassData> passData = AZStd::make_shared<RPI::EnvironmentCubeMapPassData>();
            passData->m_position = m_transform.GetTranslation();
            passData->m_faceID = static_cast<u32>(face);

            RPI::PassDescriptor environmentCubeMapPassDescriptor(Name("EnvironmentCubeMapPass"));
            environmentCubeMapPassDescriptor.m_passData = passData;

            RPI::Ptr<RPI::EnvironmentCubeMapPass> cubemapPass = RPI::EnvironmentCubeMapPass::Create(environmentCubeMapPassDescriptor);
            cubemapPass->SetRenderPipeline(renderpipeline.get());
            cubemapPass->OverrideOutputImage(m_image);

            renderpipeline->GetRootPass()->AddChild(cubemapPass);

            return renderpipeline;
        }

        AZStd::string AZ::Render::CubemapProcess::GetPipelineNameForFace(const CubemapFace face) const
        {
            return AZStd::string::format("%s_%s", m_name.GetCStr(), CubemapProcess_private::GetStringForFace(face).c_str());
        }

    } // namespace Render
} // namespace AZ
