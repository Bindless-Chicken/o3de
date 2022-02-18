/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Math/MatrixUtils.h>
#include <Atom/RHI/FrameScheduler.h>
#include <Atom/RHI/Factory.h>
#include <Atom/RHI.Reflect/Format.h>
#include <Atom/RPI.Public/Image/AttachmentImage.h>
#include <Atom/RPI.Public/Image/AttachmentImagePool.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <Atom/RPI.Public/Pass/PassUtils.h>
#include <Atom/RPI.Public/Pass/Specific/EnvironmentCubeMapPass.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RHI/RHISystemInterface.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <Atom/RPI.Public/Pass/RenderPass.h>

namespace AZ
{
    namespace RPI
    {
        namespace EnvironmentCubeMapPass_private
        {
            static const AZStd::string IntermediateSlotName("TransientResult");
            static const AZStd::string OutputSlotName("Output");

            static const AZStd::string TransientResourceName("TransientResult");

            static const AZStd::string CopyPassTemplateName("FullscreenCopyTemplate");
            static const AZStd::string CopyPassName("Copy");
            static const AZStd::string CopyPassInputName("Input");
            static const AZStd::string CopyPassOutputName("Output");

        } // EnvironmentCubeMapPass_private

        Ptr<EnvironmentCubeMapPass> EnvironmentCubeMapPass::Create(const PassDescriptor& passDescriptor)
        {
            Ptr<EnvironmentCubeMapPass> pass = aznew EnvironmentCubeMapPass(passDescriptor);
            return pass;
        }

        EnvironmentCubeMapPass::EnvironmentCubeMapPass(const PassDescriptor& passDescriptor)
            : ParentPass(passDescriptor)
        {
            // load pass data
            const EnvironmentCubeMapPassData* passData = PassUtils::GetPassData<EnvironmentCubeMapPassData>(passDescriptor);
            if (passData == nullptr)
            {
                AZ_Error("PassSystem", false, "[EnvironmentCubeMapPass '%s']: Trying to construct without valid EnvironmentCubeMapPassData!", GetPathName().GetCStr());
                return;
            }

            m_position = passData->m_position;
            m_faceID = passData->m_faceID;

            // setup viewport
            m_viewportState.m_minX = 0;
            m_viewportState.m_minY = 0;
            m_viewportState.m_maxX = static_cast<float>(CubeMapFaceSize);
            m_viewportState.m_maxY = static_cast<float>(CubeMapFaceSize);

            // setup scissor
            m_scissorState.m_minX = 0;
            m_scissorState.m_minY = 0;
            m_scissorState.m_maxX = static_cast<int16_t>(CubeMapFaceSize);
            m_scissorState.m_maxY = static_cast<int16_t>(CubeMapFaceSize);
        }

        EnvironmentCubeMapPass::~EnvironmentCubeMapPass()
        {
            for (uint32_t i = 0; i < NumCubeMapFaces; ++i)
            {
                delete [] m_textureData[i];
            }
        }

        void EnvironmentCubeMapPass::SetDefaultView()
        {
            if (m_pipeline)
            {
                m_pipeline->SetDefaultView(m_view);
            }
        }
        void EnvironmentCubeMapPass::OverrideOutputImage(Data::Instance<RPI::AttachmentImage> image)
        {
            m_overrideImage = image;
        }


