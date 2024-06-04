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

#ifndef FAST_LOADER_FAST_LOADER_GRAPH_H
#define FAST_LOADER_FAST_LOADER_GRAPH_H

#include <hedgehog/hedgehog.h>
#include "../data/index_request.h"
#include "fast_loader_configuration.h"
#include "../view/unified_view.h"
#include "../../core/task/view_counter.h"
#include "../../core/task/view_loader.h"
#include "../../core/task/view_waiter.h"
#include "../../core/fast_loader_memory_manager.h"
#include "../../core/fast_loader_execution_pipeline.h"
#include "../../core/task/copy_physical_to_view.h"


/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader graph (FLG) used to acquire part of an image (views) out of tiles from a file optionally surrounded by values.
/// @details To execute a FLG needs a FastLoader configuration object, responsible to tune the acquisition of data and give access to the file.
/// @details The default usage is:
/// -# Create a configuration object (FastLoaderConfiguration) that uses an AbstractTileLoader
/// -# Create a FLG from the configuration object
/// -# Execute the graph
/// -# Request the view(s)
/// -# Notify that no more view will be requested (allows termination)
/// -# Get the result and return them
/// -# Wait for FLG termination
/// @attention The configuration needs to be moved into the constructor.
/// @tparam ViewType Type of the view
/// @code
/// // Create the Tile Loader
/// auto tl = std::make_shared<VirtualFileTileLoader>(10, {100, 100, 100}, {10, 10, 10});
/// // Create a configuration
/// auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
/// // Set options
/// options->radius(radius);
/// options->viewAvailable({10});
/// options->cacheCapacityMB({10});
/// // Create the FastLoaderGraph by setting the configuration
/// auto fl = fl::FastLoaderGraph<fl::DefaultView<int>>(std::move(options));
///
/// // Execute the graph, if not called, nothing happened
/// fl.executeGraph(true);
/// // Request tiles from pyramid level 0
/// fl.requestAllViews(0);
/// // Notify no more tile will be requested
/// fl.finishRequestingViews();
///
/// // Get the resulting view
/// while (auto viewVariantFL = fl.getBlockingResult()) {
///   auto view = std::get<std::shared_ptr<fl::DefaultView<int>>>(*viewVariantFL);
///   // Do stuff
///   // Return the view to the FastLoaderGraph
///   view->returnToMemoryManager();
/// }
/// // Wait for FastLoaderGraph to terminate
/// fl.waitForTermination();
/// @endcode
/// @warning A FLG is an Hedgehog graph, it can not be modified or used directly after being attached to other Hedgehog nodes / graph.
/// @tparam ViewType Type of the view.
template<class ViewType>
class FastLoaderGraph : public hh::Graph<1, IndexRequest, ViewType> {
 protected:
  std::unique_ptr<FastLoaderConfiguration<ViewType>> configuration_{}; ///< FastLoader configuration

  std::shared_ptr<hh::Graph<1, IndexRequest, internal::TileRequest<ViewType>>>
      levelGraph_ = {}; ///< Internal graph for each levels

  std::shared_ptr<AbstractTileLoader<ViewType>>
      tileLoader_ = {}; ///< User defined Tile Loader

  size_t
      nbDimensions_{}, ///< Number of dimensions in the file
      nbPyramidLevels_{}; ///< Number of pyramid levels in the file

  bool
      finishRequestingTiles_ = false; ///< Flag to indicate tiles will not be requested anymore

  std::shared_ptr<std::vector<std::vector<size_t>>>
      fullDimensionPerLevel_{}, ///< Full dimensions acquired by the TL
      tileDimensionPerLevel_{}, ///< Tile dimensions acquired by the TL
      viewDimensionPerLevel_{}; ///< View dimensions computed

