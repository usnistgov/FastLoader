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

#ifndef FASTLOADER_ABSTRACT_TILE_LOADER_H
#define FASTLOADER_ABSTRACT_TILE_LOADER_H

#include <hedgehog/hedgehog.h>
#include <utility>
#include "../internal/data/tile_request.h"
#include "../internal/cache.h"

/// @brief FastLoader namespace
namespace fl {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/// @brief FastLoaderGraph Forward declaration
/// @tparam ViewType AbstractView Type
template<class ViewType>
class FastLoaderGraph;

template<class ViewType>
class AdaptiveFastLoaderGraph;
#endif //DOXYGEN_SHOULD_SKIP_THIS

/// @brief TileLoader interface
/// @details A TileLoader is the main interface to access the file to load. A view is a composition of tiles that
/// will be loaded with a AbstractTileLoader.
/// In order to create a concrete TileLoader, the following methods need to be overloaded:
/// - AbstractTileLoader< ViewType >::loadTileFromFile [mandatory] Main method to load a tile from a file,
/// - AbstractTileLoader< ViewType >::copyTileLoader [mandatory] Copy method to duplicate user defined attribute,
/// - AbstractTileLoader< ViewType >::fullHeight [mandatory] Image/File height,
/// - AbstractTileLoader< ViewType >::fullWidth [mandatory] Image/File width,
/// - AbstractTileLoader< ViewType >::tileHeight [mandatory] Tile height,
/// - AbstractTileLoader< ViewType >::tileWidth [mandatory] Tile width,
/// - AbstractTileLoader< ViewType >::bitsPerSample [mandatory] Size of a sample (element of the file),
/// - AbstractTileLoader< ViewType >::numberPyramidLevels [mandatory] Pyramid height (minimal 1),
/// - AbstractTileLoader< ViewType >::downScaleFactor [optional] Pyramid down scale factor for each level.
/// @note By default in the dot file the miss rate will be shown.
/// @tparam ViewType Type of the view
template<class ViewType>
class AbstractTileLoader : public hh::AbstractTask<internal::TileRequest<ViewType>, internal::TileRequest<ViewType>> {
  friend FastLoaderGraph<ViewType>; ///< Define FastLoaderGraph<ViewType> as friend
  friend AdaptiveFastLoaderGraph<ViewType>; ///< Define FastLoaderGraph<ViewType> as friend
  using DataType = typename ViewType::data_t; ///< Sample type (AbstractView element type)
 private:
  std::shared_ptr<std::vector<std::shared_ptr<internal::Cache<DataType>>>>
      allCaches_ = {}; ///< All caches for each pyramid levels

  std::shared_ptr<internal::Cache<DataType>>
      cache_ = {}; ///< Tile Cache

 protected:
  std::string
      filePath_ = ""; ///< File's path to load

 public:
  /// @brief AbstractTileLoader constructor
  /// @param name AbstractTileLoader name
  /// @param filePath File's path to load
  AbstractTileLoader(std::string_view const &name, std::string filePath)
      : hh::AbstractTask<internal::TileRequest<ViewType>, internal::TileRequest<ViewType>>(name),
        filePath_(std::move(filePath)) {}

  /// @brief AbstractTileLoader constructor
  /// @param name AbstractTileLoader name
  /// @param numberThreads Number of AbstractTileLoader used in FastLoader
  /// @param filePath File's path to load
  AbstractTileLoader(std::string_view const &name, size_t numberThreads, std::string filePath)
      : hh::AbstractTask<internal::TileRequest<ViewType>, internal::TileRequest<ViewType>>(name, numberThreads),
        filePath_(std::move(filePath)) {}

  /// @brief Default destructor
  virtual ~AbstractTileLoader() = default;

  /// @brief File's path accessor
  /// @return File's path
  [[nodiscard]] std::string const &filePath() const { return filePath_; }

  /// @brief Initialize the AbstractTileLoader (set as final, if an initialization step is needed,
  /// initializeTileLoader should be used).
  /// @details  Set the cache for the correct pyramid level from allCaches_
  void initialize() final {
    cache_ = allCaches_->at(this->graphId()); initializeTileLoader();
  }

