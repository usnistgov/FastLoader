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

#ifndef FAST_LOADER_MAPPER_LOGICAL_PHYSICAL_H
#define FAST_LOADER_MAPPER_LOGICAL_PHYSICAL_H

#include <utility>
#include <hedgehog/hedgehog.h>

#include "../data/adaptive_tile_request.h"
#include "../data/view/adaptive_view.h"
#include "../cache.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {

/// @brief Mapper task: create n AdaptiveTileRequest from the logical tile request.
/// @tparam ViewType Type of the view
template<class ViewType>
class MapperLogicalPhysical : public hh::AbstractTask<1, TileRequest<ViewType>, AdaptiveTileRequest<ViewType>> {
 private:
  using DataType = typename ViewType::data_t; ///< Sample type (AbstractView element type)

  std::shared_ptr<std::vector<std::vector<size_t>>> const
      physicalTileDimensionPerLevel_{}, ///< Dimension of the physical tiles
  logicalTileDimensionPerLevel_{}, ///< Dimension of the logical tiles
  fullDimensionPerLevel_{}; ///< Dimension of the file


  std::vector<std::vector<size_t>>
      nbLogicalTilesPerDimensionPerLevel_{}; ///< Number of logical tiles per dimension per level

  std::shared_ptr<std::vector<std::shared_ptr<internal::Cache<typename ViewType::data_t>>>> const
      logicalTileCaches_; ///< All shared logical tile caches

  std::vector<std::string> const dimensionNames_{}; ///< Dimension names

  std::shared_ptr<internal::Cache<DataType>>
      cache_{}; ///< Logical tile Cache

  size_t
      nbElementDirectToCopy_ = 0, ///< Counter of number AdaptiveTileRequest that have a cached tile ready
  nbElementsToTL_ = 0; ///< Counter of number AdaptiveTileRequest that have an empty tile ready

 public:
  /// @brief Mapper constructor, compute the number of logical tiles per dimension per level
  /// @param physicalTileDimensionPerLevel Physical tile dimension per level
  /// @param logicalTileDimensionPerLevel Logical tile dimension per level
  /// @param fullDimensionPerLevel Full dimension per level
  /// @param logicalTileCaches Caches instance
  /// @param dimensionNames Dimension names
  MapperLogicalPhysical(
      std::shared_ptr<std::vector<std::vector<size_t>>> physicalTileDimensionPerLevel,
      std::shared_ptr<std::vector<std::vector<size_t>>> logicalTileDimensionPerLevel,
      std::shared_ptr<std::vector<std::vector<size_t>>> fullDimensionPerLevel,
      std::shared_ptr<std::vector<std::shared_ptr<internal::Cache<DataType>>>> logicalTileCaches,
      std::vector<std::string> const &dimensionNames)
      : hh::AbstractTask<1, TileRequest<ViewType>, AdaptiveTileRequest<ViewType>>("LogicalToPhysicalMapper"),
        physicalTileDimensionPerLevel_(std::move(physicalTileDimensionPerLevel)),
        logicalTileDimensionPerLevel_(std::move(logicalTileDimensionPerLevel)),
        fullDimensionPerLevel_(std::move(fullDimensionPerLevel)),
        logicalTileCaches_(logicalTileCaches), dimensionNames_(dimensionNames) {

    for (size_t level = 0; level < physicalTileDimensionPerLevel_->size(); ++level) {
      nbLogicalTilesPerDimensionPerLevel_.emplace_back();
      std::transform(
          fullDimensionPerLevel_->at(level).cbegin(), fullDimensionPerLevel_->at(level).cend(),
          logicalTileDimensionPerLevel_->at(level).cbegin(),
          std::back_insert_iterator<std::vector<size_t>>(nbLogicalTilesPerDimensionPerLevel_.at(level)),
          [](auto const &fullDimension, auto const &logicalTileDimension) {
            return (size_t) ceil((double) (fullDimension) / (double) (logicalTileDimension));
          }
      );
    }
  }

