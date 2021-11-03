//
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/20/21.
//

#ifndef INC_3DFASTLOADER_ADAPTIVE_TILE_REQUEST_H
#define INC_3DFASTLOADER_ADAPTIVE_TILE_REQUEST_H

#include <cinttypes>

#include "tile_request.h"
#include "view/adaptive_view.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {

template<class ViewType>
class AdaptiveTileRequest : public TileRequest<ViewType> {
 private:
  using DataType = typename ViewType::data_t;

  size_t
      numberPhysicalTileRequests_ = 0, ///< Number of physical Tile request made to fill a logical cached tile
  id_ = 0; ///< Unique Id

  std::shared_ptr<TileRequest<ViewType>> const
      logicalTileRequest_{}; ///< Original Tile Request, used after in the view counter to build the final view

  std::shared_ptr<fl::internal::CachedTile<DataType>> const
      logicalCachedTile_{}; ///< Logical Cached Tile

  std::shared_ptr<fl::internal::AdaptiveView<ViewType>>
      adaptiveView_{}; ///< Fake view, mimicking a requested view but with a logical cache as piece of data


  bool const
      needCopyFromPhysicalTileLoader_{}; ///< Flag to change the flow of data

 public:
  // Constructor used if logical tile is already cached
  AdaptiveTileRequest(
      std::shared_ptr<TileRequest<ViewType>> const &logicalTileRequest,
      std::shared_ptr<fl::internal::CachedTile<DataType>> const &logicalCachedTile) :
  // Dummy values because not used
      TileRequest<ViewType>(
          logicalTileRequest->indexRowTileAsked(),
          logicalTileRequest->indexColTileAsked(),
          logicalTileRequest->indexLayerTileAsked(),
          logicalTileRequest->view()),
      logicalTileRequest_(logicalTileRequest),
      logicalCachedTile_(logicalCachedTile),
      needCopyFromPhysicalTileLoader_(false) {}

  // Constructor used if logical tile is not already cached
  AdaptiveTileRequest(size_t indexRowTileAsked,
                      size_t indexColTileAsked,
                      size_t indexLayerTileAsked,
                      std::shared_ptr<fl::internal::AdaptiveView<ViewType>> adaptiveView,
                      std::shared_ptr<TileRequest<ViewType>> const &logicalTileRequest,
                      std::shared_ptr<fl::internal::CachedTile<DataType>> const &logicalCachedTile) :
      TileRequest<ViewType>(
          indexRowTileAsked, indexColTileAsked, indexLayerTileAsked, std::static_pointer_cast<ViewType>(adaptiveView)
      ),
      logicalTileRequest_(logicalTileRequest),
      logicalCachedTile_(logicalCachedTile),
      adaptiveView_(adaptiveView),
      needCopyFromPhysicalTileLoader_(true) {

  }

  virtual ~AdaptiveTileRequest() = default;
//
  std::shared_ptr<TileRequest<ViewType>> logicalTileRequest() const { return logicalTileRequest_; }
  [[nodiscard]] bool needCopyFromPhysicalTileLoader() const { return needCopyFromPhysicalTileLoader_; }
  std::shared_ptr<fl::internal::CachedTile<DataType>> logicalCachedTile() const { return logicalCachedTile_; }
  [[nodiscard]] size_t numberPhysicalTileRequests() const { return numberPhysicalTileRequests_; }
  [[nodiscard]] size_t id() const { return id_; }

  void numberPhysicalTileRequests(size_t numberPhysicalTileRequests) {
    this->numberPhysicalTileRequests_ = numberPhysicalTileRequests;
  }

  void id(size_t id) { id_ = id; }
};
}
}

#endif //INC_3DFASTLOADER_ADAPTIVE_TILE_REQUEST_H
