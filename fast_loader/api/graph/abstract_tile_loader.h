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

#ifndef FAST_LOADER_ABSTRACT_TILE_LOADER_H
#define FAST_LOADER_ABSTRACT_TILE_LOADER_H

#include <hedgehog/hedgehog.h>

#include <utility>

#include "../../core/data/tile_request.h"
#include "../../core/cache.h"

/// @brief FastLoader namespace
namespace fl {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
/// @brief FastLoaderGraph Forward declaration
/// @tparam ViewType Type of the view
template<class ViewType>
class FastLoaderGraph;

/// @brief AdaptiveFastLoaderGraph forward declaration
/// @tparam ViewType Type of the view
template<class ViewType>
class AdaptiveFastLoaderGraph;
#endif //DOXYGEN_SHOULD_SKIP_THIS

/// @brief Interface to create a tile loader
/// @details A tile loader is the interface used by Fast Loader to access the file.
/// By construction Fast Loader is agnostic to the underlying file format and structure.
/// Fast Loader only sees buffers (tiles) and used them to build views served to the end-user.
/// To get the data out of the file a TileLoader needs to be created for the format that is needed for the computation.
/// In essence it provides a pre-allocated buffer and its position in the files (index + level) and the tiles loader
/// needs to fill it with file data.
///  While FastLoader uses caches to hit the files a little as possible, the performance of FastLoader it partially
/// determined by the performance of the Tile Loader to load efficiently the requested buffer.
/// Besides loading buffer from the files, the Tile Loader is also the interface to get information from the file, as
/// its dimension, metadata...
/// @tparam ViewType Type of the view
template<class ViewType>
class AbstractTileLoader
    : public hh::AbstractTask<
        1,
        internal::TileRequest<ViewType>,
        std::pair<std::shared_ptr<internal::TileRequest<ViewType>>,
                  std::shared_ptr<internal::CachedTile<typename ViewType::data_t>>>
    > {
 private:
  friend FastLoaderGraph<ViewType>; ///< Define FastLoaderGraph<ViewType> as friend
  friend AdaptiveFastLoaderGraph<ViewType>; ///< Define FastLoaderGraph<ViewType> as friend
  using DataType = typename ViewType::data_t; ///< Sample type (AbstractView element type)
  std::shared_ptr<std::vector<std::shared_ptr<internal::Cache<DataType>>>>
      allCaches_ = {}; ///< All caches for each pyramid levels

  std::shared_ptr<internal::Cache<DataType>>
      cache_ = {}; ///< Tile Cache used by the tile loader

  std::chrono::nanoseconds
      fileLoadingTime_ = std::chrono::nanoseconds::zero(); ///< Loading data from file duration

 protected:
  std::filesystem::path const filePath_; ///< File path
  std::shared_ptr<std::unordered_map<std::string, std::string>>
      metadata_ = std::make_shared<std::unordered_map<std::string, std::string>>(); ///< Metadata representation

 public:
  /// @brief Tile loader abstraction constructor
  /// @param name Name of the tile loader
  /// @param filePath Path to the file
  /// @param nbThreads Number of threads used to load buffers from the file
  AbstractTileLoader(std::string const &name, std::filesystem::path filePath, size_t const nbThreads = 1)
      : hh::AbstractTask<
      1,
      internal::TileRequest<ViewType>,
      std::pair<std::shared_ptr<internal::TileRequest<ViewType>>,
                std::shared_ptr<internal::CachedTile<typename ViewType::data_t>>>>(
      name, nbThreads, false), filePath_(std::move(filePath)
  ) {}

  /// @brief Default destructor
  ~AbstractTileLoader() = default;

  /// @brief File path accessor
  /// @return The file path
  [[nodiscard]] std::filesystem::path const &filePath() const { return filePath_; }

  /// @brief Initialize the Tile loader to set the correct cache for the level it will act with and call user-defined
  /// initialization
  void initialize() final {
    cache_ = allCaches_->at(this->graphId());
    initializeTileLoader();
  }

  /// @brief End-used customizable tile loader
  virtual void initializeTileLoader() {};

  /// @brief Tile loader main logic
  /// @details Acquire the tile out of the cache.
  /// If the cached tile is new, load it from the file using loadTileFromFile.
  /// Once filled, data from the cached tile are copied to the view.
  /// @param tileRequestData Tile request
  void execute(std::shared_ptr<internal::TileRequest<ViewType>> tileRequestData) final {
    std::shared_ptr<internal::CachedTile<DataType>> cachedTile;
    auto index = tileRequestData->index();
    // Get the tile from the cache
    cachedTile = cache_->lockedTile(index);

    //If new load from user interface
    if (cachedTile->newTile()) {
      cachedTile->newTile(false);
      auto begin = std::chrono::system_clock::now();
      loadTileFromFile(cachedTile->data(), index, tileRequestData->view()->level());
      auto end = std::chrono::system_clock::now();
      fileLoadingTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    }

    this->addResult(
        std::make_shared<std::pair<std::shared_ptr<internal::TileRequest<ViewType>>,
                                   std::shared_ptr<internal::CachedTile<typename ViewType::data_t>>>>(
            tileRequestData, cachedTile
        )
    );
  }

  /// @brief Copy the TileLoader by calling user-defined copyTileLoader method and setting the caches
  /// @return New tile loader for a group
  std::shared_ptr<hh::AbstractTask<1,
                                   internal::TileRequest<ViewType>,
                                   std::pair<std::shared_ptr<internal::TileRequest<ViewType>>,
                                             std::shared_ptr<internal::CachedTile<typename ViewType::data_t>>>>> copy() final {
    std::shared_ptr<AbstractTileLoader> tileLoader = copyTileLoader();
    if (tileLoader) {
      tileLoader->metadata_ = this->metadata_;
      tileLoader->allCaches_ = this->allCaches_;
      return tileLoader;
    } else {
      throw (std::runtime_error("The copyTileLoader method redefined for the tile loader return a non valid TileLoader."));
    }
  }

  /// @brief Information printed for the tile loader including miss rate and access time
  /// @return String containing Tile loader information
  [[nodiscard]]std::string extraPrintingInformation() const override {
    std::ostringstream oss;
    oss << "Miss rate: "
        << std::setprecision(3)
        << (double) (this->cache_->miss()) / (double) (this->cache_->miss() + this->cache_->hit()) * 100 << "%"
        << std::endl
        << "File Loading time: " << durationPrinter(fileLoadingTime_) << std::endl
        << "Cache Access time: " << durationPrinter(cache_->accessTime()) << std::endl
        << "Cache Recycle time: " << durationPrinter(cache_->recycleTime()) << std::endl;
    return oss.str();
  }

  /// @brief Metadata accessor
  /// @return Metadata
  [[nodiscard]] std::shared_ptr<std::unordered_map<std::string, std::string>> const &metadata() const {
    return metadata_;
  }

  /// @brief Accessor to the dimension of the file for a given dimension
  /// @param dim Dimension index
  /// @param level Level requested
  /// @return Dimension of the file for a given index / level
  [[nodiscard]] size_t const &fullDim(std::size_t const dim, std::size_t const level = 0) const {
    return fullDims(level).at(dim);
  }

  /// @brief Accessor to the dimension of the tile for a given dimension
  /// @param dim Dimension index
  /// @param level Level requested
  /// @return Dimension of the tile for a given index / level
  [[nodiscard]] size_t const &tileDim(std::size_t const dim, std::size_t const level = 0) const {
    return tileDims(level).at(dim);
  }

  /// @brief Accessor to the dimension of the file for a given dimension
  /// @param dimName Dimension name
  /// @param level Level requested
  /// @return Dimension of the file for a given name / level
  [[nodiscard]] size_t const &fullDim(std::string const &dimName, std::size_t const level = 0) const {
    return fullDim(dimIndex(dimName), level);
  }

  /// @brief Accessor to the dimension of the tile for a given dimension
  /// @param dimName Dimension name
  /// @param level Level requested
  /// @return Dimension of the tile for a given name / level
  [[nodiscard]] size_t const &tileDim(std::string const &dimName, std::size_t const level = 0) const {
    return tileDim(dimIndex(dimName), level);
  }

  /// @brief Tests if a given name exists in the dimension names
  /// @param dimName Dimensions name to test
  /// @return True if the name exists, else false
  [[nodiscard]] bool hasDim(std::string const &dimName) const {
    auto const &names = dimNames();
    return std::find(names.cbegin(), names.cend(), dimName) != names.cend();
  }

  /// @brief Transform a dimension name to its index
  /// @param dimName Dimension name
  /// @return Dimension index
  [[nodiscard]] size_t dimIndex(std::string const &dimName) const {
    auto const &names = dimNames();
    auto it = std::find(names.cbegin(), names.cend(), dimName);
    if (it != names.cend()) { return (size_t)std::distance(names.cbegin(), it); }
    else {
      std::ostringstream oss;
      oss << "The dimension \"" << dimName << "\"" << " does not exist.";
      throw std::runtime_error(oss.str());
    }
  }

  /// @brief User-defined copy Function
  /// @return ATileLoader copied
  virtual std::shared_ptr<AbstractTileLoader> copyTileLoader() { return nullptr; };

  /// @brief Downscale factor for pyramidal images accessor
  /// @param level Pyramidal level
  /// @return Downscale factor for pyramidal images
  virtual float downScaleFactor([[maybe_unused]] std::size_t const level) { return 1; }

  /// @brief Load a tile from the file.
  /// @details Provide an allocated buffer and the position of the requested tile. The aim of this interface is to fill
  /// the buffer from data from the file.
  /// @param tile Allocated buffer to fill
  /// @param index Position of the tile
  /// @param level Level of the tile
  virtual void loadTileFromFile(std::shared_ptr<std::vector<DataType>> tile,
                                std::vector<size_t> const &index,
                                size_t level) = 0;

  /// @brief Number of dimensions accessor
  /// @return Number of dimensions
  [[nodiscard]] virtual size_t nbDims() const = 0;

  /// @brief Number of pyramidal levels accessor
  /// @return Number of pyramidal levels
  [[nodiscard]] virtual size_t nbPyramidLevels() const = 0;

  /// @brief File dimensions accessor
  /// @param level Pyramidal level
  /// @return File dimensions
  [[nodiscard]] virtual std::vector<size_t> const &fullDims([[maybe_unused]] std::size_t level) const = 0;

  /// @brief Tile dimensions accessor
  /// @param level Pyramidal level
  /// @return Tile dimensions
  [[nodiscard]] virtual std::vector<size_t> const &tileDims([[maybe_unused]] std::size_t level) const = 0;

  /// @brief Dimension names accessor
  /// @return Dimension names
  [[nodiscard]] virtual std::vector<std::string> const &dimNames() const = 0;

 private:
  /// @brief Helper function taking a duration in nanoseconds and printing it properly
  /// @param ns Duration in nanoseconds
  /// @return String containing the representation of the duration with the right unit
  static std::string durationPrinter(std::chrono::nanoseconds const &ns) {
    std::ostringstream oss;

    // Cast with precision loss
    auto s = std::chrono::duration_cast<std::chrono::seconds>(ns);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(ns);

    if (s > std::chrono::seconds::zero()) {
      oss << s.count() << "." << std::setfill('0') << std::setw(3) << (ms - s).count() << "s";
    } else if (ms > std::chrono::milliseconds::zero()) {
      oss << ms.count() << "." << std::setfill('0') << std::setw(3) << (us - ms).count() << "ms";
    } else if (us > std::chrono::microseconds::zero()) {
      oss << us.count() << "." << std::setfill('0') << std::setw(3) << (ns - us).count() << "us";
    } else {
      oss << std::setw(3) << ns.count() << "ns";
    }
    return oss.str();
  }

};

}
#endif //FAST_LOADER_ABSTRACT_TILE_LOADER_H
