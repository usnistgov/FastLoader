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

#ifndef FAST_LOADER_ADAPTIVE_FAST_LOADER_GRAPH_H
#define FAST_LOADER_ADAPTIVE_FAST_LOADER_GRAPH_H

#include <utility>

#include "../fast_loader_graph.h"
#include "../../../core/task/mapper_logical_physical.h"
#include "../../../core/task/copy_logical_tile_to_view.h"
#include "../../../core/state/direct_to_copy_state.h"
#include "../../../core/state/to_tile_loader_state.h"
#include "../../../core/state/tile_loader_counter_state.h"


/// @brief FastLoader namespace
namespace fl {

/// @brief Special type of FastLoaderGraph that presents views with a user-specified tiling that may be different from the one existing in the file.
/// Works exactly the same way as the FastLoaderGraph but requires the new tile dimensions for all levels:
/// -# logicalTileDimensionRequestedPerDimensionPerLevel: dimensions of the tiles for all levels {{d00, ..,, d0n}, ..., {dm0, ..., dmn}} for n dimensions tiles with m levels
/// -# nbLogicalTilesCachePerLevel: Cache size used  for the transformation between physical tiles to requested tiles for every level
/// @tparam ViewType Type of the view
template<class ViewType>
class AdaptiveFastLoaderGraph : public fl::FastLoaderGraph<ViewType> {
 private:
  using DataType = typename ViewType::data_t; ///< Sample type (AbstractView element type)
  std::shared_ptr<std::vector<std::vector<size_t>>>
      physicalTileDimensionPerLevel_ = ///< Physical tile dimension
      std::make_shared<std::vector<std::vector<size_t>>>();
  std::shared_ptr<std::vector<size_t>> logicalTileCacheMBPerLevel_{}; ///< Logical tile cache size for every level
  std::vector<std::vector<size_t>> const
      logicalTileDimensionRequestedPerDimensionPerLevel_{}; ///< Logical tile dimensions requested per dimension per level