  /// @brief Specializable method used to initialise a Tile Loader
  virtual void initializeTileLoader () {};

  /// @brief Execute loop for the AbstractTileLoader
  /// @details When a TileRequest<ViewType>> is received:
  /// -# Get a tile from the cache
  /// -# Load the tile from the file
  /// -# Copy the (full or part of) tile to the view
  /// -# Send the view to the ViewCounter<ViewType>
  /// @param tileRequestData Tile Request to process
  void execute(std::shared_ptr<internal::TileRequest<ViewType>> tileRequestData) final {
    std::shared_ptr<internal::CachedTile<DataType>> cachedTile;
    uint32_t row = tileRequestData->indexRowTileAsked();
    uint32_t col = tileRequestData->indexColTileAsked();
    uint32_t layer = tileRequestData->indexLayerTileAsked();

    // Get locked tile from the cache, can be empty or not
    cachedTile = cache_->lockedTile(row, col, layer);

    // Set the tile if empty
    if (cachedTile->isNewTile()) {
      cachedTile->newTile(false);
      loadTileFromFile(cachedTile->data(), row, col, layer, tileRequestData->view()->level());
    }

    // Copy the tile or part of it into the view
    copyTileToView(tileRequestData, cachedTile);
    cachedTile->unlock();

    this->addResult(tileRequestData);
  }

  /// @brief Copy the internal attribute of an AbstractTileLoader.
  /// @details Get a copy of the user defined AbstractTileLoader from AbstractTileLoader<ViewType>::copyTileLoader(),
  /// and then copy the internal attribute.
  /// @throw std::runtime_error if the return of from AbstractTileLoader<ViewType>::copyTileLoader() is not valid
  /// @return The copy of the current AbstractTileLoader
  std::shared_ptr<hh::AbstractTask<internal::TileRequest<ViewType>, internal::TileRequest<ViewType>>> copy() final {
    std::shared_ptr<AbstractTileLoader> tileLoader = copyTileLoader();
    if (tileLoader) {
      tileLoader->allCaches_ = this->allCaches_;
      return tileLoader;
    } else {
      throw (std::runtime_error("The copyTileLoader method redefined for the tile loader return a non valid "
                                "TileLoader."));
    }
  }

  /// @brief Load a tile from the file at position (indexRowGlobalTile/indexColGlobalTile) for the pyramidal level
  /// "level"
  /// @param indexRowGlobalTile Tile's row index in the file to load
  /// @param indexColGlobalTile Tile's col index in the file to load
  /// @param indexLayerGlobalTile Tile's layer index in the file to load
  /// @param level Tile's pyramidal level in the file to load
  virtual void loadTileFromFile(std::shared_ptr<std::vector<DataType>> tile,
                                uint32_t indexRowGlobalTile,
                                uint32_t indexColGlobalTile,
                                uint32_t indexLayerGlobalTile,
                                uint32_t level) = 0;

  /// \brief Copy Function
  /// \return ATileLoader copied
  virtual std::shared_ptr<AbstractTileLoader> copyTileLoader() = 0;

  /// \brief Getter to full Height
  /// @param level file's level considered
  /// \return Image height
  [[nodiscard]] virtual uint32_t fullHeight(uint32_t level) const = 0;

  /// \brief Getter to full Width
  /// @param level file's level considered
  /// \return Image Width
  [[nodiscard]] virtual uint32_t fullWidth(uint32_t level) const = 0;

  /// \brief Getter to full Depth (default 1)
  /// @param level file's level considered
  /// \return Image Depth
  [[nodiscard]] virtual uint32_t fullDepth([[maybe_unused]] uint32_t level) const {
    return 1;
  }

  /// \brief Getter to the number of channels (default 1)
  /// \return Number of pixel's channels
  [[nodiscard]] virtual uint32_t numberChannels() const {
    return 1;
  }

  /// \brief Getter to tile Width
  /// @param level tile's level considered
  /// \return Tile Width
  [[nodiscard]] virtual uint32_t tileWidth(uint32_t level) const = 0;

  /// \brief Getter to tile Height
  /// @param level tile's level considered
  /// \return Tile Height
  [[nodiscard]] virtual uint32_t tileHeight(uint32_t level) const = 0;