        void EnvironmentCubeMapPass::BuildInternal()
        {
            // Create the transient image attachement used to run the render pipeline
            {
                m_outputImageDesc = RHI::ImageDescriptor::Create2D(
                    RHI::ImageBindFlags::Color | RHI::ImageBindFlags::ShaderReadWrite | RHI::ImageBindFlags::CopyRead, CubeMapFaceSize, CubeMapFaceSize,
                    RHI::Format::R16G16B16A16_FLOAT);

                // create output PassAttachment
                m_passAttachment = aznew PassAttachment();
                m_passAttachment->m_name = EnvironmentCubeMapPass_private::TransientResourceName;
                AZ::Name attachmentPath(AZStd::string::format("%s.%s", GetPathName().GetCStr(), m_passAttachment->m_name.GetCStr()));
                m_passAttachment->m_path = attachmentPath;
                m_passAttachment->m_lifetime = RHI::AttachmentLifetimeType::Transient;
                m_passAttachment->m_descriptor = m_outputImageDesc;

                // create pass attachment binding
                PassAttachmentBinding outputAttachment;
                outputAttachment.m_name = EnvironmentCubeMapPass_private::IntermediateSlotName;
                outputAttachment.m_slotType = PassSlotType::InputOutput;
                outputAttachment.m_attachment = m_passAttachment;
                outputAttachment.m_scopeAttachmentUsage = RHI::ScopeAttachmentUsage::RenderTarget;

                m_attachmentBindings.push_back(outputAttachment);
            }

            // Create the output (imported) image attachement
            {
                m_outputPassAttachment = aznew PassAttachment();
                m_outputPassAttachment->m_name = "Output";
                m_outputPassAttachment->m_importedResource = m_overrideImage;
                m_outputPassAttachment->m_lifetime = RHI::AttachmentLifetimeType::Imported;
                m_outputPassAttachment->m_descriptor = m_overrideImage->GetDescriptor();
                m_outputPassAttachment->ComputePathName(GetPathName());

                // create pass attachment binding
                PassAttachmentBinding outputAttachment;
                outputAttachment.m_name = EnvironmentCubeMapPass_private::OutputSlotName;
                outputAttachment.m_slotType = PassSlotType::InputOutput;
                outputAttachment.m_attachment = m_outputPassAttachment;
                outputAttachment.m_scopeAttachmentUsage = RHI::ScopeAttachmentUsage::RenderTarget;

                m_attachmentBindings.push_back(outputAttachment);

            }

            ParentPass::BuildInternal();
        }

        void EnvironmentCubeMapPass::CreateChildPassesInternal()
        {
            PassSystemInterface* passSystem = PassSystemInterface::Get();

            // First create the user defined pipeline for cubemap rendering
            {
                PassRequest childRequest;
                childRequest.m_templateName = "EnvironmentCubeMapPipeline";
                childRequest.m_passName = "Face";

                PassConnection childInputConnection;
                childInputConnection.m_localSlot = "Output";
                childInputConnection.m_attachmentRef.m_pass = "Parent";
                childInputConnection.m_attachmentRef.m_attachment = EnvironmentCubeMapPass_private::IntermediateSlotName;
                childRequest.m_connections.emplace_back(childInputConnection);

                m_childPass = passSystem->CreatePassFromRequest(&childRequest);
                AddChild(m_childPass);
            }

            // Create the copy pipeline
            {
                // [IRC:TP 18-02-22]
                // USE CLONE!!!!!
                // Not using clone here will directly edit the library version, breaking any following passes!
                AZStd::shared_ptr<RPI::PassTemplate> copyPassTemplate =
                    RPI::PassSystemInterface::Get()->GetPassTemplate(AZ::Name(EnvironmentCubeMapPass_private::CopyPassTemplateName))->Clone();
                for (PassSlot& slot : copyPassTemplate->m_slots)
                {
                    if (slot.m_name == AZ::Name(EnvironmentCubeMapPass_private::CopyPassOutputName))
                    {
                        slot.m_imageViewDesc = AZStd::make_shared<RHI::ImageViewDescriptor>();
                        slot.m_imageViewDesc->m_mipSliceMin = 0;
                        slot.m_imageViewDesc->m_mipSliceMax = 0;
                        slot.m_imageViewDesc->m_arraySliceMin = m_faceID;
                        slot.m_imageViewDesc->m_arraySliceMax = m_faceID;
                        slot.m_imageViewDesc->m_isCubemap = true;

                        break;
                    }
                }

                PassConnection copyInputConnection;
                copyInputConnection.m_localSlot = EnvironmentCubeMapPass_private::CopyPassInputName;
                copyInputConnection.m_attachmentRef.m_pass = "Parent";
                copyInputConnection.m_attachmentRef.m_attachment = EnvironmentCubeMapPass_private::IntermediateSlotName;
                copyPassTemplate->AddOutputConnection(copyInputConnection);

                PassConnection copyOutputConnection;
                copyOutputConnection.m_localSlot = EnvironmentCubeMapPass_private::CopyPassOutputName;
                copyOutputConnection.m_attachmentRef.m_pass = "Parent";
                copyOutputConnection.m_attachmentRef.m_attachment = EnvironmentCubeMapPass_private::OutputSlotName;
                copyPassTemplate->AddOutputConnection(copyOutputConnection);

                m_copyPass = passSystem->CreatePassFromTemplate(copyPassTemplate, AZ::Name(EnvironmentCubeMapPass_private::CopyPassName));
                AddChild(m_copyPass);
            }
        }

