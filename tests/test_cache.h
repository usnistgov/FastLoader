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
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/28/20.
//

#ifndef INC_3DFASTLOADER_TEST_CACHE_H
#define INC_3DFASTLOADER_TEST_CACHE_H

#include <gtest/gtest.h>
#include <cmath>
#include "../fast_loader/internal/cache.h"

void cacheInitialization(uint32_t numTileCache,
                         uint32_t numTilesHeight, uint32_t numTilesWidth, uint32_t numTilesDepth,
                         uint32_t tileHeight, uint32_t tileWidth, uint32_t tileDepth, uint32_t numberChannels) {
  fl::internal::Cache<int> cache(
      numTileCache, numTilesHeight, numTilesWidth, numTilesDepth, tileHeight, tileWidth, tileDepth, numberChannels);
  ASSERT_EQ(cache.hit(), (uint32_t) 0);
  ASSERT_EQ(cache.miss(), (uint32_t) 0);
  if (numTileCache == 0) { numTileCache = 2 * numTilesWidth; }
  if (numTilesHeight * numTilesWidth * numTilesDepth < numTileCache) {
    ASSERT_EQ(cache.nbTilesCache(), numTilesHeight * numTilesWidth * numTilesDepth);
  } else {
    ASSERT_EQ(cache.nbTilesCache(), numTileCache);
  }
  ASSERT_EQ(cache.nbTilesCache(), cache.pool().size());
  auto const & mapCache = cache.mapCache();
  for (uint32_t layer = 0; layer < numTilesDepth; ++layer) {
    for (uint32_t row = 0; row < numTilesHeight; ++row) {
      for (uint32_t col = 0; col < numTilesWidth; ++col) {
        ASSERT_TRUE(mapCache.at(layer).at(row).at(col).get() == nullptr);
      }
    }
  }
  ASSERT_EQ(cache.lru().size(), (uint32_t) 0);
}

void getNewTiles(uint32_t numTileCache,
                 uint32_t numTilesHeight, uint32_t numTilesWidth, uint32_t numTilesDepth,
                 uint32_t tileHeight, uint32_t tileWidth, uint32_t tileDepth, uint32_t numberChannels) {
  std::shared_ptr<fl::internal::CachedTile<int>> tile = nullptr;
  fl::internal::Cache<int> cache(
      numTileCache,
      numTilesHeight, numTilesWidth, numTilesDepth,
      tileHeight, tileWidth, tileDepth, numberChannels);

  uint32_t
      nbTilesCache = cache.nbTilesCache();

  ASSERT_THROW(cache.lockedTile(numTilesHeight + 1, 0, 0), std::runtime_error);
  ASSERT_THROW(cache.lockedTile(numTilesHeight + 1, numTilesWidth + 1, 0), std::runtime_error);
  ASSERT_THROW(cache.lockedTile(0, numTilesWidth + 1, 0), std::runtime_error);
  ASSERT_THROW(cache.lockedTile(numTilesHeight + 1, 0, numTilesDepth + 1), std::runtime_error);
  ASSERT_THROW(cache.lockedTile(numTilesHeight + 1, numTilesWidth + 1, numTilesDepth + 1), std::runtime_error);
  ASSERT_THROW(cache.lockedTile(0, numTilesWidth + 1, numTilesDepth + 1), std::runtime_error);

  tile = cache.lockedTile(0, 0, 0);
  ASSERT_EQ(tile->isNewTile(), true);
  tile->newTile(false);
  tile->unlock();
  ASSERT_EQ(cache.pool().size(), nbTilesCache - 1);
  ASSERT_EQ(cache.lru().front().get(), (cache.mapCache().at(0).at(0).at(0).get()));

  tile = cache.lockedTile(0, 0, 0);
  ASSERT_EQ(tile->isNewTile(), false);
  tile->unlock();
  ASSERT_EQ(cache.pool().size(), nbTilesCache - 1);
  ASSERT_EQ(cache.lru().front().get(), (cache.mapCache().at(0).at(0).at(0).get()));

  tile = cache.lockedTile(numTilesHeight - 1, numTilesWidth - 1, numTilesDepth - 1);
  ASSERT_EQ(tile->isNewTile(), true);
  tile->unlock();

  ASSERT_EQ(cache.pool().size(),
            (size_t) std::max((int32_t) nbTilesCache - 2, (int32_t) 0));
  ASSERT_EQ(cache.lru().front().get(),
            (cache.mapCache().at(numTilesDepth - 1).at(numTilesHeight - 1).at(numTilesWidth - 1).get()));

  uint32_t
      twoDGridSize = numTilesWidth * numTilesHeight,
      indexLayer = 0,
      indexRow = 0,
      indexColumn = 0;
  for (uint32_t alreadyUsedTiles = 1; alreadyUsedTiles < nbTilesCache; ++alreadyUsedTiles) {
    indexLayer = alreadyUsedTiles / twoDGridSize;
    indexRow = (alreadyUsedTiles % twoDGridSize) / numTilesWidth;
    indexColumn = (alreadyUsedTiles % twoDGridSize) % numTilesWidth;

    tile = cache.lockedTile(indexRow, indexColumn, indexLayer);
    ASSERT_EQ(tile->isNewTile(), true);
    tile->newTile(false);
    tile->unlock();
  }

  ASSERT_EQ(cache.pool().size(), (size_t) 0);
  tile = cache.lockedTile(0, 0, 0);
  ASSERT_EQ(tile->isNewTile(), nbTilesCache != numTilesHeight * numTilesWidth * numTilesDepth);
  tile->newTile(false);
  tile->unlock();
  ASSERT_EQ(cache.pool().size(), (size_t) 0);
  ASSERT_EQ(cache.lru().front().get(), (cache.mapCache().at(0).at(0).at(0).get()));
}
#endif //INC_3DFASTLOADER_TEST_CACHE_H
