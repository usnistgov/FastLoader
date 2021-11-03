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
//
// Created by anb22 on 11/8/19.
//

#ifndef FASTLOADER_FAST_LOADER_GRAPH_H
#define FASTLOADER_FAST_LOADER_GRAPH_H

#include <hedgehog/hedgehog.h>
#include "fast_loader_configuration.h"

#include "../internal/graph/fast_loader_execution_pipeline.h"
#include "../internal/graph/tasks/view_loader.h"
#include "../internal/graph/tasks/view_counter.h"
#include "../internal/graph/tasks/view_waiter.h"
#include "../internal/graph/memory_manager/fast_loader_memory_manager.h"
#include "../internal/data/view_data/default_view_data.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief Main FastLoader object to request view's from a tile-based file.
/// @details Hedgehog Graph, main accessor of views of type ViewType from a file. Created from a FastLoaderConfiguration.
/// Made to access planar (2D) and pyramidal files.
/// @attention The FastLoaderConfiguration can not be modified after being attached to a FastLoaderGraph.
/// @details The default usage is:
/// -# Create the configuration (by creating an AbstractTileLoader)
/// -# Create the FastLoaderGraph
/// -# Execute the graph
/// -# Request the tiles
/// -# Notify No more tile will be requested (allows termination)
/// -# Get the result and return them
/// -# Wait for FastLoaderGraph termination
/// @attention The configuration needs to be moved into the constructor.
/// /// @code
/// std::string path = "";
/// // Create the Tile Loader
/// auto tl = std::make_shared<GrayscaleTiffStripLoader<int>>(1, path,512, 512, 1);
/// // Create a configuration
/// auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
/// // Set the radius to 1
/// options->radius(1);
/// // Create the FastLoaderGraph by setting the configuration
/// auto fl = fl::FastLoaderGraph<fl::DefaultView<int>>(std::move(options));
/// // Execute the graph, if not called, nothing happened
/// fl.executeGraph();
/// // Request tiles from pyramid level 0
/// fl.requestAllViews(0);
/// // Notify no more tile will be requested
/// fl.finishRequestingViews();
///
/// // Get the resulting view
/// while (auto res = fl.getBlockingResult()) {
///   // Do stuff
///   // Return the view to the FastLoaderGraph
///   res->returnToMemoryManager();
/// }
/// // Wait for FastLoaderGraph to terminate
/// fl.waitForTermination();
/// @endcode
/// @details Because FastLoaderGraph is a Hedgehog Graph it can be embedded into another Bigger graph to do a
/// specific computation.
/// @attention When embedded into another graph, finishRequestingTiles should not be called,
/// generateIndexRequest/generateIndexRequestForAllViews should be called is ordering is needed.
/// @tparam ViewType Type of the view.
template<class ViewType>
class FastLoaderGraph : public hh::Graph<ViewType, IndexRequest> {
 protected:
  std::unique_ptr<FastLoaderConfiguration<ViewType>>
      configurations_ = {}; ///< FastLoaderGraph configurations

  std::shared_ptr<hh::Graph<internal::TileRequest<ViewType>, IndexRequest>>
      levelGraph_ = {}; ///< Internal graph for each levels

  std::shared_ptr<AbstractTileLoader<ViewType>>
      tileLoader_ = {}; ///< User defined Tile Loader

  bool
      finishRequestingTiles_ = false; ///< Flag to indicate tiles will not be requested anymore

  size_t
      numberChannels_{},
      numberPyramidLevels_{};

  std::shared_ptr<std::vector<size_t>>
      fullHeightPerLevel_ = std::make_shared<std::vector<size_t>>(),
      fullWidthPerLevel_ = std::make_shared<std::vector<size_t>>(),
      fullDepthPerLevel_ = std::make_shared<std::vector<size_t>>(),
      tileHeightPerLevel_ = std::make_shared<std::vector<size_t>>(),
      tileWidthPerLevel_ = std::make_shared<std::vector<size_t>>(),
      tileDepthPerLevel_ = std::make_shared<std::vector<size_t>>(),
      viewHeightPerLevel_ = std::make_shared<std::vector<size_t>>(),
      viewWidthPerLevel_ = std::make_shared<std::vector<size_t>>(),
      viewDepthPerLevel_ = std::make_shared<std::vector<size_t>>();

