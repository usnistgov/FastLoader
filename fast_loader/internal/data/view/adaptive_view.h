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

#ifndef INC_FAST_LOADER_ADAPTIVE_VIEW_H
#define INC_FAST_LOADER_ADAPTIVE_VIEW_H

#include "abstract_view.h"
#include "../view_data/adaptive_view_data.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {

/// @brief AdaptiveView that uses a logical cache as a data
/// @tparam ViewType Type of the FL view
template<class ViewType>
class AdaptiveView : public ViewType {
 private:
  static_assert(
      std::is_base_of_v<fl::internal::AbstractView<typename ViewType::data_t>, ViewType>,
      "The template argument of an AdaptiveView should be an AbstractView");
  using DataType = typename ViewType::data_t; ///< Sample type (AbstractView element type)

  std::shared_ptr<fl::internal::AdaptiveViewData<DataType>> const
      adaptiveViewData_; ///< Adaptive view data holding the cache data

 public:
  /// @brief AdaptiveView constructor
  /// @param adaptiveViewData Main data used by the AdaptiveView
  explicit AdaptiveView(std::shared_ptr<fl::internal::AdaptiveViewData<DataType>> const adaptiveViewData)
      : adaptiveViewData_(adaptiveViewData) {}

  /// @brief ViewData Accessor to the AbstractViewData
  /// @return Smart pointer to the internal view data
  std::shared_ptr<internal::AbstractViewData<DataType>> viewData() const override {
    return std::static_pointer_cast<internal::AbstractViewData<DataType>>(adaptiveViewData_);
  }
};

}
}
#endif //INC_FAST_LOADER_ADAPTIVE_VIEW_H
