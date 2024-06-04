// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.

#ifndef FAST_LOADER_UNIFIED_VIEW_DATA_H
#define FAST_LOADER_UNIFIED_VIEW_DATA_H

#ifdef HH_USE_CUDA

#include <cuda_runtime.h>
#include <hedgehog/hedgehog.h>

#include "abstract_view_data.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {
/// @brief View Data Used by CUDA computation,
/// @details The class provides useful functions for handling CUDA unified memory.
/// The prefetchMemory(int deviceId, cudaStream_t stream = 0) function can be used to asynchronously prefetch memory
/// to a device (cudaCpuDeviceId for CPU). After calling prefetchMemory, can use the same stream and call
/// recordEvent(cudaStream_t stream = 0), which will record the last operation on the stream. If the last event were a
/// prefetch, then this can be used with "synchronizeEvent()" to ensure that the copy had finished prior to using the memory on specified
/// device. If this is not done, then access to the memory could result in page faults, reducing performance.
/// @tparam DataType Type of data inside the View
template<class DataType>
class UnifiedViewData : public AbstractViewData<DataType>, public hh::ManagedMemory {
 private:
  DataType *data_ = nullptr; ///< Real Data
  cudaEvent_t event_ = {}; ///< cudaEvent useful for synchronizing data copying between host and device
  bool eventCreated_ = false; ///< flag used to track if the event has been created
  size_t viewSize_ = 0; ///< The total size of the view

 public:
  /// @brief Default constructor
  UnifiedViewData() = default;

  /// @brief Copy constructor
  /// @param rhs DefaultViewData to copy
  UnifiedViewData(UnifiedViewData const &rhs) : AbstractViewData<DataType>(rhs) {
    viewSize_ = std::accumulate(this->viewDims().cbegin(), this->viewDims().cend(), (size_t) 1, std::multiplies<>());
    checkCudaErrors(cudaMallocManaged((void **) &this->data_, sizeof(DataType) * viewSize_));
    std::copy_n(rhs.data_, viewSize_, this->data_);
  }

  /// @brief Constructor from the size of the view and the number of releases
  /// @param viewSize Size of the view (number of elements)
  /// @param numberOfRelease Number of releases for a view
  UnifiedViewData(size_t viewSize, size_t nbOfRelease) : AbstractViewData<DataType>(nbOfRelease),
                                                             viewSize_(viewSize) {
    checkCudaErrors(cudaMallocManaged((void **) &this->data_, sizeof(DataType) * viewSize));
  }

  /// @brief Constructor from the size of the view and the number of releases for all pyramid's level
  /// @param sizesPerLevel Sizes of the view (number of elements) for all the pyramid's level
  /// @param releasesPerLevel Number of releases for a view for all the pyramid's level
  /// @param level Level of the pyramid
  UnifiedViewData(std::vector<size_t> sizesPerLevel, std::vector<size_t> releasesPerLevel, size_t level) :
      UnifiedViewData(sizesPerLevel[level], releasesPerLevel[level]) {}

  /// @brief DefaultViewData destructor, will destroy created events and cudaFree raw array
  ~UnifiedViewData() override {
    if (eventCreated_) { checkCudaErrors(cudaEventDestroy(event_)); }
    checkCudaErrors(cudaFree(this->data_));
  }

  /// @brief Asynchronously prefetches memory to a particular GPU or CPU device
  /// @param deviceId the device Id to send too, use cudaCpuDeviceId to copy to CPU
  /// @param stream the cudaStream to use when copying asynchronously
  void prefetchMemory(int deviceId, cudaStream_t stream = nullptr) {
    checkCudaErrors(cudaMemPrefetchAsync(this->data_, sizeof(DataType) * this->viewSize_, deviceId, stream));
  }

  /// @brief Records the event associated with the memory
  /// @param the cudaStream to record on
  void recordEvent(cudaStream_t stream = nullptr) {
    if (!eventCreated_) {
      checkCudaErrors(cudaEventCreate(&event_));
      eventCreated_ = true;
    }
    checkCudaErrors(cudaEventRecord(event_, stream));
  }

  /// @brief Synchronizes the cudaEvent. Useful for ensuring data has been fully prefetched.
  void synchronizeEvent() {
    if (eventCreated_) {
      checkCudaErrors(cudaEventSynchronize(event_));
    }
  }

  /// @brief Raw data accessor
  /// @return Raw data
  DataType *data() const final { return data_; }

  /// @brief ViewRequest used method, to increment the releaseCount
  void postProcess() override { ++this->releaseCount_; }

  /// @brief Recycling flag accessor used by hedgehog
  /// @return True is the view can be recycled
  bool canBeRecycled() override { return this->releaseCount_ == this->nbOfRelease_; }

  /// @brief Return the memory to the Hedgehog memory manager
  void returnToMemoryManager() final { hh::ManagedMemory::returnToMemoryManager(); }

  /// @brief Get the piece from the memory manager
  void preProcess() final { this->synchronizeEvent(); }

  /// @brief Recycles the unified memory, and prefetch it back to the CPU before returning it to FastLoader's memory manager
  void clean() override {
    // Record event with default stream that is associated with the prefetch
    this->prefetchMemory(cudaCpuDeviceId);
    this->recordEvent();
  }
};

} // fl
} // internal

#endif // HH_USE_CUDA
#endif //FAST_LOADER_UNIFIED_VIEW_DATA_H