 public:
  /// @brief Main FastLoaderGraph constructor
  /// @param configuration FastLoaderGraph configuration. Need to be moved, and can not be modified after being set.
  /// @param name FastLoaderGraph constructor
  /// @throw std::runtime_error If configuration is not valid
  explicit FastLoaderGraph(std::unique_ptr<FastLoaderConfiguration<ViewType>> configuration,
                           std::string_view const &name = "Fast Loader") : hh::Graph<ViewType, IndexRequest>(
      name), configurations_(std::move(configuration)) {

    auto tileLoaderAllCaches =
        std::make_shared<std::vector<std::shared_ptr<internal::Cache<typename ViewType::data_t>>>>();

    std::vector<size_t> sizeMemoryManagerPerLevel = {};

    if (!configurations_) {
      std::ostringstream oss;
      oss << "The configuration given to FastLoaderConfiguration is not valid.";
      throw (std::runtime_error(oss.str()));
    }

    tileLoader_ = configurations_->tileLoader_;
    numberChannels_ = this->tileLoader_->numberChannels();
    numberPyramidLevels_ = this->tileLoader_->numberPyramidLevels();
    numberChannels_ = tileLoader_->numberChannels();

    fullHeightPerLevel_->reserve(numberPyramidLevels_);
    fullWidthPerLevel_->reserve(numberPyramidLevels_);
    fullDepthPerLevel_->reserve(numberPyramidLevels_);
    tileHeightPerLevel_->reserve(numberPyramidLevels_);
    tileWidthPerLevel_->reserve(numberPyramidLevels_);
    tileDepthPerLevel_->reserve(numberPyramidLevels_);
    viewHeightPerLevel_->reserve(numberPyramidLevels_);
    viewWidthPerLevel_->reserve(numberPyramidLevels_);
    viewDepthPerLevel_->reserve(numberPyramidLevels_);

    sizeMemoryManagerPerLevel.reserve(numberPyramidLevels_);

    // Getting the size for all levels
    for (size_t level = 0; level < numberPyramidLevels_; ++level) {
      fullHeightPerLevel_->push_back(tileLoader_->fullHeight(level));
      fullWidthPerLevel_->push_back(tileLoader_->fullWidth(level));
      fullDepthPerLevel_->push_back(tileLoader_->fullDepth(level));
      tileWidthPerLevel_->push_back(tileLoader_->tileWidth(level));
      tileHeightPerLevel_->push_back(tileLoader_->tileHeight(level));
      tileDepthPerLevel_->push_back(tileLoader_->tileDepth(level));
      viewWidthPerLevel_->push_back(tileWidthPerLevel_->at(level) + 2 * configurations_->radiusWidth_);
      viewHeightPerLevel_->push_back(tileHeightPerLevel_->at(level) + 2 * configurations_->radiusHeight_);
      viewDepthPerLevel_->push_back(tileDepthPerLevel_->at(level) + 2 * configurations_->radiusDepth_);

      sizeMemoryManagerPerLevel.push_back(
          numberChannels_ *
              viewHeightPerLevel_->at(level) * viewWidthPerLevel_->at(level) * viewDepthPerLevel_->at(level));

      tileLoaderAllCaches->emplace_back(
          std::make_shared<internal::Cache<typename ViewType::data_t>>(
              configurations_->cacheCapacity_[level],
              numberTileHeight(level),
              numberTileWidth(level),
              numberTileDepth(level),
              tileHeight(level),
              tileWidth(level),
              tileDepth(level),
              numberChannels_));
    }

    tileLoader_->allCaches_ = tileLoaderAllCaches;

    // Tasks
    auto viewCounter =
        std::make_shared<internal::ViewCounter<ViewType>>(configurations_->borderCreator_, configurations_->ordered_);
    // Internal graph
    levelGraph_ =
        std::make_shared<hh::Graph<internal::TileRequest<ViewType>, IndexRequest>>("Fast Loader Level");

    // Task & memory manager Memory Manager
    if constexpr(std::is_base_of<DefaultView<typename ViewType::data_t>, ViewType>::value) {
      using ViewDataType = internal::DefaultViewData<typename ViewType::data_t>;
      auto viewLoader =
          std::make_shared<internal::ViewLoader<ViewType, ViewDataType>>(configurations_->borderCreator_);

      auto viewWaiter = std::make_shared<internal::ViewWaiter<ViewType, ViewDataType>>(
          viewCounter, configurations_->ordered_,
          fullHeightPerLevel_, fullWidthPerLevel_, fullDepthPerLevel_,
          tileHeightPerLevel_, tileWidthPerLevel_, tileDepthPerLevel_,
          numberChannels_,
          configurations_->radiusHeight_, configurations_->radiusWidth_, configurations_->radiusDepth_,
          configurations_->fillingType_
      );

      auto mm = std::make_shared<internal::FastLoaderMemoryManager<ViewDataType>>(
          this->configurations_->viewAvailablePerLevel_, sizeMemoryManagerPerLevel, configurations_->nbReleasePyramid_);
      viewWaiter->connectMemoryManager(mm);
      levelGraph_->input(viewWaiter);
      levelGraph_->addEdge(viewWaiter, viewLoader);
      levelGraph_->addEdge(viewLoader, tileLoader_);
    }
#ifdef HH_USE_CUDA
      else if constexpr (std::is_base_of<UnifiedView<typename ViewType::data_t>, ViewType>::value){
        using ViewDataType = internal::UnifiedViewData<typename ViewType::data_t>;
        auto viewLoader =
            std::make_shared<internal::ViewLoader<ViewType, ViewDataType>>(configurations_->borderCreator_);

        auto viewWaiter = std::make_shared<internal::ViewWaiter<ViewType, ViewDataType>>(
            viewCounter, configurations_->ordered_,
            fullHeightPerLevel, fullWidthPerLevel, fullDepthPerLevel,
            tileHeightPerLevel, tileWidthPerLevel, tileDepthPerLevel,
            numberChannels,
            configurations_->radiusHeight_, configurations_->radiusWidth_, configurations_->radiusDepth_,
            configurations_->fillingType_);

        auto mm = std::make_shared<internal::FastLoaderMemoryManager<ViewDataType>>(
            this->configurations_->viewAvailablePerLevel_, sizeMemoryManagerPerLevel, configurations_->nbReleasePyramid_);
        viewWaiter->connectMemoryManager(mm);
        levelGraph_->input(viewWaiter);
        levelGraph_->addEdge(viewWaiter, viewLoader);
        levelGraph_->addEdge(viewLoader, tileLoader_);
      }
#endif // HH_USE_CUDA
    else {
      throw std::runtime_error("The View Data Type inside of the used View is not known to construct the graph.");
    }

    levelGraph_->output(tileLoader_);

    // Internal Execution pipeline
    auto levelExecutionPipeline =
        std::make_shared<internal::FastLoaderExecutionPipeline<ViewType>>(levelGraph_,
                                                                          tileLoader_->numberPyramidLevels());

    // Fast Loader graph
    this->input(levelExecutionPipeline);
    this->addEdge(levelExecutionPipeline, viewCounter);
    this->output(viewCounter);
  }

