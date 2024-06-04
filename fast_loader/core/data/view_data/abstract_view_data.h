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

#include <utility>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ostream>
#include "../../../api/data/data_type.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief AbstractView's data managed by FastLoaderMemoryManager, part of the AbstractView api
/// @tparam DataType Type of data inside the View
template<class DataType>
class AbstractViewData {
 protected:
  std::size_t
      nbOfRelease_ = 0, ///< AbstractView's number of release
  releaseCount_ = 0, ///< AbstractView's release counts
  nbTilesToLoad_ = 0, ///< Number of tiles to load
      level_ = 0; ///< Pyramidal level

  std::vector<std::size_t>
      fullDimension_{}, ///< File dimensions
      tileDimension_{}, ///< Tile dimensions
      viewDimension_{}, ///< View dimensions
      radii_{}, ///< Radii values
      minPos_{}, ///< Minimum position in the view in the global space
      maxPos_{}, ///< Maximum position in the view in the global space
      minTileIndex_{}, ///< Minimum tile index composing the view
      indexCentralTile_{}, ///< Index of the central tile
      maxTileIndex_{}, ///< Maximum tile index composing the view
      frontFill_{}, ///< Number of front pixels that needs to be filled by the border creator
      backFill_{}, ///< Number of back pixels that needs to be filled by the border creator
      nbTilesPerDimension_{}; ///< Number of tiles for each dimensions

  std::vector<std::string> dimensionNames_{}; ///< Dimension names


  FillingType fillingType_ = FillingType::CONSTANT;   ///< Type of filling used to construct the view

 public:
  /// @brief ViewDataType Default constructor
  AbstractViewData() = default;

  /// @brief ViewDataType constructor used indirectly by FastLoaderMemoryManager
  /// @param nbOfRelease Number of time the view should be released
  explicit AbstractViewData(std::size_t nbOfRelease) : nbOfRelease_(nbOfRelease) {}

  /// @brief ViewDataType constructor used by FastLoaderMemoryManager
  /// @param releasesPerLevel All view's number of releases for all levels
  /// @param level AbstractView's level
  AbstractViewData(std::vector<std::size_t> releasesPerLevel, std::size_t level) :
      AbstractViewData(releasesPerLevel[level]) {}

  /// @brief Copy constructor
  /// @param viewData AbstractViewData to deep copy
  AbstractViewData(AbstractViewData<DataType> const &viewData) {
    //copy view metadata
    initialize(
        viewData.fullDimension_, viewData.tileDimension_,
        viewData.radii_, viewData.indexCentralTile_, viewData.nbTilesPerDimension_,
        viewData.dimensionNames_, viewData.fillingType_, viewData.level_
    );

    //those have not meaning outside of fast loader
    nbOfRelease_ = viewData.nbOfRelease_;
    nbTilesToLoad_ = viewData.nbTilesToLoad_;
  }

  /// @brief Default destructor
  virtual ~AbstractViewData() = default;