 public:
  /// @brief Main FastLoaderGraph constructor
  /// @param configuration FastLoaderGraph configuration. Need to be moved, and can not be modified after being set.
  /// @param name FastLoaderGraph constructor
  /// @throw std::runtime_error If configuration is not valid
  explicit FastLoaderGraph(std::unique_ptr<FastLoaderConfiguration<ViewType>> configuration,
                           std::string const &name = "Fast Loader")
      : hh::Graph<1, IndexRequest, ViewType>(name), configuration_(std::move(configuration)) {

    auto tileLoaderAllCaches =
        std::make_shared<std::vector<std::shared_ptr<internal::Cache<typename ViewType::data_t>>>>();

    std::vector<size_t> sizeMemoryManagerPerLevel = {};

    fullDimensionPerLevel_ = std::make_shared<std::vector<std::vector<size_t>>>();
    tileDimensionPerLevel_ = std::make_shared<std::vector<std::vector<size_t>>>();
    viewDimensionPerLevel_ = std::make_shared<std::vector<std::vector<size_t>>>();

    if (!configuration_) {
      std::ostringstream oss;
      oss << "The configuration given to FastLoaderConfiguration is not valid.";
      throw (std::runtime_error(oss.str()));
    }

    tileLoader_ = configuration_->tileLoader_;
    nbDimensions_ = tileLoader_->nbDims();
    nbPyramidLevels_ = tileLoader_->nbPyramidLevels();

    fullDimensionPerLevel_->reserve(nbPyramidLevels_);
    tileDimensionPerLevel_->reserve(nbPyramidLevels_);
    viewDimensionPerLevel_->reserve(nbPyramidLevels_);
    sizeMemoryManagerPerLevel.reserve(nbPyramidLevels_);

    std::vector<size_t> viewDimension;

    for (size_t level = 0; level < nbPyramidLevels_; ++level) {
      fullDimensionPerLevel_->push_back(tileLoader_->fullDims(level));
      tileDimensionPerLevel_->push_back(tileLoader_->tileDims(level));


      double const sizeTileMB = (double) std::accumulate(
          tileDimensionPerLevel_->back().cbegin(), tileDimensionPerLevel_->back().cend(),
          (size_t) 1, std::multiplies<>()) * sizeof(typename ViewType::data_t) / (double) (1024 * 1024);


      std::transform(
          tileDimensionPerLevel_->at(level).cbegin(), tileDimensionPerLevel_->at(level).cend(),
          configuration_->radii_.cbegin(), std::back_insert_iterator<std::vector<size_t>>(viewDimension),
          [](auto const &tileSize, auto const &radiusSize) {
            return tileSize + 2 * radiusSize;
          });
      viewDimensionPerLevel_->push_back(viewDimension);
      viewDimension.clear();

      sizeMemoryManagerPerLevel.push_back(
          std::accumulate(
              viewDimensionPerLevel_->at(level).cbegin(), viewDimensionPerLevel_->at(level).cend(),
              (size_t) 1, std::multiplies<>()
          )
      );

      tileLoaderAllCaches->emplace_back(
          std::make_shared<internal::Cache<typename ViewType::data_t>>(
              nbTilesDims(level),
              (size_t) std::max(
                  (double) 1,
                  (double) (configuration_->cacheCapacityMB().at(level)) / sizeTileMB
              ),
              tileDimensionPerLevel_->at(level)
          ));

    }
    tileLoader_->allCaches_ = tileLoaderAllCaches;

    // Create the tasks
    auto viewCounter
        = std::make_shared<internal::ViewCounter<ViewType>>(configuration_->borderCreator_, configuration_->ordered_);
    // Internal graph
    levelGraph_ =
        std::make_shared<hh::Graph<1, IndexRequest, internal::TileRequest<ViewType>>>("Fast Loader Level");

    auto cpyPhysicalToView = std::make_shared<internal::CopyPhysicalToView<ViewType>>(this->configuration_->nbThreadsCopyPhysicalCacheView());

    // Task & memory manager
    if constexpr (std::is_base_of<DefaultView<typename ViewType::data_t>, ViewType>::value) {
      using ViewDataType = internal::DefaultViewData<typename ViewType::data_t>;
      auto viewLoader =
          std::make_shared<internal::ViewLoader<ViewType, ViewDataType>>(configuration_->borderCreator_);
      auto viewWaiter = std::make_shared<internal::ViewWaiter<ViewType, ViewDataType>>(
          configuration_->ordered_, 0, configuration_->fillingType_, viewCounter,
          fullDimensionPerLevel_, tileDimensionPerLevel_, configuration_->radii_, tileLoader_->dimNames()
      );
      auto mm = std::make_shared<internal::FastLoaderMemoryManager<ViewDataType>>(
          this->configuration_->viewAvailablePerLevel_, sizeMemoryManagerPerLevel, configuration_->nbReleasePyramid_);
      viewWaiter->connectMemoryManager(mm);
      levelGraph_->inputs(viewWaiter);
      levelGraph_->edges(viewWaiter, viewLoader);
      levelGraph_->edges(viewLoader, tileLoader_);
    }
#ifdef HH_USE_CUDA
    else if constexpr (std::is_base_of<UnifiedView<typename ViewType::data_t>, ViewType>::value) {
      using ViewDataType = internal::UnifiedViewData<typename ViewType::data_t>;
      auto viewLoader =
          std::make_shared<internal::ViewLoader<ViewType, ViewDataType>>(configuration_->borderCreator_);
      auto viewWaiter = std::make_shared<internal::ViewWaiter<ViewType, ViewDataType>>(
          configuration_->ordered_, 0, configuration_->fillingType_, viewCounter,
          fullDimensionPerLevel_, tileDimensionPerLevel_, configuration_->radii_, tileLoader_->dimNames());
      auto mm = std::make_shared<internal::FastLoaderMemoryManager<ViewDataType>>(
          this->configuration_->viewAvailablePerLevel_, sizeMemoryManagerPerLevel, configuration_->nbReleasePyramid_);
      viewWaiter->connectMemoryManager(mm);
      levelGraph_->inputs(viewWaiter);
      levelGraph_->edges(viewWaiter, viewLoader);
      levelGraph_->edges(viewLoader, tileLoader_);
    }
#endif // HH_USE_CUDA
    else {
      throw std::runtime_error("The View Data Type inside of the used View is not known to construct the graph.");
    }
    levelGraph_->edges(tileLoader_, cpyPhysicalToView);
    levelGraph_->outputs(cpyPhysicalToView);

    // Internal Execution pipeline
    auto levelExecutionPipeline =
        std::make_shared<internal::FastLoaderExecutionPipeline<ViewType>>(levelGraph_,
                                                                          tileLoader_->nbPyramidLevels());

    // Fast Loader graph
    this->inputs(levelExecutionPipeline);
    this->edges(levelExecutionPipeline, viewCounter);
    this->outputs(viewCounter);
  }

