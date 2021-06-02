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
// Created by Bardakoff, Alexandre (IntlAssoc) on 10/29/20.
//

#ifndef INC_3DFASTLOADER_TEST_REQUESTS_H
#define INC_3DFASTLOADER_TEST_REQUESTS_H

#include <gtest/gtest.h>
#include "tile_loaders/virtual_file_tile_loader.h"
#include "../fast_loader/internal/traversal/naive_traversal.h"

void basicRequest(){
  uint32_t
      fileSize = 5,
      physicalTile = 1;
  auto tl = std::make_shared<VirtualFileTileLoader>(
      1,
      fileSize, fileSize, fileSize,
      physicalTile, physicalTile, physicalTile);
  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  options->radius(0);
  auto fl = fl::FastLoaderGraph<fl::DefaultView<int>>(std::move(options));
  fl.executeGraph();
  fl.requestAllViews(0);
  fl.finishRequestingViews();

  while (auto res = fl.getBlockingResult()) {
    ASSERT_TRUE(res->originCentralTile()[0] ==
        (int)(100 * (res->tileLayerIndex() + 1) + 10 * (res->tileRowIndex() + 1) + res->tileColIndex() + 1));

    res->returnToMemoryManager();
  }
  fl.waitForTermination();
}

void testOrdering(){
  uint32_t
      fileSize = 5,
      physicalTile = 1;
  auto tl = std::make_shared<VirtualFileTileLoader>(
      1,
      fileSize, fileSize, fileSize,
      physicalTile, physicalTile, physicalTile);

  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  options->radius(1);

  auto fl = fl::FastLoaderGraph<fl::DefaultView<int>>(std::move(options));

  fl::internal::NaiveTraversal<fl::DefaultView<int>> traversal(std::static_pointer_cast<fl::AbstractTileLoader<fl::DefaultView<int>>>(tl));

  auto truth = traversal.traversal(0);
  size_t numberReceived = 0;

  fl.executeGraph();
  fl.requestAllViews(0);
  fl.finishRequestingViews();

  while (auto res = fl.getBlockingResult()) {
    ASSERT_TRUE(
        truth.at(numberReceived)[0] == res->tileRowIndex()
        && truth.at(numberReceived)[1] == res->tileColIndex()
        && truth.at(numberReceived)[2] == res->tileLayerIndex()
    );
    numberReceived ++;
    res->returnToMemoryManager();
  }
  fl.waitForTermination();

}

#endif //INC_3DFASTLOADER_TEST_REQUESTS_H