  /// \brief Getter to tile Height (default 1)
  /// @param level tile's level considered
  /// \return Tile Height
  [[nodiscard]] virtual uint32_t tileDepth([[maybe_unused]] uint32_t level) const {
    return 1;
  }

  /// @brief Number tiles in height accessor for a level
  /// @param level Level asked [default 0]
  /// @return Number tiles in height for a level
  [[nodiscard]] uint32_t numberTileHeight(uint32_t level = 0) const {
    return (uint32_t) ceil((double) (fullHeight(level)) / tileHeight(level));
  }
  /// @brief Number tiles in width accessor for a level
  /// @param level Level asked [default 0]
  /// @return Number tiles in width for a level
  [[nodiscard]] uint32_t numberTileWidth(uint32_t level = 0) const {
    return (uint32_t) ceil((double) (fullWidth(level)) / tileWidth(level));
  }

  /// @brief Number tiles in depth accessor for a level
  /// @param level Level asked [default 0]
  /// @return Number tiles in depth for a level
  [[nodiscard]] uint32_t numberTileDepth(uint32_t level = 0) const {
    return (uint32_t) ceil((double) (fullDepth(level)) / tileDepth(level));
  }

  /// \brief Get file bits per samples
  /// \return File bits per sample
  [[nodiscard]] virtual short bitsPerSample() const = 0;

  /// \brief Get number of pyramid levels
  /// \return Number of Pyramid levels
  [[nodiscard]] virtual uint32_t numberPyramidLevels() const = 0;

  /// \brief Get the down scalar factor for a specific pyramid level
  /// \param level Pyramid level
  /// \return The down scalar factor for a specific pyramid level
  virtual float downScaleFactor([[maybe_unused]] uint32_t level) { return 1; }

  /// @brief Default print information in the dot file, add miss rate
  /// @return A string with the miss rate
  [[nodiscard]]std::string extraPrintingInformation() const override {
    std::ostringstream oss;
    oss << "Miss rate: "
        << std::setprecision(2) << this->cache_->miss() * 1. / (this->cache_->miss() + this->cache_->hit()) << "%"
        << std::endl
        << "Disk Access time: " << durationPrinter(cache_->accessTime()) << std::endl
        << "Disk Recycle time: " << durationPrinter(cache_->recycleTime()) << std::endl;
    return oss.str();
  }

