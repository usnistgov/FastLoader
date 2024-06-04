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

#ifndef FAST_LOADER_ADAPTIVE_VIEW_DATA_H
#define FAST_LOADER_ADAPTIVE_VIEW_DATA_H

#include <stdexcept>
#include "abstract_view_data.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief Data used by an AdaptiveView using a logical cached tile as data.
/// @tparam DataType Type of data inside the View
template<class DataType>
class AdaptiveViewData : public AbstractViewData<DataType>{
 private:
  DataType * const
      dataOrigin_; ///< Origin of the logical cached tile data
 public:
  /// @brief AdaptiveViewData constructor accepting the origin of the logical cached tile data
  /// @param dataOrigin Origin of the logical cached tile data
  explicit AdaptiveViewData(DataType *const dataOrigin) : dataOrigin_(dataOrigin) {}

  /// @brief Default destructor
  ~AdaptiveViewData() override = default;

  /// @brief Data accessor
  /// @return Origin of the logical cached tile data
  DataType *data() const final { return dataOrigin_;}

  /// @brief returnToMemoryManager implementation from Hedgehog library, throw an exception in every case
  /// @warning Should not be used because data is a piece of cache and not an Hedgehog managed memory
  /// @throw std::runtime_error in every case, should not be used
  void returnToMemoryManager() final {
    throw std::runtime_error("An Adaptive view is an internal view not made to be used with a memory manager.");
  }
};

} // internal
} // fl

#endif //FAST_LOADER_ADAPTIVE_VIEW_DATA_H
