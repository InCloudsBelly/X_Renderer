#ifndef X_GRAPHICSBLAS_H
#define X_GRAPHICSBLAS_H

#include "Common.h"
#include "Buffer.h"

#include <vector>

namespace X {

    namespace Graphics {

        class GraphicsDevice;
        class MemoryManager;
        class ASBuilder;

        struct BLASDesc {
            VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;

            std::vector<VkAccelerationStructureGeometryKHR> geometries;
            std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges;
        };

        class BLAS {

            friend GraphicsDevice;
            friend ASBuilder;

        public:
            BLAS(GraphicsDevice* device, BLASDesc desc);

            ~BLAS();

            void Allocate();

            VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
            VkAccelerationStructureBuildSizesInfoKHR sizesInfo;
            VkAccelerationStructureBuildRangeInfoKHR* rangeInfo;

            Ref<Buffer> buffer;
            VkAccelerationStructureKHR accelerationStructure = VK_NULL_HANDLE;

            VkDeviceAddress bufferDeviceAddress;

        private:
            GraphicsDevice* device;

            std::vector<VkAccelerationStructureGeometryKHR> geometries;
            std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges;

            VkBuildAccelerationStructureFlagsKHR flags;

        };

    }

}

#endif