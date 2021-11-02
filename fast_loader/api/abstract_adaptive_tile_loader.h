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
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/23/20.
//

#ifndef INC_3DFASTLOADER_ABSTRACT_ADAPTIVE_TILE_LOADER_H
#define INC_3DFASTLOADER_ABSTRACT_ADAPTIVE_TILE_LOADER_H

#include <utility>

#include "abstract_tile_loader.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief Adaptive Tile Loader abstraction
/// @details This TileLoader is an interface to access the file to load. It wont rely on the file layout to generate
/// the tiles served to the system. It proposes an interface to define a logical geometry for the tile. The
/// transformation from the physical to the logical geometry are made by the Adaptive Tile Loader, no action is
/// required. However it uses a cache for the loaded physical tile to avoid too much I/O, the size of the cache is
/// customizable.
/// In order to create a specialized TileLoader for a file type, the following methods need to be overloaded:
/// - AbstractAdaptiveTileLoader< ViewType >::loadPhysicalTileFromFile [mandatory] Main method to load a tile from a file,
/// - AbstractAdaptiveTileLoader< ViewType >::copyTileLoader [mandatory] Copy method to duplicate user defined attribute,
/// - AbstractAdaptiveTileLoader< ViewType >::fullHeight [mandatory] Image/File height,
/// - AbstractAdaptiveTileLoader< ViewType >::fullWidth [mandatory] Image/File width,
/// - AbstractAdaptiveTileLoader< ViewType >::fullDepth [optional] Image/File depth,
/// - AbstractAdaptiveTileLoader< ViewType >::physicalTileWidth [mandatory] Physical tile width,
/// - AbstractAdaptiveTileLoader< ViewType >::physicalTileHeight [mandatory] Physical tile height,
/// - AbstractAdaptiveTileLoader< ViewType >::physicalTileDepth [optional] Physical tile depth,
/// - AbstractAdaptiveTileLoader< ViewType >::bitsPerSample [mandatory] Size of a sample (element of the file),
/// - AbstractAdaptiveTileLoader< ViewType >::numberPyramidLevels [mandatory] Pyramid height (minimal 1),
/// - AbstractAdaptiveTileLoader< ViewType >::downScaleFactor [optional] Pyramid down scale factor for each level.
/// @tparam ViewType Type of the View
template<class ViewType>
class AbstractAdaptiveTileLoader : public AbstractTileLoader<ViewType> {
  using DataType = typename ViewType::data_t; ///< Type of data inside a View
 private:
  std::vector<std::shared_ptr<internal::Cache < DataType>>>
  physicalTileCache_{}; ///< Cache used to store physical tile

  std::vector<uint32_t>
      tileHeightRequestedPerLevel_{},   ///< Logical requested tile Height
      tileWidthRequestedPerLevel_{},    ///< Logical requested tile Width
      tileDepthRequestedPerLevel_{},    ///< Logical requested tile Depth
      numberTilesCachePerLevel_{};      ///< Number of tiles to cache per pyramidal level

 public:
  /// @brief Adaptive Tile loader constructor
  /// @param name Tile Loader name
  /// @param filePath File path
  /// @param tileHeightRequestedPerLevel Logical requested tile Height
  /// @param tileWidthRequestedPerLevel Logical requested tile Width
  /// @param tileDepthRequestedPerLevel Logical requested tile Depth
  /// @param numberTilesCachePerLevel Number of tiles to cache per pyramidal level
  AbstractAdaptiveTileLoader(std::string_view const &name,
                             std::string const &filePath,
                             std::vector<uint32_t> const &tileHeightRequestedPerLevel,
                             std::vector<uint32_t> const &tileWidthRequestedPerLevel,
                             std::vector<uint32_t> const &tileDepthRequestedPerLevel,
                             std::vector<uint32_t> const &numberTilesCachePerLevel = {})
      : AbstractTileLoader<ViewType>(name, filePath),
        tileHeightRequestedPerLevel_(tileHeightRequestedPerLevel),
        tileWidthRequestedPerLevel_(tileWidthRequestedPerLevel),
        tileDepthRequestedPerLevel_(tileDepthRequestedPerLevel),
        numberTilesCachePerLevel_(numberTilesCachePerLevel) {}

