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
// Created by anb22 on 11/15/19.
//

#ifndef FASTLOADER_FAST_LOADER_MEMORY_MANAGER_H
#define FASTLOADER_FAST_LOADER_MEMORY_MANAGER_H

#include <utility>
#include <hedgehog/hedgehog.h>

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief static memory manager used to generate all the views for all levels at graph initialization
/// @tparam DataType Internal view's type
template<class ViewDataType>
class FastLoaderMemoryManager
    : public hh::StaticMemoryManager<ViewDataType, std::vector<size_t>, std::vector<size_t>, size_t> {
 private:
  size_t
      level_ = {}; ///< Memory's manager level
  std::vector<size_t> const
      viewAvailablePerLevel_ = {}, ///< Number of views available for all levels
  sizePerLevel_ = {}, ///< AbstractView's size for all levels
  releasePerLevel_ = {}; ///< Number of release for all levels
 public:
/// @brief FastLoader memory manager constructor
/// @param viewAvailablePerLevel Number of views available for all levels
/// @param sizePerLevel AbstractView's size for all levels
/// @param releasePerLevel Number of release for all levels
/// @param level Memory manager level
  FastLoaderMemoryManager(
      const std::vector<size_t> &viewAvailablePerLevel,
      const std::vector<size_t> &sizePerLevel,
      const std::vector<size_t> &releasePerLevel,
      size_t level = 0) :
      hh::StaticMemoryManager<ViewDataType, std::vector<size_t>, std::vector<size_t>, size_t>
          (viewAvailablePerLevel[level], sizePerLevel, releasePerLevel, level),
      level_(level),
      viewAvailablePerLevel_(viewAvailablePerLevel),
      sizePerLevel_(sizePerLevel),
      releasePerLevel_(releasePerLevel) {
    ++level_;
  }

/// @brief Copy method to copy the first memory manager instance for all levels
/// @return Copy of the current AbstractMemoryManager
  std::shared_ptr<hh::MemoryManager<ViewDataType>> copy() override {
    return std::make_shared<FastLoaderMemoryManager<ViewDataType>>(
        viewAvailablePerLevel_, sizePerLevel_, releasePerLevel_, level_++);
  }
};
}
}
#endif //FASTLOADER_FAST_LOADER_MEMORY_MANAGER_H
