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

#ifndef FAST_LOADER_ABSTRACT_VIEW_DATA_H
#define FAST_LOADER_ABSTRACT_VIEW_DATA_H

#include <memory>
#include <vector>
#include <cmath>

#include <hedgehog/hedgehog.h>
#include "../../data_type.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief AbstractView's data managed by FastLoaderMemoryManager, part of the AbstractView api
/// @tparam DataType Internal type of data
template<class DataType>
class AbstractViewData {
 protected:
  DataType *
      data_ = nullptr; ///< Real Data

  std::size_t
      numberOfRelease_ = 0, ///< AbstractView's number of release
  releaseCount_ = 0; ///< AbstractView's release counts

  size_t
  // Image size
  fullHeight_ = 0,            ///< Full height
  fullWidth_ = 0,             ///< Full width
  fullDepth_ = 0,             ///< Full depth
  // Tile size
  tileHeight_ = 0,            ///< Tile Height
  tileWidth_ = 0,             ///< Tile Width
  tileDepth_ = 0,             ///< Tile Depth
  // Radius
  radiusHeight_ = 0,          ///< Number of pixel in height surrounding the center tile
  radiusWidth_ = 0,           ///< Number of pixel in width surrounding the center tile
  radiusDepth_ = 0,           ///< Number of pixel in depth surrounding the center tile
  // Pyramid level
  level_ = 0,                 ///< Image Pyramid level
  // AbstractView Size
  viewHeight_ = 0,            ///< AbstractView Height
  viewWidth_ = 0,             ///< AbstractView Width
  viewDepth_ = 0,             ///< AbstractView Depth
  numberTilesToLoad_ = 0,     ///< Number of tiles to load
  numTilesHeight_ = 0,        ///< Number of tiles in height
  numTilesWidth_ = 0,         ///< Number of tiles in width
  numTilesDepth_ = 0,         ///< Number of tiles in depth
  numberChannels_ = 0,        ///< Number of channels
  // Central Tile
  minRowCentralTile_ = 0,     ///< Minimum row central tile
  minColCentralTile_ = 0,     ///< Minimum col central tile
  minLayerCentralTile_ = 0,   ///< Minimum layer central tile
  // Index Tile into view
  indexRowMinTile_ = 0,       ///< Row index minimum tile
  indexColMinTile_ = 0,       ///< Column index minimum tile
  indexLayerMinTile_ = 0,     ///< Layer index minimum tile
  indexRowCenterTile_ = 0,    ///< Row index central tile
  indexColCenterTile_ = 0,    ///< Column index central tile
  indexLayerCenterTile_ = 0,  ///< Layer index central tile
  indexRowMaxTile_ = 0,       ///< Row index maximum tile
  indexColMaxTile_ = 0,       ///< Col index maximum tile
  indexLayerMaxTile_ = 0,     ///< Layer index maximum tile
  // Position AbstractView in File
  minRowFile_ = 0,            ///< File minimum row
  maxRowFile_ = 0,            ///< File maximum row
  minColFile_ = 0,            ///< File minimum col
  maxColFile_ = 0,            ///< File maximum col
  minLayerFile_ = 0,          ///< File minimum layer
  maxLayerFile_ = 0,          ///< File maximum layer
  // Filling Ghost Region
  rowFilledFromFile_ = 0,     ///< Number of rows filled from the file
  colFilledFromFile_ = 0,     ///< Number of columns filled from the file
  layerFilledFromFile_ = 0,   ///< Number of layers filled from the file
  topFill_ = 0,               ///< Top rows to fill with ghost data
  leftFill_ = 0,              ///< Left columns to fill with ghost data
  bottomFill_ = 0,            ///< Bottom rows to fill with ghost data
  rightFill_ = 0,             ///< Right columns to fill with ghost data
  frontFill_ = 0,             ///< Front layer to fill with ghost data
  backFill_ = 0;              ///< Back layer to fill with ghost data

  FillingType fillingType_ = FillingType::CONSTANT;   ///< Type of filling used to construct the view
 public:
  /// @brief ViewDataType Default constructor
  AbstractViewData() = default;

  /// @brief ViewDataType constructor used indirectly by FastLoaderMemoryManager
  /// @param numberOfRelease Number of time the view should be released
  explicit AbstractViewData(size_t numberOfRelease) : numberOfRelease_(numberOfRelease) {}

  /// @brief ViewDataType constructor used by FastLoaderMemoryManager
  /// @param releasesPerLevel All view's number of releases for all levels
  /// @param level AbstractView's level
  AbstractViewData(std::vector<size_t> releasesPerLevel, size_t level) :
      AbstractViewData(releasesPerLevel[level]) {}