  /// @brief Adaptive Tile loader constructor
  /// @param name Tile Loader name
  /// @param numberThreads Number of threads associated to the tile loader
  /// @param filePath File path
  /// @param tileHeightRequestedPerLevel Logical requested tile Height
  /// @param tileWidthRequestedPerLevel Logical requested tile Width
  /// @param tileDepthRequestedPerLevel Logical requested tile Depth
  /// @param numberTilesCachePerLevel Number of tiles to cache per pyramidal level
  AbstractAdaptiveTileLoader(std::string_view const &name,
                             size_t numberThreads,
                             std::string const &filePath,
                             std::vector<uint32_t> const &tileHeightRequestedPerLevel,
                             std::vector<uint32_t> const &tileWidthRequestedPerLevel,
                             std::vector<uint32_t> const &tileDepthRequestedPerLevel,
                             std::vector<uint32_t> const &numberTilesCachePerLevel = {})
      : AbstractTileLoader<ViewType>(name, numberThreads, filePath),
        tileHeightRequestedPerLevel_(tileHeightRequestedPerLevel),
        tileWidthRequestedPerLevel_(tileWidthRequestedPerLevel),
        tileDepthRequestedPerLevel_(tileDepthRequestedPerLevel),
        numberTilesCachePerLevel_(numberTilesCachePerLevel) {}

  /// @brief Default adaptive tile loader
  virtual ~AbstractAdaptiveTileLoader() = default;

  /// @brief Initialize an adaptive tile loader (final specialization)
  void initializeTileLoader() final { validateAndInitialize(); }

  /// @brief Logical requested tile height for all levels
  /// @return Vector containing all logical requested tile height
  [[nodiscard]] std::vector<uint32_t> const &tileHeightRequestedPerLevel() const {
    return tileHeightRequestedPerLevel_;
  }
  /// @brief Logical requested tile width for all levels
  /// @return Vector containing all logical requested tile width
  [[nodiscard]] std::vector<uint32_t> const &tileWidthRequestedPerLevel() const {
    return tileWidthRequestedPerLevel_;
  }
  /// @brief Logical requested tile depth for all levels
  /// @return Vector containing all logical requested tile depth
  [[nodiscard]] std::vector<uint32_t> const &tileDepthRequestedPerLevel() const {
    return tileDepthRequestedPerLevel_;
  }

  /// @brief Number of physical tiles to cache for all levels
  /// @return Vector containing all number of physical tiles to cache for all levels
  [[nodiscard]] std::vector<uint32_t> const &numberTilesCachePerLevel() const {
    return numberTilesCachePerLevel_;
  }

  /// @brief Physical tile width accessor for a level
  /// @param level Pyramidal level
  /// @return Physical tile width for a level
  [[nodiscard]] virtual uint32_t physicalTileWidth(uint32_t level) const = 0;

  /// @brief Physical tile height accessor for a level
  /// @param level Pyramidal level
  /// @return Physical tile height for a level
  [[nodiscard]] virtual uint32_t physicalTileHeight(uint32_t level) const = 0;

  /// @brief Physical tile depth accessor for a level
  /// @param level Pyramidal level
  /// @return Physical tile depth for a level
  [[nodiscard]] virtual uint32_t physicalTileDepth(uint32_t level) const = 0;

  /// @brief Load a physical tile from a file
  /// @param physicalTile Piece of memory to fill
  /// @param indexFileRowGlobalTile Tile file row index requested
  /// @param indexFileColGlobalTile Tile file column index requested
  /// @param indexFileLayerGlobalTile Tile file depth index requested
  /// @param fileLevel Pyramidal level
  virtual void loadPhysicalTileFromFile(
      std::shared_ptr<std::vector<DataType>> physicalTile,
      uint32_t indexFileRowGlobalTile,
      uint32_t indexFileColGlobalTile,
      uint32_t indexFileLayerGlobalTile,
      uint32_t fileLevel
  ) = 0;

