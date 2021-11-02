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
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/30/20.
//

#ifndef INC_3DFASTLOADER_TEST_ADAPTIVE_TILE_LOADER_H
#define INC_3DFASTLOADER_TEST_ADAPTIVE_TILE_LOADER_H

#include <gtest/gtest.h>

#include "tile_loaders/virtual_file_tile_loader.h"
#include "tile_loaders/virtual_file_adaptive_tile_loader.h"

std::shared_ptr<fl::AdaptiveFastLoaderGraph<fl::DefaultView<int>>> adaptiveFL(
    uint32_t fileSize, uint32_t logicalSize, uint32_t physicalTile) {

  auto tl = std::make_shared<VirtualFileTileLoader>(
      1,
      fileSize, fileSize, fileSize,
      physicalTile, physicalTile, physicalTile);
  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  options->radius(2);
  options->ordered(true);
  return std::make_shared<fl::AdaptiveFastLoaderGraph<fl::DefaultView<int>>>(
      std::move(options),
      std::vector<uint32_t>(1, logicalSize), std::vector<uint32_t>(1, logicalSize), std::vector<uint32_t>(1, logicalSize));
}

std::shared_ptr<fl::FastLoaderGraph<fl::DefaultView<int>>> basicFL(uint32_t fileSize, uint32_t physicalTile) {
  auto tl = std::make_shared<VirtualFileTileLoader>(
      1,
      fileSize, fileSize, fileSize,
      physicalTile, physicalTile, physicalTile);
  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  options->radius(2);
  options->ordered(true);
  return std::make_shared<fl::FastLoaderGraph<fl::DefaultView<int>>>(std::move(options));
}

void testAdaptiveTL() {
  uint32_t const fileSize = 5;
  std::vector<uint32_t> const tileSize{1, 2, 3, 5};

  for (auto physicalTile : tileSize) {
    for (auto logicalTile : tileSize) {
      std::shared_ptr<fl::FastLoaderGraph<fl::DefaultView<int>>>
          basicGraph = basicFL(fileSize, logicalTile),
          adaptiveGraph = adaptiveFL(fileSize, logicalTile, physicalTile);

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

#endif //INC_3DFASTLOADER_TEST_ADAPTIVE_TILE_LOADER_H
