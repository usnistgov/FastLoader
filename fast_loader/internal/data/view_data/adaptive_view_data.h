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

/// @brief Data used by an AdaptiveView using a logical cached tile as data.
/// @tparam DataType
template<class DataType>
class AdaptiveViewData : public AbstractViewData<DataType>{
 private:
  DataType * const
    dataOrigin_; ///< Origin of the logical cached tile data
 public:
  /// @brief AdaptiveViewData constructor accepting the origin of the logical cached tile data
  /// @param dataOrigin Origin of the logical cached tile data
  explicit AdaptiveViewData(DataType *const dataOrigin) : dataOrigin_(dataOrigin) {}

  /// @brief Default destructor
  ~AdaptiveViewData() override = default;

  /// @brief Data accessor
  /// @return Origin of the logical cached tile data
  DataType *data() override { return dataOrigin_; }

  /// @brief returnToMemoryManager implementation from Hedgehog library, throw an exception in every case
  /// @warning Should not be used because data is a piece of cache and not an Hedgehog managed memory
  /// @throw std::runtime_error in every case, should not be used
  void returnToMemoryManager() final {
    throw std::runtime_error("An Adaptive view is an internal view not made to be used with a memory manager.");
  }

};

}
}

#endif //INC_3DFASTLOADER_ADAPTIVE_VIEW_DATA_H
