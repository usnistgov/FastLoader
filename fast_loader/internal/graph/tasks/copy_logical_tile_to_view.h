//
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/25/21.
//

#ifndef INC_3DFASTLOADER_COPY_LOGICAL_TILE_TO_VIEW_H
#define INC_3DFASTLOADER_COPY_LOGICAL_TILE_TO_VIEW_H

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {

/// @brief Multi-threaded task to copy [parts of] logical caches to the view
/// @tparam ViewType Type of the view
template<class ViewType>
class CopyLogicalCacheToView :
    public hh::AbstractTask<fl::internal::TileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>> {
 private:
  size_t const numberChannels_; ///< Number of channels
 public:
  /// @brief CopyLogicalCacheToView constructor
  /// @param numberThreads Number of thread associated to the task
  /// @param numberChannels Number of channels per pixels
  explicit CopyLogicalCacheToView(size_t numberThreads, size_t const numberChannels) :
      hh::AbstractTask<
          fl::internal::TileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>>
          ("CopyLogicalCacheToView", numberThreads),
      numberChannels_(numberChannels) {}

  /// @brief Default destructor
  virtual ~CopyLogicalCacheToView() = default;

  /// @brief Implementation of the Hedgehog execute method
  /// @param adaptiveTileRequest AdaptiveTileRequest containing all information to achieve the copy
  void execute(std::shared_ptr<fl::internal::AdaptiveTileRequest<ViewType>> adaptiveTileRequest) override {
    std::shared_ptr<fl::internal::TileRequest<ViewType>> logicalTileRequest = adaptiveTileRequest->logicalTileRequest();
    std::shared_ptr<fl::internal::CachedTile<typename ViewType::data_t>> logicalCachedTile =
        adaptiveTileRequest->logicalCachedTile();

    size_t
        tileWidth = logicalTileRequest->view()->tileWidth(),
        tileHeight = logicalTileRequest->view()->tileHeight(),
        viewWidth = logicalTileRequest->view()->viewWidth(),
        viewHeight = logicalTileRequest->view()->viewHeight(),
        rowBeginSrc = 0, colBeginSrc = 0, layerBeginSrc = 0,
        rowBeginDest = 0, colBeginDest = 0, layerBeginDest = 0,
        width = 0, height = 0, depth = 0;

    auto dest = logicalTileRequest->view()->viewOrigin();

    // Do Every copy for a tile to a view
    for (auto copy: logicalTileRequest->copies()) {
      rowBeginSrc = copy.from().rowBegin();
      colBeginSrc = copy.from().colBegin();
      layerBeginSrc = copy.from().layerBegin();

      rowBeginDest = copy.to().rowBegin();
      colBeginDest = copy.to().colBegin();
      layerBeginDest = copy.to().layerBegin();

      width = copy.from().width();
      height = copy.from().height();
      depth = copy.from().depth();

      // Handle symmetry for the copy
      if (copy.reverseLayers()) {
        if (copy.reverseCols()) {
          if (copy.reverseRows()) {
            // Layer Col Row Symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::reverse_copy(
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc) * numberChannels_,
                    logicalCachedTile->data()->begin()
                        + (layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + ((rowBeginSrc + row) * tileWidth
                            + colBeginSrc
                            + width) * numberChannels_,
                    dest
                        + (layerBeginDest + depth - layer - 1) * viewHeight * viewWidth
                        + (((rowBeginDest + height - row - 1) * viewWidth
                            + colBeginDest) * numberChannels_)
                );
              }
            }
          } else {
            // Layer Col symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::reverse_copy(
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + ((rowBeginSrc + row) * tileWidth + colBeginSrc)) * numberChannels_,
                    logicalCachedTile->data()->begin()
                        + (((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc
                            + width)) * numberChannels_,
                    dest
                        + ((layerBeginDest + depth - layer - 1) * viewHeight * viewWidth
                            + (rowBeginDest + row) * viewWidth
                            + colBeginDest) * numberChannels_
                );
              }
            }
          }
        } else {
          if (copy.reverseRows()) {
            // Layers Rows symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::copy_n(
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc) * numberChannels_,
                    width * numberChannels_,
                    dest
                        + ((layerBeginDest + depth - layer - 1) * viewHeight * viewWidth
                            + (rowBeginDest + height - row - 1) * viewWidth
                            + colBeginDest) * numberChannels_
                );
              }
            }
          } else {
            // Layer symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::copy_n(
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc) * numberChannels_,
                    width * numberChannels_,
                    dest
                        + ((layerBeginDest + depth - layer - 1) * viewHeight * viewWidth
                            + (rowBeginDest + row) * viewWidth
                            + colBeginDest) * numberChannels_
                );
              }
            }
          } // End Reverse Row
        } // End Reverse Columns
      } else {
        if (copy.reverseCols()) {
          if (copy.reverseRows()) {
            // Rows Cols symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::reverse_copy(
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc) * numberChannels_,
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc
                            + width) * numberChannels_,
                    dest
                        + ((layerBeginDest + layer) * viewWidth * viewHeight
                            + (rowBeginDest + height - row - 1) * viewWidth
                            + colBeginDest) * numberChannels_
                );
              }
            }
          } else {
            // Cols symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::reverse_copy(
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc) * numberChannels_,
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc
                            + width) * numberChannels_,
                    dest
                        + ((layerBeginDest + layer) * viewWidth * viewHeight
                            + (rowBeginDest + row) * viewWidth
                            + colBeginDest) * numberChannels_
                );
              }
            }
          }
        } else {
          if (copy.reverseRows()) {
            // Rows symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::copy_n(
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc) * numberChannels_,
                    width * numberChannels_,
                    dest
                        + ((layerBeginDest + layer) * viewWidth * viewHeight
                            + (rowBeginDest + height - row - 1) * viewWidth
                            + colBeginDest) * numberChannels_
                );
              }
            }
          } else {
            // No symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::copy_n(
                    logicalCachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                            + (rowBeginSrc + row) * tileWidth
                            + colBeginSrc) * numberChannels_,
                    width * numberChannels_,
                    dest
                        + ((layerBeginDest + layer) * viewWidth * viewHeight
                            + (rowBeginDest + row) * viewWidth
                            + colBeginDest) * numberChannels_
                );

              }
            }
          } // End Reverse Row
        } // End Reverse Columns
      } // End Reverse Layers
    } // For all copies

    logicalCachedTile->unlock();
    this->addResult(logicalTileRequest);
  }

  /// @brief Copy method implementation from Hedgehog library
  /// @return A copy of the CopyLogicalCacheToView task
  std::shared_ptr<hh::AbstractTask<fl::internal::TileRequest<ViewType>,
                                   fl::internal::AdaptiveTileRequest<ViewType>>>
  copy() override {
    return std::make_shared<CopyLogicalCacheToView<ViewType>>(this->numberThreads(), this->numberChannels_);
  }
};

}
}

#endif //INC_3DFASTLOADER_COPY_LOGICAL_TILE_TO_VIEW_H
