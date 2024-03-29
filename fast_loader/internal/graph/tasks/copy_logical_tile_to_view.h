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

#ifndef INC_FAST_LOADER_COPY_LOGICAL_TILE_TO_VIEW_H
#define INC_FAST_LOADER_COPY_LOGICAL_TILE_TO_VIEW_H

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

#endif //INC_FAST_LOADER_COPY_LOGICAL_TILE_TO_VIEW_H