 protected:
  explicit FastLoaderGraph(std::string_view const &name) : hh::Graph<ViewType, IndexRequest>(name) {}

 public:
  /// @brief FastLoaderGraph destructor
  virtual ~FastLoaderGraph() = default;

  /// @brief AbstractView's radius height accessor
  /// @return AbstractView's radius height
  [[nodiscard]] size_t radiusHeight() const { return configurations_->radiusHeight_; }

  /// @brief AbstractView's radius width accessor
  /// @return AbstractView's radius width
  [[nodiscard]] size_t radiusWidth() const { return configurations_->radiusWidth_; }

  /// @brief AbstractView's radius depth accessor
  /// @return AbstractView's radius depth
  [[nodiscard]] size_t radiusDepth() const { return configurations_->radiusDepth_; }

  /// @brief Full height accessor for a level
  /// @param level Level asked [default 0]
  /// @return Full height for a level
  [[nodiscard]] size_t fullHeight(size_t level = 0) const { return tileLoader_->fullHeight(level); }

  /// @brief Full width accessor for a level
  /// @param level Level asked [default 0]
  /// @return Full width for a level
  [[nodiscard]] size_t fullWidth(size_t level = 0) const { return tileLoader_->fullWidth(level); }

  /// @brief Full depth accessor for a level
  /// @param level Level asked [default 0]
  /// @return Full depth for a level
  [[nodiscard]] size_t fullDepth(size_t level = 0) const { return tileLoader_->fullDepth(level); }

  /// @brief AbstractView height accessor for a level
  /// @param level Level asked [default 0]
  /// @return AbstractView height for a level
  [[nodiscard]] size_t viewHeight(size_t level = 0) const {
    return this->tileHeight(level) + 2 * radiusHeight();
  }
  /// @brief AbstractView width accessor for a level
  /// @param level Level asked [default 0]
  /// @return AbstractView width for a level
  [[nodiscard]] size_t viewWidth(size_t level = 0) const {
    return this->tileWidth(level) + 2 * radiusWidth();
  }
  /// @brief AbstractView depth accessor for a level
  /// @param level Level asked [default 0]
  /// @return AbstractView depth for a level
  [[nodiscard]] size_t viewDepth(size_t level = 0) const {
    return this->tileDepth(level) + 2 * radiusDepth();
  }

