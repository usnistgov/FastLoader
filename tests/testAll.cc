//
// Created by anb22 on 11/21/19.
//
#include <gtest/gtest.h>

#include <random>
#include "test_cache.h"
#include "test_filling.h"
#include "test_requests.h"
#include "test_adaptive_tile_loader.h"
#include "test_channels.h"
#include "test_radius.h"

TEST(TEST_3DFL, TEST_CACHE) {
  std::vector<uint32_t>
      numbersTilesToCache{0, 1, 10, 100},
      numbersTiles{5, 10};

  std::random_device rd;
  std::uniform_int_distribution<uint32_t> distribution(1, 20);

  for (auto numberTilesToCache : numbersTilesToCache) {
    for (auto numTilesHeight : numbersTiles) {
      for (auto numTilesWidth : numbersTiles) {
        for (auto numTilesDepth : numbersTiles) {
          ASSERT_NO_FATAL_FAILURE(
              cacheInitialization(
                  numberTilesToCache,
                  numTilesHeight, numTilesWidth, numTilesDepth,
                  distribution(rd), distribution(rd), distribution(rd), distribution(rd)
              )
          );
          ASSERT_NO_FATAL_FAILURE(
              getNewTiles(
                  numberTilesToCache,
                  numTilesHeight, numTilesWidth, numTilesDepth,
                  distribution(rd), distribution(rd), distribution(rd), distribution(rd)
              )
          );
        }
      }
    }
  }
}

TEST(TEST_3DFL, TEST_FILLING){
  testFillingConstant();
  testFillingReplicate();
}

TEST(TEST_3DFL, TEST_REQUEST){
  ASSERT_NO_THROW(basicRequest());
  ASSERT_NO_THROW(testOrdering());
}

TEST(TEST_3DFL, TEST_ADAPTIVE_TL){
  ASSERT_NO_THROW(testAdaptiveTL());
}

TEST(TEST_3DFL, TEST_CHANNELS){
  ASSERT_THROW(testChannelZero(), std::runtime_error);
  ASSERT_NO_THROW(testBasicChannel());
  ASSERT_NO_THROW(testViewWithRadiusConstant());
  ASSERT_NO_THROW(testViewWithRadiusReplicateFilling());
  ASSERT_NO_THROW(testAdaptiveTLChannels());
}

TEST(TEST_3DFL, TEST_RADIUS){
  ASSERT_NO_THROW(testBasicRadius());
  ASSERT_NO_THROW(testAdaptiveTLChannelsRadii(false));
  ASSERT_NO_THROW(testAdaptiveTLChannelsRadii(true));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}