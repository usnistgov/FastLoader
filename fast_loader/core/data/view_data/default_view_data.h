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

#ifndef FAST_LOADER_DEFAULT_VIEW_DATA_H
#define FAST_LOADER_DEFAULT_VIEW_DATA_H

#include <hedgehog/hedgehog.h>
#include <algorithm>
#include "abstract_view_data.h"

/// @brief FastLoader namespace
namespace fl {

/// @brief FastLoader internal namespace
namespace internal {
/// @brief
/// @tparam DataType Type of data inside the View
template <class DataType>
 class DefaultViewData : public AbstractViewData<DataType>, public hh::ManagedMemory{
  private:
   DataType *data_ = nullptr; ///< Real Data

  public:
   /// @brief Default constructor
   DefaultViewData() = default;

   /// @brief Copy constructor
   /// @param rhs DefaultViewData to copy
   DefaultViewData(DefaultViewData const &rhs) : AbstractViewData<DataType>(rhs) {
     auto viewSize = std::accumulate(this->viewDims().cbegin(),
                                     this->viewDims().cend(), (size_t)1, std::multiplies<>());
     this->data_ = new DataType[viewSize];
     std::copy_n(rhs.data_, viewSize, this->data_);
   }

   /// @brief Constructor from the size of the view and the number of releases
   /// @param viewSize Size of the view (number of elements)
   /// @param nbOfRelease Number of releases for a view
   DefaultViewData(size_t viewSize, size_t nbOfRelease) : AbstractViewData<DataType>(nbOfRelease) {
     this->data_ = new DataType[viewSize];
   }

   /// @brief Constructor from the size of the view and the number of releases for all pyramid's level
   /// @param sizesPerLevel Sizes of the view (number of elements) for all the pyramid's level
   /// @param releasesPerLevel Number of releases for a view for all the pyramid's level
   /// @param level Level of the pyramid
   DefaultViewData(std::vector<size_t> sizesPerLevel, std::vector<size_t> releasesPerLevel, size_t level) :
       DefaultViewData(sizesPerLevel[level], releasesPerLevel[level]) {}

   /// @brief DefaultViewData destructor, clean the raw array
   ~DefaultViewData() override {
     delete[] this->data_;
     this->data_ = nullptr;
   }

   /// @brief Raw data accessor
   /// @return Raw data
   DataType *data() const final { return data_; }

   /// @brief Increase the release count when the view is done processing
   void postProcess() override {
     ++this->releaseCount_;
   }

   /// @brief Recycling flag accessor used by hedgehog
   /// @return True is the view can be recycled
   bool canBeRecycled() override { return this->releaseCount_ == this->nbOfRelease_; }

   /// @brief Return the piece of memory to the memory manager
   void returnToMemoryManager() final { hh::ManagedMemory::returnToMemoryManager(); }
 };

} // fl
} // internal

#endif //FAST_LOADER_DEFAULT_VIEW_DATA_H
