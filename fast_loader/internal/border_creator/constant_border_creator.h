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
// Created by anb22 on 11/13/19.
//

#ifndef FASTLOADER_CONSTANT_BORDER_CREATOR_H
#define FASTLOADER_CONSTANT_BORDER_CREATOR_H

#include "../../api/abstract_border_creator.h"
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief Border creator that will fill the border with a value
/// @details Given a domain: \n
/// | 0 	| 1 	| 2 	|\n
/// | 3 	| 4 	| 5 	|\n
/// | 6 	| 7 	| 8 	|\n
///
/// The border creation will fill as follows:\n
/// | v  	| v  	| v 	| v  	| v  	| v  	| v  	|\n
/// | v  	| v  	| v 	| v 	| v 	| v  	| v  	|\n
/// | v  	| v  	| 0 	| 1 	| 2 	| v  	| v  	|\n
/// | v  	| v  	| 3 	| 4 	| 5 	| v     | v  	|\n
/// | v  	| v  	| 6 	| 7  	| 8 	| v  	| v  	|\n
/// | v  	| v  	| v  	| v  	| v  	| v  	| v  	|\n
/// | v  	| v  	| v  	| v  	| v  	| v  	| v  	|\n
/// In 1D, the domain looks like: \n
/// | 0 	| 1 	| 2 	|\n
/// The copy will look like: \n
/// | v 	| v 	| 0 	| 1 	| 2 	| v 	| v 	|\n
/// @tparam ViewType Type of the view \n
template<class ViewType>
class ConstantBorderCreator : public AbstractBorderCreator<ViewType> {
 private:
  typename ViewType::data_t const value_; ///< Value to fill border
 public:

  /// @brief ConstantBorderCreator constructor, set the value to fill
  /// @param value Value to fill
  explicit ConstantBorderCreator(typename ViewType::data_t const value) : value_(value) {}

  /// @brief Generate the tile requests for the Replicate border creator, in this case do nothing
  /// @return List of tile request, empty
  std::list<std::shared_ptr<TileRequest < ViewType>>>
  tileRequestsToFillBorders([[maybe_unused]]std::shared_ptr<ViewType> const &) override { return {}; }

  /// @brief Fill the border with a value
  /// @param view AbstractView to fill
  void fillBorderWithExistingValues(std::shared_ptr<ViewType> const &view) override {
    uint32_t
        topFill = view->viewData()->topFill(),
        bottomFill = view->viewData()->bottomFill(),
        rightFill = view->viewData()->rightFill(),
        leftFill = view->viewData()->leftFill(),
        frontFill = view->viewData()->frontFill(),
        backFill = view->viewData()->backFill(),
        viewHeight = view->viewData()->viewHeight(),
        viewWidth = view->viewData()->viewWidth(),
        viewDepth = view->viewData()->viewDepth(),
        numberChannels = view->viewData()->numberChannels();
    
    auto origin = view->viewOrigin();

    // Front
    std::fill_n(origin, viewHeight * viewWidth * frontFill * numberChannels, value_);

    // Back
    std::fill_n(
        origin + ((viewDepth - backFill) * (viewHeight * viewWidth)) * numberChannels,
        viewHeight * viewWidth * backFill * numberChannels,
        value_);

    for(uint32_t layer = frontFill; layer < viewDepth - backFill; ++layer) {
      for (uint32_t row = topFill; row < viewHeight - bottomFill; ++row) {
        // L
        std::fill_n(
            origin + (layer * (viewHeight * viewWidth) + (row * viewWidth)) * numberChannels,
            leftFill * numberChannels,
            value_);
        // R
        std::fill_n(
            origin + (layer * (viewHeight * viewWidth) + ((row + 1) * viewWidth - rightFill)) * numberChannels,
            rightFill * numberChannels,
            value_);
      }
      for (uint32_t row = 0; row < topFill; ++row) {
        // UL
        std::fill_n(
            origin + (layer * (viewHeight * viewWidth) + (row * viewWidth)) * numberChannels,
            leftFill * numberChannels,
            value_);
        // U
        std::fill_n(
            origin + (layer * (viewHeight * viewWidth) + (row * viewWidth + leftFill)) * numberChannels,
            (viewWidth - leftFill - rightFill) * numberChannels,
            value_);
        // UR
        std::fill_n(
            origin + (layer * (viewHeight * viewWidth) + ((row + 1) * viewWidth - rightFill)) * numberChannels,
            rightFill * numberChannels,
            value_);
      }
      for (uint32_t row = viewHeight - bottomFill; row < viewHeight; ++row) {
        // BL
        std::fill_n(
            origin + (layer * (viewHeight * viewWidth) + (row * viewWidth)) * numberChannels,
            leftFill * numberChannels,
            value_);
        // B
        std::fill_n(
            origin + (layer * (viewHeight * viewWidth) + (row * viewWidth + leftFill)) * numberChannels,
            (viewWidth - leftFill - rightFill) * numberChannels,
            value_);
        // BR
        std::fill_n(
            origin + (layer * (viewHeight * viewWidth) + ((row + 1) * viewWidth - rightFill)) * numberChannels,
            rightFill * numberChannels,
            value_);
      }
    }
  }

};
}
}
#endif //FASTLOADER_CONSTANT_BORDER_CREATOR_H
