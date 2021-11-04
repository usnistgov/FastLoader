//
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/22/21.
//

#ifndef INC_3DFASTLOADER_ADAPTIVE_VIEW_H
#define INC_3DFASTLOADER_ADAPTIVE_VIEW_H

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
#endif //INC_3DFASTLOADER_ADAPTIVE_VIEW_H