  /// \brief Getter to the number of channels
  /// \return Number of pixel's channels
  [[nodiscard]] size_t numberChannels() const { return tileLoader_->numberChannels(); }

  /// @brief Tile height accessor for a level
  /// @param level Level asked [default 0]
  /// @return Tile height for a level
  [[nodiscard]] size_t tileHeight(size_t level = 0) const {
    return this->tileHeightPerLevel_->at(level);
  }

  /// @brief Tile width accessor for a level
  /// @param level Level asked [default 0]
  /// @return Tile width for a level
  [[nodiscard]] size_t tileWidth(size_t level = 0) const {
    return this->tileWidthPerLevel_->at(level);
  }

  /// @brief Tile depth accessor for a level
  /// @param level Level asked [default 0]
  /// @return Tile depth for a level
  [[nodiscard]] size_t tileDepth(size_t level = 0) const {
    return this->tileDepthPerLevel_->at(level);
  }

  /// @brief Number tiles in height accessor for a level
  /// @param level Level asked
  /// @return Number tiles in height for a level
  [[nodiscard]] size_t numberTileHeight(size_t level = 0) const {
    return (size_t) ceil((double) (this->fullHeight(level)) / this->tileHeight(level));
  }

  /// @brief Number tiles in width accessor for a level
  /// @param level Level asked
  /// @return Number tiles in width for a level
  [[nodiscard]] size_t numberTileWidth(size_t level = 0) const {
    return (size_t) ceil((double) (this->fullWidth(level)) / this->tileWidth(level));
  }

  /// @brief Number tiles in depth accessor for a level
  /// @param level Level asked
  /// @return Number tiles in depth for a level
  [[nodiscard]] size_t numberTileDepth(size_t level = 0) const {
    return (size_t) ceil((double) (this->fullDepth(level)) / this->tileDepth(level));
  }

  /// @brief Number tiles accessor for a level
  /// @param level Level asked [default 0]
  /// @return Number tiles for a level
  [[nodiscard]] size_t numberTile(size_t level = 0) const {
    return numberTileHeight(level) * numberTileWidth(level) * numberTileDepth(level);
  }

  /// @brief Indicate no more view will be requested
  void finishRequestingViews() {
    if (!finishRequestingTiles_) {
      finishRequestingTiles_ = true;
      this->finishPushingData();
    }
  }

  /// @brief Request a view
  /// @param rowIndex AbstractView's row requested
  /// @param colIndex AbstractView's column requested
  /// @param layerIndex AbstractView's layer requested (default 0)
  /// @param level AbstractView's level requested (default 0)
  void requestView(size_t rowIndex, size_t colIndex, size_t layerIndex = 0, size_t level = 0) {
    if (finishRequestingTiles_) { return; }
    this->pushData(generateIndexRequest(rowIndex, colIndex, layerIndex, level));
  }

  /// @brief Request all the views for a level following the traversal set in configuration
  /// @param level AbstractView's level requested
  void requestAllViews(size_t level = 0) {
    if (finishRequestingTiles_) { return; }
    for (auto indexRequest: generateIndexRequestForAllViews(level)) { this->pushData(indexRequest); }
  }

 private:
  /// @brief Request a AbstractView for a level, can be used when the FastLoaderGraph is embedded into another graph to generate
  /// easily FastImageGraph inputs
  /// @param rowIndex AbstractView's row requested
  /// @param colIndex AbstractView's column requested
  /// @param layerIndex AbstractView's layer requested
  /// @param level AbstractView's level requested (default 0)
  /// @return IndexRequest
  std::shared_ptr<IndexRequest> generateIndexRequest(size_t rowIndex,
                                                     size_t colIndex,
                                                     size_t layerIndex = 0,
                                                     size_t level = 0) {
    return std::make_shared<IndexRequest>(rowIndex, colIndex, layerIndex, level);
  }

  /// @brief Generate all the AbstractView request for a level following the traversal set in configuration, can be used when
  /// the FastLoaderGraph is embedded into another graph to generate easily FastImageGraph inputs
  /// @param level Level asked [default 0]
  /// @return A vector of ViewRequest
  std::vector<std::shared_ptr<IndexRequest>> generateIndexRequestForAllViews(size_t level = 0) {
    std::vector<std::shared_ptr<IndexRequest>> ret = {};
    for (auto step: this->configurations_->traversal_->traversal(
        this->numberTileHeight(level), this->numberTileWidth(level), this->numberTileDepth(level))) {
      ret.push_back(generateIndexRequest(step[0], step[1], step[2], level));
    }
   return ret;
  }

};

}
#endif //FASTLOADER_FAST_LOADER_GRAPH_H
