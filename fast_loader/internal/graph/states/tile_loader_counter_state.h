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

template<class Viewtype>
class TileLoaderCounterState :
    public hh::AbstractState<fl::internal::AdaptiveTileRequest<Viewtype>, fl::internal::TileRequest<Viewtype>> {
 private:
  std::map<size_t, size_t> mapIdTTL_{};
 public:
  TileLoaderCounterState() = default;
  virtual ~TileLoaderCounterState() = default;

  void execute(std::shared_ptr<fl::internal::TileRequest<Viewtype>> ptr) override {
//    global_mutex.lock();
//    std::cout << "Counter status: " << std::endl;
//    for(auto & elem : mapIdTTL_){
//      std::cout << "\t" << elem.first  << ": " << elem.second << std::endl;
//     }
//    global_mutex.unlock();
    auto adaptiveTileRequest = std::dynamic_pointer_cast<fl::internal::AdaptiveTileRequest<Viewtype>>(ptr);
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
//      global_mutex.lock();
//      std::cout << "Counter status AFTER: " << std::endl;
//      for(auto & elem : mapIdTTL_){
//        std::cout << "\t" << elem.first  << ": " << elem.second << std::endl;
//      }
//      global_mutex.unlock();
    } else {
      throw std::runtime_error(
          "The tile Request sent to an AdaptiveTileLoaderCounterState should be an AdaptiveTileRequest");
    }
  }
};

}
}

#endif //INC_3DFASTLOADER_TILE_LOADER_COUNTER_STATE_H
