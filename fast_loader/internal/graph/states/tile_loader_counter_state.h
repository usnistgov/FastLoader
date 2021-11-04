//
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/25/21.
//

#ifndef INC_3DFASTLOADER_TILE_LOADER_COUNTER_STATE_H
#define INC_3DFASTLOADER_TILE_LOADER_COUNTER_STATE_H

#include <hedgehog/hedgehog.h>
#include "../../data/adaptive_tile_request.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief State used to count the number of copies made by the tile loader for a specific logical tile, and release
/// the logical tile when all the copies are done
/// @details Accepts an AdaptiveTileRequest under the form of a mother class TileRequest, get the id, and if not
/// registered add the total number of copies. When registered, the number of copies decreases by 1, when it reaches
/// 0, the id is removed from the map and the tile request is send to the rest of the graph
/// @tparam ViewType Type of the view
template<class ViewType>
class TileLoaderCounterState :
    public hh::AbstractState<fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::TileRequest<ViewType>> {
 private:
  std::map<size_t, size_t> mapIdTTL_{}; ///< Map used to associated a specific AdaptiveTileRequest id to the number
  /// of copies left to do
 public:
  /// @brief Default constructor
  TileLoaderCounterState() = default;
  /// @brief Default destructor
  virtual ~TileLoaderCounterState() = default;

  /// @brief Execute implementation to manage the AdaptiveTileRequest
  /// @param ptr AdaptiveTileRequest under the form of a TileRequest
  void execute(std::shared_ptr<fl::internal::TileRequest<ViewType>> ptr) override {
    auto adaptiveTileRequest = std::dynamic_pointer_cast<fl::internal::AdaptiveTileRequest<ViewType>>(ptr);
    if (adaptiveTileRequest != nullptr) {
      auto id = adaptiveTileRequest->id();
      if (mapIdTTL_.find(id) == mapIdTTL_.end()){
        mapIdTTL_.insert({id, adaptiveTileRequest->numberPhysicalTileRequests()});
      }
      mapIdTTL_.at(id) = mapIdTTL_.at(id) - 1;
      if(mapIdTTL_.at(id) == 0){
        mapIdTTL_.erase(id);
        this->push(adaptiveTileRequest);
      }
    } else {
      throw std::runtime_error(
          "The tile Request sent to an AdaptiveTileLoaderCounterState should be an AdaptiveTileRequest");
    }
  }
};

}
}

#endif //INC_3DFASTLOADER_TILE_LOADER_COUNTER_STATE_H
