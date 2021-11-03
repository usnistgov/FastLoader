//
// Created by Bardakoff, Alexandre (IntlAssoc) on 3/3/21.
//

#ifndef INC_3DFASTLOADER_TEST_RADIUS_H
#define INC_3DFASTLOADER_TEST_RADIUS_H

#include <gtest/gtest.h>
#include "tile_loaders/virtual_file_tile_channel_loader.h"

std::vector<int> groundTruthGenerator(
    size_t indexRow, size_t indexColumn, size_t indexLayer,
    size_t radiusHeight, size_t radiusWidth, size_t radiusDepth,
    size_t numberChannel) {

  static std::array<int, 125> const file =
      {111, 112, 113, 114, 115, 121, 122, 123, 124, 125, 131, 132, 133, 134, 135, 141, 142, 143, 144, 145, 151, 152,
       153, 154, 155, 211, 212, 213, 214, 215, 221, 222, 223, 224, 225, 231, 232, 233, 234, 235, 241, 242, 243, 244,
       245, 251, 252, 253, 254, 255, 311, 312, 313, 314, 315, 321, 322, 323, 324, 325, 331, 332, 333, 334, 335, 341,
       342, 343, 344, 345, 351, 352, 353, 354, 355, 411, 412, 413, 414, 415, 421, 422, 423, 424, 425, 431, 432, 433,
       434, 435, 441, 442, 443, 444, 445, 451, 452, 453, 454, 455, 511, 512, 513, 514, 515, 521, 522, 523, 524, 525,
       531, 532, 533, 534, 535, 541, 542, 543, 544, 545, 551, 552, 553, 554, 555};

  static std::array<std::array<uint8_t, 6>, 3> const // Access [Index][Radius]
  minFile = {
      {
          {{0, 0, 0, 0, 0, 0}},
          {{2, 1, 0, 0, 0, 0}},
          {{4, 3, 2, 1, 0, 0}}
      }
  },
      minView = {
      {
          {{0, 1, 2, 3, 4, 5}},
          {{0, 0, 0, 1, 2, 3}},
          {{0, 0, 0, 0, 0, 1}}
      }
  },
      size = {
      {
          {{2, 3, 4, 5, 5, 5}},
          {{2, 4, 5, 5, 5, 5}},
          {{1, 2, 3, 4, 5, 5}}
      }
  };

  size_t const twoDimensionView = (2 + 2 * radiusWidth) * (2 + 2 * radiusHeight);

  std::vector<int>
      truth((2 + 2 * radiusDepth) * (2 + 2 * radiusWidth) * (2 + 2 * radiusHeight) * numberChannel, 0);

  for (size_t layer = 0; layer < size[indexLayer][radiusDepth]; ++layer) {
    for (size_t row = 0; row < size[indexRow][radiusHeight]; ++row) {
      for (size_t col = 0; col < size[indexColumn][radiusWidth]; ++col) {
        for (size_t channel = 0; channel < numberChannel; ++channel) {
          truth.at(
              ((layer + minView[indexLayer][radiusDepth]) * (twoDimensionView)
                  + (row + minView[indexRow][radiusHeight]) * (2 + 2 * radiusWidth)
                  + col + minView[indexColumn][radiusWidth]) * numberChannel + channel
          ) = file[
              (layer + minFile[indexLayer][radiusDepth]) * 25
                  + (row + minFile[indexRow][radiusHeight]) * 5
                  + col + minFile[indexColumn][radiusWidth]];
        }
      }
    }
  }

  return truth;
}

void testBasicRadius() {
  size_t numberThreads = 5;
  uint16_t const fileHeight = 5;
  uint16_t const fileWidth = 5;
  uint16_t const fileDepth = 5;
  uint16_t const tileHeight = 2;
  uint16_t const tileWidth = 2;
  uint16_t const tileDepth = 2;

  std::vector<size_t> const vectorChannels{1, 2, 3};
  std::vector<size_t> const radii{0, 1, 2, 3, 4, 5};

  for (auto radiusHeight : radii) {
    for (auto radiusWidth : radii) {
      for (auto radiusDepth : radii) {
        for (auto numberChannels : vectorChannels) {

          auto tl = std::make_shared<VirtualFileTileChannelLoader>(
              numberThreads,
              fileHeight, fileWidth, fileDepth,
              tileHeight, tileWidth, tileDepth,
              numberChannels);

          auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
          options->radius(radiusDepth, radiusHeight, radiusWidth);
          options->ordered(true);
          options->borderCreatorConstant(0);
          auto fl = fl::FastLoaderGraph<fl::DefaultView<int >>(std::move(options));
          fl.executeGraph();
          fl.requestAllViews();
          fl.finishRequestingViews();

          while (auto view = fl.getBlockingResult()) {
            ASSERT_EQ(view->viewHeight(), view->tileHeight() + 2 * radiusHeight);
            ASSERT_EQ(view->viewWidth(), view->tileWidth() + 2 * radiusWidth);
            ASSERT_EQ(view->viewDepth(), view->tileDepth() + 2 * radiusDepth);
            auto truth = groundTruthGenerator(
                view->tileRowIndex(), view->tileColIndex(), view->tileLayerIndex(),
                radiusHeight, radiusWidth, radiusDepth,
                numberChannels
            );
            ASSERT_TRUE(std::equal(truth.cbegin(), truth.cend(), view->viewOrigin()));

            view->returnToMemoryManager();
          }
          fl.waitForTermination();
        }
      }
    }
  }
}

