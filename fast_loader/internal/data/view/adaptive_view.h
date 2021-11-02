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

template<class ViewType>
class AdaptiveView : public ViewType {
 private:
  static_assert(
      std::is_base_of_v<fl::internal::AbstractView<typename ViewType::data_t>, ViewType>,
      "The template argument of an AdaptiveView should be an AbstractView");
  using DataType = typename ViewType::data_t;

  std::shared_ptr<fl::internal::AdaptiveViewData<DataType>> const
      adaptiveViewData_;

 public:
  explicit AdaptiveView(std::shared_ptr<fl::internal::AdaptiveViewData<DataType>> const adaptiveViewData)
      : adaptiveViewData_(adaptiveViewData) {}

  /// @brief ViewData accessor
  /// @return Smart pointer to the internal view data
  std::shared_ptr<internal::AbstractViewData<DataType>> viewData() const override {
    return std::static_pointer_cast<internal::AbstractViewData<DataType>>(adaptiveViewData_);
  }

//  void viewData(std::shared_ptr<internal::AbstractViewData<DataType>> const viewData) override {
//    auto adaptiveViewData = std::dynamic_pointer_cast<internal::AdaptiveViewData<DataType>>(viewData);
//    if (adaptiveViewData) {
//      adaptiveViewData_ = adaptiveViewData;
////      this->viewData_ = std::static_pointer_cast<fl::internal::AbstractViewData<DataType>>(this->adaptiveViewData_);
//    } else {
//      throw std::runtime_error("Internal error: ViewDataType for a DefaultView is not a DefaultViewData");
//    }
//  }

//  void initialize(){
//    static_cast<fl::internal::AbstractView<DataType> *>(this)
//        ->viewData(std::static_pointer_cast<fl::internal::AbstractViewData<DataType>>(this->adaptiveViewData_));
//  }

};

}
}
#endif //INC_3DFASTLOADER_ADAPTIVE_VIEW_H
