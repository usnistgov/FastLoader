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

/// @brief Special Tile request that adapts a logical tile request to a physical tile request with the corresponding
/// logical tile cache.
/// @tparam ViewType Type of the view.
template<class ViewType>
class AdaptiveTileRequest : public TileRequest<ViewType> {
 private:
  using DataType = typename ViewType::data_t; ///< Sample type (AbstractView element type)

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
  /// @brief Constructor used if logical tile is already cached
  /// @param logicalTileRequest Original logical tile request
  /// @param logicalCachedTile Logical cached tile already filled
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

  /// @brief Constructor used if logical tile is not already cached
  /// @param indexRowTileAsked Row index physical tile requested to the tile loader
  /// @param indexColTileAsked Column index physical tile requested to the tile loader
  /// @param indexLayerTileAsked Layer index physical tile requested to the tile loader
  /// @param adaptiveView Fake view, mimicking a requested view but with a logical cache as piece of data
  /// @param logicalTileRequest Original logical tile request
  /// @param logicalCachedTile Logical cached tile to fill
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
      needCopyFromPhysicalTileLoader_(true) {}

  /// @brief Default destructor
  virtual ~AdaptiveTileRequest() = default;

  /// @brief Base logical tile request accessor
  /// @return Base logical tile request
  std::shared_ptr<TileRequest<ViewType>> logicalTileRequest() const { return logicalTileRequest_; }

  /// @brief Accessor to know if the cached tile need to filled by the tile loader
  /// @return true if the cached tile need to filled by the tile loader, else false
  [[nodiscard]] bool needCopyFromPhysicalTileLoader() const { return needCopyFromPhysicalTileLoader_; }

  /// @brief Accessor to the logical cached tile
  /// @return The logical cached tile
  std::shared_ptr<fl::internal::CachedTile<DataType>> logicalCachedTile() const { return logicalCachedTile_; }

  /// @brief Accessor to the number of physical tile requests
  /// @return The number of physical tile requests
  [[nodiscard]] size_t numberPhysicalTileRequests() const { return numberPhysicalTileRequests_; }

  /// @brief Accessor to the logical tile id
  /// @return Logical tile id
  [[nodiscard]] size_t id() const { return id_; }

  /// @brief Setter to the number of physical tile requests
  /// @param numberPhysicalTileRequests Number of physical tile requests to set
  void numberPhysicalTileRequests(size_t numberPhysicalTileRequests) {
    this->numberPhysicalTileRequests_ = numberPhysicalTileRequests;
  }

  /// @brief Setter to the logical tile id
  /// @param id Logical tile id
  void id(size_t id) { id_ = id; }
};
}
}

#endif //INC_3DFASTLOADER_ADAPTIVE_TILE_REQUEST_H