  /// @brief Dimensions name accessor
  /// @return Dimensions name
  [[nodiscard]] std::vector<std::string> const &dimNames() const { return tileLoader_->dimNames(); }

  /// @brief File dimensions accessor for a level
  /// @param level Pyramidal Level
  /// @return File dimensions for a level
  [[nodiscard]] std::vector<size_t> const &fullDims([[maybe_unused]] std::size_t const level = 0) const {
    return tileLoader_->fullDims(level);
  }

  /// @brief Tile dimensions accessor for a level
  /// @param level Pyramidal Level
  /// @return Tile dimensions for a level
  [[nodiscard]] std::vector<size_t> const &tileDims([[maybe_unused]] std::size_t const level = 0) const {
    return tileLoader_->tileDims(level);
  }

  /// @brief Number tiles per dimension accessor for a level
  /// @param level Pyramidal Level
  /// @return Number tiles per dimension for a level
  [[nodiscard]] std::vector<size_t> nbTilesDims(std::size_t const level = 0) const {
    std::vector<size_t> nbTilesPerDimensions;
    std::transform(
        fullDimensionPerLevel_->at(level).cbegin(),
        fullDimensionPerLevel_->at(level).cend(),
        tileDimensionPerLevel_->at(level).cbegin(),
        std::back_insert_iterator<std::vector<size_t>>(nbTilesPerDimensions),
        [](auto const &fullDimension, auto const &tileDimension) {
          return (size_t) ceil((double) (fullDimension) / (double) (tileDimension));
        });
    return nbTilesPerDimensions;
  }

