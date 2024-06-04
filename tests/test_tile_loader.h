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

#ifndef FAST_LOADER_TEST_TILE_LOADER_H
#define FAST_LOADER_TEST_TILE_LOADER_H

#include <gtest/gtest.h>
#include "tile_loaders/virtual_file_tile_loader.h"

void testFastLoaderCustom(size_t const numberThreads,
                          std::vector<size_t> const &fullDimension, std::vector<size_t> const &tileDimension) {
  auto tl = std::make_shared<VirtualFileTileLoader>(numberThreads, fullDimension, tileDimension);
  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  options->ordered(true);
  auto fl = fl::FastLoaderGraph<fl::DefaultView<int >>(std::move(options));
  fl.executeGraph();
  fl.requestAllViews();
  fl.finishRequestingViews();
  fl.waitForTermination();
}

void testBasicFastLoader() {
  size_t const nbThreads = 1;
  std::vector<size_t> const vectorChannels{1, 2, 3};
  std::vector<size_t> fullDimension{5, 5, 5}, tileDimension{2, 2, 2};

  for (auto numberChannels : vectorChannels) {
    fullDimension.push_back(numberChannels);
    tileDimension.push_back(numberChannels);

    auto tl = std::make_shared<VirtualFileTileLoader>(nbThreads, fullDimension, tileDimension);
    auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
    options->ordered(true);
    auto fl = fl::FastLoaderGraph<fl::DefaultView<int>>(std::move(options));

    fl.executeGraph();
    fl.requestAllViews();
    fl.finishRequestingViews();

    while (auto viewVariant = fl.getBlockingResult()) {
      auto view = std::get<std::shared_ptr<fl::DefaultView<int>>>(*viewVariant);
      ASSERT_TRUE(view->fullDims() == fullDimension);
      ASSERT_TRUE(view->tileDims() == tileDimension);
      ASSERT_TRUE(view->viewDims() == tileDimension);

      auto realDataDimension = view->viewRealDataDims();
      auto startPosition = view->globalPositionCentralTile();

      auto a = std::accumulate(tileDimension.cbegin() + 1, tileDimension.cend(), (size_t) 1, std::multiplies<>());
      auto b = std::accumulate(tileDimension.cbegin() + 2, tileDimension.cend(), (size_t) 1, std::multiplies<>());
      auto c = tileDimension.back();

      for (size_t realDataDim0 = 0; realDataDim0 < realDataDimension.at(0); ++realDataDim0) {
        for (size_t realDataDim1 = 0; realDataDim1 < realDataDimension.at(1); ++realDataDim1) {
          for (size_t realDataDim2 = 0; realDataDim2 < realDataDimension.at(2); ++realDataDim2) {
            for (size_t realDataDim3 = 0; realDataDim3 < realDataDimension.at(3); ++realDataDim3) {
              ASSERT_EQ(
                  view->viewOrigin()[realDataDim0 * a + realDataDim1 * b + realDataDim2 * c + realDataDim3],
                  (int) ((startPosition.at(0) + realDataDim0) * 1000
                      + (startPosition.at(1) + realDataDim1) * 100
                      + (startPosition.at(2) + realDataDim2) * 10
                      + (startPosition.at(3) + realDataDim3))
              );
            }
          }
        }
      }

      view->returnToMemoryManager();
    }

    fl.waitForTermination();
    fullDimension.pop_back();
    tileDimension.pop_back();
  }
}