 public:
  /// @brief Adaptive Fast Loader graph constructor
  /// @param configuration Fast Loader graph configuration
  /// @param logicalTileDimensionRequestedPerDimensionPerLevel Dimensions of the tiles for all levels {{d00, ..,, d0n}, ..., {dm0, ..., dmn}} for n dimensions tiles with m levels
  /// @param logicalTileCacheMBPerLevel Cache size in MB used  for the transformation between physical tiles to requested tiles for every level
  /// @param nbThreadsCopyLogicalCacheView Number of threads associated with the copy from the logical cache to view task
  /// @param name Fast Loader graph name
  AdaptiveFastLoaderGraph(
      std::unique_ptr<FastLoaderConfiguration<ViewType>> configuration,
      std::vector<std::vector<size_t>> const &logicalTileDimensionRequestedPerDimensionPerLevel,
      std::vector<size_t> logicalTileCacheMBPerLevel = {},
      size_t nbThreadsCopyLogicalCacheView = 2,
      std::string const &name = "Adaptive Tile Loader")
      : fl::FastLoaderGraph<ViewType>(name),
        logicalTileCacheMBPerLevel_(std::make_shared<std::vector<size_t>>(logicalTileCacheMBPerLevel)),
        logicalTileDimensionRequestedPerDimensionPerLevel_(logicalTileDimensionRequestedPerDimensionPerLevel) {
    auto
        tileLoaderAllCaches = std::make_shared<std::vector<std::shared_ptr<internal::Cache<DataType>>>>(),
        allAdaptiveCaches = std::make_shared<std::vector<std::shared_ptr<internal::Cache<DataType>>>>();
    std::vector<size_t> sizeMemoryManagerPerLevel = {};

    this->configuration_ = std::move(configuration);

    if (!this->configuration_) {
      std::ostringstream oss;
      oss << "The configuration given to FastLoaderConfiguration is not valid.";
      throw (std::runtime_error(oss.str()));
    }

    this->tileLoader_ = this->configuration_->tileLoader_;
    this->nbDimensions_ = this->tileLoader_->nbDims();
    this->nbPyramidLevels_ = this->tileLoader_->nbPyramidLevels();

    validateInputs(logicalTileDimensionRequestedPerDimensionPerLevel_);
    if (nbThreadsCopyLogicalCacheView == 0) { nbThreadsCopyLogicalCacheView = 2; }

    this->fullDimensionPerLevel_->reserve(this->nbPyramidLevels_);
    this->tileDimensionPerLevel_->reserve(this->nbPyramidLevels_);
    this->viewDimensionPerLevel_->reserve(this->nbPyramidLevels_);
    logicalTileCacheMBPerLevel_->reserve(this->nbPyramidLevels_);
    physicalTileDimensionPerLevel_->reserve(this->nbPyramidLevels_);
    tileLoaderAllCaches->reserve(this->nbPyramidLevels_);
    allAdaptiveCaches->reserve(this->nbPyramidLevels_);

    std::vector<size_t> tmpDimension;
    for (size_t level = 0; level < this->nbPyramidLevels_; ++level) {
      this->fullDimensionPerLevel_->push_back(this->tileLoader_->fullDims(level));
      this->tileDimensionPerLevel_->push_back(logicalTileDimensionRequestedPerDimensionPerLevel_.at(level));
      physicalTileDimensionPerLevel_->push_back(this->tileLoader_->tileDims(level));

      double const sizeLogicalTileMB = (double) std::accumulate(
          this->tileDimensionPerLevel_->back().cbegin(), this->tileDimensionPerLevel_->back().cend(),
          (size_t) 1, std::multiplies<>()) * sizeof(typename ViewType::data_t) / (double) (1024 * 1024);
      double const sizePhysicalTileMB = (double) std::accumulate(
          physicalTileDimensionPerLevel_->at(level).cbegin(), physicalTileDimensionPerLevel_->at(level).cend(),
          (size_t) 1, std::multiplies<>()) * sizeof(typename ViewType::data_t) / (double) (1024 * 1024);

      std::transform(
          this->tileDimensionPerLevel_->at(level).cbegin(),
          this->tileDimensionPerLevel_->at(level).cend(),
          this->radii().cbegin(),
          std::back_insert_iterator<std::vector<size_t>>(tmpDimension),
          [](auto const &tileDimension, auto const &radius) { return tileDimension + 2 * radius; });
      this->viewDimensionPerLevel_->push_back(tmpDimension);
      tmpDimension.clear();

      sizeMemoryManagerPerLevel.push_back(
          std::accumulate(
              this->viewDimensionPerLevel_->at(level).cbegin(), this->viewDimensionPerLevel_->at(level).cend(),
              (size_t) 1, std::multiplies<>())
      );

      // Cache creation per level
      // Tile loader cache
      std::transform(
          this->fullDimensionPerLevel_->at(level).cbegin(),
          this->fullDimensionPerLevel_->at(level).cend(),
          physicalTileDimensionPerLevel_->at(level).cbegin(),
          std::back_insert_iterator<std::vector<size_t>>(tmpDimension),
          [](auto const &fullDimension, auto const &physicalTileDimension) {
            return (size_t) ceil((double) (fullDimension) / (double) (physicalTileDimension));
          });

      tileLoaderAllCaches->push_back(
          std::make_shared<internal::Cache<DataType>>(
              tmpDimension,
              (size_t) std::max(
                  (double) 1,
                  (double) (this->configuration_->cacheCapacityMB().at(level)) / sizePhysicalTileMB
              ),
              physicalTileDimensionPerLevel_->at(level)
          )
      );
      tmpDimension.clear();

      // Adaptive cache
      allAdaptiveCaches->push_back(
          std::make_shared<internal::Cache<DataType>>(
              this->nbTilesDims(level),
              (size_t) std::max(
                  (double) 1,
                  (double) (logicalTileCacheMBPerLevel_->at(level)) / sizeLogicalTileMB
              ),
              this->tileDimensionPerLevel_->at(level)
          )
      );
    }
    this->tileLoader_->allCaches_ = tileLoaderAllCaches;

    // Tasks
    auto viewCounter =
        std::make_shared<internal::ViewCounter<ViewType>>(this->configuration_->borderCreator_,
                                                          this->configuration_->ordered_);
    auto cpyPhysicalToView = std::make_shared<internal::CopyPhysicalToView<ViewType>>(this->configuration_->nbThreadsCopyPhysicalCacheView());
    // Internal graph
    this->levelGraph_ =
        std::make_shared<hh::Graph<1, IndexRequest, internal::TileRequest<ViewType>>>("Fast Loader Level");

    // Task & memory manager
    if constexpr (std::is_base_of<DefaultView<DataType>, ViewType>::value) {
      using ViewDataType = internal::DefaultViewData<DataType>;

      auto viewWaiter = std::make_shared<internal::ViewWaiter<ViewType, ViewDataType>>(
          this->configuration_->ordered_, 0, this->configuration_->fillingType_,
          viewCounter,
          this->fullDimensionPerLevel_, this->tileDimensionPerLevel_, this->radii(), this->tileLoader_->dimNames()
      );

      auto mm = std::make_shared<internal::FastLoaderMemoryManager<ViewDataType>>(
          this->configuration_->viewAvailablePerLevel_,
          sizeMemoryManagerPerLevel,
          this->configuration_->nbReleasePyramid_);

      auto viewLoader =
          std::make_shared<internal::ViewLoader<ViewType, ViewDataType>>(this->configuration_->borderCreator_);

      auto mapperLogicalPhysical = std::make_shared<internal::MapperLogicalPhysical<ViewType>>(
          this->physicalTileDimensionPerLevel_, this->tileDimensionPerLevel_, this->fullDimensionPerLevel_,
          allAdaptiveCaches, this->tileLoader_->dimNames()
      );

      auto directToCopyStateManager = std::make_shared<
          hh::StateManager<
              1, fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>
          >
      >(std::make_shared<internal::DirectToCopyState<ViewType>>(), "Direct to copy");

      auto copyLogicalTileToView = std::make_shared<fl::internal::CopyLogicalTileToView<ViewType>>(
          nbThreadsCopyLogicalCacheView
      );

      auto toTLStateManager = std::make_shared<
          hh::StateManager<1, fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::TileRequest<ViewType>>>
          (std::make_shared<fl::internal::ToTileLoaderState<ViewType>>(), "To TL");

      auto tlCounterStateManager = std::make_shared<
          hh::StateManager<1, fl::internal::TileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>>>
          (std::make_shared<fl::internal::TileLoaderCounterState<ViewType>>(), "Counter SM");

      viewWaiter->connectMemoryManager(mm);
      this->levelGraph_->inputs(viewWaiter);
      this->levelGraph_->edges(viewWaiter, viewLoader);

      this->levelGraph_->edges(viewLoader, mapperLogicalPhysical);
      // Direct Copy if cache found
      this->levelGraph_->edges(mapperLogicalPhysical, directToCopyStateManager);
      this->levelGraph_->edges(directToCopyStateManager, copyLogicalTileToView);


      //Route with tileLoader
      this->levelGraph_->edges(mapperLogicalPhysical, toTLStateManager);
      this->levelGraph_->edges(toTLStateManager, this->tileLoader_);
      this->levelGraph_->edges(this->tileLoader_, cpyPhysicalToView);
      this->levelGraph_->edges(cpyPhysicalToView, tlCounterStateManager);
      this->levelGraph_->edges(tlCounterStateManager, copyLogicalTileToView);

      // Output
      this->levelGraph_->outputs(copyLogicalTileToView);

    }
#ifdef HH_USE_CUDA
    else if constexpr (std::is_base_of<UnifiedView<typename ViewType::data_t>, ViewType>::value) {
      using ViewDataType = internal::UnifiedViewData<typename ViewType::data_t>;

      auto viewWaiter = std::make_shared<internal::ViewWaiter<ViewType, ViewDataType>>(
          this->configuration_->ordered_, 0, this->configuration_->fillingType_,
          viewCounter,
          this->fullDimensionPerLevel_, this->tileDimensionPerLevel_, this->radii(), this->tileLoader_->dimNames()
      );

      auto mm = std::make_shared<internal::FastLoaderMemoryManager<ViewDataType>>(
          this->configuration_->viewAvailablePerLevel_,
          sizeMemoryManagerPerLevel,
          this->configuration_->nbReleasePyramid_);

      auto viewLoader =
          std::make_shared<internal::ViewLoader<ViewType, ViewDataType>>(this->configuration_->borderCreator_);

      auto mapperLogicalPhysical = std::make_shared<internal::MapperLogicalPhysical<ViewType>>(
          this->physicalTileDimensionPerLevel_, this->tileDimensionPerLevel_, this->fullDimensionPerLevel_,
          this->nbLogicalTileCachePerLevel_, allAdaptiveCaches, this->tileLoader_->dimNames()
      );

      auto directToCopyStateManager = std::make_shared<
          hh::StateManager<
              1, fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>
          >
      >(std::make_shared<internal::DirectToCopyState<ViewType>>());

      auto copyLogicalTileToView = std::make_shared<fl::internal::CopyLogicalTileToView<ViewType>>(
          nbThreadsCopyLogicalCacheView);

      auto toTLStateManager = std::make_shared<
          hh::StateManager<1, fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::TileRequest<ViewType>>>
          (std::make_shared<fl::internal::ToTileLoaderState<ViewType>>(), "To TL SM");

      auto tlCounterStateManager = std::make_shared<
          hh::StateManager<1, fl::internal::TileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>>>
          (std::make_shared<fl::internal::TileLoaderCounterState<ViewType>>(), "Counter SM");

      viewWaiter->connectMemoryManager(mm);
      this->levelGraph_->inputs(viewWaiter);
      this->levelGraph_->edges(viewWaiter, viewLoader);

      this->levelGraph_->edges(viewLoader, mapperLogicalPhysical);
      // Direct Copy if cache found
      this->levelGraph_->edges(mapperLogicalPhysical, directToCopyStateManager);
      this->levelGraph_->edges(directToCopyStateManager, copyLogicalTileToView);


      //Route with tileLoader
      this->levelGraph_->edges(mapperLogicalPhysical, toTLStateManager);
      this->levelGraph_->edges(toTLStateManager, this->tileLoader_);
      this->levelGraph_->edges(this->tileLoader_, cpyPhysicalToView);
      this->levelGraph_->edges(cpyPhysicalToView, tlCounterStateManager);
      this->levelGraph_->edges(tlCounterStateManager, copyLogicalTileToView);

      // Output
      this->levelGraph_->outputs(copyLogicalTileToView);

    }
#endif // HH_USE_CUDA
    else {
      throw std::runtime_error("The View Data Type inside of the used View is not known to construct the graph.");
    }
    // Internal Execution pipeline
    auto levelExecutionPipeline =
        std::make_shared<internal::FastLoaderExecutionPipeline<ViewType>>(
            this->levelGraph_, this->tileLoader_->nbPyramidLevels());

    // Fast Loader graph
    this->inputs(levelExecutionPipeline);
    this->edges(levelExecutionPipeline, viewCounter);
    this->outputs(viewCounter);
  }

