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

#include "../../../api/abstract_border_creator.h"
#include "../../data/view_data/abstract_view_data.h"
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief AbstractView Loader class, made to generate the different TileRequest to the AbstractTileLoader
/// @details The generated TileRequest come from two different sources. The first one is the system itself that will
/// generate all the TileRequest to fill the views with the maximum data from the file. In case of ghost region that
/// need to be filled with other part of the file per copy, the borderCreator is used. Theses two set of TileRequests
/// are merged to do all the copies from a requested tile at once.
/// @tparam ViewType AbstractView's type

template<class ViewType, class ViewDataType>
class ViewLoader : public hh::AbstractTask<TileRequest<ViewType>, ViewDataType> {
 private:
  std::shared_ptr<AbstractBorderCreator<ViewType>>
      borderCreator_{}; ///< BorderCreator used to fill ghost region with copy

 public:
  /// @brief ViewLoader constructor
  /// @param borderCreator BorderCreator used to fill ghost region
  explicit ViewLoader(std::shared_ptr<AbstractBorderCreator<ViewType>> borderCreator)
      : hh::AbstractTask<TileRequest<ViewType>, ViewDataType>("ViewLoader"),
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

    size_t
        tileHeight = viewData->tileHeight(),
        tileWidth = viewData->tileWidth(),
        tileDepth = viewData->tileDepth(),
        minRowFile = viewData->minRowFile(),
        minColFile = viewData->minColFile(),
        minLayerFile = viewData->minLayerFile(),
        maxRowFile = viewData->maxRowFile(),
        maxColFile = viewData->maxColFile(),
        maxLayerFile = viewData->maxLayerFile(),
        rowAlreadyFilled = 0,
        colAlreadyFilled = 0,
        layerAlreadyFilled = viewData->frontFill(),
        rowFrom = 0,
        colFrom = 0,
        layerFrom = 0,
        heightToCopy = 0,
        widthToCopy = 0,
        depthToCopy = 0,
        frontUpperLeftTileRowGlobal = 0,
        frontUpperLeftTileColGlobal = 0,
        frontUpperLeftTileLayerGlobal = 0;

    std::set<std::shared_ptr<TileRequest<ViewType>>> tileRequests{};

    std::ostringstream oss;

    // Add tile request for tiles that exists in file
    for (size_t layer = viewData->indexLayerMinTile(); layer < viewData->indexLayerMaxTile(); ++layer) {
      frontUpperLeftTileLayerGlobal = layer * tileDepth;
      layerFrom = frontUpperLeftTileLayerGlobal <= minLayerFile ? (minLayerFile - frontUpperLeftTileLayerGlobal) : 0;
      depthToCopy =
          std::min(maxLayerFile, frontUpperLeftTileLayerGlobal + tileDepth) - layerFrom - frontUpperLeftTileLayerGlobal;

      rowAlreadyFilled = viewData->topFill();
      for (size_t row = viewData->indexRowMinTile(); row < viewData->indexRowMaxTile(); ++row) {
        frontUpperLeftTileRowGlobal = row * tileHeight;
        rowFrom = frontUpperLeftTileRowGlobal <= minRowFile ? (minRowFile - frontUpperLeftTileRowGlobal) : 0;
        heightToCopy =
            std::min(maxRowFile, frontUpperLeftTileRowGlobal + tileHeight) - rowFrom - frontUpperLeftTileRowGlobal;

        colAlreadyFilled = viewData->leftFill();
        for (size_t column = viewData->indexColMinTile(); column < viewData->indexColMaxTile(); ++column) {
          frontUpperLeftTileColGlobal = column * tileWidth;
          colFrom = frontUpperLeftTileColGlobal <= minColFile ? (minColFile - frontUpperLeftTileColGlobal) : 0;
          widthToCopy =
              std::min(maxColFile, frontUpperLeftTileColGlobal + tileWidth) - colFrom - frontUpperLeftTileColGlobal;

          auto tileRequest = std::make_shared<TileRequest<ViewType>>(row, column, layer, view);
          tileRequest->addCopy(
              CopyVolume(
                  {rowFrom, colFrom, layerFrom, heightToCopy, widthToCopy, depthToCopy},
                  {rowAlreadyFilled, colAlreadyFilled, layerAlreadyFilled, heightToCopy, widthToCopy, depthToCopy}
              )
          );

          tileRequests.insert(tileRequest);
          colAlreadyFilled += widthToCopy;
        } // End Column

        rowAlreadyFilled += heightToCopy;
      } // End Row

      layerAlreadyFilled += depthToCopy;
    } // End Layer

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

    viewData->numberTilesToLoad(tileRequests.size());
    // Send the tile request to the TileLoader
    for (auto tileRequest : tileRequests) { this->addResult(tileRequest); }
  }

  /// @brief Copy method to copy ViewLoader
  /// @return New ViewLoader
  std::shared_ptr<hh::AbstractTask<TileRequest<ViewType>, ViewDataType>> copy() override {
    return std::make_shared<ViewLoader<ViewType, ViewDataType>>(borderCreator_);
  }
};
}
}
#endif //FAST_LOADER_VIEW_LOADER_H
