//
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/20/21.
//

#ifndef INC_3DFASTLOADER_MAPPER_LOGICAL_PHYSICAL_H
#define INC_3DFASTLOADER_MAPPER_LOGICAL_PHYSICAL_H

#include <hedgehog/hedgehog.h>

#include <utility>
#include "../../data/adaptive_tile_request.h"
#include "../../data/view/adaptive_view.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {

template<class ViewType>
class MapperLogicalPhysical : public hh::AbstractTask<AdaptiveTileRequest<ViewType>, TileRequest<ViewType>> {
 private:
  using DataType = typename ViewType::data_t;

  std::shared_ptr<std::vector<uint32_t>> const
      physicalTileHeights_ = std::make_shared<std::vector<uint32_t>>(),
      physicalTileWidths_ = std::make_shared<std::vector<uint32_t>>(),
      physicalTileDepths_ = std::make_shared<std::vector<uint32_t>>(),
      logicalTileHeights_ = std::make_shared<std::vector<uint32_t>>(),
      logicalTileWidths_ = std::make_shared<std::vector<uint32_t>>(),
      logicalTileDepths_ = std::make_shared<std::vector<uint32_t>>(),
      fullHeights_{},
      fullWidths_{},
      fullDepths_{},
      numberLogicalTilesCache_ = std::make_shared<std::vector<uint32_t>>();

  uint32_t const
      numberChannels_{};

  std::shared_ptr<std::vector<std::shared_ptr<internal::Cache<typename ViewType::data_t>>>>
      logicalTileCaches_;

  std::shared_ptr<internal::Cache<DataType>>
      cache_ = {}; ///< Tile Cache

  size_t
      numberElementDirectToCopy_ = 0,
      numberElementsToTL_ = 0;

 public:
  MapperLogicalPhysical(std::shared_ptr<std::vector<uint32_t>> physicalTileHeights,
                        std::shared_ptr<std::vector<uint32_t>> physicalTileWidths,
                        std::shared_ptr<std::vector<uint32_t>> physicalTileDepths,
                        std::shared_ptr<std::vector<uint32_t>> logicalTileHeights,
                        std::shared_ptr<std::vector<uint32_t>> logicalTileWidths,
                        std::shared_ptr<std::vector<uint32_t>> logicalTileDepths,
                        std::shared_ptr<std::vector<uint32_t>> fullHeights,
                        std::shared_ptr<std::vector<uint32_t>> fullWidths,
                        std::shared_ptr<std::vector<uint32_t>> fullDepths,
                        std::shared_ptr<std::vector<uint32_t>> numberLogicalTilesCaches,
                        uint32_t const numberChannels,
                        std::shared_ptr<std::vector<std::shared_ptr<internal::Cache<DataType>>>>
                        logicalTileCaches)
      : hh::AbstractTask<AdaptiveTileRequest<ViewType>, TileRequest<ViewType>>("LogicalToPhysicalMapper"),
        physicalTileHeights_(std::move(physicalTileHeights)),
        physicalTileWidths_(std::move(physicalTileWidths)),
        physicalTileDepths_(std::move(physicalTileDepths)),
        logicalTileHeights_(std::move(logicalTileHeights)),
        logicalTileWidths_(std::move(logicalTileWidths)),
        logicalTileDepths_(std::move(logicalTileDepths)),
        fullHeights_(std::move(fullHeights)), fullWidths_(std::move(fullWidths)), fullDepths_(std::move(fullDepths)),
        numberLogicalTilesCache_(std::move(numberLogicalTilesCaches)),
        numberChannels_(numberChannels),
        logicalTileCaches_(std::move(logicalTileCaches)) {}

  virtual ~MapperLogicalPhysical() = default;

  void initialize() override {cache_ = this->logicalTileCaches_->at(this->graphId());}