std::shared_ptr<fl::FastLoaderGraph<fl::DefaultView<int>>> basicFLChannelsRadii(
    size_t fileSize, size_t physicalTile, size_t numberChannels,
    size_t radiusHeight, size_t radiusWidth, size_t radiusDepth, bool useReplicate = false) {
  auto tl = std::make_shared<VirtualFileTileChannelLoader>(
      1,
      fileSize, fileSize, fileSize,
      physicalTile, physicalTile, physicalTile, numberChannels);
  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  if (useReplicate){
    options->borderCreator(fl::FillingType::REPLICATE);
  }
  options->radius(radiusDepth, radiusHeight, radiusWidth);
  options->ordered(true);
  return std::make_shared<fl::FastLoaderGraph<fl::DefaultView<int>>>(std::move(options));
}

std::shared_ptr<fl::FastLoaderGraph<fl::DefaultView<int>>> adaptiveFLChannelsRadii(
    size_t fileSize, size_t logicalSize, size_t physicalTile, size_t numberChannels,
    size_t radiusHeight, size_t radiusWidth, size_t radiusDepth, bool useReplicate = false) {
  auto tl = std::make_shared<VirtualFileAdaptiveTileChannelLoader>(
      1,
      fileSize, fileSize, fileSize,
      std::vector<size_t>{logicalSize}, std::vector<size_t>{logicalSize}, std::vector<size_t>{logicalSize},
      std::vector<size_t>{5},
      physicalTile, physicalTile, physicalTile, numberChannels);
  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  if (useReplicate){
    options->borderCreator(fl::FillingType::REPLICATE);
  }
  options->radius(radiusDepth, radiusHeight, radiusWidth);
  options->ordered(true);
  return std::make_shared<fl::FastLoaderGraph<fl::DefaultView<int>>>(std::move(options));
}

void testAdaptiveTLChannelsRadii(bool useReplicate = false) {
  size_t const fileSize = 5;
  std::vector<size_t> const tileSize{1, 2, 5};
  std::vector<size_t> const vNumberChannels{1, 5};
  std::vector<size_t> const radii{0, 1, 2, 3, 4, 5};

  for (auto numberChannels: vNumberChannels) {
    for (auto radiusHeight : radii) {
      for (auto radiusWidth : radii) {
        for (auto radiusDepth : radii) {
          for (auto physicalTile : tileSize) {
            for (auto logicalTile : tileSize) {
              std::shared_ptr<fl::FastLoaderGraph<fl::DefaultView<int>>>
                  basicGraph = basicFLChannelsRadii(
                  fileSize, logicalTile, numberChannels, radiusHeight, radiusWidth, radiusDepth, useReplicate),
                  adaptiveGraph = adaptiveFLChannelsRadii(
                  fileSize, logicalTile, physicalTile, numberChannels, radiusHeight, radiusWidth, radiusDepth, useReplicate);

              basicGraph->executeGraph();
              basicGraph->requestAllViews(0);
              basicGraph->finishRequestingViews();

              adaptiveGraph->executeGraph();
              adaptiveGraph->requestAllViews(0);
              adaptiveGraph->finishRequestingViews();

              while (auto basicView = basicGraph->getBlockingResult()) {
                auto adaptiveView = adaptiveGraph->getBlockingResult();

                ASSERT_TRUE(std::equal(
                    basicView->viewOrigin(),
                    basicView->viewOrigin() + basicView->viewWidth() * basicView->viewDepth() * basicView->viewHeight(),
                    adaptiveView->viewOrigin()
                ));

                basicView->returnToMemoryManager();
                adaptiveView->returnToMemoryManager();
              }
              basicGraph->waitForTermination();
              adaptiveGraph->waitForTermination();
            }
          }
        }
      }
    }
  }

}

#endif //INC_3DFASTLOADER_TEST_RADIUS_H
