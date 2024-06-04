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

#ifndef FAST_LOADER_VIEW_LOADER_H
#define FAST_LOADER_VIEW_LOADER_H

#include <hedgehog/hedgehog.h>
#include "../data/tile_request.h"
#include "../../api/graph/options/abstract_border_creator.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief AbstractView Loader class, made to generate the different TileRequest to the AbstractTileLoader
/// @details The generated TileRequest come from two different sources. The first one is the system itself that will
/// generate all the TileRequest to fill the views with the maximum data from the file. In case of ghost region that
/// need to be filled with other part of the file per copy, the borderCreator is used. Theses two set of TileRequests
/// are merged to do all the copies from a requested tile at once.
/// @tparam ViewType Type of the view
/// @tparam ViewDataType View data type
template<class ViewType, class ViewDataType>
class ViewLoader : public hh::AbstractTask<1, ViewDataType, TileRequest<ViewType>> {
 private:
  std::shared_ptr<AbstractBorderCreator<ViewType>> const
      borderCreator_{}; ///< BorderCreator used to fill ghost region with copy

 public:
  /// @brief ViewLoader constructor
  /// @param borderCreator BorderCreator used to fill ghost region
  explicit ViewLoader(std::shared_ptr<AbstractBorderCreator<ViewType>> borderCreator)
      : hh::AbstractTask<1, ViewDataType, TileRequest<ViewType>>("ViewLoader"),
        borderCreator_(borderCreator) {}

  /// @brief Execute routine for ViewLoader
  /// @details Generate TileRequest come from two different sources: the first one is the system itself that will
  /// generate all the TileRequest to fill the views with the maximum data from the file. In case of ghost region that
  /// need to be filled with other part of the file per copy, the borderCreator is used. Theses two set of TileRequests
  /// are merged to do all the copies from a requested tile at once.
  /// @param viewData
  void execute(std::shared_ptr<ViewDataType> viewData) override {
    auto view = std::make_shared<ViewType>();
    view->viewData(viewData);

    std::vector<size_t> const &
        tileDimension = viewData->tileDims(),
        minPos = viewData->minPos(),
        maxPos = viewData->maxPos(),
        minTileIndex = viewData->minTileIndex(),
        maxTileIndex = viewData->maxTileIndex();

    std::vector<size_t>
        posFrom(view->nbDims()),
        posTo(view->nbDims()),
        dimensionToCopy(view->nbDims()),
        indexTileRequest(view->nbDims());

    std::set<std::shared_ptr<TileRequest<ViewType>>> tileRequests{};

    createCopies(
        minTileIndex, maxTileIndex, tileDimension, minPos, viewData->frontFill(),
        maxPos, view, posFrom, posTo, dimensionToCopy, indexTileRequest, tileRequests, viewData->nbDims());

    // Add and merge border tile Request
    std::list<std::shared_ptr<TileRequest<ViewType>>>
        borderTileRequests = this->borderCreator_->tileRequestsToFillBorders(view);
    for (std::shared_ptr<TileRequest<ViewType>> &borderTileRequest : borderTileRequests) {
      auto tileRequestFound = std::find_if(
          tileRequests.begin(), tileRequests.end(),
          [&borderTileRequest](auto tileRequest) { return *borderTileRequest == *tileRequest; });
      if (tileRequestFound != tileRequests.end()) {
        (*tileRequestFound)->merge(*borderTileRequest);
      } else { tileRequests.insert(borderTileRequest); }
    }

    viewData->nbTilesToLoad(tileRequests.size());
    // Send the tile request to the TileLoader
    for (auto tileRequest : tileRequests) {
      this->addResult(tileRequest);
    }
  }

  /// @brief Copy method to copy ViewLoader
  /// @return New ViewLoader
  std::shared_ptr<hh::AbstractTask<1, ViewDataType, TileRequest<ViewType>>> copy() override {
    return std::make_shared<ViewLoader<ViewType, ViewDataType>>(borderCreator_);
  }

 private:
  /// @brief Create the copies
  /// @param minTileIndex Minimum tile index composing the view
  /// @param maxTileIndex Maximum tile index composing the view
  /// @param tileDimension Tile dimension
  /// @param minPos Minimum position
  /// @param frontFill Amount of data upfront no coming from the file
  /// @param maxPos Maximum position
  /// @param view Destination view
  /// @param positionFrom Source position
  /// @param positionTo Destination position
  /// @param dimensionToCopy Copy dimension
  /// @param indexTileRequest Index tile request
  /// @param tileRequests Vector of result tileRequests
  /// @param nbDimensions Total number of dimensions
  /// @param dimension Current dimension
  inline void createCopies(
      std::vector<size_t> const &minTileIndex, std::vector<size_t> const &maxTileIndex,
      std::vector<size_t> const &tileDimension, std::vector<size_t> const &minPos, std::vector<size_t> const &frontFill,
      std::vector<size_t> const &maxPos,
      auto const &view,
      std::vector<std::size_t> &positionFrom, std::vector<std::size_t> &positionTo,
      std::vector<std::size_t> &dimensionToCopy, std::vector<std::size_t> &indexTileRequest,
      std::set<std::shared_ptr<TileRequest<ViewType>>> &tileRequests, size_t const nbDimensions, size_t const dimension = 0) const {

    positionTo.at(dimension) = frontFill.at(dimension);
    for (size_t index = minTileIndex.at(dimension); index < maxTileIndex.at(dimension); ++index) {
      indexTileRequest.at(dimension) = index;
      size_t frontGlobalPosition = index * tileDimension.at(dimension);
      positionFrom.at(dimension) =
          frontGlobalPosition <= minPos.at(dimension) ? (minPos.at(dimension) - frontGlobalPosition) : 0;
      dimensionToCopy.at(dimension) =
          std::min(maxPos.at(dimension), frontGlobalPosition + tileDimension.at(dimension))
              - positionFrom.at(dimension) - frontGlobalPosition;
      if (dimension == nbDimensions - 1) {
        auto tileRequest = std::make_shared<TileRequest<ViewType>>(indexTileRequest, view);
        tileRequest->addCopy(CopyVolume(positionFrom, positionTo, dimensionToCopy));
        tileRequests.insert(tileRequest);
      } else {
        createCopies(
            minTileIndex, maxTileIndex, tileDimension, minPos, frontFill, maxPos, view, positionFrom, positionTo,
            dimensionToCopy, indexTileRequest, tileRequests, nbDimensions, dimension + 1);
      }
      positionTo.at(dimension) += dimensionToCopy.at(dimension);
    }
  }
};

} // fl
} // internal

#endif //FAST_LOADER_VIEW_LOADER_H
