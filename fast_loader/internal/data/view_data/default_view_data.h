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
// Created by Bardakoff, Alexandre (IntlAssoc) on 9/23/20.
//

#ifndef FASTLOADER_DEFAULT_VIEW_DATA_H
#define FASTLOADER_DEFAULT_VIEW_DATA_H

#include "abstract_view_data.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief View Data Used by default for CPU computation
/// @tparam DataType Type of Data
template<class DataType>
class DefaultViewData : public AbstractViewData<DataType>, public hh::MemoryData<DefaultViewData<DataType>> {
 public:
  /// @brief Default constructor
  DefaultViewData() = default;

  /// @brief Copy constructor
  /// @param rhs DefaultViewData to copy
  DefaultViewData(DefaultViewData const &rhs) : AbstractViewData<DataType>(rhs) {
    auto viewSize =
        (this->tileWidth() + 2 * this->radiusWidth()) *
            (this->tileHeight() + 2 * this->radiusHeight()) *
            (this->tileDepth() + 2 * this->radiusDepth()) *
            this->numberChannels();
    this->data_ = new DataType[viewSize];
    std::copy_n(rhs.data_, viewSize, this->data_);
  }

  /// @brief Constructor from the size of the view and the number of releases
  /// @param viewSize Size of the view (number of elements)
  /// @param numberOfRelease Number of releases for a view
  DefaultViewData(size_t viewSize, size_t numberOfRelease) : AbstractViewData<DataType>(numberOfRelease) {
    this->data_ = new DataType[viewSize];
  }

  /// @brief Constructor from the size of the view and the number of releases for all pyramid's level
  /// @param sizesPerLevel Sizes of the view (number of elements) for all the pyramid's level
  /// @param releasesPerLevel Number of releases for a view for all the pyramid's level
  /// @param level Level of the pyramid
  DefaultViewData(std::vector<size_t> sizesPerLevel, std::vector<size_t> releasesPerLevel, size_t level) :
      DefaultViewData(sizesPerLevel[level], releasesPerLevel[level]) {}

  /// @brief DefaultViewData destructor, clean the raw array
  virtual ~DefaultViewData() {
    delete[] this->data_;
    this->data_ = nullptr;
  }

  /// @brief ViewRequest used method, to increment the releaseCount
  void used() override { ++this->releaseCount_; }

  /// @brief Recycling flag accessor used by hedgehog
  /// @return True is the view can be recycled
  bool canBeRecycled() override { return this->releaseCount_ == this->numberOfRelease_; }

  /// @brief Return the piece of memory to the memory manager
  void returnToMemoryManager() final {
    hh::MemoryData<DefaultViewData<DataType>>::returnToMemoryManager();
  }

};

}
}

#endif //FASTLOADER_DEFAULT_VIEW_DATA_H
