//
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/25/21.
//

#ifndef INC_3DFASTLOADER_DIRECT_TO_COPY_STATE_H
#define INC_3DFASTLOADER_DIRECT_TO_COPY_STATE_H

#include <hedgehog/hedgehog.h>
#include "../../data/adaptive_tile_request.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

template<class Viewtype>
class DirectToCopyState :
    public hh::AbstractState<fl::internal::AdaptiveTileRequest<Viewtype>, fl::internal::AdaptiveTileRequest<Viewtype>> {

 public:
  DirectToCopyState() = default;
  virtual ~DirectToCopyState() = default;

  void execute(std::shared_ptr<fl::internal::AdaptiveTileRequest<Viewtype>> ptr) override {
    if (!(ptr->needCopyFromPhysicalTileLoader())) {
      this->push(ptr);
    }
  }

};

}
}

#endif //INC_3DFASTLOADER_DIRECT_TO_COPY_STATE_H