  /// @brief Initialise a view data with metadata
  /// @param fullDimension File / full dimensions
  /// @param tileDimension Tile dimensions
  /// @param radii View radii
  /// @param indexCentralTile View / central tile index
  /// @param nbTilesPerDimension Number tiles per dimension
  /// @param dimensionNames Dimension names
  /// @param fillingType Type of filling
  /// @param level Pyramidal level
  void initialize(std::vector<std::size_t> const &fullDimension, std::vector<std::size_t> const &tileDimension,
      std::vector<std::size_t> const &radii, std::vector<std::size_t> const &indexCentralTile,
      std::vector<std::size_t> const &nbTilesPerDimension, std::vector<std::string> const& dimensionNames,
      FillingType fillingType, std::size_t level) {
    clean();
    size_t const nbDimensions = fullDimension.size();
    std::vector<size_t> totalFill(nbDimensions), minPosCentralTile;

    dimensionNames_ = dimensionNames;

    fullDimension_ = fullDimension;
    tileDimension_ = tileDimension;
    radii_ = radii;
    indexCentralTile_ = indexCentralTile;
    nbTilesPerDimension_ = nbTilesPerDimension;

    releaseCount_ = 0;
    nbTilesToLoad_ = 0; // Really set when the TileRequests are made
    level_ = level;
    fillingType_ = fillingType;

    minTileIndex_.reserve(nbDimensions);
    maxTileIndex_.reserve(nbDimensions);
    minPos_.reserve(nbDimensions);
    maxPos_.reserve(nbDimensions);

    frontFill_.reserve(nbDimensions);
    backFill_.reserve(nbDimensions);

    std::transform(
        tileDimension_.cbegin(), tileDimension_.cend(), radii_.cbegin(), std::back_inserter(viewDimension_),
        [](auto const &tileSize, auto const &radius) { return tileSize + 2 * radius; }
    );

    std::transform(
        indexCentralTile_.cbegin(), indexCentralTile_.cend(), tileDimension_.cbegin(),
        std::back_inserter(minPosCentralTile),
        [](auto const &index, auto const &dimension) { return index * dimension; }
    );

    for (size_t dimension = 0; dimension < nbDimensions; ++dimension) {
      minTileIndex_.push_back(
          (size_t) std::max(
              (int64_t) indexCentralTile_.at(dimension)
                  - (int64_t) std::ceil((double) radii_.at(dimension) / (double) tileDimension_.at(dimension)),
              (int64_t) 0)
      );
      maxTileIndex_.push_back(
          std::min(
              indexCentralTile_.at(dimension)
                  + (size_t) std::ceil((double) radii_.at(dimension) / (double) tileDimension_.at(dimension)) + 1,
              nbTilesPerDimension_.at(dimension)));

      minPos_.push_back((size_t) std::max(
          ((int64_t) minPosCentralTile.at(dimension) - (int64_t) radii_.at(dimension)),
          (int64_t) 0));
      maxPos_.push_back(std::min(
          (indexCentralTile_.at(dimension) + 1) * tileDimension_.at(dimension) + radii_.at(dimension),
          fullDimension_.at(dimension)));

      totalFill.at(dimension) = maxPos_.at(dimension) - minPos_.at(dimension);
      frontFill_.push_back(
          ((int64_t) minPosCentralTile.at(dimension) - (int64_t) radii_.at(dimension)) < 0 ?
          radii_.at(dimension) - minPosCentralTile.at(dimension) : 0);
      backFill_.push_back(
          (frontFill_.at(dimension) + totalFill.at(dimension)) < viewDimension_.at(dimension) ?
          viewDimension_.at(dimension) - (frontFill_.at(dimension) + totalFill.at(dimension)) : 0);
    }
  }

  /// @brief Data buffer accessor
  /// @return Data buffer
  [[nodiscard]] virtual DataType *data() const = 0;
  /// @brief Number of tiles to load accessor
  /// @return Number of tiles to load
  [[nodiscard]] size_t nbTilesToLoad() const { return nbTilesToLoad_; }
  /// @brief Pyramidal level accessor
  /// @return Pyramidal accessor
  [[nodiscard]] size_t level() const { return level_; }
  /// @brief Number of dimensions accessor
  /// @return Number of dimensions
  [[nodiscard]] size_t nbDims() const { return fullDimension_.size(); }
  /// @brief File / full dimensions accessor
  /// @return File / full dimensions
  [[nodiscard]] std::vector<std::size_t> const &fullDims() const { return fullDimension_; }
  /// @brief Tile dimensions accessor
  /// @return Tile dimensions
  [[nodiscard]] std::vector<std::size_t> const &tileDims() const { return tileDimension_; }
  /// @brief View dimensions accessor
  /// @return View dimensions
  [[nodiscard]] std::vector<std::size_t> const &viewDims() const { return viewDimension_; }
  /// @brief Radii accessor
  /// @return Radii
  [[nodiscard]] std::vector<std::size_t> const &radii() const { return radii_; }
  /// @brief Accessor to the minimum global position in the view
  /// @return Minimum global position in the view
  [[nodiscard]] std::vector<std::size_t> const &minPos() const { return minPos_; }
  /// @brief Accessor to the maximum global position in the view
  /// @return Maximum global position in the view
  [[nodiscard]] std::vector<std::size_t> const &maxPos() const { return maxPos_; }
  /// @brief Accessor to the minimum tile index constituting the view
  /// @return Minimum tile index constituting the view
  [[nodiscard]] std::vector<std::size_t> const &minTileIndex() const { return minTileIndex_; }
  /// @brief Central tile / view index accessor
  /// @return Central tile / view index
  [[nodiscard]] std::vector<std::size_t> const &indexCentralTile() const { return indexCentralTile_; }
  /// @brief Accessor to the maximum tile index constituting the view
  /// @return Maximum tile index constituting the view
  [[nodiscard]] std::vector<std::size_t> const &maxTileIndex() const { return maxTileIndex_; }
  /// @brief Front fill accessor
  /// @return Front fill
  [[nodiscard]] std::vector<std::size_t> const &frontFill() const { return frontFill_; }
  /// @brief Back fill accessor
  /// @return Back fill
  [[nodiscard]] std::vector<std::size_t> const &backFill() const { return backFill_; }
  /// @brief Filling type accessor
  /// @return Filling type
  [[nodiscard]] FillingType fillingType() const { return fillingType_; }
  /// @brief Dimension names accessor
  /// @return Dimension names
  [[nodiscard]] std::vector<std::string> const &dimNames() const { return dimensionNames_; }

