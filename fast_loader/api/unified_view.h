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
//
// Created by Bardakoff, Alexandre (IntlAssoc) on 9/23/20.
//

#ifndef FASTLOADER_UNIFIED_VIEW_H
#define FASTLOADER_UNIFIED_VIEW_H

#ifdef HH_USE_CUDA
#include "../internal/data/view/abstract_view.h"
#include "../internal/data/view_data/unified_view_data.h"

/// @brief FastLoader namespace
namespace fl {
template<class DataType>
 class UnifiedView : public internal::AbstractView<DataType> {
 private:
  std::shared_ptr<internal::UnifiedViewData<DataType>>
      viewData_{}; ///< Internal data of the view

 public:
  UnifiedView() = default;

  UnifiedView(UnifiedView<DataType> const &view) : internal::AbstractView<DataType>(view) {
    viewData_ = std::make_shared<internal::UnifiedViewData<DataType>>(*view.viewData_.get());
  }

  std::shared_ptr<internal::AbstractView<DataType>> deepCopy() override {
    return std::static_pointer_cast<internal::AbstractView<DataType>>(std::make_shared<fl::UnifiedView<DataType>>(*this));
  }

  std::shared_ptr<internal::AbstractViewData<DataType>> viewData() const override {
    return std::static_pointer_cast<internal::AbstractViewData<DataType>>(viewData_);
  }

  void prefetchMemory(int deviceId, cudaStream_t stream = 0) {
    this->viewData_->prefetchMemory(deviceId, stream);
  }

  void recordEvent(cudaStream_t stream = 0) {
    this->viewData_->recordEvent(stream);
  }

  void synchronizeEvent() {
    this->viewData_->synchronizeEvent();
  }

  void viewData(std::shared_ptr<internal::AbstractViewData<DataType>> const viewData) override {
    auto viewDataCast = std::dynamic_pointer_cast<internal::UnifiedViewData<DataType>>(viewData);
    if (viewDataCast) {
      viewData_ = viewDataCast;
    } else {
      throw std::runtime_error("Internal error: ViewDataType for a UnifiedView is not a UnifiedViewData");
    }
  }

};
}

#endif // HH_USE_CUDA

#endif //FASTLOADER_UNIFIED_VIEW_H
