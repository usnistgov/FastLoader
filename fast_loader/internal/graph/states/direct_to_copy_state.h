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

/// @brief State used to filter the AdaptiveTileRequest that does not need copies from the tile loader
/// @tparam Viewtype Type of the view
template<class Viewtype>
class DirectToCopyState :
    public hh::AbstractState<fl::internal::AdaptiveTileRequest<Viewtype>, fl::internal::AdaptiveTileRequest<Viewtype>> {
 public:
  /// @brief Default constructor
  DirectToCopyState() = default;
  /// @brief Default destructor
  virtual ~DirectToCopyState() = default;

  /// @brief Execute method implementation used to filter the AdaptiveTileRequest
  /// @param ptr AdaptiveTileRequest to filter
  void execute(std::shared_ptr<fl::internal::AdaptiveTileRequest<Viewtype>> ptr) override {
    if (!(ptr->needCopyFromPhysicalTileLoader())) {
      this->push(ptr);
    }
  }

};

}
}

#endif //INC_3DFASTLOADER_DIRECT_TO_COPY_STATE_H
