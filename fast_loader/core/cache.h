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

#ifndef FAST_LOADER_CACHE_H
#define FAST_LOADER_CACHE_H

#include <queue>
#include <list>
#include <unordered_map>
#include <chrono>
#include <utility>
#include <algorithm>
#include "data/cached_tile.h"


/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief
/// @tparam DataType Type of data inside the View
template<class DataType>
/// @brief FastLoader LRU cache, provides locked cache tile
/// @tparam DataType Type inside of the cache
class Cache {
 private:
  using CachedTile_t = std::shared_ptr<CachedTile<DataType>>; ///< Helper to define the cache's tile type
  std::vector<size_t> cacheDimension_{}; ///< Dimension of the cache
  std::size_t const
    maxNbTilesCache_{}, ///< Maximum number tiles in cache
    nbTilesCache_{}; ///< Number tiles in cache
  std::vector<CachedTile_t> mapCache_{}; ///< Map between the Tile and its position
  std::queue<CachedTile_t> pool_{}; ///< Pool of available tile
  std::list<CachedTile_t> lru_{}; ///< List to save the Tile order
  std::unordered_map<CachedTile_t, typename std::list<CachedTile_t>::const_iterator> mapLRU_{}; ///< Map between the Tile and it's position
  std::mutex cacheMutex_{}; ///< Cache mutex
  std::size_t
    miss_{}, ///< Number of tile miss (tile get from the disk)
    hit_{};  ///< Number of tile hit (tile get from the cache)

  std::chrono::nanoseconds
      accessTime_ = std::chrono::nanoseconds::zero(), ///< Time to get a tile from the cache (use for statistics)
      recycleTime_ = std::chrono::nanoseconds::zero(); ///< Time to release a tile from the cache (use for statistics)

 public:
  /// @brief Cache constructor
  /// @param cacheDimension Cache dimensions
  /// @param nbTilesCache Number tiles in cache
  /// @param tileDimension Tile dimensions
  Cache(std::vector<size_t> cacheDimension, size_t nbTilesCache, std::vector<size_t> tileDimension) :
      cacheDimension_(std::move(cacheDimension)),
      maxNbTilesCache_(std::accumulate(cacheDimension_.begin(), cacheDimension_.end(), (size_t) 1, std::multiplies<>())),
      nbTilesCache_(
          nbTilesCache == 0 ?
          (maxNbTilesCache_ < 18 ? maxNbTilesCache_ : 18) :
          (maxNbTilesCache_ < nbTilesCache ? maxNbTilesCache_ : nbTilesCache)
      ) {
    mapCache_ = std::vector<CachedTile_t>(maxNbTilesCache_);
    for (size_t tileCnt = 0; tileCnt < nbTilesCache_; ++tileCnt) {
      pool_.push(std::make_shared<CachedTile<DataType>>(tileDimension));
    }
  }

  /// @brief Default destructor
  virtual ~Cache() = default;

  /// @brief Number tiles in the cache accessor
  /// @return Number tiles in the cache
  [[nodiscard]] size_t nbTilesCache() const { return nbTilesCache_; }
  /// @brief Cache miss accessor
  /// @return Cache miss counter
  [[nodiscard]] size_t miss() const { return miss_; }
  /// @brief Cache hit accessor
  /// @return Cache hit counter
  [[nodiscard]] size_t hit() const { return hit_; }
  /// @brief Matrix of cached tiles accessor
  /// @return Matrix of cached tiles
  std::vector<CachedTile_t> const &mapCache() const { return mapCache_; }
  /// @brief Pool accessor
  /// @return Pool of available tile
  std::queue<CachedTile_t> const &pool() const { return pool_; }
  /// @brief Tile order accessor
  /// @return Tile order
  std::list<CachedTile_t> const &lru() const { return lru_; }
  /// @brief Access time accessor
  /// @return Cache access time
  [[nodiscard]] std::chrono::nanoseconds const &accessTime() const { return accessTime_; }
  /// @brief Recycle time accessor
  /// @return Cache Recycle time
  [[nodiscard]] std::chrono::nanoseconds const &recycleTime() const { return recycleTime_; }

