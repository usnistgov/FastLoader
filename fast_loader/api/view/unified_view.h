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

#ifndef FAST_LOADER_UNIFIED_VIEW_H
#define FAST_LOADER_UNIFIED_VIEW_H

#ifdef HH_USE_CUDA

#include "../../core/data/view/abstract_view.h"
#include "../../core/data/view_data/unified_view_data.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief View for CUDA computation using unified memory
/// @tparam DataType Type of data inside the View
template<class DataType>
class UnifiedView : public internal::AbstractView<DataType> {
 private:
  std::shared_ptr<internal::UnifiedViewData<DataType>>
      viewData_{}; ///< Internal data of the view

 public:
  /// @brief Default constructor
  UnifiedView() = default;

  /// @brief Default copy constructor
  /// @param view View to copy
  UnifiedView(UnifiedView<DataType> const &view) : internal::AbstractView<DataType>(view) {
    viewData_ = std::make_shared<internal::UnifiedViewData<DataType>>(*view.viewData_.get());
  }

  /// @brief Make a deep copy of the view
  /// @return A UnifiedView copied from this
  std::shared_ptr<internal::AbstractView<DataType>> deepCopy() override {
    return std::static_pointer_cast<internal::AbstractView<DataType>>(std::make_shared<fl::UnifiedView<DataType>>(*this));
  }

  /// @brief Data accessor
  /// @return Internal data
  std::shared_ptr<internal::AbstractViewData<DataType>> viewData() const override {
    return std::static_pointer_cast<internal::AbstractViewData<DataType>>(viewData_);
  }

  /// @brief Asynchronously prefetches memory to a particular GPU or CPU device
  /// @param deviceId the device Id to send too, use cudaCpuDeviceId to copy to CPU
  /// @param stream the cudaStream to use when copying asynchronously
  void prefetchMemory(int deviceId, cudaStream_t stream = nullptr) {
    this->viewData_->prefetchMemory(deviceId, stream);
  }

  /// @brief Records the event associated with the memory
  /// @param the cudaStream to record on
  void recordEvent(cudaStream_t stream = nullptr) { this->viewData_->recordEvent(stream); }

  /// @brief Synchronizes the cudaEvent. Useful for ensuring data has been fully prefetched.
  void synchronizeEvent() { this->viewData_->synchronizeEvent(); }

  /// @brief ViewData setter
  /// @param viewData Data to set
  /// @warning The viewData should be of type UnifiedViewData
  /// @throw std::runtime If the viewData is not of UnifiedViewData type
  void viewData(std::shared_ptr<internal::AbstractViewData<DataType>> const viewData) override {
    auto viewDataCast = std::dynamic_pointer_cast<internal::UnifiedViewData<DataType>>(viewData);
    if (viewDataCast) {
      viewData_ = viewDataCast;
    } else {
      throw std::runtime_error("Internal error: ViewDataType for a UnifiedView is not a UnifiedViewData");
    }
  }

};

} // fl

#endif // HH_USE_CUDA
#endif //FAST_LOADER_UNIFIED_VIEW_H
