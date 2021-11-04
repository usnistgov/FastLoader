// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of 
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.

#ifndef INC_FAST_LOADER_ADAPTIVE_TILE_REQUEST_H
#define INC_FAST_LOADER_ADAPTIVE_TILE_REQUEST_H

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

#endif //INC_FAST_LOADER_ADAPTIVE_TILE_REQUEST_H