        void EnvironmentCubeMapPass::FrameBeginInternal(FramePrepareParams params)
        {
            params.m_scissorState = m_scissorState;
            params.m_viewportState = m_viewportState;

            RHI::FrameGraphAttachmentInterface attachmentDatabase = params.m_frameGraphBuilder->GetAttachmentDatabase();
            attachmentDatabase.CreateTransientImage(RHI::TransientImageDescriptor(m_passAttachment->GetAttachmentId(), m_outputImageDesc));
            attachmentDatabase.ImportImage(m_outputPassAttachment->GetAttachmentId(), m_overrideImage->GetRHIImage());

            ParentPass::FrameBeginInternal(params);;
        }

        void EnvironmentCubeMapPass::FrameEndInternal()
        {
            // [IRC:TP 18-02-22] TODO: Renable the readback feature (will be in the feature processor tho)
            // 
            //m_readBackLock.lock();
            //if (m_renderFace < NumCubeMapFaces)
            //{
            //    if (!m_readBackRequested)
            //    {
            //        // delay a number of frames before requesting the readback
            //        if (m_readBackDelayFrames < NumReadBackDelayFrames)
            //        {
            //            m_readBackDelayFrames++;
            //        }
            //        else
            //        {
            //            m_readBackRequested = true;
            //            AZStd::string readbackName = AZStd::string::format("%s_%s", m_passAttachment->GetAttachmentId().GetCStr(), GetName().GetCStr());
            //            m_attachmentReadback->ReadPassAttachment(m_passAttachment.get(), AZ::Name(readbackName));
            //        }
            //    }

            //    // set the appropriate render camera transform for the next frame
            //    AZ::Matrix3x4 viewTransform;
            //    const Vector3* basis = &CameraBasis[m_renderFace][0];
            //    viewTransform.SetBasisAndTranslation(basis[0], basis[1], basis[2], m_position);

            //    m_view->SetCameraTransform(viewTransform);
            //    m_pipeline->SetDefaultView(m_view);
            //}
            //m_readBackLock.unlock();

            ParentPass::FrameEndInternal();
        }      

        void EnvironmentCubeMapPass::AttachmentReadbackCallback(const AZ::RPI::AttachmentReadback::ReadbackResult& readbackResult)
        {
            RHI::ImageSubresourceLayout imageLayout = RHI::GetImageSubresourceLayout(readbackResult.m_imageDescriptor.m_size, readbackResult.m_imageDescriptor.m_format);

            delete [] m_textureData[m_renderFace];

            // copy face texture data
            m_textureData[m_renderFace] = new uint8_t[imageLayout.m_bytesPerImage];
            uint32_t bytesRead = (uint32_t)readbackResult.m_dataBuffer->size();
            memcpy(m_textureData[m_renderFace], readbackResult.m_dataBuffer->data(), bytesRead);

            m_textureFormat = readbackResult.m_imageDescriptor.m_format;

            m_readBackLock.lock();
            {
                // move to the next face
                m_renderFace++;
                m_readBackRequested = false;
                m_readBackDelayFrames = 0;
            }
            m_readBackLock.unlock();
        }

    }   // namespace RPI
}   // namespace AZ
