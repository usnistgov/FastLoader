//
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/25/21.
//

#ifndef INC_3DFASTLOADER_TO_TILE_LOADER_STATE_H
#define INC_3DFASTLOADER_TO_TILE_LOADER_STATE_H

#include <hedgehog/hedgehog.h>
#include "../../data/adaptive_tile_request.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief Filter that test if an AdaptiveTileRequest needs to have data copied from a tile loader
/// @tparam ViewType Type of the view
template<class ViewType>
class ToTileLoaderState :
    public hh::AbstractState<fl::internal::TileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>> {
 public:
  /// @brief Default constructor
  ToTileLoaderState() = default;
  /// @brief Default destructor
  virtual ~ToTileLoaderState() = default;

  /// @brief Execute method implementation form Hedgehog library
  /// @param ptr AdaptiveTileRequest to test
  void execute(std::shared_ptr<fl::internal::AdaptiveTileRequest<ViewType>> ptr) override {
    if (ptr->needCopyFromPhysicalTileLoader()) {
      this->push(std::static_pointer_cast<fl::internal::TileRequest<ViewType>>(ptr));
    }
  }

};
}
}

#endif //INC_3DFASTLOADER_TO_TILE_LOADER_STATE_H
