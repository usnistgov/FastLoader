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

template<class Viewtype>
class ToTileLoaderState :
    public hh::AbstractState<fl::internal::TileRequest<Viewtype>, fl::internal::AdaptiveTileRequest<Viewtype>> {
 public:
  ToTileLoaderState() = default;
  virtual ~ToTileLoaderState() = default;

  void execute(std::shared_ptr<fl::internal::AdaptiveTileRequest<Viewtype>> ptr) override {
    if (ptr->needCopyFromPhysicalTileLoader()) {
//      global_mutex.lock();
//      std::cout << "Transmitting Tile (" << ptr->indexRowLogicalTile() << ", " << ptr->indexColLogicalTile() << ", " << ptr->indexLayerLogicalTile() << ") #id" << ptr->id() << std::endl;
//      global_mutex.unlock();
      this->push(std::static_pointer_cast<fl::internal::TileRequest<Viewtype>>(ptr));
    }
  }

};
}
}

#endif //INC_3DFASTLOADER_TO_TILE_LOADER_STATE_H
