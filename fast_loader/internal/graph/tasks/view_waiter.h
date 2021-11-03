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
// Created by anb22 on 11/12/19.
//

#ifndef FASTLOADER_VIEW_WAITER_H
#define FASTLOADER_VIEW_WAITER_H

#include <hedgehog/hedgehog.h>
#include "../../data/view/abstract_view.h"
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief Wait for a ViewDataType from a memory manager, and initializeSparseMatrix it from the received IndexRequest.
/// @details If ordering is needed, then the ViewWaiter will give an IndexRequest to the viewCounter.
/// @tparam ViewType AbstractView's type
template<class ViewType, class ViewDataType>
class ViewWaiter : public hh::AbstractTask<ViewDataType, IndexRequest> {
 private:
  std::shared_ptr<ViewCounter < ViewType>>viewCounter_ {}; ///< FastLoader's ViewCounter
  bool ordered_ = false; ///< Ordering flag

  std::shared_ptr<std::vector<size_t>>
      vFullHeight_{},   ///< List of file height for all levels
      vFullWidth_{},    ///< List of file width for all levels
      vFullDepth_{},    ///< List of file depth for all levels
      vTileHeight_{},   ///< List of tile height for all levels
      vTileWidth_{},    ///< List of tile width for all levels
      vTileDepth_{};    ///< List of tile depth for all levels

  size_t
      fullHeight_ = 0,      ///< Full height for this level
      fullWidth_ = 0,       ///< Full width for this level
      fullDepth_ = 0,       ///< Full depth for this level
      tileHeight_ = 0,      ///< Tile height for this level
      tileWidth_ = 0,       ///< Tile width for this level
      tileDepth_ = 0,       ///< Tile depth for this level
      numberChannels_ = 0,  ///< Number of channels
      radiusHeight_ = 0,    ///< AbstractView's radius height
      radiusWidth_ = 0,     ///< AbstractView's radius width
      radiusDepth_ = 0,     ///< AbstractView's radius depth
      numTilesHeight_ = 0,  ///< Number of tiles in height for a view
      numTilesWidth_ = 0,   ///< Number of tiles in column for a view
      numTilesDepth_ = 0,   ///< Number of tiles in depth for a view
      level_ = 0;           ///< Level

  FillingType
      fillingType_ = FillingType::CONSTANT; ///< Filling used to fill the view

 public:
  /// @brief ViewWaiter constructor used to initializeSparseMatrix all level attributes
  /// @param viewCounter FastLoader's ViewCounter
  /// @param ordered Ordering flag
  /// @param fullHeight Full height for the current level
  /// @param fullWidth Full width for the current level
  /// @param fullDepth Full width for the current level
  /// @param tileHeight Tile height for the current level
  /// @param tileWidth Tile width for the current level
  /// @param tileDepth Tile width for the current level
  /// @param numberChannels Number of channels in a pixel
  /// @param radiusHeight View radius in height
  /// @param radiusWidth View radius in width
  /// @param radiusDepth View radius in depth
  /// @param fillingType AbstractView's fillingType
  /// @param level Current level
  ViewWaiter(
      std::shared_ptr<ViewCounter<ViewType>> viewCounter,
      bool ordered,
      size_t fullHeight, size_t fullWidth, size_t fullDepth,
  size_t tileHeight, size_t tileWidth, size_t tileDepth, size_t numberChannels,
  size_t radiusHeight, size_t radiusWidth, size_t radiusDepth,
  FillingType fillingType, size_t level)
  : hh::AbstractTask<ViewDataType, IndexRequest>("AbstractView Waiter"),
  viewCounter_ (viewCounter), ordered_(ordered),
  fullHeight_(fullHeight), fullWidth_(fullWidth), fullDepth_(fullDepth),
  tileHeight_(tileHeight), tileWidth_(tileWidth), tileDepth_(tileDepth), numberChannels_(numberChannels),
  radiusHeight_(radiusHeight), radiusWidth_(radiusWidth), radiusDepth_(radiusDepth),
  level_(level), fillingType_(fillingType) {
    numTilesWidth_ = (size_t) ceil((double) fullWidth_ / tileWidth_);
    numTilesHeight_ = (size_t) ceil((double) fullHeight_ / tileHeight_);
    numTilesDepth_ = (size_t) ceil((double) fullDepth_ / tileDepth_);
  }