  /// @brief From a logical tile request, request n physical tiles, copy needed data
  /// @param logicalTile Logical tile to fill
  /// @param indexRowGlobalTile Logical index row to fill
  /// @param indexColGlobalTile Logical index column to fill
  /// @param indexLayerGlobalTile Logical index layer to fill
  /// @param level Logical pyramidal level
  void loadTileFromFile(std::shared_ptr<std::vector<DataType>> logicalTile,
                        uint32_t indexRowGlobalTile,
                        uint32_t indexColGlobalTile,
                        uint32_t indexLayerGlobalTile,
                        uint32_t level) final {
    uint32_t const
        logicalTileWidth = tileWidth(level),
        logicalTileHeight = tileHeight(level),
        logicalTileDepth = tileDepth(level),
        physicalTileWidth = this->physicalTileWidth(level),
        physicalTileHeight = this->physicalTileHeight(level),
        physicalTileDepth = this->physicalTileDepth(level),
        minRow = indexRowGlobalTile * logicalTileHeight,
        minColumn = indexColGlobalTile * logicalTileWidth,
        minLayer = indexLayerGlobalTile * logicalTileDepth,
        maxRow = std::min((indexRowGlobalTile + 1) * logicalTileHeight, this->fullHeight(level)),
        maxColumn = std::min((indexColGlobalTile + 1) * logicalTileWidth, this->fullWidth(level)),
        maxLayer = std::min((indexLayerGlobalTile + 1) * logicalTileDepth, this->fullDepth(level)),

        numberPhysicalTilesRow = (uint32_t) ceil((double) (this->fullHeight(level)) / physicalTileHeight),
        numberPhysicalTilesCol = (uint32_t) ceil((double) (this->fullWidth(level)) / physicalTileWidth),
        numberPhysicalTilesLayer = (uint32_t) ceil((double) (this->fullDepth(level)) / physicalTileDepth),

        indexMinPhysicalRow = minRow / physicalTileHeight,
        indexMinPhysicalColumn = minColumn / physicalTileWidth,
        indexMinPhysicalLayer = minLayer / physicalTileDepth,
        indexMaxPhysicalRow = std::min((maxRow / physicalTileHeight) + 1, numberPhysicalTilesRow),
        indexMaxPhysicalColumn = std::min((maxColumn / physicalTileWidth) + 1, numberPhysicalTilesCol),
        indexMaxPhysicalLayer = std::min((maxLayer / physicalTileDepth) + 1, numberPhysicalTilesLayer);

    uint32_t
        startRowCopy = 0,
        startColCopy = 0,
        startLayerCopy = 0,
        endRowCopy = 0,
        endColCopy = 0,
        endLayerCopy = 0,
        deltaPhysicalLayer = 0,
        deltaLogicalLayer = 0,
        deltaPhysicalRow = 0,
        deltaLogicalRow = 0;

    for (size_t indexLayer = indexMinPhysicalLayer; indexLayer < indexMaxPhysicalLayer; ++indexLayer) {
      startLayerCopy = std::max(uint32_t(indexLayer * physicalTileDepth), minLayer);
      endLayerCopy = std::min(uint32_t((indexLayer + 1) * physicalTileDepth), maxLayer);

      for (uint32_t layerCopy = startLayerCopy; layerCopy < endLayerCopy; ++layerCopy) {
        deltaPhysicalLayer =
            (layerCopy - (indexLayer * physicalTileDepth)) * physicalTileWidth * physicalTileHeight;
        deltaLogicalLayer =
            (layerCopy - ((layerCopy / logicalTileDepth) * logicalTileDepth)) * logicalTileWidth * logicalTileHeight;

        for (size_t indexRow = indexMinPhysicalRow; indexRow < indexMaxPhysicalRow; ++indexRow) {
          startRowCopy = std::max(uint32_t(indexRow * physicalTileHeight), minRow);
          endRowCopy = std::min(uint32_t((indexRow + 1) * physicalTileHeight), maxRow);

          for (uint32_t rowCopy = startRowCopy; rowCopy < endRowCopy; ++rowCopy) {
            deltaPhysicalRow =
                (rowCopy - (indexRow * physicalTileHeight)) * physicalTileWidth;
            deltaLogicalRow =
                (rowCopy - ((rowCopy / logicalTileHeight) * logicalTileHeight)) * logicalTileWidth;

            for (size_t indexColumn = indexMinPhysicalColumn; indexColumn < indexMaxPhysicalColumn; ++indexColumn) {
              startColCopy = std::max(uint32_t(indexColumn * physicalTileWidth), minColumn);
              endColCopy = std::min(uint32_t((indexColumn + 1) * physicalTileWidth), maxColumn);

              auto cachedTile = physicalTileCache_.at(level)->lockedTile(indexRow, indexColumn, indexLayer);
              auto physicalTile = cachedTile->data();

              if (cachedTile->isNewTile()) {
                cachedTile->newTile(false);
                loadPhysicalTileFromFile(physicalTile, indexRow, indexColumn, indexLayer, level);
              }

              std::copy_n(
                  physicalTile->begin()
                      + (deltaPhysicalLayer
                      + deltaPhysicalRow
                      + startColCopy - (startColCopy / physicalTileWidth) * physicalTileWidth) * this->numberChannels(),
                  (endColCopy - startColCopy) * this->numberChannels(),
                  logicalTile->begin()
                      + (deltaLogicalLayer
                      + deltaLogicalRow
                      + startColCopy - (startColCopy / logicalTileWidth) * logicalTileWidth) * this->numberChannels()
              );

              cachedTile->unlock();
            }
          }

        }
      }
    }
  }

