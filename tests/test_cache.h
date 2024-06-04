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

#ifndef FAST_LOADER_TEST_CACHE_H
#define FAST_LOADER_TEST_CACHE_H

#include <gtest/gtest.h>
#include <cmath>
#include <utility>
#include "../fast_loader/core/cache.h"

void cacheInitialization(std::vector<size_t> const &cacheDimension,
                         size_t nbTilesCache,
                         std::vector<size_t> const &tileDimension) {
  fl::internal::Cache<int> cache(cacheDimension, nbTilesCache, tileDimension);
  ASSERT_EQ(cache.hit(), (size_t) 0);
  ASSERT_EQ(cache.miss(), (size_t) 0);
  if (nbTilesCache == 0) { nbTilesCache = 18; }

  size_t productDimension = std::accumulate(cacheDimension.begin(), cacheDimension.end(), (size_t) 1, std::multiplies<>());

  if (productDimension < nbTilesCache) { ASSERT_EQ(cache.nbTilesCache(), productDimension); }
  else { ASSERT_EQ(cache.nbTilesCache(), nbTilesCache); }

  ASSERT_EQ(cache.nbTilesCache(), cache.pool().size());
  auto const &mapCache = cache.mapCache();

  ASSERT_TRUE(std::all_of(mapCache.cbegin(), mapCache.cend(), [](auto const &elem) { return elem == nullptr; }));
  ASSERT_EQ(cache.lru().size(), (size_t) 0);
}

void getNewTiles(std::vector<size_t> const &cacheDimension, size_t nbTilesCache, std::vector<size_t> const &tileDimension) {
  std::shared_ptr<fl::internal::CachedTile<int>> tile = nullptr;
  fl::internal::Cache<int> cache(cacheDimension, nbTilesCache, tileDimension);

  nbTilesCache = cache.nbTilesCache();
  std::vector<size_t> testIndex;

  for (size_t dimension = 0; dimension < cacheDimension.size(); ++dimension) {
    testIndex = cacheDimension;
    std::transform(testIndex.cbegin(), testIndex.cend(), testIndex.begin(), [](auto const &elem) { return elem - 1; });
    ++(testIndex.at(dimension));
    ASSERT_THROW(cache.lockedTile(testIndex), std::runtime_error);
  }


  tile = cache.lockedTile(std::vector<size_t>(cacheDimension.size()));
  ASSERT_EQ(tile->newTile(), true);
  tile->newTile(false);
  tile->unlock();
  ASSERT_EQ(cache.pool().size(), nbTilesCache - 1);
  ASSERT_EQ(cache.lru().front().get(), (cache.mapCache().at(0).get()));

  tile = cache.lockedTile(std::vector<size_t>(cacheDimension.size()));
  ASSERT_EQ(tile->newTile(), false);
  tile->unlock();
  ASSERT_EQ(cache.pool().size(), nbTilesCache - 1);
  ASSERT_EQ(cache.lru().front().get(), (cache.mapCache().at(0).get()));


  ASSERT_EQ(tile->newTile(), true);
  tile->unlock();

  ASSERT_EQ(cache.pool().size(), (size_t) std::max((int32_t) nbTilesCache - 2, (int32_t) 0));
  ASSERT_EQ(cache.lru().front().get(), (cache.mapCache().at(
      std::inner_product(
          testIndex.cbegin(), testIndex.cend(), cacheDimension.cbegin(),
          (size_t) 1, std::plus<>(), std::multiplies<>())
      ).get()));
}

void testCache(){
  std::vector<size_t>
      numberDimensions{1, 3, 5, 7},
      numberTilesToCache{0, 1, 10},
      numberTiles{5, 10};

  std::random_device rd;
  std::uniform_int_distribution<size_t> distribution(1, 20);
  auto gen = [&distribution, &rd](){ return distribution(rd); };

  std::vector<size_t> tileSize;

  for (auto const &numberDimension : numberDimensions) {
    tileSize.clear();
    tileSize.resize(numberDimension);
    for (auto const &numberTile : numberTiles) {
      for (auto const &numberTileToCache : numberTilesToCache) {
        std::generate(tileSize.begin(), tileSize.end(), gen);

        ASSERT_NO_FATAL_FAILURE(
            cacheInitialization(
                std::vector<size_t>(numberDimension, numberTile),
                numberTileToCache,
                tileSize
            )
        );
        ASSERT_NO_FATAL_FAILURE(
            cacheInitialization(
                std::vector<size_t>(numberDimension, numberTile),
                numberTileToCache,
                tileSize
            )
        );
      }
    }
  }
}

#endif //FAST_LOADER_TEST_CACHE_H