  void execute(std::shared_ptr<TileRequest<ViewType>> tileRequest) override {

    uint32_t const
        level = tileRequest->view()->level(),
        indexRowLogicalTile = tileRequest->indexRowTileAsked(),
        indexColLogicalTile = tileRequest->indexColTileAsked(),
        indexLayerLogicalTile = tileRequest->indexLayerTileAsked();
    
//    global_mutex.lock();
//    std::cout << "Try to get: (" << indexRowLogicalTile << ", " << indexColLogicalTile << ", " << indexLayerLogicalTile << ")" << std::endl;
//    global_mutex.unlock();

    std::shared_ptr<fl::internal::CachedTile<DataType>> logicalCachedTile =
        this->cache_->lockedTile(indexRowLogicalTile, indexColLogicalTile, indexLayerLogicalTile);
    
//    global_mutex.lock();
//    std::cout << "Got: (" << indexRowLogicalTile << ", " << indexColLogicalTile << ", " << indexLayerLogicalTile << ")" << std::endl;
//    global_mutex.unlock();

    if (!logicalCachedTile->isNewTile()) {
//      global_mutex.lock();
//      std::cout << "Reuse" << std::endl;
//      global_mutex.unlock();
      ++ this->numberElementDirectToCopy_;
      this->addResult(std::make_shared<fl::internal::AdaptiveTileRequest<ViewType>>(tileRequest, logicalCachedTile));
    } else {
//      global_mutex.lock();
//      std::cout << "New Tile" << std::endl;
//      global_mutex.unlock();
      logicalCachedTile->newTile(false);
      //Need to create N AdaptiveTileRequest to fill the logical Cache Tile from tile loader
      std::vector<std::shared_ptr<fl::internal::AdaptiveTileRequest<ViewType>>> adaptiveTileRequests{};

      uint32_t const
          logicalTileHeight = logicalTileHeights_->at(level),
          logicalTileWidth = logicalTileWidths_->at(level),
          logicalTileDepth = logicalTileDepths_->at(level),

          physicalTileHeight = physicalTileHeights_->at(level),
          physicalTileWidth = physicalTileWidths_->at(level),
          physicalTileDepth = physicalTileDepths_->at(level),

          minRow = indexRowLogicalTile * logicalTileHeight,
          minColumn = indexColLogicalTile * logicalTileWidth,
          minLayer = indexLayerLogicalTile * logicalTileDepth,
          maxRow = std::min((indexRowLogicalTile + 1) * logicalTileHeight, fullHeights_->at(level)),
          maxColumn = std::min((indexColLogicalTile + 1) * logicalTileWidth, fullWidths_->at(level)),
          maxLayer = std::min((indexLayerLogicalTile + 1) * logicalTileDepth, fullDepths_->at(level)),

          numberPhysicalTilesRow = (uint32_t) ceil((double) (fullHeights_->at(level)) / physicalTileHeight),
          numberPhysicalTilesCol = (uint32_t) ceil((double) (fullWidths_->at(level)) / physicalTileWidth),
          numberPhysicalTilesLayer = (uint32_t) ceil((double) (fullDepths_->at(level)) / physicalTileDepth),

          numberLogicalTilesRow = (uint32_t) ceil((double) (fullHeights_->at(level)) / logicalTileHeight),
          numberLogicalTilesCol = (uint32_t) ceil((double) (fullWidths_->at(level)) / logicalTileWidth),
          numberLogicalTilesLayer = (uint32_t) ceil((double) (fullDepths_->at(level)) / logicalTileDepth),

          indexMinPhysicalRow = minRow / physicalTileHeight,
          indexMinPhysicalColumn = minColumn / physicalTileWidth,
          indexMinPhysicalLayer = minLayer / physicalTileDepth,

          indexMaxPhysicalRow = std::min((maxRow / physicalTileHeight) + 1, numberPhysicalTilesRow),
          indexMaxPhysicalColumn = std::min((maxColumn / physicalTileWidth) + 1, numberPhysicalTilesCol),
          indexMaxPhysicalLayer = std::min((maxLayer / physicalTileDepth) + 1, numberPhysicalTilesLayer),
          // TODO: Check if ID works also with multiple levels ! 
          id =
      indexLayerLogicalTile * numberLogicalTilesLayer * numberLogicalTilesRow * numberLogicalTilesCol
          + indexRowLogicalTile * numberLogicalTilesRow
          + indexColLogicalTile * numberLogicalTilesCol * numberLogicalTilesRow;

      uint32_t
          startRowCopy = 0,
          startColCopy = 0,
          startLayerCopy = 0,
          endRowCopy = 0,
          endColCopy = 0,
          endLayerCopy = 0,
          deltaLogicalRow = 0,
          deltaLogicalColumn = 0,
          deltaLogicalLayer = 0,
          depthToCopy = 0,
          heightToCopy = 0,
          widthToCopy = 0;

      for (size_t indexLayer = indexMinPhysicalLayer; indexLayer < indexMaxPhysicalLayer; ++indexLayer) {
        startLayerCopy = std::max(uint32_t(indexLayer * physicalTileDepth), minLayer);
        endLayerCopy = std::min(uint32_t((indexLayer + 1) * physicalTileDepth), maxLayer);
        depthToCopy = endLayerCopy - startLayerCopy;
        deltaLogicalRow = 0;

        for (size_t indexRow = indexMinPhysicalRow; indexRow < indexMaxPhysicalRow; ++indexRow) {
          startRowCopy = std::max(uint32_t(indexRow * physicalTileHeight), minRow);
          endRowCopy = std::min(uint32_t((indexRow + 1) * physicalTileHeight), maxRow);
          heightToCopy = endRowCopy - startRowCopy;
          deltaLogicalColumn = 0;
          for (size_t indexColumn = indexMinPhysicalColumn; indexColumn < indexMaxPhysicalColumn; ++indexColumn) {
            startColCopy = std::max(uint32_t(indexColumn * physicalTileWidth), minColumn);
            endColCopy = std::min(uint32_t((indexColumn + 1) * physicalTileWidth), maxColumn);
            widthToCopy = endColCopy - startColCopy;


            // Create a view data that uses the cached logical data as internal data
            auto adaptiveViewData = std::make_shared<AdaptiveViewData<DataType>>
                (logicalCachedTile->data()->data());
            // Describe the view data
            adaptiveViewData->initialize(
                fullHeights_->at(level), fullWidths_->at(level), fullDepths_->at(level),
                logicalTileHeight, logicalTileWidth, logicalTileDepth,
                numberChannels_,
                0, 0, 0, // Request only the central tile --> radius = 0
                level,
                indexRowLogicalTile, indexColLogicalTile, indexLayerLogicalTile,
                numberLogicalTilesRow, numberLogicalTilesCol, numberLogicalTilesLayer,
                FillingType::REPLICATE
            );

            // Create the view from the view data
            auto adaptiveView = std::make_shared<AdaptiveView<ViewType>>(adaptiveViewData);

            // Create an AdaptiveTileRequest with this view
            auto adaptiveTileRequest = std::make_shared<AdaptiveTileRequest<ViewType>>(
                indexRow, indexColumn, indexLayer, adaptiveView,
                tileRequest, logicalCachedTile);

            adaptiveTileRequest->addCopy(
                CopyVolume(
                    {
                        (uint32_t) (startRowCopy - indexRow * physicalTileHeight),
                        (uint32_t) (startColCopy - indexColumn * physicalTileWidth),
                        (uint32_t) (startLayerCopy - indexLayer * physicalTileDepth),
                        heightToCopy, widthToCopy, depthToCopy},
                    {
                        deltaLogicalRow, deltaLogicalColumn, deltaLogicalLayer,
                        heightToCopy, widthToCopy, depthToCopy
                    }
                )
            );
            adaptiveTileRequest->id(id);

            adaptiveTileRequests.push_back(adaptiveTileRequest);
            deltaLogicalColumn += widthToCopy;
          }
          deltaLogicalRow += heightToCopy;
        }
        deltaLogicalLayer += depthToCopy;
      }
      
      
      size_t const numberOfAdaptiveTileRequests = adaptiveTileRequests.size();
      
//      global_mutex.lock();
//      std::cout << "Tile (" << indexRowLogicalTile << ", " << indexColLogicalTile << ", " << indexLayerLogicalTile << ") #id" << id << " has " << numberOfAdaptiveTileRequests << " parts" << std::endl;
//      global_mutex.unlock();
      
      for (auto tr : adaptiveTileRequests){
//        global_mutex.lock();
//        std::cout << "Sending Tile (" << indexRowLogicalTile << ", " << indexColLogicalTile << ", " << indexLayerLogicalTile << ") #id" << id << std::endl;
//        global_mutex.unlock();
        ++ this->numberElementsToTL_;
        tr->numberPhysicalTileRequests(numberOfAdaptiveTileRequests);
        this->addResult(tr);
        
      }
      
      
//      std::for_each(adaptiveTileRequests.begin(), adaptiveTileRequests.end(),
//                    [&numberOfAdaptiveTileRequests, this]
//                        (std::shared_ptr<fl::internal::AdaptiveTileRequest<ViewType>> tr) {
//                      ++ this->numberElementsToTL_;
//                      tr->numberPhysicalTileRequests(numberOfAdaptiveTileRequests);
//                      this->addResult(tr);
//                    });

    }

  }