  /// @brief Get a locked tile from its index
  /// @param index Tile index
  /// @return Locked tile corresponding to the requested index
  CachedTile_t lockedTile(std::vector<size_t> const &index) {
    assert(testIndex(index));
    CachedTile_t tile;
    this->lockCache();
    auto begin = std::chrono::system_clock::now();
    if (isInCache(index)) {
      // Tile is in cache
      hit_ += 1;
      tile = cachedLockedTile(index);
    } else {
      // Tile is not in the cache
      miss_ += 1;
      if (pool_.empty()) { recycleTile(); }
      tile = newLockedTile(index);
    }
    auto end = std::chrono::system_clock::now();
    accessTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    this->unlockCache();
    return tile;
  }

 private:
  /// \brief Lock the cache.
  void lockCache() { cacheMutex_.lock(); }

  /// \brief Unlock the cache.
  void unlockCache() { cacheMutex_.unlock(); }

  /// @brief Test if an index is valid
  /// @param index Index to test
  /// @return True if the index is valid, else throw a std::runtime_error
  bool testIndex(std::vector<size_t> const &index) {
    if (index >= cacheDimension_) {
      std::stringstream message;
      message << "Tile Loader ERROR: The tile index (";
      std::copy(index.cbegin(), index.cend(), std::ostream_iterator<size_t>(message, ", "));
      message << ") is not correct";
      std::string m = message.str();
      throw (std::runtime_error(message.str()));
    }
    return true;
  }

  /// @brief Test if an index is in cache
  /// @param index Index to test
  /// @return True is the view corresponding to the index is in the cache, else False
  [[nodiscard]] bool isInCache(std::vector<size_t> const &index) {
    return nullptr != mapCache_.at(mapIndex(index));
  }

  /// @brief Flatten the index for the map
  /// @param index Requested index
  /// @param dimension Current dimension
  /// @return Flattened index
  inline size_t mapIndex(std::vector<size_t> const &index, size_t const dimension = 0) {
    if (dimension == (index.size() - 1)) {
      return index.at(dimension);
    } else {
      return index.at(dimension) * std::accumulate(
          cacheDimension_.cbegin() + (long) dimension + 1, cacheDimension_.cend(), (size_t) 1, std::multiplies<>()
      ) + mapIndex(index, dimension + 1);
    }
  }

  /// @brief Get a cached tile
  /// @param index Tile's index
  /// @return The cached tile
  [[nodiscard]] CachedTile_t cachedLockedTile(std::vector<size_t> const &index) {
    assert(isInCache(index));

    // Get the tile
    CachedTile_t tile = mapCache_.at(mapIndex(index));
    tile->lock();

    // Update the tile position in the LRU
    lru_.erase(mapLRU_[tile]);
    lru_.push_front(tile);
    mapLRU_[tile] = lru_.begin();

    return tile;
  }

  /// @brief Get a new tile
  /// @param index Tile's index
  /// @return The new tile
  [[nodiscard]] CachedTile_t newLockedTile(std::vector<size_t> const &index) {
    // Get tile from the pool
    CachedTile_t tile = pool_.front();
    tile->lock();
    pool_.pop();

    // Set tile information except data
    tile->index(index);

    // Register the tile
    mapCache_.at(mapIndex(index)) = tile;
    lru_.push_front(tile);
    mapLRU_[tile] = lru_.begin();
    return tile;
  }

  /// @brief Recycle the tile
  void recycleTile() {
    CachedTile_t toRecycle;

    auto begin = std::chrono::system_clock::now();

    // Get LRU Tile
    toRecycle = lru_.back();
    toRecycle->lock();
    lru_.pop_back();

    // Clean The Tile
    mapLRU_.erase(toRecycle);
    mapCache_.at(mapIndex(toRecycle->index())) = nullptr;
    toRecycle->newTile(true);

    // Put it back in the pool
    pool_.push(toRecycle);

    auto end = std::chrono::system_clock::now();
    recycleTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

    toRecycle->unlock();
  }

};

} // fl
} // internal

#endif //FAST_LOADER_CACHE_H
