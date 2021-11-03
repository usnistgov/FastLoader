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

#ifndef FASTLOADER_REPLICATE_BORDER_CREATOR_H
#define FASTLOADER_REPLICATE_BORDER_CREATOR_H

#include "../../api/abstract_border_creator.h"
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief Border creator that will replicate the view's border
/// @details Given the domain: \n
/// | 0 	| 1 	| 2 	|\n
/// | 3 	| 4 	| 5 	|\n
/// | 6 	| 7 	| 8 	|\n
///
/// The border creation will copy as follows:\n
/// | 0  	| 0  	| 0 	| 1  	| 2  	| 2  	| 2  	|\n
/// | 0  	| 0  	| 0 	| 1 	| 2 	| 2  	| 2  	|\n
/// | 0  	| 0  	| 0 	| 1 	| 2 	| 2  	| 2  	|\n
/// | 3  	| 3  	| 3 	| 4 	| 5 	| 5     | 5  	|\n
/// | 6  	| 6  	| 6 	| 7  	| 8 	| 8  	| 8  	|\n
/// | 6  	| 6  	| 6  	| 7  	| 8  	| 8  	| 8  	|\n
/// | 6  	| 6  	| 6  	| 7  	| 8  	| 8  	| 8  	|\n
/// In 1D, the domain look like: \n
/// | 0 	| 1 	| 2 	|\n
/// The copy will look like: \n
/// | 0 	| 0 	| 0 	| 1 	| 2 	| 2 	| 2 	|\n
/// @tparam ViewType Type of the view \n
template<class ViewType>
class ReplicateBorderCreator : public AbstractBorderCreator<ViewType> {
 public:
  /// @brief Generate the tile requests for the Replicate border creator, in this case do nothing
  /// @return List of tile requests, empty
  std::list<std::shared_ptr<TileRequest < ViewType>>>
  tileRequestsToFillBorders([[maybe_unused]]std::shared_ptr<ViewType>
  const &) final { return {}; }

  /// @brief Copy border values, from the view into the view
  /// @param view AbstractView to fill
  void fillBorderWithExistingValues(std::shared_ptr<ViewType> const &view) final {
    size_t
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

    for (size_t layer = frontFill; layer < viewDepth - backFill; ++layer) {
      for (size_t row = topFill; row < viewHeight - bottomFill; ++row) {
        // L
        std::fill_n(
            origin + (layer * viewWidth * viewHeight + row * viewWidth) * numberChannels,
            leftFill * numberChannels,
            *(origin + (layer * viewWidth * viewHeight + row * viewWidth + leftFill) * numberChannels));
        // R
        std::fill_n(
            origin + (layer * viewWidth * viewHeight + row * viewWidth + viewWidth - rightFill) * numberChannels,
            rightFill * numberChannels,
            *(origin +
              ((layer * viewWidth * viewHeight + row * viewWidth + viewWidth - rightFill - 1) * numberChannels)));
      }
      for (size_t row = 0; row < topFill; ++row) {
        // UL
        std::fill_n(
            origin + (layer * viewWidth * viewHeight + row * viewWidth) * numberChannels,
            leftFill * numberChannels,
            *(origin + ((layer * viewWidth * viewHeight + topFill * viewWidth + leftFill) * numberChannels)));
        // U
        std::copy_n(
            origin + (layer * viewWidth * viewHeight + topFill * viewWidth + leftFill) * numberChannels,
            (viewWidth - leftFill - rightFill) * numberChannels,
            origin + ((layer * viewWidth * viewHeight + row * viewWidth + leftFill) * numberChannels));
        // UR
        std::fill_n(
            origin + (layer * viewWidth * viewHeight + row * viewWidth + viewWidth - rightFill) * numberChannels,
            rightFill * numberChannels,
            *(origin +
              ((layer * viewWidth * viewHeight + row * viewWidth + viewWidth - rightFill - 1) * numberChannels)));
      }
      for (size_t row = viewHeight - bottomFill; row < viewHeight; ++row) {
        // BL
        std::fill_n(
            origin + (layer * viewWidth * viewHeight + row * viewWidth) * numberChannels,
            leftFill * numberChannels,
            *(origin +
                (layer * viewWidth * viewHeight + (viewHeight - bottomFill - 1) * viewWidth + leftFill)
                    * numberChannels));
        // B
        std::copy_n(
            origin +
                (layer * viewWidth * viewHeight + (viewHeight - bottomFill - 1) * viewWidth + leftFill)
                    * numberChannels,
            (viewWidth - leftFill - rightFill) * numberChannels,
            origin + ((layer * viewWidth * viewHeight + row * viewWidth + leftFill) * numberChannels));
        // BR
        std::fill_n(
            origin + (layer * viewWidth * viewHeight + row * viewWidth + viewWidth - rightFill) * numberChannels,
            rightFill * numberChannels,
            *(origin +
                (layer * viewWidth * viewHeight + (viewHeight - bottomFill) * viewWidth - rightFill - 1)
                    * numberChannels));
      }
    }

    // Front
    for (size_t layer = 0; layer < frontFill; ++layer) {
      std::copy_n(
          origin + (frontFill * viewHeight * viewWidth) * numberChannels,
          viewHeight * viewWidth * numberChannels,
          origin + ((layer * viewHeight * viewWidth) * numberChannels)
      );
    }

    // Back
    for (size_t layer = viewDepth - backFill; layer < viewDepth; ++layer) {
      std::copy_n(
          origin + ((viewDepth - backFill - 1) * viewHeight * viewWidth) * numberChannels,
          viewHeight * viewWidth * numberChannels,
          origin + ((layer * viewHeight * viewWidth) * numberChannels)
      );
    }
  }
};
}
}
#endif //FASTLOADER_REPLICATE_BORDER_CREATOR_H