  /// @brief Default destructor
  ~AdaptiveFastLoaderGraph() override = default;

  /// @brief Estimate the maximum memory usage used by FastLoader in bytes
  /// @return Estimate the maximum memory usage used by FastLoader in bytes
  size_t estimatedMaximumMemoryUsageMB() override {
    size_t sum = 0;
    size_t const sizeVoxelByte = sizeof(typename ViewType::data_t);
    for (size_t level = 0; level < this->configuration_->nbLevels_; ++level) {
      auto const &logicalTileDims = this->tileDims(level);
      std::vector<size_t> logicalViewDims;

      std::transform(
          logicalTileDims.cbegin(), logicalTileDims.cend(), this->radii().cbegin(), std::back_inserter(logicalViewDims),
          [](auto const tileDim, auto const radius) { return tileDim + 2 * radius; }
      );

      size_t const
          logicalViewSizeMB =
          std::accumulate(logicalViewDims.cbegin(), logicalViewDims.cend(), (size_t) 1, std::multiplies<>())
              * sizeVoxelByte / (size_t)(1024 * 1024);

      // Add cache size
      sum += this->configuration_->cacheCapacityMB().at(level);
      sum += this->logicalTileCacheMBPerLevel_->at(level);
      // Add views size flowing
      sum += this->configuration_->viewAvailablePerLevel_.at(level) * logicalViewSizeMB;
    }
    return sum;
  }