  /// @brief Number of tiles to load setter
  /// @param nbTilesToLoad Number of tiles to load
  void nbTilesToLoad(size_t nbTilesToLoad) { nbTilesToLoad_ = nbTilesToLoad; }

  /// @brief Output stream operator for the view data
  /// @param os Output stream
  /// @param data Data to print
  /// @return Output stream containing view data
  friend std::ostream &operator<<(std::ostream &os, AbstractViewData const &data) {
    os << "ViewData [";
    std::copy_n(data.indexCentralTile().cbegin(), data.indexCentralTile().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "] level: " << data.level() << " radii: [";
    std::copy_n(data.radii().cbegin(), data.radii().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "]\n Dimensions names: [";
    std::copy_n(data.dimNames().cbegin(), data.dimNames().size(), std::ostream_iterator<std::string>(os, ", "));
    os << "]\nDimension: Full: [";
    std::copy_n(data.fullDims().cbegin(), data.fullDims().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "] Tile: [";
    std::copy_n(data.tileDims().cbegin(), data.tileDims().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "] View: [";
    std::copy_n(data.viewDims().cbegin(), data.viewDims().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "]\nPos: min: [";
    std::copy_n(data.minPos().cbegin(), data.minPos().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "] max: [";
    std::copy_n(data.maxPos().cbegin(), data.maxPos().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "]\nIndex: min: [";
    std::copy_n(data.minTileIndex().cbegin(), data.minTileIndex().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "] max: [";
    std::copy_n(data.maxTileIndex().cbegin(), data.maxTileIndex().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "]\nFill: front: [";
    std::copy_n(data.frontFill().cbegin(), data.frontFill().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "] back: [";
    std::copy_n(data.backFill().cbegin(), data.backFill().size(), std::ostream_iterator<size_t>(os, ", "));
    os << "]\nData [";
    std::copy_n(data.data(),
                std::accumulate(data.viewDimension_.cbegin(), data.viewDimension_.cend(), 1, std::multiplies<>()),
                std::ostream_iterator<size_t>(os, ", "));
      os << "]";
    return os;
  }

  /// @brief Return the memory (ViewData) to the memory manager
  virtual void returnToMemoryManager() = 0;



 private:
  /// @brief Clean the view data
  void clean() {
    dimensionNames_.clear();
    viewDimension_.clear();
    minPos_.clear();
    maxPos_.clear();
    frontFill_.clear();
    backFill_.clear();
    minTileIndex_.clear();
    maxTileIndex_.clear();
  }
};

} // fl
} // internal

#endif //FAST_LOADER_ABSTRACT_VIEW_DATA_H
