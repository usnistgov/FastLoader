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

#ifndef FASTLOADER_CACHE_H
#define FASTLOADER_CACHE_H

#include <queue>
#include <mutex>
#include <unordered_map>
#include <list>
#include <sstream>
#include "data/cached_tile.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief FastLoader's LRU Cache used as middleware between the AbstractTileLoader and the file.
/// @tparam DataType AbstractView's internal type
template<class DataType>
class Cache {
  using CachedTileType = std::shared_ptr<CachedTile<DataType>>; ///< Helper to define the cache's tile type
 private:
  std::queue<CachedTileType>
      pool_{};                  ///< Pool of new tile

  std::vector<std::vector<std::vector<CachedTileType>>>
      mapCache_{};              ///< Matrix of cached tiles

  std::unordered_map<CachedTileType, typename std::list<CachedTileType>::const_iterator>
      mapLRU_{};                ///< Map between the Tile and it's position

  std::list<CachedTileType>
      lru_{};                   ///< List to save the Tile order

  std::mutex
      cacheMutex_{};            ///< Cache mutex

  size_t
      nbTilesCache_,          ///< Number of tiles allocated
      numTilesHeight_,        ///< Number of tiles in a column
      numTilesWidth_,         ///< Number of tiles in a row
      numTilesDepth_,         ///< Number of tiles in a depth
      miss_,                  ///< Number of tile miss (tile get from the disk)
      hit_;                   ///< Number of tile hit (tile get from the cache)

 private:
  std::chrono::nanoseconds
      accessTime_ = std::chrono::nanoseconds::zero(), ///< Time to get a tile from the cache (use for statistics)
      recycleTime_ = std::chrono::nanoseconds::zero(); ///< Time to release a tile from the cache (use for statistics)

 public:

  /// @brief Cache constructor
  /// @param nbTilesCache Number of tiles to cache
  /// @param numTilesHeight Number of tiles in height
  /// @param numTilesWidth Number of tiles in width
  /// @param numTilesDepth Number of tiles in depth
  /// @param tileHeight Tile height
  /// @param tileWidth Tile width
  /// @param tileDepth Tile depth
  /// @param numberChannels Number of pixel channels
  Cache(
      size_t nbTilesCache,
      size_t numTilesHeight, size_t numTilesWidth, size_t numTilesDepth,
      size_t tileHeight, size_t tileWidth, size_t tileDepth, size_t numberChannels)
      : nbTilesCache_(nbTilesCache),
        numTilesHeight_(numTilesHeight), numTilesWidth_(numTilesWidth), numTilesDepth_(numTilesDepth),
        miss_(0), hit_(0), accessTime_(0), recycleTime_(0) {
    size_t nbTilesInImage = numTilesHeight * numTilesWidth * numTilesDepth;

    // If the number of tiles to be cached has been set to 0 (default value), set the number to 2 * number of tiles
    // in a row
    if (nbTilesCache_ == 0) { nbTilesCache_ = 2 * numTilesWidth; }

    // If the number of tiles to be cached is superior to the number of tiles in the file, set it to the number of
    // tiles in the file
    if (nbTilesInImage < nbTilesCache_) { nbTilesCache_ = nbTilesInImage; }

    // Create the matrix
    mapCache_ = std::vector<std::vector<std::vector<CachedTileType>>>(
        numTilesDepth_,
        std::vector<std::vector<CachedTileType>>(
            numTilesHeight_, std::vector<CachedTileType>(
                numTilesWidth_,
                nullptr
            )
        )
    );

    // Fill the pool
    for (size_t tileCnt = 0; tileCnt < nbTilesCache_; ++tileCnt) {
      pool_.push(std::make_shared<CachedTile<DataType>>(tileWidth, tileHeight, tileDepth, numberChannels));
    }
  }

  /// @brief Cache destructor
  virtual  ~Cache() = default;

  /// @brief Hit mergeCount accessor
  /// @return Hit mergeCount
  [[nodiscard]] size_t hit() const { return hit_; }

  /// @brief Miss mergeCount accessor
  /// @return Miss mergeCount
  [[nodiscard]] size_t miss() const { return miss_; }

  /// @brief Maximum number of tiles to cache accessor
  /// @return Maximum number of tiles to cache
  [[nodiscard]] size_t nbTilesCache() const { return nbTilesCache_; }

  /// @brief Cache's pool accessor
  /// @return Cache's pool
  const std::queue<CachedTileType> &pool() const { return pool_; }

  /// @brief Cache's map accessor
  /// @return Cache's map
  const std::vector<std::vector<std::vector<CachedTileType>>> &mapCache() const { return mapCache_; }

  /// @brief Cache's LRU list accessor
  /// @return Cache's LRU list
  const std::list<CachedTileType> &lru() const { return lru_; }

  /// @brief Access time accessor
  /// @return Access time
  [[nodiscard]] std::chrono::nanoseconds const & accessTime() const { return accessTime_; }
  /// @brief Recycle time accessor
  /// @return Recycle time
  [[nodiscard]] std::chrono::nanoseconds const & recycleTime() const { return recycleTime_; }