  /// @brief Default destructor
  virtual ~MapperLogicalPhysical() = default;

  /// @brief Initialize method implementation from Hedgehog library
  /// @details Set the cache for a specific pyramid level (=graphId)
  void initialize() override { cache_ = this->logicalTileCaches_->at(this->graphId()); }

  /// @brief Execute method  implementation from Hedgehog library
  /// @param tileRequest Logical Tile request to map into AdaptiveTileRequest
  void execute(std::shared_ptr<TileRequest<ViewType>> tileRequest) override {
    auto const &level = tileRequest->view()->level();
    auto const &nbDimensions = tileRequest->view()->nbDims();
    auto const &requestedIndex = tileRequest->index();

    std::shared_ptr<CachedTile<DataType>> logicalCachedTile = cache_->lockedTile(requestedIndex);
    if (!logicalCachedTile->newTile()) {
      ++nbElementDirectToCopy_;
      this->addResult(std::make_shared<fl::internal::AdaptiveTileRequest<ViewType>>(tileRequest, logicalCachedTile));
    } else {
      //Need to create N AdaptiveTileRequest to fill the logical Cache Tile from tile loader
      std::vector<std::shared_ptr<fl::internal::AdaptiveTileRequest<ViewType>>> adaptiveTileRequests{};

      std::vector<size_t> const &
          logicalTileDimension = logicalTileDimensionPerLevel_->at(level),
          physicalTileDimension = physicalTileDimensionPerLevel_->at(level);

      std::vector<size_t>
          minPos, maxPos,
          nbPhysicalTilePerDimension, nbLogicalTilePerDimension,
          indexMinPhysical, indexMaxPhysical,
          positionCopyFrom(nbDimensions, 0),
          positionCopyTo(nbDimensions, 0),
          dimensionCopy(nbDimensions, 0),
          indexPhysicalTile(nbDimensions, 0);

      std::size_t logicalTileId;

      logicalCachedTile->newTile(false);

      std::transform(
          requestedIndex.cbegin(), requestedIndex.cend(), logicalTileDimension.cbegin(),
          std::back_insert_iterator<std::vector<size_t>>(minPos),
          [](auto const &i, auto const &tileDimension) { return i * tileDimension; });

      std::transform(
          fullDimensionPerLevel_->at(level).cbegin(),
          fullDimensionPerLevel_->at(level).cend(),
          physicalTileDimension.cbegin(),
          std::back_insert_iterator<std::vector<size_t>>(nbPhysicalTilePerDimension),
          [](auto const &fullDimension, auto const &physicalDimension) {
            return (size_t) ceil((double) (fullDimension) / (double) physicalDimension);
          }
      );

      std::transform(
          fullDimensionPerLevel_->at(level).cbegin(),
          fullDimensionPerLevel_->at(level).cend(),
          logicalTileDimension.cbegin(),
          std::back_insert_iterator<std::vector<size_t>>(nbLogicalTilePerDimension),
          [](auto const &fullDimension, auto const &logicalDimension) {
            return (size_t) ceil((double) (fullDimension) / (double) logicalDimension);
          }
      );

      // index min physical  = min pos / physical dimension
      std::transform(
          minPos.cbegin(), minPos.cend(), physicalTileDimension.cbegin(),
          std::back_insert_iterator<std::vector<size_t>>(indexMinPhysical),
          [](auto const &pos, auto const &physicalDimension) { return pos / physicalDimension; });

      for (size_t dimension = 0; dimension < nbDimensions; ++dimension) {
        maxPos.push_back(
            std::min(
                (requestedIndex.at(dimension) + 1) * logicalTileDimension.at(dimension),
                fullDimensionPerLevel_->at(level).at(dimension))
        );
        indexMaxPhysical.push_back(
            std::min(
                (std::size_t) std::ceil((double) maxPos.at(dimension) / (double) physicalTileDimension.at(dimension)),
                nbPhysicalTilePerDimension.at(dimension))
        );
      }

      logicalTileId = computeLogicalTileId(requestedIndex, nbLogicalTilePerDimension);

      createCopies(
          logicalTileId, level, logicalCachedTile->data()->data(), fullDimensionPerLevel_->at(level),
          indexMinPhysical, indexMaxPhysical,
          nbLogicalTilePerDimension, requestedIndex,
          tileRequest, logicalCachedTile, physicalTileDimension, logicalTileDimension,
          minPos, maxPos, positionCopyFrom, positionCopyTo, dimensionCopy,
          indexPhysicalTile, adaptiveTileRequests, nbDimensions
      );

      size_t const nbOfAdaptiveTileRequests = adaptiveTileRequests.size();

      for (auto &tr : adaptiveTileRequests) {
        ++nbElementsToTL_;
        tr->nbPhysicalTileRequests(nbOfAdaptiveTileRequests);
        this->addResult(tr);
      }
    }
  }

