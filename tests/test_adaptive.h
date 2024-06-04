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

#ifndef FAST_LOADER_TEST_ADAPTIVE_H
#define FAST_LOADER_TEST_ADAPTIVE_H

#include "tile_loaders/virtual_file_tile_loader.h"

auto createFL(size_t const nbDimensions, size_t const fullSize, size_t const tileSize, size_t const radius) {
  std::vector<size_t> const
      fs(nbDimensions, fullSize),
      ts(nbDimensions, tileSize);

  auto tl = std::make_shared<VirtualFileTileLoader>(1, fs, ts);
  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  options->radius(radius);
  options->ordered(true);
  options->viewAvailable({1});
  return fl::FastLoaderGraph<fl::DefaultView<int>>(std::move(options));
}

auto createAdaptiveFL(size_t const nbDimensions,
                      size_t const fullSize,
                      size_t const tileSize,
                      size_t const physicalTileSize,
                      size_t const radius) {
  std::vector<size_t> const
      fs(nbDimensions, fullSize),
      pts(nbDimensions, physicalTileSize),
      ts(nbDimensions, tileSize);

  auto tl = std::make_shared<VirtualFileTileLoader>(1, fs, pts);
  auto options = std::make_unique<fl::FastLoaderConfiguration<fl::DefaultView<int>>>(tl);
  options->radius(radius);
  options->ordered(true);
  options->viewAvailable({1});
  return fl::AdaptiveFastLoaderGraph<fl::DefaultView<int>>(std::move(options), {ts});
}


void testAdaptiveFL() {
  std::vector<size_t> const
      nbDimensions{1, 2, 3},
      fullSize{2, 5, 9},
      tileSize{1, 2},
      physicalTileSize{1, 2},
      radius{0, 1, 2};
  for (auto dim : nbDimensions) {
    for (auto fs : fullSize) {
      for (auto ts : tileSize) {
        for (auto pts : physicalTileSize) {
          for (auto r : radius) {
            auto fl = createFL(dim, fs, ts, r);
            auto afl = createAdaptiveFL(dim, fs, ts, pts, r);

            ASSERT_EQ(fl.nbTilesDims(0), afl.nbTilesDims(0));

            fl.executeGraph();
            fl.createDotFile("fl.dot", hh::ColorScheme::EXECUTION, hh::StructureOptions::QUEUE, hh::InputOptions::GATHERED);
            fl.requestAllViews(0);
            fl.finishRequestingViews();

            afl.executeGraph();

            afl.createDotFile("afl.dot", hh::ColorScheme::EXECUTION, hh::StructureOptions::QUEUE, hh::InputOptions::GATHERED);
            afl.requestAllViews(0);
            afl.finishRequestingViews();

            while (auto viewVariantFL = fl.getBlockingResult()) {
              auto flRes = std::get<std::shared_ptr<fl::DefaultView<int>>>(*viewVariantFL);
              auto indexFL = flRes->indexCentralTile();

              auto viewVariantAFL = afl.getBlockingResult();
              auto aflRes = std::get<std::shared_ptr<fl::DefaultView<int>>>(*viewVariantAFL);
              auto indexAFL = aflRes->indexCentralTile();

              ASSERT_EQ(indexAFL, indexFL);

              ASSERT_TRUE(std::equal(
                  flRes->viewOrigin(),
                  flRes->viewOrigin()
                      + std::accumulate(flRes->viewDims().cbegin(),
                                        flRes->viewDims().cend(),
                                        (long) 1,
                                        std::multiplies<>()),
                  aflRes->viewOrigin()));

              flRes->returnToMemoryManager();
              aflRes->returnToMemoryManager();
            }

            fl.waitForTermination();
            afl.waitForTermination();
          }
        }
      }
    }
  }

}

#endif //FAST_LOADER_TEST_ADAPTIVE_H