  /// @brief Get a locked tile from the cache
  /// @param indexRow Locked tile's index row
  /// @param indexCol Locked tile's index column
  /// @param indexLayer Locked tile's index layer
  /// @return Locked tile
  CachedTileType lockedTile(size_t indexRow, size_t indexCol, size_t indexLayer) {
    CachedTileType tile;
    if (!(indexRow < numTilesHeight_ && indexCol < numTilesWidth_ && indexLayer < numTilesDepth_)) {
      std::stringstream message;
      message << "Tile Loader ERROR: The index is not correct: ("
              << indexRow << ", " << indexCol << ", " << indexLayer << ")";
      std::string m = message.str();
      throw (std::runtime_error(message.str()));
    }
    this->lockCache();
    auto begin = std::chrono::system_clock::now();
    if (isInCache(indexRow, indexCol, indexLayer)) {
      // Tile is in cache
      hit_ += 1;
      tile = cachedLockedTile(indexRow, indexCol, indexLayer);
    } else {
      // Tile is not in the cache
      miss_ += 1;
      if (pool_.empty()) { recycleTile(); }
      tile = newLockedTile(indexRow, indexCol, indexLayer);
    }
    auto end = std::chrono::system_clock::now();
    accessTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    this->unlockCache();

    return tile;
  }

  /// @brief FastLoader cache Output stream operator
  /// @param os Output stream to print into
  /// @param cache FastLoader cache to print
  /// @return Output stream
  friend std::ostream &operator<<(std::ostream &os, const Cache &cache) {
    size_t numLayer = 0;
    os << "-------------------------------------------" << std::endl;
    os << "Cache AbstractView:" << std::endl;
    os << "Waiting Queue: " << std::endl;
    print_queue(cache.pool_);
    os << "MapCache: " << std::endl;
    for (auto layer = cache.mapCache_.begin(); layer != cache.mapCache_.end(); ++layer) {
      os << "Layer #" << numLayer++ << std::endl;
      for (auto row = layer->begin(); row != layer->end(); ++row) {
        for (auto col = row->begin(); col != row->end(); ++col) {
          os << *col << " ";
        }
        os << std::endl;
      }
      os << std::endl;
    }
    os << "MapLRU: " << std::endl;
    for (auto elem : cache.mapLRU_)
      os << "    " << elem.first << ": "
         << std::distance(cache.lru_.begin(), elem.second) << std::endl;

    os << "ListLRU: " << std::endl;
    for (auto elem : cache.lru_)
      os << elem << " ";
    os << std::endl;
    os << "timeGet: " << cache.accessTime_.count() << " ns / timeRelease: " << cache.recycleTime_.count()
    << " ns / nbTilesCache: " << cache.nbTilesCache_ << " / miss: " << cache.miss_ << " / hit: " << cache.hit_
    << "\n-------------------------------------------\n";
    return os;
  }

 private:

  /// \brief Lock the cache.
  void lockCache() { cacheMutex_.lock(); }

  /// \brief Unlock the cache.
  void unlockCache() { cacheMutex_.unlock(); }

  /// @brief Test if a Tile is available in cache
  /// @param indexRow Tile's row
  /// @param indexCol Tile's column
  /// @param indexLayer Tile's layer
  /// @return True if the tile is already available, else False
  [[nodiscard]] bool isInCache(size_t indexRow, size_t indexCol, size_t indexLayer) {
    return nullptr != mapCache_[indexLayer][indexRow][indexCol];
  }

  /// @brief Recycle the LRU tile
  void recycleTile() {
    CachedTileType toRecycle;

    auto begin = std::chrono::system_clock::now();

    // Get LRU Tile
    toRecycle = lru_.back();
    toRecycle->lock();
    lru_.pop_back();

    // Clean The Tile
    mapLRU_.erase(toRecycle);
    mapCache_[toRecycle->indexLayer()][toRecycle->indexRow()][toRecycle->indexCol()] = nullptr;
    toRecycle->indexRow(0);
    toRecycle->indexCol(0);
    toRecycle->indexLayer(0);
    toRecycle->newTile(true);

    // Put it back in the pool
    pool_.push(toRecycle);

    auto end = std::chrono::system_clock::now();
    recycleTime_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);

    toRecycle->unlock();
  }

  /// @brief Get a tile from the pool and add it to the cache
  /// @param indexRow Tile's row index
  /// @param indexCol Tile's column index
  /// @param indexLayer Tile's layer index
  /// @return Locked tile
  [[nodiscard]] CachedTileType newLockedTile(size_t indexRow, size_t indexCol, size_t indexLayer) {
    // Get tile from the pool
    CachedTileType tile = pool_.front();
    tile->lock();
    pool_.pop();

    // Set tile information except data
    tile->indexCol(indexCol);
    tile->indexRow(indexRow);
    tile->indexLayer(indexLayer);

    // Register the tile
    mapCache_[indexLayer][indexRow][indexCol] = tile;
    lru_.push_front(tile);
    mapLRU_[tile] = lru_.begin();
    return tile;
  }

  /// @brief Get a tile from the cache and update the LRU position
  /// @param indexRow Tile's row index
  /// @param indexCol Tile's column index
  /// @param indexLayer Tile's layer index
  /// @return Locked tile
  [[nodiscard]] CachedTileType cachedLockedTile(size_t indexRow, size_t indexCol, size_t indexLayer) {
    assert(mapCache_[indexLayer][indexRow][indexCol] != nullptr);

    // Get the tile
    CachedTileType tile = mapCache_[indexLayer][indexRow][indexCol];
    tile->lock();

    // Update the tile position in the LRU
    lru_.erase(mapLRU_[tile]);
    lru_.push_front(tile);
    mapLRU_[tile] = lru_.begin();

    return tile;
  }

  /// @brief Method to print a queue (do a copy before)
  /// @param q Queue to print
  static void print_queue(std::queue<CachedTileType> q) {
    while (!q.empty()) {
      std::cout << q.front() << " ";
      q.pop();
    }
    std::cout << std::endl;
  }

};
}
}
#endif //FASTLOADER_CACHE_H
