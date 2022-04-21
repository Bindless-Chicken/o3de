/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#include <RHI/Buffer.h>
#include <RHI/BufferPool.h>
#include <RHI/BufferPoolResolver.h>
#include <RHI/MemoryView.h>
#include <RHI/Conversion.h>

#include <algorithm>

namespace AZ
{
    namespace Vulkan
    {
        BufferPoolResolver::BufferPoolResolver(Device& device, [[maybe_unused]] const RHI::BufferPoolDescriptor& descriptor)
            : ResourcePoolResolver(device)
        {
        }

        void* BufferPoolResolver::MapBuffer(const RHI::BufferMapRequest& request)
        {
            AZ_Assert(request.m_byteCount > 0, "ByteCount of request is null");
            auto* buffer = static_cast<Buffer*>(request.m_buffer);
            RHI::Ptr<Buffer> stagingBuffer = m_device.AcquireStagingBuffer(request.m_byteCount);
            if (stagingBuffer)
            {
                m_uploadPacketsLock.lock();
                m_uploadPackets.emplace_back();
                BufferUploadPacket& uploadRequest = m_uploadPackets.back();
                m_uploadPacketsLock.unlock();

                uploadRequest.m_attachmentBuffer = buffer;
                uploadRequest.m_byteOffset = request.m_byteOffset;
                uploadRequest.m_stagingBuffer = stagingBuffer;
                uploadRequest.m_byteSize = request.m_byteCount;

                return stagingBuffer->GetBufferMemoryView()->Map(RHI::HostMemoryAccess::Write);
            }

            return nullptr;
        }

        void BufferPoolResolver::Compile(const RHI::HardwareQueueClass hardwareClass)
        {
            // [IRC:TP] TEMP FIX AR000JB03D
            // Unmap all the staging buffers
            for (const BufferUploadPacket& packet : m_uploadPackets)
            {
                packet.m_stagingBuffer->GetBufferMemoryView()->Unmap(RHI::HostMemoryAccess::Write);
            }

            BuildUniquePacketList();

            // Clear our original list of packets to release owning references
            m_uploadPackets.clear();
            // END [IRC:TP] TEMP FIX AR000JB03D

            auto supportedQueuePipelineStages = m_device.GetCommandQueueContext().GetCommandQueue(hardwareClass).GetSupportedPipelineStages();

            VkBufferMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.pNext = nullptr;

            // [IRC:TP] TEMP FIX AR000JB03D
            for (const BufferUploadPacket& packet : m_uniqueUploadPackets)
            // END [IRC:TP] TEMP FIX AR000JB03D
            {
                // Filter stages and access flags
                VkPipelineStageFlags bufferPipelineFlags = RHI::FilterBits(GetResourcePipelineStateFlags(packet.m_attachmentBuffer->GetDescriptor().m_bindFlags), supportedQueuePipelineStages);
                VkAccessFlags bufferAccessFlags = RHI::FilterBits(GetResourceAccessFlags(packet.m_attachmentBuffer->GetDescriptor().m_bindFlags), GetSupportedAccessFlags(bufferPipelineFlags));

                const BufferMemoryView* destBufferMemoryView = packet.m_attachmentBuffer->GetBufferMemoryView();

                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.buffer = destBufferMemoryView->GetNativeBuffer();
                barrier.offset = destBufferMemoryView->GetOffset() + packet.m_byteOffset;
                barrier.size = packet.m_byteSize;

                {
                    barrier.srcAccessMask = bufferAccessFlags;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    m_prologueBarriers.emplace_back();
                    BarrierInfo& barrierInfo = m_prologueBarriers.back();
                    barrierInfo.m_srcStageMask = bufferPipelineFlags;
                    barrierInfo.m_dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    barrierInfo.m_barrier = barrier;
                }

                {
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask = bufferAccessFlags;
                    m_epilogueBarriers.emplace_back();
                    BarrierInfo& barrierInfo = m_epilogueBarriers.back();
                    barrierInfo.m_srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    barrierInfo.m_dstStageMask = bufferPipelineFlags;
                    barrierInfo.m_barrier = barrier;
                }
            }
        }