 private:
  /// @brief Test validity of user inputs
  /// @param logicalTileDimensionRequestedPerDimensionPerLevel User defined requested dimensions of the tiles for all levels
  void validateInputs(std::vector<std::vector<size_t>> const &logicalTileDimensionRequestedPerDimensionPerLevel) {
    if (logicalTileCacheMBPerLevel_->empty()) {
      logicalTileCacheMBPerLevel_->reserve(this->nbPyramidLevels_);
      for (size_t level = 0; level < this->nbPyramidLevels_; ++level) { logicalTileCacheMBPerLevel_->push_back(10); }
    }

    if (!this->configuration_) {
      std::ostringstream oss;
      oss << "The configuration given to FastLoaderConfiguration is not valid.";
      throw (std::runtime_error(oss.str()));
    }

    if (logicalTileDimensionRequestedPerDimensionPerLevel.size() != this->nbPyramidLevels_) {
      throw std::runtime_error(
          "The number of logical tile dimensions and number of logical caches requested should match the number of pyramid level.");
    }

    if (std::any_of(
        logicalTileDimensionRequestedPerDimensionPerLevel.cbegin(),
        logicalTileDimensionRequestedPerDimensionPerLevel.cend(),
        [](auto const &dimension) {
          return std::any_of(dimension.cbegin(), dimension.cend(), [](auto const &measure) { return measure == 0; });
        })) {
      throw std::runtime_error("The logical tile requested should be superior to 0.");
    }
    if (std::any_of(logicalTileCacheMBPerLevel_->cbegin(), logicalTileCacheMBPerLevel_->cend(),
                    [](size_t const &val) { return val == 0; })) {
      throw std::runtime_error("The logical tile cache requested should be superior to 0 MB.");
    }
  }
};

} // fl

#endif //FAST_LOADER_ADAPTIVE_FAST_LOADER_GRAPH_H
