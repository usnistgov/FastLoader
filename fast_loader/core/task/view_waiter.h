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

#ifndef FAST_LOADER_VIEW_WAITER_H
#define FAST_LOADER_VIEW_WAITER_H

#include <hedgehog/hedgehog.h>
#include "view_counter.h"
#include "../data/view/abstract_view.h"
#include "../../api/data/index_request.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief
/// @tparam ViewType Type of the view
/// @tparam ViewDataType
template<class ViewType, class ViewDataType>
class ViewWaiter : public hh::AbstractTask<1, IndexRequest, ViewDataType> {
 private:
  bool const ordered_; ///< Ordering flag
  size_t const level_; ///< Pyramidal level
  FillingType const fillingType_; ///< Filling used to fill the view
  std::shared_ptr<ViewCounter<ViewType>> const viewCounter_; ///< FastLoader's ViewCounter

  std::shared_ptr<std::vector<std::vector<size_t>>> const
      fullDimensionPerLevel_{}, ///< Full / file dimensions per level
      tileDimensionPerLevel_{}; ///< Tile dimensions per level

  std::vector<size_t> const &
      fullDimension_{}, ///< Full dimension for this level
      tileDimension_{}; ///< Tile dimension for this level

  std::vector<size_t>
      radii_{}, ///< View radii
      nbTilesPerDimension_{}; ///< Number tiles per dimension

  std::vector<std::string> const dimensionNames_{}; ///< Dimension names
 public:
  /// @brief View waiter, get an available view from the memory manager, and attache the request to it
  /// @param ordered Flag to indicate if the views need to be served in the same order they have been requested
  /// @param level Pyramidal level
  /// @param fillingType Type of fill for ghost values
  /// @param viewCounter Related ViewCounter used to manage the ordered output
  /// @param fullDimensionPerLevel Full / file dimensions per level
  /// @param tileDimensionPerLevel Tile dimensions per level
  /// @param radii View radii
  /// @param dimensionNames Dimension names
  ViewWaiter(
      bool const ordered, size_t const level, FillingType const fillingType,
      std::shared_ptr<ViewCounter<ViewType>> const viewCounter,
      std::shared_ptr<std::vector<std::vector<size_t>>> const &fullDimensionPerLevel,
      std::shared_ptr<std::vector<std::vector<size_t>>> const &tileDimensionPerLevel,
      std::vector<size_t> const &radii, std::vector<std::string> const& dimensionNames)
      : hh::AbstractTask<1, IndexRequest, ViewDataType>("View Waiter"),
        ordered_(ordered), level_(level), fillingType_(fillingType), viewCounter_(viewCounter),
        fullDimensionPerLevel_(fullDimensionPerLevel), tileDimensionPerLevel_(tileDimensionPerLevel),
        fullDimension_(fullDimensionPerLevel_->at(level_)), tileDimension_(tileDimensionPerLevel_->at(level_)),
        radii_(radii), dimensionNames_(dimensionNames) {
    std::transform(
        fullDimensionPerLevel_->at(level_).cbegin(), fullDimensionPerLevel_->at(level_).cend(),
        tileDimensionPerLevel_->at(level_).cbegin(), std::back_insert_iterator<std::vector<size_t>>(nbTilesPerDimension_),
        [](auto const &fullDimension, auto const &tileDimension) {
          return (size_t) std::ceil((double) fullDimension / (double) tileDimension);
        });
  }

  /// @brief Default destructor
  ~ViewWaiter() override = default;

  /// @brief From an indexRequest, get an available view from a memory manager and attach the request
  /// @param indexRequest Index request for getting a view
  void execute(std::shared_ptr<IndexRequest> indexRequest) override {
    if (!indexRequest) { throw (std::runtime_error("You can not create a view from an empty view request.")); }
    else {
      bool isIndexRequestValid = indexRequest->level_ < fullDimensionPerLevel_->size();
      if(indexRequest->index_.size() == nbTilesPerDimension_.size()){
        for(size_t i = 0; i < indexRequest->index_.size() && isIndexRequestValid; ++i){
          isIndexRequestValid &= indexRequest->index_.at(i) < nbTilesPerDimension_.at(i);
        }
      }else { isIndexRequestValid = false; }

      if (!isIndexRequestValid) {
        std::ostringstream oss;
        oss << "The tile requested [";
        std::copy(indexRequest->index_.cbegin(), indexRequest->index_.cend(), std::ostream_iterator<size_t>(oss, ", "));
        oss << "] for the level " << indexRequest->level_ << " can't be requested.";
        throw (std::runtime_error(oss.str()));
      } else {
        auto viewData = std::dynamic_pointer_cast<ViewDataType>(this->getManagedMemory());
        viewData->initialize(
            fullDimension_, tileDimension_, radii_, indexRequest->index_, nbTilesPerDimension_, dimensionNames_, fillingType_, level_
        );
        if (ordered_) { viewCounter_->addIndexRequest(indexRequest); }
        this->addResult(viewData);
      }
    }
  }

  /// @brief Copy method for duplicating this Hedgehog task
  /// @return New instance of this task
  std::shared_ptr<hh::AbstractTask<1, IndexRequest, ViewDataType>> copy() override {
    return std::make_shared<ViewWaiter>(ordered_, level_ + 1, fillingType_, viewCounter_, fullDimensionPerLevel_,
                                        tileDimensionPerLevel_, radii_, dimensionNames_);
  }
};

} // fl
} // internal

#endif //FAST_LOADER_VIEW_WAITER_H
