//
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/22/21.
//

#ifndef INC_3DFASTLOADER_ADAPTIVE_VIEW_DATA_H
#define INC_3DFASTLOADER_ADAPTIVE_VIEW_DATA_H

#include "abstract_view_data.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

template<class DataType>
class AdaptiveViewData : public AbstractViewData<DataType>{
 private:
  DataType * const
    dataOrigin_;
 public:
  explicit AdaptiveViewData(DataType *const dataOrigin) : dataOrigin_(dataOrigin) {}
  ~AdaptiveViewData() override = default;

  DataType *data() override { return dataOrigin_; }

  void returnToMemoryManager() override {
    throw std::runtime_error("An Adaptive view is an internal view not made to be used with a memory manager.");
  }

};

}
}

#endif //INC_3DFASTLOADER_ADAPTIVE_VIEW_DATA_H