  /// @brief Extra printing information implementation from Hedgehog library
  /// @return string containing extra printing information concerning cache properties
  [[nodiscard]] std::string extraPrintingInformation() const override {
    std::ostringstream oss;
    oss << "Miss rate: "
        << std::setprecision(3)
        << (double) (cache_->miss()) / (double) (cache_->miss() + cache_->hit()) * 100 << "%\n";
    return oss.str();
  }

  /// @brief Copy implementation from Hedgehog library
  /// @return Copy of a MapperLogicalPhysical
  std::shared_ptr<hh::AbstractTask<1, TileRequest<ViewType>, AdaptiveTileRequest<ViewType>>> copy() override {
    return std::make_shared<MapperLogicalPhysical<ViewType>>(
        physicalTileDimensionPerLevel_, logicalTileDimensionPerLevel_, fullDimensionPerLevel_,
        logicalTileCaches_, dimensionNames_
    );
  }

 private:
  /// @brief Compute unique logical Id, used as key to map the logicalTile in the TileLoaderCounterState
  /// @param requestedIndex Tile index
  /// @param nbLogicalTilePerDimension Number of tiles per dimension
  /// @param dimension Current dimension
  /// @return Unique logical tile id
  inline size_t computeLogicalTileId(
      std::vector<size_t> const &requestedIndex,
      std::vector<size_t> const &nbLogicalTilePerDimension,
      size_t dimension = 0) {
    if (dimension == requestedIndex.size() - 1) { return requestedIndex.at(dimension); }
    else {
      return
          requestedIndex.at(dimension)
              * std::accumulate(nbLogicalTilePerDimension.cbegin() + (long) dimension + 1,
                                nbLogicalTilePerDimension.cend(), (size_t) 1, std::multiplies<>())
              + computeLogicalTileId(requestedIndex, nbLogicalTilePerDimension, dimension + 1);
    }
  }