  /// @brief Logical tile width accessor
  /// @param level Pyramidal level
  /// @return Logical tile width
  [[nodiscard]] uint32_t tileWidth(uint32_t level) const final {
    return tileWidthRequestedPerLevel_.at(level);
  }
  /// @brief Logical tile height accessor
  /// @param level Pyramidal level
  /// @return Logical tile height
  [[nodiscard]] uint32_t tileHeight(uint32_t level) const final {
    return tileHeightRequestedPerLevel_.at(level);
  }
  /// @brief Logical tile depth accessor
  /// @param level Pyramidal level
  /// @return Logical tile depth
  [[nodiscard]] uint32_t tileDepth(uint32_t level) const final {
    return tileDepthRequestedPerLevel_.at(level);
  }

 private:
  /// @brief Validate an adaptive tile loader
  /// @throw std::runtime_error if the adaptive tile loader is not valid
  void validateAndInitialize() {
    if (tileHeightRequestedPerLevel_.size() != this->numberPyramidLevels()
        || tileWidthRequestedPerLevel_.size() != this->numberPyramidLevels()
        || tileDepthRequestedPerLevel_.size() != this->numberPyramidLevels()
        ) {
      throw (std::runtime_error(
          "The tile size requested should have information for all levels of the file."));
    }
    auto validateHeight = std::all_of(
        tileHeightRequestedPerLevel_.cbegin(), tileHeightRequestedPerLevel_.cend(),
        [](uint32_t const &height) { return height > 0; });
    auto validateWidth = std::all_of(
        tileWidthRequestedPerLevel_.cbegin(), tileWidthRequestedPerLevel_.cend(),
        [](uint32_t const &width) { return width > 0; });
    auto validateDepth = std::all_of(
        tileDepthRequestedPerLevel_.cbegin(), tileDepthRequestedPerLevel_.cend(),
        [](uint32_t const &depth) { return depth > 0; });

    for (uint32_t level = 0; level < this->numberPyramidLevels(); ++level) {
      validateHeight &= tileHeightRequestedPerLevel_.at(level) <= this->fullHeight(level);
      validateWidth &= tileWidthRequestedPerLevel_.at(level) <= this->fullWidth(level);
      validateHeight &= tileDepthRequestedPerLevel_.at(level) <= this->fullDepth(level);
    }

    if (!(validateHeight && validateWidth && validateDepth)) {
      throw (std::runtime_error(
          "The tile size requested should be > 0 and <= file size."));
    }

    if (numberTilesCachePerLevel_.empty()) {
      numberTilesCachePerLevel_ = std::vector<uint32_t>(this->numberPyramidLevels(), 1);
      for (uint32_t level = 0; level < this->numberPyramidLevels(); ++level) {
        this->physicalTileCache_.push_back(
            std::make_shared<internal::Cache<DataType>>(
                1,
                (uint32_t) ceil((double) (this->fullHeight(level)) / physicalTileHeight(level)),
                (uint32_t) ceil((double) (this->fullWidth(level)) / physicalTileWidth(level)),
                (uint32_t) ceil((double) (this->fullDepth(level)) / physicalTileDepth(level)),
                physicalTileHeight(level),
                physicalTileWidth(level),
                physicalTileDepth(level),
                this->numberChannels()
            )
        );
      }
    } else {
      auto validateCache = std::all_of(
          numberTilesCachePerLevel_.cbegin(), numberTilesCachePerLevel_.cend(),
          [](uint32_t const &numberTilesCache) { return numberTilesCache > 0; });
      if (validateCache) {
        std::copy_n(
            numberTilesCachePerLevel_.cbegin(),
            this->numberPyramidLevels(),
            std::back_inserter(numberTilesCachePerLevel_));
        for (uint32_t level = 0; level < this->numberPyramidLevels(); ++level) {
          this->physicalTileCache_.push_back(
              std::make_shared<internal::Cache<DataType>>(
                  numberTilesCachePerLevel_.at(level),
                  (uint32_t) ceil((double) (this->fullHeight(level)) / physicalTileHeight(level)),
                  (uint32_t) ceil((double) (this->fullWidth(level)) / physicalTileWidth(level)),
                  (uint32_t) ceil((double) (this->fullDepth(level)) / physicalTileDepth(level)),
                  physicalTileHeight(level),
                  physicalTileWidth(level),
                  physicalTileDepth(level),
                  this->numberChannels()
              )
          );
        }
      } else {
        throw (std::runtime_error("The number of tiles to cache for an adaptive tile loader should be greater than 0."));
      }
    }
  }
};
}
#endif //INC_3DFASTLOADER_ABSTRACT_ADAPTIVE_TILE_LOADER_H
