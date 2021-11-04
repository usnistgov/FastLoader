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


#ifndef INC_FAST_LOADER_ADAPTIVE_FAST_LOADER_GRAPH_H
#define INC_FAST_LOADER_ADAPTIVE_FAST_LOADER_GRAPH_H

#include <utility>

#include "fast_loader_graph.h"
#include "../internal/graph/tasks/mapper_logical_physical.h"
#include "../internal/graph/tasks/copy_logical_tile_to_view.h"
#include "../internal/graph/states/direct_to_copy_state.h"
#include "../internal/graph/states/to_tile_loader_state.h"
#include "../internal/graph/states/tile_loader_counter_state.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief Special type of FastLoaderGraph that transforms the tiling existing in the file.
/// Works exactly the same way of the FastLoaderGraph but ask for the the new tile dimensions (for all image level):
/// -# logicalTileHeightRequestedPerLevel: heights of the tiles per level
/// -# physicalTileWidthPerLevel_: widths of the tiles per level
/// -# physicalTileDepthPerLevel_: depths of the tiles per level
/// and the cache size for logical tiles (numberLogicalTilesCachePerLevel). This cache has been added to avoid extra construction of tiles from the
/// physical tiles coming from the tile loader (the file).
/// @tparam ViewType Type of the view.
template<class ViewType>
class AdaptiveFastLoaderGraph : public fl::FastLoaderGraph<ViewType> {

 private:
  using DataType = typename ViewType::data_t; ///< Sample type (AbstractView element type)
  std::shared_ptr<std::vector<size_t>>
      physicalTileHeightPerLevel_ = std::make_shared<std::vector<size_t>>(), ///< Physical tile heights per level
                                                                              /// from the tile loader
      physicalTileWidthPerLevel_ = std::make_shared<std::vector<size_t>>(), ///< Physical tile widths per level
                                                                              /// from the tile loader
      physicalTileDepthPerLevel_ = std::make_shared<std::vector<size_t>>(), ///< Physical tile depths per level
                                                                              /// from the tile loader
      numberLogicalTilesCachePerLevel_ = std::make_shared<std::vector<size_t>>(); ///< Number of logical tiles cached

