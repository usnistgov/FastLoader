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

#ifndef FASTLOADER_HILBERT_traversalH
#define FASTLOADER_HILBERT_traversalH
// TODO: Update for 3D
//#include "../../api/abstract_traversal.h"
///// @brief FastLoader namespace
//namespace fl {
//
///// @brief FastLoader internal namespace
//namespace internal {
//template<class ViewType>
//class HilbertTraversal : public AbstractTraversal<ViewType> {
// public:
//  explicit HilbertTraversal(std::shared_ptr<AbstractTileLoader < ViewType>>
//  tl)
//  : AbstractTraversal<ViewType>("Hilbert", tl) {}
//
//  virtual ~HilbertTraversal() = default;
//
//  [[nodiscard]] std::vector<std::pair<uint32_t, uint32_t>> traversal(uint32_t level) const override {
//    std::vector<std::pair<uint32_t, uint32_t>> traversal;
//
//    uint32_t
//        minX = 0,
//        minY = 0,
//        pos = 0,
//        rowTemp = 0,
//        colTemp = 0,
//        minSize = (uint32_t) pow(2,
//                                 (uint32_t) std::min(log2(this->numberTileHeight(level)),
//                                                     log2(this->numberTileWidth(level))));
//
//    traversal.resize(this->numberTileHeight(level) * this->numberTileWidth(level),
//                     std::pair<uint32_t, uint32_t>(0, 0));
//
//    for (uint32_t d = 0; d < minSize * minSize; ++d) {
//      d2xy(minSize, pos, rowTemp, colTemp);
//      traversal[pos].first = rowTemp;
//      traversal[pos].second = colTemp;
//      if (traversal[pos].first > minX) minX = traversal[pos].first;
//      if (traversal[pos].second > minY) minY = traversal[pos].second;
//      pos += 1;
//    }
//
//    // Filling empty case
//    for (uint32_t row = 0; row < this->numberTileHeight(level); ++row) {
//      if (row % 2 == 0) {
//        for (uint32_t col = minY + 1; col < this->numberTileWidth(level); ++col) {
//          traversal[pos].first = row;
//          traversal[pos].second = col;
//          pos += 1;
//        }
//      } else {
//        for (uint32_t col = this->numberTileWidth(level) - 1; col > minY; --col) {
//          traversal[pos].first = row;
//          traversal[pos].second = col;
//          pos += 1;
//        }
//      }
//    }
//    for (uint32_t i = minY + 1; i < this->numberTileHeight(level); ++i) {
//      if (i % 2 == 0) {
//        for (uint32_t j = 0; j < minX + 1; ++j) {
//          traversal[pos].first = i;
//          traversal[pos].second = j;
//          pos += 1;
//        }
//      } else {
//        for (int32_t j = minX; j >= 0; --j) {
//          traversal[pos].first = i;
//          traversal[pos].second = (uint32_t) j;
//          pos += 1;
//        }
//      }
//    }
//
//    return traversal;
//  }
//
// private:
//
//  /// \brief Function used in Hilbert _traversal
//  /// \param n Square region size
//  /// \param x Column
//  /// \param y Row
//  /// \param rx Column rotational
//  /// \param ry Row rotational
//  static void rot(uint32_t n, uint32_t &x, uint32_t &y, uint32_t rx, uint32_t ry) {
//    if (ry == 0) {
//      if (rx == 1) {
//        x = n - 1 - x;
//        y = n - 1 - y;
//      }
//      std::swap(x, y);
//    }
//  }
//
//  /// \brief Function used in the hilbert algorithm
//  /// \param n Square region size
//  /// \param d Distance from the origin
//  /// \param x Column
//  /// \param y Row
//  static void d2xy(uint32_t n, uint32_t d, uint32_t &x, uint32_t &y) {
//    uint32_t rx, ry, s, t = d;
//    x = y = 0;
//    for (s = 1; s < n; s *= 2) {
//      rx = (uint32_t) 1 & (t / (uint32_t) 2);
//      ry = (uint32_t) 1 & (t ^ rx);
//      rot(s, x, y, rx, ry);
//      x += s * rx;
//      y += s * ry;
//      t /= 4;
//    }
//  }
//};
//}
//}
#endif //FASTLOADER_HILBERT_traversalH