  [[nodiscard]] std::string extraPrintingInformation() const override {
//    size_t const
//      toCopy =  this->numberElementDirectToCopy_,
//      toTL = this->numberElementsToTL_,
//      total = toCopy + toTL;
//
//    std::ostringstream oss;
//    oss
//      << "#elements direct to copy: " << toCopy << "(" << (double) toCopy / total * 100 << "%)\n"
//      << "#elements to TileLoader: " << toTL << "(" << (double) toTL / total * 100 << "%)\n";
//    return oss.str();

    std::ostringstream oss;
    oss << "Miss rate: "
        << std::setprecision(2) << this->cache_->miss() * 1. / (this->cache_->miss() + this->cache_->hit()) << "%"
        << "\n" << this->numberElementDirectToCopy_  << "/" << this->numberElementsToTL_ << "\n";

//        << "Disk Access time: " << durationPrinter(cache_->accessTime()) << std::endl
//        << "Disk Recycle time: " << durationPrinter(cache_->recycleTime()) << std::endl;
    return oss.str();
  }

  std::shared_ptr<hh::AbstractTask<AdaptiveTileRequest<ViewType>, TileRequest<ViewType>>> copy() override {
    return std::make_shared<MapperLogicalPhysical<ViewType>>(
        this->physicalTileHeights_, this->physicalTileWidths_, this->physicalTileDepths_,
        this->logicalTileHeights_, this->logicalTileWidths_, this->logicalTileDepths_,
        this->fullHeights_, this->fullWidths_, this->fullDepths_,
        this->numberLogicalTilesCache_, this->numberChannels_, this->logicalTileCaches_);
  }

};
}
}

#endif //INC_3DFASTLOADER_MAPPER_LOGICAL_PHYSICAL_H