  /// @brief File dimension accessor for a dimension index and a level
  /// @param dim Dimension index
  /// @param level Pyramidal level
  /// @return File dimension for a dimension index and a level
  [[nodiscard]] size_t const &fullDim(std::size_t const dim, std::size_t const level = 0) const {
    return tileLoader_->fullDims(level).at(dim);
  }
  /// @brief Tile dimension accessor for a dimension index and a level
  /// @param dim Dimension index
  /// @param level Pyramidal level
  /// @return Tile dimension for a dimension index and a level
  [[nodiscard]] size_t const &tileDim(std::size_t const dim, std::size_t const level = 0) const {
    return tileLoader_->tileDims(level).at(dim);
  }

  /// @brief Number of tiles accessor for a dimension index and a level
  /// @param dim Dimension index
  /// @param level Pyramidal level
  /// @return Number of tiles for a dimension index and a level
  [[nodiscard]] size_t nbTilesDim(std::size_t const dim, std::size_t const level = 0) const {
    return (size_t) ceil((double) (fullDim(dim, level)) / (double) (tileDim(dim, level)));
  }

  /// @brief File dimension accessor for a dimension name and a level
  /// @param dimName Dimension name
  /// @param level Pyramidal level
  /// @return File dimension for a dimension name and a level
  [[nodiscard]] size_t const &fullDim(std::string const &dimName, std::size_t const level = 0) const {
    return tileLoader_->fullDim(dimIndex(dimName), level);
  }

  /// @brief Tile dimension accessor for a dimension name and a level
  /// @param dimName Dimension name
  /// @param level Pyramidal level
  /// @return Tile dimension for a dimension name and a level
  [[nodiscard]] size_t const &tileDim(std::string const &dimName, std::size_t const level = 0) const {
    return tileLoader_->tileDim(dimIndex(dimName), level);
  }

  /// @brief Number of tiles accessor for a dimension name and a level
  /// @param dimName Dimension name
  /// @param level Pyramidal level
  /// @return Number of tiles for a dimension name and a level
  [[nodiscard]] size_t nbTilesDim(std::string const &dimName, std::size_t const level = 0) const {
    return (size_t) ceil((double) (fullDim(dimIndex(dimName), level)) / (double) (tileDim(dimIndex(dimName), level)));
  }

  /// @brief Test if a dimension name is registered in the tile loader
  /// @param dimName Dimension name to test
  /// @return True if the dimension exists, else false
  [[nodiscard]] bool hasDim(std::string const &dimName) const {
    auto const &names = dimNames();
    return std::find(names.cbegin(), names.cend(), dimName) != names.cend();
  }

  /// @brief Request a view
  /// @param indexCentralTile View Index
  /// @param level Pyramidal level
  void requestView(std::vector<size_t> const &indexCentralTile, size_t level = 0) {
    if (finishRequestingTiles_) { return; }
    assert(testIndex(indexCentralTile, level));
    this->pushData(std::make_shared<IndexRequest>(indexCentralTile, level));
  }

  /// @brief Request all the views for a level following the traversal set in configuration
  /// @param level AbstractView's level requested
  void requestAllViews(size_t level = 0) {
    if (finishRequestingTiles_) { return; }
    for (std::shared_ptr<IndexRequest> const &indexRequest : generateIndexRequestForAllViews(level)) {
      this->pushData(indexRequest);
    }
  }

  /// @brief Indicate no more view will be requested
  void finishRequestingViews() {
    if (!finishRequestingTiles_) {
      finishRequestingTiles_ = true;
      this->finishPushingData();
    }
  }

  /// @brief Get the index of a dimension from its name
  /// @param name Dimension name
  /// @return Dimension index
  [[nodiscard]] size_t dimIndex(std::string const &name) const {
    auto const &names = dimNames();
    auto it = std::find(names.cbegin(), names.cend(), name);
    if (it != names.cend()) { return std::distance(names.cbegin(), it); }
    else {
      std::ostringstream oss;
      oss << "The dimension \"" << name << "\"" << " does not exist.";
      throw std::runtime_error(oss.str());
    }
  }