  /// @brief copy constructor
  /// @param viewData AbstractViewData to deep copy
  AbstractViewData(AbstractViewData<DataType> const &viewData) {
    //copy view metadata
    initialize(
        viewData.fullHeight_, viewData.fullWidth_, viewData.fullDepth_,
        viewData.tileHeight_, viewData.tileWidth_, viewData.tileDepth_,
        viewData.numberChannels_,
        viewData.radiusHeight_, viewData.radiusWidth_, viewData.radiusDepth_,
        viewData.level_,
        viewData.indexRowCenterTile_, viewData.indexColCenterTile_, viewData.indexLayerCenterTile_,
        viewData.numTilesHeight_, viewData.numTilesWidth_, viewData.numTilesDepth_,
        viewData.fillingType_
    );

    //those have not meaning outside of fast loader
    numberOfRelease_ = viewData.numberOfRelease_;
    numberTilesToLoad_ = viewData.numberTilesToLoad_;
  }

  virtual ~AbstractViewData() = default;

  /// @brief Initialize all the metadata
  /// @param fullHeight Full height
  /// @param fullWidth Full width
  /// @param fullDepth Full depth
  /// @param tileHeight Tile height
  /// @param tileWidth Tile width
  /// @param tileDepth Tile depth
  /// @param numberChannels Number of channels in a pixel
  /// @param radiusHeight View radius in height
  /// @param radiusWidth View radius in width
  /// @param radiusDepth View radius in depth
  /// @param level AbstractView's level
  /// @param indexRowCenterTile AbstractView's row index
  /// @param indexColCenterTile AbstractView's column index
  /// @param indexLayerCenterTile AbstractView's layer index
  /// @param numTilesHeight Number of tiles in file height
  /// @param numTilesWidth Number of tiles in file width
  /// @param numTilesDepth Number of tiles in file depth
  /// @param fillingType Type of view's filling
  void initialize(
      size_t fullHeight, size_t fullWidth, size_t fullDepth,
      size_t tileHeight, size_t tileWidth, size_t tileDepth,
      size_t numberChannels,
      size_t radiusHeight, size_t radiusWidth, size_t radiusDepth,
      size_t level,
      size_t indexRowCenterTile, size_t indexColCenterTile, size_t indexLayerCenterTile,
      size_t numTilesHeight, size_t numTilesWidth, size_t numTilesDepth,
      FillingType fillingType) {

    fullHeight_ = fullHeight;
    fullWidth_ = fullWidth;
    fullDepth_ = fullDepth;
    tileHeight_ = tileHeight;
    tileWidth_ = tileWidth;
    tileDepth_ = tileDepth;
    numberChannels_ = numberChannels;
    radiusHeight_ = radiusHeight;
    radiusWidth_ = radiusWidth;
    radiusDepth_ = radiusDepth;
    level_ = level;
    numTilesHeight_ = numTilesHeight;
    numTilesWidth_ = numTilesWidth;
    numTilesDepth_ = numTilesDepth;

    // AbstractView Size
    viewHeight_ = tileHeight + 2 * radiusHeight_;
    viewWidth_ = tileWidth + 2 * radiusWidth_;
    viewDepth_ = tileDepth + 2 * radiusDepth_;
    // Front top left central tile pixel
    indexRowCenterTile_ = indexRowCenterTile;
    indexColCenterTile_ = indexColCenterTile;
    indexLayerCenterTile_ = indexLayerCenterTile;

    minRowCentralTile_ = indexRowCenterTile_ * tileHeight;
    minColCentralTile_ = indexColCenterTile_ * tileWidth;
    minLayerCentralTile_ = indexLayerCenterTile_ * tileDepth;
    // Index of the tile overlapped by the view.
    indexRowMinTile_ = (size_t) std::max(
        (int32_t) indexRowCenterTile_ - (int32_t) std::ceil((double) radiusHeight_ / tileHeight_), (int32_t) 0);
    indexColMinTile_ = (size_t) std::max(
        (int32_t) indexColCenterTile_ - (int32_t) std::ceil((double) radiusWidth_ / tileWidth_), (int32_t) 0);
    indexLayerMinTile_ = (size_t) std::max(
        (int32_t) indexLayerCenterTile_ - (int32_t) std::ceil((double) radiusDepth_ / tileDepth_), (int32_t) 0);
    indexRowMaxTile_ = std::min(
        indexRowCenterTile_ + (int32_t) std::ceil((double) radiusHeight_ / tileHeight_) + 1, numTilesHeight_);
    indexColMaxTile_ = std::min(
        indexColCenterTile_ + (int32_t) std::ceil((double) radiusWidth_ / tileWidth_) + 1, numTilesWidth_);
    indexLayerMaxTile_ = std::min(
        indexLayerCenterTile_ + (int32_t) std::ceil((double) radiusDepth_ / tileDepth_) + 1, numTilesDepth_);


    // Position of the lines / columns / layers to copy from the file
    minRowFile_ = (size_t) std::max((int32_t) (minRowCentralTile_ - radiusHeight_), (int32_t) 0);
    maxRowFile_ = std::min((indexRowCenterTile_ + 1) * tileHeight_ + radiusHeight_, fullHeight_);
    minColFile_ = (size_t) std::max((int32_t) (minColCentralTile_ - radiusWidth_), (int32_t) 0);
    maxColFile_ = std::min((indexColCenterTile_ + 1) * tileWidth_ + radiusWidth_, fullWidth_);
    minLayerFile_ = (size_t) std::max((int32_t) (minLayerCentralTile_ - radiusDepth_), (int32_t) 0);
    maxLayerFile_ = std::min((indexLayerCenterTile_ + 1) * tileDepth_ + radiusDepth_, fullDepth_);

    // Count of the lines / columns to copy from the file
    rowFilledFromFile_ = maxRowFile_ - minRowFile_;
    colFilledFromFile_ = maxColFile_ - minColFile_;
    layerFilledFromFile_ = maxLayerFile_ - minLayerFile_;

    // Number of pixels to fill with ghost values
    topFill_ = (int32_t) (minRowCentralTile_ - radiusHeight_) < 0 ? radiusHeight_ - minRowCentralTile_ : 0;
    leftFill_ = (int32_t) (minColCentralTile_ - radiusWidth_) < 0 ? radiusWidth_ - minColCentralTile_ : 0;
    frontFill_ = (int32_t) (minLayerCentralTile_ - radiusDepth_) < 0 ? radiusDepth_ - minLayerCentralTile_ : 0;

    bottomFill_ = (topFill_ + rowFilledFromFile_) < viewHeight_ ? viewHeight_ - (topFill_ + rowFilledFromFile_) : 0;
    rightFill_ = (leftFill_ + colFilledFromFile_) < viewWidth_ ? viewWidth_ - (leftFill_ + colFilledFromFile_) : 0;
    backFill_ = (frontFill_ + layerFilledFromFile_) < viewDepth_ ? viewDepth_ - (frontFill_ + layerFilledFromFile_) : 0;

    releaseCount_ = 0;
    numberTilesToLoad_ = 0; // Really set when the TileRequests are made
    level_ = level;
    fillingType_ = fillingType;
  }