        void BufferPoolResolver::Resolve(CommandList& commandList)
        {
            auto& device = static_cast<Device&>(commandList.GetDevice());

            // [IRC:TP] TEMP FIX AR000JB03D
            for (const BufferUploadPacket& packet : m_uniqueUploadPackets)
            // END [IRC:TP] TEMP FIX AR000JB03D
            {
                Buffer* stagingBuffer = packet.m_stagingBuffer.get();
                Buffer* destBuffer = packet.m_attachmentBuffer;
                AZ_Assert(stagingBuffer, "Staging Buffer is null.");
                AZ_Assert(destBuffer, "Attachment Buffer is null.");

                RHI::CopyBufferDescriptor copyDescriptor;
                copyDescriptor.m_sourceBuffer = stagingBuffer;
                copyDescriptor.m_sourceOffset = 0;
                copyDescriptor.m_destinationBuffer = destBuffer;
                copyDescriptor.m_destinationOffset = static_cast<uint32_t>(packet.m_byteOffset);
                copyDescriptor.m_size = static_cast<uint32_t>(packet.m_byteSize);

                commandList.Submit(RHI::CopyItem(copyDescriptor));
                device.QueueForRelease(stagingBuffer);                
            }
        }

        void BufferPoolResolver::Deactivate()
        {
            m_uploadPackets.clear();
            m_epilogueBarriers.clear();
            m_prologueBarriers.clear();

            // [IRC:TP] TEMP FIX AR000JB03D
            m_uniqueUploadPackets.clear();
            // END [IRC:TP] TEMP FIX AR000JB03D
        }

        template<class T, class Predicate>
        void EraseResourceFromList(AZStd::vector<T>& list, const Predicate& predicate)
        {
            list.erase(AZStd::remove_if(list.begin(), list.end(), predicate), list.end());
        }

        void BufferPoolResolver::OnResourceShutdown(const RHI::Resource& resource)
        {
            AZStd::lock_guard<AZStd::mutex> lock(m_uploadPacketsLock);
            const Buffer* buffer = static_cast<const Buffer*>(&resource);

            auto eraseBeginIt = std::stable_partition(
                m_uploadPackets.begin(),
                m_uploadPackets.end(),
                [&buffer](const BufferUploadPacket& packet)
                {
                    // We want the elements to erase at the back of the vector
                    // so we can easily erase them by resizing the vector.
                    return packet.m_attachmentBuffer != buffer;
                }
            );
            for (auto it = eraseBeginIt; it != m_uploadPackets.end(); ++it)
            {
                it->m_stagingBuffer->GetBufferMemoryView()->Unmap(RHI::HostMemoryAccess::Write);
            }
            m_uploadPackets.resize(AZStd::distance(m_uploadPackets.begin(), eraseBeginIt));

            auto predicateBarriers = [&buffer](const BarrierInfo& barrierInfo)
            {
                const BufferMemoryView* bufferView = buffer->GetBufferMemoryView();
                if (barrierInfo.m_barrier.buffer != bufferView->GetNativeBuffer())
                {
                    return false;
                }

                VkDeviceSize barrierBegin = barrierInfo.m_barrier.offset;
                VkDeviceSize barrierEnd = barrierBegin + barrierInfo.m_barrier.size;
                VkDeviceSize bufferBegin = bufferView->GetOffset();
                VkDeviceSize bufferEnd = bufferBegin + bufferView->GetSize();

                return barrierBegin < bufferEnd && bufferBegin < barrierEnd;
            };
            EraseResourceFromList(m_prologueBarriers, predicateBarriers);
            EraseResourceFromList(m_epilogueBarriers, predicateBarriers);
        }

        void BufferPoolResolver::QueuePrologueTransitionBarriers(CommandList& commandList)
        {
            EmmitBarriers(commandList, m_prologueBarriers);
        }