  /// @brief Generate all the AbstractView request for a level following the traversal set in configuration, can be used when
  /// the FastLoaderGraph is embedded into another graph to generate easily FastImageGraph inputs
  /// @param level Level asked [default 0]
  /// @return A vector of ViewRequest
  [[nodiscard]] std::vector<std::shared_ptr<IndexRequest>> generateIndexRequestForAllViews(size_t level = 0) const {
    std::vector<std::shared_ptr<IndexRequest>> ret = {};
    auto nbTiles = this->nbTilesDims(level);
    for (auto const &step : configuration_->traversal_->traversal(nbTiles)) {
      ret.push_back(std::make_shared<IndexRequest>(step, level));
    }
    return ret;
  }

  /// @brief Request a AbstractView for a level, can be used when the FastLoaderGraph is embedded into another graph to
  /// generate easily FastImageGraph inputs
  /// @param index Index to request
  /// @param level AbstractView's level requested (default 0)
  /// @return IndexRequest
  std::shared_ptr<IndexRequest> generateIndexRequest(std::vector<size_t> const &index, size_t const &level = 0) {
    return std::make_shared<IndexRequest>(index, level);
  }

  /// @brief Estimate the maximum memory usage used by FastLoader in bytes
  /// @return Estimate the maximum memory usage used by FastLoader in bytes
  [[nodiscard]] virtual size_t estimatedMaximumMemoryUsageMB() {
    size_t sum = 0;
    size_t const sizeVoxelByte = sizeof(typename ViewType::data_t);
    for (size_t level = 0; level < this->configuration_->nbLevels_; ++level) {
      auto const &viewDims = this->viewDimensionPerLevel_->at(level);

      size_t const
          viewSizeMB =
          std::accumulate(viewDims.cbegin(), viewDims.cend(), (size_t) 1, std::multiplies<>())
              * sizeVoxelByte / (size_t)(1024 * 1024);

      // Add cache size
      sum += this->configuration_->cacheCapacityMB().at(level);
      // Add views size flowing
      sum += this->configuration_->viewAvailablePerLevel_.at(level) * viewSizeMB;
    }
    return sum;
  }

 protected:
  /// @brief FastLoaderGraph graph
  /// @param name Name of the FastLoaderGraph
  explicit FastLoaderGraph(std::string const &name) : hh::Graph<1, IndexRequest, ViewType>(name) {
    fullDimensionPerLevel_ = std::make_shared<std::vector<std::vector<size_t>>>();
    tileDimensionPerLevel_ = std::make_shared<std::vector<std::vector<size_t>>>();
    viewDimensionPerLevel_ = std::make_shared<std::vector<std::vector<size_t>>>();
  }

  /// @brief AbstractView's radii accessor
  /// @return AbstractView's radii
  [[nodiscard]] std::vector<size_t> const &radii() const { return configuration_->radii_; }
  /// @brief Get a radius from a dimension index
  /// @param dim Dimension index
  /// @return Radius size
  [[nodiscard]] size_t const &radius(std::size_t const dim) const { return configuration_->radii_.at(dim); }
  /// @brief Get a radius from a dimension name
  /// @param dimName Dimension name
  /// @return Radius size
  [[nodiscard]] size_t const &radius(std::string const &dimName) const {
    return configuration_->radii_.at(dimIndex(dimName));
  }

 private:
  /// @brief Test if a tile index exists
  /// @param indexCentralTile View index
  /// @param level Pyramidal index
  /// @return True if the index exists, else false
  [[nodiscard]] bool testIndex(std::vector<size_t> const &indexCentralTile, size_t level = 0) const {
    bool result = indexCentralTile.size() == this->nbDimensions_ && level < this->nbPyramidLevels_;
    if (result) { result = indexCentralTile < this->nbTilesDims(level); }
    return result;
  }

};
} // namespace fl

#endif //FAST_LOADER_FAST_LOADER_GRAPH_H