 public:
  /// @brief AdaptiveFastLoaderGraph constructor, requires to provide the dimension of the tiles use to build the views
  /// @param configuration FastLoaderGraph configuration. Need to be moved, and can not be modified after being set.
  /// @param logicalTileHeightRequestedPerLevel Logical tile heights per level requested
  /// @param logicalTileWidthRequestedPerLevel Logical tile widths per level requested
  /// @param logicalTileDepthRequestedPerLevel Logical tile depths per level requested
  /// @param numberLogicalTilesCachePerLevel Number of logical cached tiles per level
  /// @param name Graph name (default: Adaptive Tile Loader)
  AdaptiveFastLoaderGraph(
      std::unique_ptr<FastLoaderConfiguration<ViewType>> configuration,
      std::vector<size_t> const &logicalTileHeightRequestedPerLevel,
      std::vector<size_t> const &logicalTileWidthRequestedPerLevel,
      std::vector<size_t> const &logicalTileDepthRequestedPerLevel,
      std::vector<size_t> numberLogicalTilesCachePerLevel = {},
      std::string_view const &name = "Adaptive Tile Loader")
      : fl::FastLoaderGraph<ViewType>(name) {

    auto
        tileLoaderAllCaches =
        std::make_shared<std::vector<std::shared_ptr<internal::Cache<DataType>>>>(),
        allAdaptiveCaches =
        std::make_shared<std::vector<std::shared_ptr<internal::Cache<DataType>>>>();

    std::vector<size_t>
        sizeMemoryManagerPerLevel = {};

    size_t
        numberThreadsCopyCacheView = 2;

    this->configurations_ = std::move(configuration);
    this->tileLoader_ = this->configurations_->tileLoader_;
    this->numberChannels_ = this->tileLoader_->numberChannels();
    this->numberPyramidLevels_ = this->tileLoader_->numberPyramidLevels();

    if (numberLogicalTilesCachePerLevel.empty()) {
      numberLogicalTilesCachePerLevel.reserve(this->numberPyramidLevels_);
      for (size_t level = 0; level < this->numberPyramidLevels_; ++level) {
        numberLogicalTilesCachePerLevel.push_back(5);
      }
    }

    if (logicalTileHeightRequestedPerLevel.size() != this->numberPyramidLevels_ ||
        logicalTileWidthRequestedPerLevel.size() != this->numberPyramidLevels_ ||
        logicalTileDepthRequestedPerLevel.size() != this->numberPyramidLevels_ ||
        numberLogicalTilesCachePerLevel.size() != this->numberPyramidLevels_) {
      throw std::runtime_error(
          "The number of logical tile dimensions and number of logical caches requested should match the number of pyramid level.");
    }

    if (std::any_of(logicalTileHeightRequestedPerLevel.begin(),
                    logicalTileHeightRequestedPerLevel.end(),
                    [](size_t const &val) { return val == 0; })
        || std::any_of(logicalTileWidthRequestedPerLevel.begin(),
                       logicalTileWidthRequestedPerLevel.end(),
                       [](size_t const &val) { return val == 0; })
        || std::any_of(logicalTileDepthRequestedPerLevel.begin(),
                       logicalTileDepthRequestedPerLevel.end(),
                       [](size_t const &val) { return val == 0; })) {
      throw std::runtime_error("The logical tile requested should be superior to 0.");
    }

    if (std::any_of(numberLogicalTilesCachePerLevel.begin(), numberLogicalTilesCachePerLevel.end(),
                    [](size_t const &val) { return val == 0; })) {
      throw std::runtime_error("The logical tile cache requested should be superior to 0.");
    }

    this->fullHeightPerLevel_->reserve(this->numberPyramidLevels_);
    this->fullWidthPerLevel_->reserve(this->numberPyramidLevels_);
    this->fullDepthPerLevel_->reserve(this->numberPyramidLevels_);
    this->tileHeightPerLevel_->reserve(this->numberPyramidLevels_);
    this->tileWidthPerLevel_->reserve(this->numberPyramidLevels_);
    this->tileDepthPerLevel_->reserve(this->numberPyramidLevels_);
    this->viewHeightPerLevel_->reserve(this->numberPyramidLevels_);
    this->viewWidthPerLevel_->reserve(this->numberPyramidLevels_);
    this->viewDepthPerLevel_->reserve(this->numberPyramidLevels_);
    physicalTileHeightPerLevel_->reserve(this->numberPyramidLevels_);
    physicalTileWidthPerLevel_->reserve(this->numberPyramidLevels_);
    physicalTileDepthPerLevel_->reserve(this->numberPyramidLevels_);
    numberLogicalTilesCachePerLevel_->reserve(this->numberPyramidLevels_);
    tileLoaderAllCaches->reserve(this->numberPyramidLevels_);
    allAdaptiveCaches->reserve(this->numberPyramidLevels_);

    if (!this->configurations_) {
      std::ostringstream oss;
      oss << "The configuration given to FastLoaderConfiguration is not valid.";
      throw (std::runtime_error(oss.str()));
    }

    // Getting the size for all levels
    for (size_t level = 0; level < this->numberPyramidLevels_; ++level) {
      this->fullHeightPerLevel_->push_back(this->tileLoader_->fullHeight(level));
      this->fullWidthPerLevel_->push_back(this->tileLoader_->fullWidth(level));
      this->fullDepthPerLevel_->push_back(this->tileLoader_->fullDepth(level));
      this->tileHeightPerLevel_->push_back(logicalTileHeightRequestedPerLevel.at(level));
      this->tileWidthPerLevel_->push_back(logicalTileWidthRequestedPerLevel.at(level));
      this->tileDepthPerLevel_->push_back(logicalTileDepthRequestedPerLevel.at(level));
      this->viewHeightPerLevel_->push_back(this->tileHeight(level) + 2 * this->radiusHeight());
      this->viewWidthPerLevel_->push_back(this->tileWidth(level) + 2 * this->radiusWidth());
      this->viewDepthPerLevel_->push_back(this->tileDepth(level) + 2 * this->radiusDepth());
      physicalTileHeightPerLevel_->push_back(this->tileLoader_->tileWidth(level));
      physicalTileWidthPerLevel_->push_back(this->tileLoader_->tileHeight(level));
      physicalTileDepthPerLevel_->push_back(this->tileLoader_->tileDepth(level));
      numberLogicalTilesCachePerLevel_->push_back(numberLogicalTilesCachePerLevel.at(level));

      sizeMemoryManagerPerLevel.push_back(
          this->numberChannels_ * this->viewHeightPerLevel_->at(level) *
              this->viewWidthPerLevel_->at(level) * this->viewDepthPerLevel_->at(level));

      tileLoaderAllCaches->push_back(
          std::make_shared<internal::Cache<DataType>>(
              this->configurations_->cacheCapacity_[level],
              (size_t) ceil((double) (this->fullHeight(level)) / physicalTileHeightPerLevel_->at(level)),
              (size_t) ceil((double) (this->fullWidth(level)) / physicalTileWidthPerLevel_->at(level)),
              (size_t) ceil((double) (this->fullDepth(level)) / physicalTileDepthPerLevel_->at(level)),
              physicalTileHeightPerLevel_->at(level),
              physicalTileWidthPerLevel_->at(level),
              physicalTileDepthPerLevel_->at(level),
              this->numberChannels_));

      allAdaptiveCaches->push_back(
          std::make_shared<internal::Cache<DataType>>(
              numberLogicalTilesCachePerLevel_->at(level),
              this->numberTileHeight(level),
              this->numberTileWidth(level),
              this->numberTileDepth(level),
              this->tileHeightPerLevel_->at(level),
              this->tileWidthPerLevel_->at(level),
              this->tileDepthPerLevel_->at(level),
              this->numberChannels_));
    }
    this->tileLoader_->allCaches_ = tileLoaderAllCaches;

    // Tasks
    auto viewCounter =
        std::make_shared<internal::ViewCounter<ViewType>>(
            this->configurations_->borderCreator_,
            this->configurations_->ordered_);
    // Internal graph
    this->levelGraph_ =
        std::make_shared<hh::Graph<internal::TileRequest<ViewType>, IndexRequest>>("Fast Loader Level");

    // Task & memory manager Memory Manager
    if constexpr(std::is_base_of<DefaultView<DataType>, ViewType>::value) {
      using ViewDataType = internal::DefaultViewData<DataType>;

      auto viewWaiter = std::make_shared<internal::ViewWaiter<ViewType, ViewDataType>>(
          viewCounter,
          this->configurations_->ordered_,
          this->fullHeightPerLevel_, this->fullWidthPerLevel_, this->fullDepthPerLevel_,
          this->tileHeightPerLevel_, this->tileWidthPerLevel_, this->tileDepthPerLevel_,
          this->numberChannels_,
          this->configurations_->radiusHeight_,
          this->configurations_->radiusWidth_,
          this->configurations_->radiusDepth_,
          this->configurations_->fillingType_
      );

      auto mm = std::make_shared<internal::FastLoaderMemoryManager<ViewDataType>>(
          this->configurations_->viewAvailablePerLevel_,
          sizeMemoryManagerPerLevel,
          this->configurations_->nbReleasePyramid_);

      auto viewLoader =
          std::make_shared<internal::ViewLoader<ViewType, ViewDataType>>(this->configurations_->borderCreator_);

      auto mapperLogicalPhysical = std::make_shared<internal::MapperLogicalPhysical<ViewType>>(
          physicalTileHeightPerLevel_, physicalTileWidthPerLevel_, physicalTileDepthPerLevel_,
          this->tileHeightPerLevel_, this->tileWidthPerLevel_, this->tileDepthPerLevel_,
          this->fullHeightPerLevel_, this->fullWidthPerLevel_, this->fullDepthPerLevel_,
          numberLogicalTilesCachePerLevel_,
          this->numberChannels_,
          allAdaptiveCaches
      );

      auto directToCopyStateManager = std::make_shared<
          hh::StateManager<
              fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>
          >
      >(std::make_shared<internal::DirectToCopyState<ViewType>>());

      auto copyLogicalTileToView = std::make_shared<fl::internal::CopyLogicalCacheToView<ViewType>>(
          numberThreadsCopyCacheView, this->numberChannels_);

      auto toTLStateManager = std::make_shared<
          hh::StateManager<fl::internal::TileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>>>
          (std::make_shared<fl::internal::ToTileLoaderState<ViewType>>());

      auto tlCounterStateManager = std::make_shared<
          hh::StateManager<fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::TileRequest<ViewType>>>
          (std::make_shared<fl::internal::TileLoaderCounterState<ViewType>>());

      viewWaiter->connectMemoryManager(mm);
      this->levelGraph_->input(viewWaiter);
      this->levelGraph_->addEdge(viewWaiter, viewLoader);

      this->levelGraph_->addEdge(viewLoader, mapperLogicalPhysical);
      // Direct Copy if cache found
      this->levelGraph_->addEdge(mapperLogicalPhysical, directToCopyStateManager);
      this->levelGraph_->addEdge(directToCopyStateManager, copyLogicalTileToView);


      //Route with tileLoader
      this->levelGraph_->addEdge(mapperLogicalPhysical, toTLStateManager);
      this->levelGraph_->addEdge(toTLStateManager, this->tileLoader_);
      this->levelGraph_->addEdge(this->tileLoader_, tlCounterStateManager);
      this->levelGraph_->addEdge(tlCounterStateManager, copyLogicalTileToView);

      // Output
      this->levelGraph_->output(copyLogicalTileToView);
    }
#ifdef HH_USE_CUDA
      else if constexpr (std::is_base_of<UnifiedView<typename ViewType::data_t>, ViewType>::value) {
        using ViewDataType = internal::UnifiedViewData<typename ViewType::data_t>;
              auto viewWaiter = std::make_shared<internal::ViewWaiter<ViewType, ViewDataType>>(
            viewCounter,
            this->configurations_->ordered_,
            this->fullHeightPerLevel_, this->fullWidthPerLevel_, this->fullDepthPerLevel_,
            this->tileHeightPerLevel_, this->tileWidthPerLevel_, this->tileDepthPerLevel_,
            this->numberChannels_,
            this->configurations_->radiusHeight_,
            this->configurations_->radiusWidth_,
            this->configurations_->radiusDepth_,
            this->configurations_->fillingType_
        );

        auto mm = std::make_shared<internal::FastLoaderMemoryManager<ViewDataType>>(
            this->configurations_->viewAvailablePerLevel_,
            sizeMemoryManagerPerLevel,
            this->configurations_->nbReleasePyramid_);

        auto viewLoader =
            std::make_shared<internal::ViewLoader<ViewType, ViewDataType>>(this->configurations_->borderCreator_);

        auto mapperLogicalPhysical = std::make_shared<internal::MapperLogicalPhysical<ViewType>>(
            physicalTileHeightPerLevel_, physicalTileWidthPerLevel_, physicalTileDepthPerLevel_,
            this->tileHeightPerLevel_, this->tileWidthPerLevel_, this->tileDepthPerLevel_,
            this->fullHeightPerLevel_, this->fullWidthPerLevel_, this->fullDepthPerLevel_,
            numberLogicalTilesCachePerLevel_,
            this->numberChannels_,
            allAdaptiveCaches
        );

        auto directToCopyStateManager = std::make_shared<
            hh::StateManager<
                fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>
            >
        >(std::make_shared<internal::DirectToCopyState<ViewType>>());

        auto copyLogicalTileToView = std::make_shared<fl::internal::CopyLogicalCacheToView<ViewType>>(
            numberThreadsCopyCacheView, this->numberChannels_);

        auto toTLStateManager = std::make_shared<
            hh::StateManager<fl::internal::TileRequest<ViewType>, fl::internal::AdaptiveTileRequest<ViewType>>>
            (std::make_shared<fl::internal::ToTileLoaderState<ViewType>>());

        auto tlCounterStateManager = std::make_shared<
            hh::StateManager<fl::internal::AdaptiveTileRequest<ViewType>, fl::internal::TileRequest<ViewType>>>
            (std::make_shared<fl::internal::TileLoaderCounterState<ViewType>>());

        viewWaiter->connectMemoryManager(mm);
        this->levelGraph_->input(viewWaiter);
        this->levelGraph_->addEdge(viewWaiter, viewLoader);

        this->levelGraph_->addEdge(viewLoader, mapperLogicalPhysical);
        // Direct Copy if cache found
        this->levelGraph_->addEdge(mapperLogicalPhysical, directToCopyStateManager);
        this->levelGraph_->addEdge(directToCopyStateManager, copyLogicalTileToView);


        //Route with tileLoader
        this->levelGraph_->addEdge(mapperLogicalPhysical, toTLStateManager);
        this->levelGraph_->addEdge(toTLStateManager, this->tileLoader_);
        this->levelGraph_->addEdge(this->tileLoader_, tlCounterStateManager);
        this->levelGraph_->addEdge(tlCounterStateManager, copyLogicalTileToView);

        // Output
        this->levelGraph_->output(copyLogicalTileToView);
      }
#endif // HH_USE_CUDA
    else {
      throw std::runtime_error("The View Data Type inside of the used View is not known to construct the graph.");
    }

    // Internal Execution pipeline
    auto levelExecutionPipeline =
        std::make_shared<internal::FastLoaderExecutionPipeline<ViewType>>(
            this->levelGraph_, this->tileLoader_->numberPyramidLevels());

    // Fast Loader graph
    this->input(levelExecutionPipeline);
    this->addEdge(levelExecutionPipeline, viewCounter);
    this->output(viewCounter);
  }

  /// @brief Default destructor
  virtual ~AdaptiveFastLoaderGraph() = default;
};

}

#endif //INC_FAST_LOADER_ADAPTIVE_FAST_LOADER_GRAPH_H
