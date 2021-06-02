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
// Created by anb22 on 2/13/20.
//

#ifndef FASTLOADER_BLOCK_RECURSIVE_TRAVERSAL_H
#define FASTLOADER_BLOCK_RECURSIVE_TRAVERSAL_H
// TODO: Update for 3D
//#include "../../api/abstract_traversal.h"
///// @brief FastLoader namespace
//namespace fl {
//
///// @brief FastLoader internal namespace
//namespace internal {
//template<class ViewType>
//class BlockRecursiveTraversal : public AbstractTraversal<ViewType> {
//
// private:
//  uint32_t depth_;
//  std::vector<std::pair<uint32_t, uint32_t>> tilesPerLevel_;
//
// public:
//
//  BlockRecursiveTraversal(std::shared_ptr<AbstractTileLoader < ViewType>>
//  tl,
//  float downscaleFactor = 2
//  )
//  : AbstractTraversal<ViewType>("Block recursive", tl) {
//    auto numTileRow = static_cast<uint32_t>(std::ceil((double) tl->fullHeight(0) / tl->tileHeight(0)));
//    auto numTileCol = static_cast<uint32_t>(std::ceil((double) tl->fullWidth(0) / tl->tileWidth(0)));
//    auto maxDim = std::max(numTileCol, numTileRow);
//    depth_ = ceil(log2(maxDim) / log2(downscaleFactor));
//    for (uint32_t l = 0; l <= depth_; l++) {
//      tilesPerLevel_.push_back({numTileRow, numTileCol});
//      numTileCol = static_cast<uint32_t>(ceil((double) numTileCol / downscaleFactor));
//      numTileRow = static_cast<uint32_t>(ceil((double) numTileRow / downscaleFactor));
//    }
//  }
//
//  virtual ~BlockRecursiveTraversal() = default;
//
//  std::vector<std::pair<uint32_t, uint32_t>> traversal([[maybe_unused]] uint32_t level) const override {
//    std::vector<std::pair<uint32_t, uint32_t >> traversal;
//    blockTraversal(0, 0, depth_ - level, traversal);
//    return traversal;
//  }
//
// private:
//
//  bool isValid(uint32_t row, uint32_t col, uint32_t depth) const {
//    return (row < tilesPerLevel_[depth].first && col < tilesPerLevel_[depth].second);
//  }
//
//  void blockTraversal(uint32_t row,
//                      uint32_t col,
//                      uint32_t depth,
//                      std::vector<std::pair<uint32_t, uint32_t >> &traversal) const {
//    if (!isValid(row, col, depth)) {
//      return;
//    }
//
//    if (depth == 0) {
//      traversal.push_back({row, col});
//    } else {
//      blockTraversal(2 * row, 2 * col, depth - 1, traversal);
//      blockTraversal(2 * row, 2 * col + 1, depth - 1, traversal);
//      blockTraversal(2 * row + 1, 2 * col, depth - 1, traversal);
//      blockTraversal(2 * row + 1, 2 * col + 1, depth - 1, traversal);
//    }
//  }
//};
//}
//}

#endif //FASTLOADER_BLOCK_RECURSIVE_TRAVERSAL_H