  /// @brief Create copies to fill logical from physical tiles
  /// @param id Logical tile id
  /// @param level Current pyramidal level
  /// @param dataOrigin  Pointer to the origin buffer
  /// @param fullDimension Full / file dimensions
  /// @param indexMinPhysical Minimum index physical tile
  /// @param indexMaxPhysical Maximum index physical tile
  /// @param nbLogicalTilesPerDimension Number of logical tiles per dimension
  /// @param requestedIndex Requested index
  /// @param tileRequest Tile request
  /// @param logicalCachedTile Logical cached tile
  /// @param physicalTileDimension Physical tile dimensions
  /// @param logicalTileDimension Logical tile dimensions
  /// @param minPos Source minimum position
  /// @param maxPos Source maximum position
  /// @param positionCopyFrom Source copy position
  /// @param positionCopyTo Destination copy position
  /// @param dimensionCopy Copy dimensions
  /// @param indexPhysicalTile Index physical tile
  /// @param adaptiveTileRequests Vector of adaptive tile requests
  /// @param nbDimensions Total number of dimensions
  /// @param dimension Current dimension
  inline void createCopies(
      size_t const &id, size_t const &level, DataType *const dataOrigin, std::vector<size_t> const &fullDimension,
      std::vector<size_t> const &indexMinPhysical, std::vector<size_t> const &indexMaxPhysical,
      std::vector<size_t> const &nbLogicalTilesPerDimension,
      std::vector<size_t> const &requestedIndex, std::shared_ptr<TileRequest<ViewType>> const tileRequest,
      std::shared_ptr<CachedTile<DataType>> const logicalCachedTile,
      std::vector<size_t> const &physicalTileDimension, std::vector<size_t> const &logicalTileDimension,
      std::vector<size_t> const &minPos, std::vector<size_t> const &maxPos,
      std::vector<size_t> &positionCopyFrom, std::vector<size_t> &positionCopyTo, std::vector<size_t> &dimensionCopy,
      std::vector<size_t> &indexPhysicalTile,
      std::vector<std::shared_ptr<fl::internal::AdaptiveTileRequest<ViewType>>> &adaptiveTileRequests,
      size_t const nbDimensions, size_t const dimension = 0) {

    positionCopyTo.at(dimension) = 0;
    for (size_t i = indexMinPhysical.at(dimension); i < indexMaxPhysical.at(dimension); ++i) {
      indexPhysicalTile.at(dimension) = i;
      // absolute pos copy from = max(index * physical dimension, minRow)
      positionCopyFrom.at(dimension) =
          std::max(size_t(i * physicalTileDimension.at(dimension)), minPos.at(dimension));

      // dimension to copy = min(index + 1 * physical dimension, max pos) - pos copy from
      dimensionCopy.at(dimension) =
          std::min(size_t((i + 1) * physicalTileDimension.at(dimension)), maxPos.at(dimension))
              - positionCopyFrom.at(dimension);

      // relative Pos copy -= index * physical dimension
      positionCopyFrom.at(dimension) -= i * physicalTileDimension.at(dimension);

      if (dimension == nbDimensions - 1) {
        // If last dimension
        // Create a view data that uses the cached logical data as internal data
        auto adaptiveViewData = std::make_shared<AdaptiveViewData<DataType>>(dataOrigin);
        // Describe the view data
        adaptiveViewData->initialize(fullDimension,
                                     logicalTileDimension,
                                     std::vector<size_t>(nbDimensions, 0),
                                     requestedIndex,
                                     nbLogicalTilesPerDimension,
                                     dimensionNames_,
                                     FillingType::CONSTANT,
                                     level);

        // Create the view from the view data
        auto adaptiveView = std::make_shared<AdaptiveView<ViewType>>(adaptiveViewData);

        // Create an AdaptiveTileRequest with this view
        auto adaptiveTileRequest = std::make_shared<AdaptiveTileRequest<ViewType>>(
            indexPhysicalTile, adaptiveView, tileRequest, logicalCachedTile);
        adaptiveTileRequest->addCopy(CopyVolume(positionCopyFrom, positionCopyTo, dimensionCopy));
        adaptiveTileRequest->id(id);
        adaptiveTileRequests.push_back(adaptiveTileRequest);

      } else {
        // If not last dimension, go to the next
        createCopies(
            id, level, dataOrigin, fullDimension, indexMinPhysical, indexMaxPhysical, nbLogicalTilesPerDimension,
            requestedIndex, tileRequest, logicalCachedTile, physicalTileDimension, logicalTileDimension,
            minPos, maxPos, positionCopyFrom, positionCopyTo, dimensionCopy, indexPhysicalTile, adaptiveTileRequests,
            nbDimensions, dimension + 1
        );
      }
      positionCopyTo.at(dimension) += dimensionCopy.at(dimension);
    }
  }

};

} // fl
} // internal

#endif //FAST_LOADER_MAPPER_LOGICAL_PHYSICAL_H