        void BufferPoolResolver::QueueEpilogueTransitionBarriers(CommandList& commandList)
        {
            EmmitBarriers(commandList, m_epilogueBarriers);
        }

        void BufferPoolResolver::EmmitBarriers(CommandList& commandList, const AZStd::vector<BarrierInfo>& barriers) const
        {
            for (const BarrierInfo& barrierInfo : barriers)
            {
                vkCmdPipelineBarrier(
                    commandList.GetNativeCommandBuffer(),
                    barrierInfo.m_srcStageMask,
                    barrierInfo.m_dstStageMask,
                    0,
                    0,
                    nullptr,
                    1,
                    &barrierInfo.m_barrier,
                    0,
                    nullptr);
            }
        }

        // [IRC:TP] TEMP FIX AR000JB03D
        void BufferPoolResolver::BuildUniquePacketList()
        {
            // This struct acts as a hasher for the unordered set below.
            struct UploadPacketHasher
            {
                std::size_t operator()(const BufferUploadPacket& packet) const
                {
                    return reinterpret_cast<size_t>(packet.m_attachmentBuffer->GetBufferMemoryView()->GetNativeBuffer());
                }
            };

            // This struct acts as an equality comparator for the unordered set below.
            struct UploadPacketComparator
            {
                bool operator()(const BufferUploadPacket& lhs, const BufferUploadPacket& rhs) const
                {
                    if (lhs.m_attachmentBuffer->GetBufferMemoryView()->GetNativeBuffer() != rhs.m_attachmentBuffer->GetBufferMemoryView()->GetNativeBuffer())
                    {
                        return false;
                    }

                    // New insertion
                    const size_t lhsStart = lhs.m_attachmentBuffer->GetBufferMemoryView()->GetOffset() + lhs.m_byteOffset;
                    const size_t lhsEnd = lhsStart + lhs.m_byteSize - 1;

                    // Existing
                    const size_t rhsStart = rhs.m_attachmentBuffer->GetBufferMemoryView()->GetOffset() + rhs.m_byteOffset;
                    const size_t rhsEnd = rhsStart + rhs.m_byteSize - 1;

                    // The new range is fully contained in the existing range, no problem
                    if (lhsStart >= rhsStart && lhsEnd <= rhsEnd)
                    {
                        AZ_TracePrintfOnce("BufferPoolResolver", "Detected fully contained range in %s, skipping the upload.\n", lhs.m_attachmentBuffer->GetName().GetCStr());
                        return true;
                    }
                    // The new range is not fully contained in the existing range, this could lead to missing data!
                    else if (rhsEnd >= lhsStart && lhsEnd >= rhsStart)
                    {
                        AZ_Error("BufferPoolResolver", false, "Detected overlaping ranges in %s, skipping the upload. This may lead to missing data in one of the mapped range.", lhs.m_attachmentBuffer->GetName().GetCStr());
                        return true;
                    }

                    return false;
                }
            };

            // This set keeps a list of unique upload packets. The key is the address of the buffer being inserted. For tie breaker
            // the equality functions compares the ranges, thus allowing multiple writes to the same buffer at different locations
            // to occur.
            using UniqueBufferList_t = AZStd::unordered_set<BufferUploadPacket, UploadPacketHasher, UploadPacketComparator>;
            UniqueBufferList_t uniqueBufferList;

            // Run in reverse order of insertion to ensure we always enqueue
            // the most recent upload request.
            for (auto it = m_uploadPackets.rbegin(); it < m_uploadPackets.rend(); ++it)
            {
                const BufferUploadPacket& uploadPacket = *it;

                const AZStd::pair<UniqueBufferList_t::iterator, bool> insertQuery = uniqueBufferList.insert(uploadPacket);

                // If that was the first insertion
                if (insertQuery.second == true)
                {
                    m_uniqueUploadPackets.push_back(uploadPacket);
                }
            }

        }
        // END [IRC:TP] TEMP FIX AR000JB03D
    }
}
