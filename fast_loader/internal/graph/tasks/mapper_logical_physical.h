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

  std::shared_ptr<std::vector<size_t>> const
      physicalTileHeights_ = std::make_shared<std::vector<size_t>>(),
      physicalTileWidths_ = std::make_shared<std::vector<size_t>>(),
      physicalTileDepths_ = std::make_shared<std::vector<size_t>>(),
      logicalTileHeights_ = std::make_shared<std::vector<size_t>>(),
      logicalTileWidths_ = std::make_shared<std::vector<size_t>>(),
      logicalTileDepths_ = std::make_shared<std::vector<size_t>>(),
      fullHeights_{},
      fullWidths_{},
      fullDepths_{},
      numberLogicalTilesCache_ = std::make_shared<std::vector<size_t>>();

  size_t const
      numberChannels_{};

  size_t
      maxNumberLogicalTilesRow_ = 0,
      maxNumberLogicalTilesCol_ = 0,
      maxNumberLogicalTilesLayer_ = 0;

  std::shared_ptr<std::vector<std::shared_ptr<internal::Cache<typename ViewType::data_t>>>>
      logicalTileCaches_;

  std::shared_ptr<internal::Cache<DataType>>
      cache_ = {}; ///< Tile Cache

  size_t
      numberElementDirectToCopy_ = 0,
      numberElementsToTL_ = 0;

 public:
  MapperLogicalPhysical(std::shared_ptr<std::vector<size_t>> physicalTileHeights,
                        std::shared_ptr<std::vector<size_t>> physicalTileWidths,
                        std::shared_ptr<std::vector<size_t>> physicalTileDepths,
                        std::shared_ptr<std::vector<size_t>> logicalTileHeights,
                        std::shared_ptr<std::vector<size_t>> logicalTileWidths,
                        std::shared_ptr<std::vector<size_t>> logicalTileDepths,
                        std::shared_ptr<std::vector<size_t>> fullHeights,
                        std::shared_ptr<std::vector<size_t>> fullWidths,
                        std::shared_ptr<std::vector<size_t>> fullDepths,
                        std::shared_ptr<std::vector<size_t>> numberLogicalTilesCaches,
                        size_t const numberChannels,
                        std::shared_ptr<std::vector<std::shared_ptr<internal::Cache<DataType>>>> logicalTileCaches)
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
        logicalTileCaches_(std::move(logicalTileCaches)) {
    for (size_t level = 0; level < fullWidths_->size(); ++level) {
      maxNumberLogicalTilesRow_ =
          std::max(
              maxNumberLogicalTilesRow_,
              (size_t) ceil((double) (fullHeights_->at(level)) / logicalTileHeights_->at(level))
          );
      maxNumberLogicalTilesCol_ =
          std::max(
              maxNumberLogicalTilesCol_,
              (size_t) ceil((double) (fullWidths_->at(level)) / logicalTileWidths_->at(level))
          );
      maxNumberLogicalTilesLayer_ =
          std::max(
              maxNumberLogicalTilesLayer_,
              (size_t) ceil((double) (fullDepths_->at(level)) / logicalTileDepths_->at(level))
          );
    }

  }

  virtual ~MapperLogicalPhysical() = default;

  void initialize() override { cache_ = this->logicalTileCaches_->at(this->graphId()); }

  void execute(std::shared_ptr<TileRequest<ViewType>> tileRequest) override {
    size_t const
        level = tileRequest->view()->level(),
        indexRowLogicalTile = tileRequest->indexRowTileAsked(),
        indexColLogicalTile = tileRequest->indexColTileAsked(),
        indexLayerLogicalTile = tileRequest->indexLayerTileAsked();
    std::shared_ptr<fl::internal::CachedTile<DataType>> logicalCachedTile =
        this->cache_->lockedTile(indexRowLogicalTile, indexColLogicalTile, indexLayerLogicalTile);

    if (!logicalCachedTile->isNewTile()) {
      ++this->numberElementDirectToCopy_;
      this->addResult(std::make_shared<fl::internal::AdaptiveTileRequest<ViewType>>(tileRequest, logicalCachedTile));
    } else {
      logicalCachedTile->newTile(false);
      //Need to create N AdaptiveTileRequest to fill the logical Cache Tile from tile loader
      std::vector<std::shared_ptr<fl::internal::AdaptiveTileRequest<ViewType>>> adaptiveTileRequests{};

      size_t const
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

          numberPhysicalTilesRow = (size_t) ceil((double) (fullHeights_->at(level)) / physicalTileHeight),
          numberPhysicalTilesCol = (size_t) ceil((double) (fullWidths_->at(level)) / physicalTileWidth),
          numberPhysicalTilesLayer = (size_t) ceil((double) (fullDepths_->at(level)) / physicalTileDepth),

          numberLogicalTilesRow = (size_t) ceil((double) (fullHeights_->at(level)) / logicalTileHeight),
          numberLogicalTilesCol = (size_t) ceil((double) (fullWidths_->at(level)) / logicalTileWidth),
          numberLogicalTilesLayer = (size_t) ceil((double) (fullDepths_->at(level)) / logicalTileDepth),

          indexMinPhysicalRow = minRow / physicalTileHeight,
          indexMinPhysicalColumn = minColumn / physicalTileWidth,
          indexMinPhysicalLayer = minLayer / physicalTileDepth,

          indexMaxPhysicalRow = std::min((maxRow / physicalTileHeight) + 1, numberPhysicalTilesRow),
          indexMaxPhysicalColumn = std::min((maxColumn / physicalTileWidth) + 1, numberPhysicalTilesCol),
          indexMaxPhysicalLayer = std::min((maxLayer / physicalTileDepth) + 1, numberPhysicalTilesLayer),
          id = indexLayerLogicalTile * maxNumberLogicalTilesRow_ * maxNumberLogicalTilesCol_
          + indexRowLogicalTile * maxNumberLogicalTilesCol_
          + indexColLogicalTile;

      size_t
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
        startLayerCopy = std::max(size_t(indexLayer * physicalTileDepth), minLayer);
        endLayerCopy = std::min(size_t((indexLayer + 1) * physicalTileDepth), maxLayer);
        depthToCopy = endLayerCopy - startLayerCopy;
        deltaLogicalRow = 0;

        for (size_t indexRow = indexMinPhysicalRow; indexRow < indexMaxPhysicalRow; ++indexRow) {
          startRowCopy = std::max(size_t(indexRow * physicalTileHeight), minRow);
          endRowCopy = std::min(size_t((indexRow + 1) * physicalTileHeight), maxRow);
          heightToCopy = endRowCopy - startRowCopy;
          deltaLogicalColumn = 0;
          for (size_t indexColumn = indexMinPhysicalColumn; indexColumn < indexMaxPhysicalColumn; ++indexColumn) {
            startColCopy = std::max(size_t(indexColumn * physicalTileWidth), minColumn);
            endColCopy = std::min(size_t((indexColumn + 1) * physicalTileWidth), maxColumn);
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
                        (size_t) (startRowCopy - indexRow * physicalTileHeight),
                        (size_t) (startColCopy - indexColumn * physicalTileWidth),
                        (size_t) (startLayerCopy - indexLayer * physicalTileDepth),
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

      for (auto tr: adaptiveTileRequests) {
        ++this->numberElementsToTL_;
        tr->numberPhysicalTileRequests(numberOfAdaptiveTileRequests);
        this->addResult(tr);

      }
    }

  }

  [[nodiscard]] std::string extraPrintingInformation() const override {
    std::ostringstream oss;
    oss << "Miss rate: "
        << std::setprecision(2) << this->cache_->miss() * 1. / (this->cache_->miss() + this->cache_->hit()) << "%"
        << "\n" << this->numberElementDirectToCopy_ << "/" << this->numberElementsToTL_ << "\n";
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