  /// @brief Raw data accessor
  /// @return Raw data
  [[nodiscard]] virtual DataType *data() { return data_; }
  /// @brief Full width accessor
  /// @return Full width
  [[nodiscard]] size_t fullWidth() const { return fullWidth_; }
  /// @brief Full height accessor
  /// @return Full height
  [[nodiscard]] size_t fullHeight() const { return fullHeight_; }
  /// @brief Full depth accessor
  /// @return Full depth
  [[nodiscard]] size_t fullDepth() const { return fullDepth_; }
  /// @brief Tile height accessor
  /// @return Tile height
  [[nodiscard]] size_t tileHeight() const { return tileHeight_; }
  /// @brief Tile width accessor
  /// @return Tile width
  [[nodiscard]] size_t tileWidth() const { return tileWidth_; }
  /// @brief Tile depth accessor
  /// @return Tile depth
  [[nodiscard]] size_t tileDepth() const { return tileDepth_; }
  /// @brief Number of channels accessor
  /// @return Number of channels
  size_t numberChannels() const { return numberChannels_; }
  /// @brief AbstractView's radius height accessor
  /// @return AbstractView's radius height
  [[nodiscard]] size_t radiusHeight() const { return radiusHeight_; }
  /// @brief AbstractView's radius width accessor
  /// @return AbstractView's radius width
  [[nodiscard]] size_t radiusWidth() const { return radiusWidth_; }
  /// @brief AbstractView's radius depth accessor
  /// @return AbstractView's radius depth
  [[nodiscard]] size_t radiusDepth() const { return radiusDepth_; }
  /// @brief AbstractView level accessor
  /// @return AbstractView level
  [[nodiscard]] size_t level() const { return level_; }
  /// @brief AbstractView height accessor
  /// @return AbstractView height
  [[nodiscard]] size_t viewHeight() const { return viewHeight_; }
  /// @brief AbstractView width accessor
  /// @return AbstractView width
  [[nodiscard]] size_t viewWidth() const { return viewWidth_; }
  /// @brief AbstractView depth accessor
  /// @return AbstractView depth
  [[nodiscard]] size_t viewDepth() const { return viewDepth_; }
  /// @brief Number of tiles to load accessor
  /// @return Number of tiles to load
  [[nodiscard]] size_t numberTilesToLoad() const { return numberTilesToLoad_; }
  /// @brief Front top left tile row index accessor
  /// @return Front top left tile row index
  [[nodiscard]] size_t indexRowMinTile() const { return indexRowMinTile_; }
  /// @brief Front top left tile column index accessor
  /// @return Front top left tile column index
  [[nodiscard]] size_t indexColMinTile() const { return indexColMinTile_; }
  /// @brief Front top left tile layer index accessor
  /// @return Front top left tile layer index
  [[nodiscard]] size_t indexLayerMinTile() const { return indexLayerMinTile_; }
  /// @brief Central tile row index accessor
  /// @return Central tile row index
  [[nodiscard]] size_t indexRowCenterTile() const { return indexRowCenterTile_; }
  /// @brief Central tile column index accessor
  /// @return Central tile column index
  [[nodiscard]] size_t indexColCenterTile() const { return indexColCenterTile_; }
  /// @brief Central tile layer index accessor
  /// @return Central tile layer index
  [[nodiscard]] size_t indexLayerCenterTile() const { return indexLayerCenterTile_; }
  /// @brief Behind bottom right tile row index accessor
  /// @return Behind bottom right tile row index
  [[nodiscard]] size_t indexRowMaxTile() const { return indexRowMaxTile_; }
  /// @brief Behind bottom right tile column index accessor
  /// @return Behind bottom right tile column index
  [[nodiscard]] size_t indexColMaxTile() const { return indexColMaxTile_; }
  /// @brief Behind bottom right tile layer index accessor
  /// @return Behind bottom right tile layer index
  [[nodiscard]] size_t indexLayerMaxTile() const { return indexLayerMaxTile_; }
  /// @brief Corresponding file's row to top view's row accessor
  /// @return Corresponding file's row to top view's row
  [[nodiscard]] size_t minRowFile() const { return minRowFile_; }
  /// @brief Corresponding file's row to bottom view's row accessor
  /// @return Corresponding file's row to bottom view's row
  [[nodiscard]] size_t maxRowFile() const { return maxRowFile_; }
  /// @brief Corresponding file's column to left view's column accessor
  /// @return Corresponding file's column to left view's column
  [[nodiscard]] size_t minColFile() const { return minColFile_; }
  /// @brief Corresponding file's column to right view's column accessor
  /// @return Corresponding file's column to right view's column
  [[nodiscard]] size_t maxColFile() const { return maxColFile_; }
  /// @brief Corresponding file's layer to front view's row accessor
  /// @return Corresponding file's layer to front view's row
  [[nodiscard]] size_t minLayerFile() const { return minLayerFile_; }
  /// @brief Corresponding file's layer to behind view's row accessor
  /// @return Corresponding file's layer to behind view's row
  [[nodiscard]] size_t maxLayerFile() const { return maxLayerFile_; }
  /// @brief Number of rows on top to fill with ghost values accessor
  /// @return Number of rows on top to fill with ghost values
  [[nodiscard]] size_t topFill() const { return topFill_; }
  /// @brief Number of columns on left to fill with ghost values accessor
  /// @return Number of columns on left to fill with ghost values
  [[nodiscard]] size_t leftFill() const { return leftFill_; }
  /// @brief Number of layer on front to fill with ghost values accessor
  /// @return Number of layer on front to fill with ghost values
  [[nodiscard]] size_t frontFill() const { return frontFill_; }
  /// @brief Number of rows on bottom to fill with ghost values accessor
  /// @return Number of rows on bottom to fill with ghost values
  [[nodiscard]] size_t bottomFill() const { return bottomFill_; }
  /// @brief Number of columns on right to fill with ghost values accessor
  /// @return Number of columns on right to fill with ghost values
  [[nodiscard]] size_t rightFill() const { return rightFill_; }
  /// @brief Number of layer on back to fill with ghost values accessor
  /// @return Number of layer on back to fill with ghost values
  [[nodiscard]] size_t backFill() const { return backFill_; }
  /// @brief Type of filling used to generate the view accessor
  /// @return Type of filling used to generate the view
  [[nodiscard]] FillingType fillingType() const { return fillingType_; }

  /// @brief Number of tiles to load setter
  /// @param numberTilesToLoad Number of tiles to load to set
  void numberTilesToLoad(size_t numberTilesToLoad) { numberTilesToLoad_ = numberTilesToLoad; }

  /// @brief Return the memory (ViewData) to the memory manager
  virtual void returnToMemoryManager() = 0;

};
}
}

#endif //FAST_LOADER_ABSTRACT_VIEW_DATA_H
