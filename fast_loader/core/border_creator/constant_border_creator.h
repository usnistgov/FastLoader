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

#ifndef FAST_LOADER_CONSTANT_BORDER_CREATOR_H
#define FAST_LOADER_CONSTANT_BORDER_CREATOR_H

#include <numeric>
#include "../../api/graph/options/abstract_border_creator.h"

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
/// The domain with the radius will look like: \n
/// | v 	| v 	| 0 	| 1 	| 2 	| v 	| v 	|\n
/// @tparam ViewType Type of the view \n
template<class ViewType>
class ConstantBorderCreator : public AbstractBorderCreator<ViewType> {
 private:
  typename ViewType::data_t const value_; ///< Value to fill border
 public:
  /// @brief ConstantBorderCreator constructor, set the value to fill
  /// @param value Value to fill
  explicit ConstantBorderCreator(typename ViewType::data_t const & value) : value_(value) {}

  /// @brief Default destructor
  ~ConstantBorderCreator() override = default;

  /// @brief Generate the tile requests for the Replicate border creator, in this case do nothing
  /// @param view View to fill
  /// @return List of tile request, empty
  std::list<std::shared_ptr<internal::TileRequest<ViewType>>>
  tileRequestsToFillBorders([[maybe_unused]] std::shared_ptr<ViewType> const &view) override {
    return {};
  }

  /// @brief Fill the border with a value
  /// @param view View to fill
  void fillBorderWithExistingValues(std::shared_ptr<ViewType> const &view) override {
    std::vector<size_t> const &
        frontFill = view->viewData()->frontFill(),
        backFill = view->viewData()->backFill(),
        viewDimension = view->viewData()->viewDims();

    auto const origin = view->viewOrigin();
    if (std::accumulate(frontFill.cbegin(), frontFill.cend(), (size_t)0, std::plus<>()) +
        std::accumulate(backFill.cbegin(), backFill.cend(), (size_t)0, std::plus<>()) != 0){
      fillFrontBack(origin, viewDimension, frontFill, backFill, viewDimension.size());
    }
  }

 private:
  /// @brief Fill constant values at the front and the back of each dimension
  /// @param data Data to fill
  /// @param dimension Data dimension
  /// @param frontFill Front fill count
  /// @param backFill  Back fill count
  /// @param nbDimensions Number of dimensions
  /// @param currentDim Current dimension
  /// @param delta Delta position to fill data into
  inline void fillFrontBack(typename ViewType::data_t *data,
                            std::vector<size_t> const &dimension,
                            std::vector<size_t> const &frontFill,
                            std::vector<size_t> const &backFill,
                            size_t const nbDimensions,
                            size_t const currentDim = 0,
                            long const  delta = 0) const {
    if (currentDim == nbDimensions - 1) {
      std::fill_n(data + delta, frontFill.at(currentDim), value_);
      std::fill_n(data + delta + dimension.at(currentDim) - backFill.at(currentDim), backFill.at(currentDim), value_);
    } else {
      size_t volume = std::accumulate(dimension.cbegin() + (long)currentDim + 1, dimension.cend(), (size_t)1, std::multiplies<>());
      std::fill_n(data + delta, frontFill.at(currentDim) * volume, value_);
      std::fill_n(
          data + delta + (dimension.at(currentDim) - backFill.at(currentDim)) * volume,
          backFill.at(currentDim) * volume,
          value_);
      for (size_t posDimension = frontFill.at(currentDim); posDimension < dimension.at(currentDim) - backFill.at(currentDim);
           ++posDimension) {
        fillFrontBack(data,
                      dimension,
                      frontFill,
                      backFill,
                      nbDimensions,
                      currentDim + 1,
                      delta + (long) (posDimension * volume));
      }
    }
  }
};

} // fl
} // internal

#endif //FAST_LOADER_CONSTANT_BORDER_CREATOR_H