  /// @brief ViewWaiter constructor used by the copy
  /// @param viewCounter FastLoader's ViewCounter
  /// @param ordered Ordering flag
  /// @param vFullHeight List of file height for all levels
  /// @param vFullWidth List of file width for all levels
  /// @param vFullDepth List of file width for all levels
  /// @param vTileHeight List of tile height for all levels
  /// @param vTileWidth List of tile width for all levels
  /// @param vTileDepth List of tile width for all levels
  /// @param numberChannels Number of channels in a pixel
  /// @param radiusHeight View radius in height
  /// @param radiusWidth View radius in width
  /// @param radiusDepth View radius in depth
  /// @param fillingType Filling used to fill the view
  /// @param level Level
  ViewWaiter(std::shared_ptr<ViewCounter<ViewType>> viewCounter,
             bool ordered,
             std::shared_ptr<std::vector<size_t>> &vFullHeight,
             std::shared_ptr<std::vector<size_t>> &vFullWidth,
             std::shared_ptr<std::vector<size_t>> &vFullDepth,
             std::shared_ptr<std::vector<size_t>> &vTileHeight,
             std::shared_ptr<std::vector<size_t>> &vTileWidth,
             std::shared_ptr<std::vector<size_t>> &vTileDepth,
             size_t numberChannels,
             size_t radiusHeight, size_t radiusWidth, size_t radiusDepth,
             FillingType fillingType, size_t level = 0 )
             : ViewWaiter(
                 viewCounter, ordered,
                 vFullHeight->at(level), vFullWidth->at(level), vFullDepth->at(level),
                 vTileHeight->at(level), vTileWidth->at(level), vTileDepth->at(level),
                 numberChannels,
                 radiusHeight, radiusWidth, radiusDepth,
                 fillingType, level ) {
    vFullHeight_ = vFullHeight;
    vFullWidth_ = vFullWidth;
    vFullDepth_ = vFullDepth;
    vTileHeight_ = vTileHeight;
    vTileWidth_ = vTileWidth;
    vTileDepth_ = vTileDepth;
  }

  /// @brief Execute routine for ViewWaiter
  /// @details Wait for a ViewDataType from a memory manager, and initializeSparseMatrix it from the received IndexRequest. If ordering
  /// is needed, the ViewWaiter will give to the viewCounter the IndexRequest.
  /// @param indexRequest AbstractView's index request
  void execute(std::shared_ptr<IndexRequest> indexRequest) final {
    if (!indexRequest) {
      throw (std::runtime_error("You can not create a view from an empty view request."));
    }
    if (
        indexRequest->level_ >= vFullHeight_->size() ||
            indexRequest->indexRow_ >= numTilesHeight_ ||
            indexRequest->indexCol_ >= numTilesWidth_ ||
            indexRequest->indexLayer_ >= numTilesDepth_) {
      std::ostringstream oss;
      oss << "The tile requested ("
          << indexRequest->indexRow_ << ", "
          << indexRequest->indexCol_ << ", "
          << indexRequest->indexLayer_ << ") for the level "
          << indexRequest->level_ << " can't be requested.";
      throw (std::runtime_error(oss.str()));
    }

    auto viewData = this->getManagedMemory();

    viewData->initialize(
        fullHeight_, fullWidth_, fullDepth_,
        tileHeight_, tileWidth_, tileDepth_,
        numberChannels_,
        radiusHeight_, radiusWidth_, radiusDepth_,
        indexRequest->level_,
        indexRequest->indexRow_, indexRequest->indexCol_, indexRequest->indexLayer_,
        numTilesHeight_, numTilesWidth_, numTilesDepth_,
        fillingType_);

    if (ordered_) { viewCounter_->addIndexRequest(indexRequest); }
    this->addResult(viewData);
  }

  /// @brief Copy the current ViewWaiter
  /// @return Return a new ViewWaiter based on the current one with an increased level
  std::shared_ptr<hh::AbstractTask<ViewDataType, IndexRequest>> copy() final {
    return std::make_shared<ViewWaiter>(
        viewCounter_, ordered_,
        vFullHeight_, vFullWidth_, vFullDepth_,
        vTileHeight_, vTileWidth_, vTileDepth_,
        numberChannels_,
        radiusHeight_, radiusWidth_, radiusDepth_,
        fillingType_, level_++);
  }

};
}
}
#endif //FASTLOADER_VIEW_WAITER_H