void testViewWithRadiusConstant() {
  size_t const nbThreads = 1;
  std::vector<size_t> const vectorChannels{1, 2, 3};
  std::vector<size_t> fullDimension{9, 5, 3}, tileDimension{6, 4, 2};
  std::vector<size_t> const radii{0, 1, 2, 3};

  for (auto numberChannels : vectorChannels) {
    fullDimension.push_back(numberChannels);
    tileDimension.push_back(numberChannels);

    for (const auto &radius : radii) {
      std::vector<size_t> viewDimension, fullDimensionWithRadius;
      std::transform(
          tileDimension.cbegin(), tileDimension.cend(),
          std::back_insert_iterator<std::vector<size_t>>(viewDimension),
          [&radius](auto const &tileSize) { return tileSize + 2 * radius; }
      );
      std::transform(
          fullDimension.cbegin(), fullDimension.cend(),
          std::back_insert_iterator<std::vector<size_t>>(fullDimensionWithRadius),
          [&radius](auto const &dimension) { return dimension + 2 * radius; }
      );

      std::shared_ptr<std::vector<int>> fileTruth = std::make_shared<std::vector<int>>(
          std::accumulate(fullDimensionWithRadius.cbegin(), fullDimensionWithRadius.cend(), 1, std::multiplies<>()), 0
      );

      for (size_t dim0FileTruth = 0; dim0FileTruth < fullDimension.at(0); ++dim0FileTruth) {
        for (size_t dim1FileTruth = 0; dim1FileTruth < fullDimension.at(1); ++dim1FileTruth) {
          for (size_t dim2FileTruth = 0; dim2FileTruth < fullDimension.at(2); ++dim2FileTruth) {
            for (size_t dim3FileTruth = 0; dim3FileTruth < fullDimension.at(3); ++dim3FileTruth) {
              fileTruth->at(
                  (dim0FileTruth + radius) * std::accumulate(fullDimensionWithRadius.cbegin() + 1,
                                                             fullDimensionWithRadius.cend(),
                                                             (size_t) 1,
                                                             std::multiplies<>())
                      + (dim1FileTruth + radius) * std::accumulate(fullDimensionWithRadius.cbegin() + 2,
                                                                   fullDimensionWithRadius.cend(),
                                                                   (size_t) 1,
                                                                   std::multiplies<>())
                      + (dim2FileTruth + radius) * std::accumulate(fullDimensionWithRadius.cbegin() + 3,
                                                                   fullDimensionWithRadius.cend(),
                                                                   (size_t) 1,
                                                                   std::multiplies<>())
                      + (dim3FileTruth + radius) * std::accumulate(fullDimensionWithRadius.cbegin() + 4,
                                                                   fullDimensionWithRadius.cend(),
                                                                   (size_t) 1,
                                                                   std::multiplies<>())
              ) = (int) (dim0FileTruth * 1000 + dim1FileTruth * 100 + dim2FileTruth * 10 + dim3FileTruth);
            }
          }
        }
      }

      size_t viewSize = std::accumulate(viewDimension.cbegin(), viewDimension.cend(), (size_t) 1, std::multiplies<>());

      auto tl = std::make_shared<VirtualFileTileLoader>(nbThreads, fullDimension, tileDimension);
      auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
      options->radius(radius);
      options->ordered(true);
      options->borderCreatorConstant(0);
      auto fl = fl::FastLoaderGraph<fl::DefaultView<int >>(std::move(options));
      fl.executeGraph();
      fl.requestAllViews();
      fl.finishRequestingViews();

      while (auto viewVariant = fl.getBlockingResult()) {
        auto view = std::get<std::shared_ptr<fl::DefaultView<int>>>(*viewVariant);
        std::vector<int> groundTruth(viewSize, 0);
        auto const &index = view->indexCentralTile();

        for (size_t pos0 = index.at(0) * tileDimension.at(0);
             pos0 < std::min((index.at(0) + 1) * tileDimension.at(0) + 2 * radius, fullDimensionWithRadius.at(0));
             ++pos0) {
          for (size_t pos1 = index.at(1) * tileDimension.at(1);
               pos1 < std::min((index.at(1) + 1) * tileDimension.at(1) + 2 * radius, fullDimensionWithRadius.at(1));
               ++pos1) {
            for (size_t pos2 = index.at(2) * tileDimension.at(2);
                 pos2 < std::min((index.at(2) + 1) * tileDimension.at(2) + 2 * radius, fullDimensionWithRadius.at(2));
                 ++pos2) {
              for (size_t pos3 = index.at(3) * tileDimension.at(3);
                   pos3 < std::min((index.at(3) + 1) * tileDimension.at(3) + 2 * radius, fullDimensionWithRadius.at(3));
                   ++pos3) {
                groundTruth.at(
                    (pos0 - index.at(0) * tileDimension.at(0))
                        * std::accumulate(viewDimension.cbegin() + 1, viewDimension.cend(), (size_t) 1, std::multiplies<>())
                        + (pos1 - index.at(1) * tileDimension.at(1))
                            * std::accumulate(viewDimension.cbegin() + 2, viewDimension.cend(), (size_t) 1, std::multiplies<>())
                        + (pos2 - index.at(2) * tileDimension.at(2))
                            * std::accumulate(viewDimension.cbegin() + 3, viewDimension.cend(), (size_t) 1, std::multiplies<>())
                        + (pos3 - index.at(3) * tileDimension.at(3))
                            * std::accumulate(viewDimension.cbegin() + 4, viewDimension.cend(),
                                              (size_t) 1, std::multiplies<>())

                ) = fileTruth->at(
                    pos0 * std::accumulate(fullDimensionWithRadius.cbegin() + 1, fullDimensionWithRadius.cend(),
                                           (size_t) 1, std::multiplies<>())
                        + pos1 * std::accumulate(fullDimensionWithRadius.cbegin() + 2, fullDimensionWithRadius.cend(),
                                                 (size_t) 1, std::multiplies<>())
                        + pos2 * std::accumulate(fullDimensionWithRadius.cbegin() + 3, fullDimensionWithRadius.cend(),
                                                 (size_t) 1, std::multiplies<>())
                        + pos3 * std::accumulate(fullDimensionWithRadius.cbegin() + 4, fullDimensionWithRadius.cend(),
                                                 (size_t) 1, std::multiplies<>())
                );
              }
            }
          }
        }

        ASSERT_TRUE(view->fullDims() == fullDimension);
        ASSERT_TRUE(view->tileDims() == tileDimension);
        ASSERT_TRUE(
            std::accumulate(view->viewDims().cbegin(), view->viewDims().cend(), (size_t) 1, std::multiplies<>())
                == viewSize);

        ASSERT_TRUE(std::equal(groundTruth.cbegin(), groundTruth.cend(), view->viewOrigin()));
        view->returnToMemoryManager();
      }
      fl.waitForTermination();
    }
    fullDimension.pop_back();
    tileDimension.pop_back();
  }
}

#endif //FAST_LOADER_TEST_TILE_LOADER_H