 private:
  /// @brief Copy a tile to the view
  /// @param tileRequestData TileRequest representing all the metadata and destination for the copy
  /// @param cachedTile File's tile to copy
  void copyTileToView(
      std::shared_ptr<internal::TileRequest<ViewType>> tileRequestData,
      std::shared_ptr<internal::CachedTile<DataType>> cachedTile) {
    uint32_t
        tileWidth = this->tileWidth(this->graphId()),
        tileHeight = this->tileHeight(this->graphId()),
        viewWidth = tileRequestData->view()->viewWidth(),
        viewHeight = tileRequestData->view()->viewHeight(),
        rowBeginSrc = 0, colBeginSrc = 0, layerBeginSrc = 0,
        rowBeginDest = 0, colBeginDest = 0, layerBeginDest = 0,
        width = 0, height = 0, depth = 0;

    auto dest = tileRequestData->view()->viewOrigin();

    // Do Every copy for a tile to a view
    for (auto copy : tileRequestData->copies()) {
      rowBeginSrc = copy.from().rowBegin();
      colBeginSrc = copy.from().colBegin();
      layerBeginSrc = copy.from().layerBegin();

      rowBeginDest = copy.to().rowBegin();
      colBeginDest = copy.to().colBegin();
      layerBeginDest = copy.to().layerBegin();

      width = copy.from().width();
      height = copy.from().height();
      depth = copy.from().depth();

      // Handle symmetry for the copy
      if (copy.reverseLayers()) {
        if (copy.reverseCols()) {
          if (copy.reverseRows()) {
            // Layer Col Row Symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::reverse_copy(
                cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc) * numberChannels(),
                    cachedTile->data()->begin()
                        + (layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + ((rowBeginSrc + row) * tileWidth
                        + colBeginSrc
                        + width) * numberChannels(),
                    dest
                        + (layerBeginDest + depth - layer - 1) * viewHeight * viewWidth
                        + (((rowBeginDest + height - row - 1) * viewWidth
                        + colBeginDest) * numberChannels())
                );
              }
            }
          } else {
            // Layer Col symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::reverse_copy(
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + ((rowBeginSrc + row) * tileWidth + colBeginSrc)) * numberChannels(),
                    cachedTile->data()->begin()
                        + (((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc
                        + width)) * numberChannels(),
                    dest
                        + ((layerBeginDest + depth - layer - 1) * viewHeight * viewWidth
                        + (rowBeginDest + row) * viewWidth
                        + colBeginDest) * numberChannels()
                );
              }
            }
          }
        } else {
          if (copy.reverseRows()) {
            // Layers Rows symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::copy_n(
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc) * numberChannels(),
                    width * numberChannels(),
                    dest
                        + ((layerBeginDest + depth - layer - 1) * viewHeight * viewWidth
                        + (rowBeginDest + height - row - 1) * viewWidth
                        + colBeginDest) * numberChannels()
                );
              }
            }
          } else {
            // Layer symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::copy_n(
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc) * numberChannels(),
                    width * numberChannels(),
                    dest
                        + ((layerBeginDest + depth - layer - 1) * viewHeight * viewWidth
                        + (rowBeginDest + row) * viewWidth
                        + colBeginDest) * numberChannels()
                );
              }
            }
          } // End Reverse Row
        } // End Reverse Columns
      } else {
        if (copy.reverseCols()) {
          if (copy.reverseRows()) {
            // Rows Cols symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::reverse_copy(
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc) * numberChannels(),
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc
                        + width) * numberChannels(),
                    dest
                        + ((layerBeginDest + layer) * viewWidth * viewHeight
                        + (rowBeginDest + height - row - 1) * viewWidth
                        + colBeginDest) * numberChannels()
                );
              }
            }
          } else {
            // Cols symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::reverse_copy(
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc) * numberChannels(),
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc
                        + width) * numberChannels(),
                    dest
                        + ((layerBeginDest + layer) * viewWidth * viewHeight
                        + (rowBeginDest + row) * viewWidth
                        + colBeginDest) * numberChannels()
                );
              }
            }
          }
        } else {
          if (copy.reverseRows()) {
            // Rows symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::copy_n(
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc) * numberChannels(),
                    width * numberChannels(),
                    dest
                        + ((layerBeginDest + layer) * viewWidth * viewHeight
                        + (rowBeginDest + height - row - 1) * viewWidth
                        + colBeginDest) * numberChannels()
                );
              }
            }
          } else {
            // No symmetry
            for (size_t layer = 0; layer < depth; ++layer) {
              for (size_t row = 0; row < height; ++row) {
                std::copy_n(
                    cachedTile->data()->begin()
                        + ((layerBeginSrc + layer) * (tileWidth * tileHeight)
                        + (rowBeginSrc + row) * tileWidth
                        + colBeginSrc) * numberChannels(),
                    width * numberChannels(),
                    dest
                        + ((layerBeginDest + layer) * viewWidth * viewHeight
                        + (rowBeginDest + row) * viewWidth
                        + colBeginDest) * numberChannels()
                );

              }
            }
          } // End Reverse Row
        } // End Reverse Columns
      } // End Reverse Layers
    } // For all copies
  }

  /// @brief Print a duration with the good unit
  /// @param duration Duration to print
  /// @return std::string with the duration and the unit
  [[nodiscard]] std::string durationPrinter(uint64_t const duration) const {
    std::ostringstream oss;
    uint64_t
        s = (duration % 1000000000) / 1000000,
        mS = (duration % 1000000) / 1000,
        uS = (duration % 1000);

    if (s > 0) {
      oss << s << "." << std::setfill('0') << std::setw(3) << mS << "s";
    } else if (mS > 0) {
      oss << mS << "." << std::setfill('0') << std::setw(3) << uS << "ms";
    } else {
      oss << duration << "us";
    }
    return oss.str();
  }
};
}
#endif //FASTLOADER_ABSTRACT_TILE_LOADER_H
