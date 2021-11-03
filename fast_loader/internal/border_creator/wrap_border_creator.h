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

#ifndef FASTLOADER_WRAP_BORDER_CREATOR_H
#define FASTLOADER_WRAP_BORDER_CREATOR_H

#include "../../api/abstract_border_creator.h"
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief Border creator that will copy data from other side of the domain
/// @details Given the domain: \n
/// | 0 	| 1 	| 2 	|\n
/// | 3 	| 4 	| 5 	|\n
/// | 6 	| 7 	| 8 	|\n
///
/// The border creation will copy as follows:\n
/// | 4 	| 5 	| 3 	| 4 	| 5 	| 3 	| 4 	|\n
/// | 7 	| 8 	| 6 	| 7 	| 8 	| 6 	| 7 	|\n
/// | 1 	| 2 	| 0 	| 1 	| 2 	| 0 	| 1 	|\n
/// | 4 	| 5 	| 3 	| 4 	| 5 	| 3 	| 4 	|\n
/// | 7 	| 8 	| 6 	| 7 	| 8 	| 6 	| 7 	|\n
/// | 1 	| 2 	| 0 	| 1 	| 2 	| 0 	| 1 	|\n
/// | 3 	| 4 	| 3 	| 4 	| 5 	| 3 	| 4 	|\n
/// In 1D, the domain look like: \n
/// | 0 	| 1 	| 2 	|\n
/// The copy will look like: \n
/// | 1 	| 2 	| 0 	| 1 	| 2 	| 0 	| 1 	|\n
/// @tparam ViewType Type of the view \n
template<class ViewType>
class WrapBorderCreator : public AbstractBorderCreator<ViewType> {
 public:
  /// @brief Generate the tile requests for the Wrap border creator
  /// @param view AbstractView to fill
  /// @return List of tile requests to fill the view with ghost value from other part of the file
  std::list<std::shared_ptr<TileRequest < ViewType>>>
  tileRequestsToFillBorders(std::shared_ptr<ViewType>
  const &view) override {
    std::list<std::shared_ptr<TileRequest < ViewType>> > tileRequests;

    size_t
        tileWidth = view->viewData()->tileWidth(),
        tileHeight = view->viewData()->tileHeight(),
        viewWidth = view->viewData()->viewWidth(),
        viewHeight = view->viewData()->viewHeight(),
        fullWidth = view->viewData()->fullWidth(),
        fullHeight = view->viewData()->fullHeight(),
        numberTilesRow = ceil(fullHeight / (double) tileHeight),
        numberTilesCol = ceil(fullWidth / (double) tileWidth),
        topFill = view->viewData()->topFill(),
        bottomFill = view->viewData()->bottomFill(),
        rightFill = view->viewData()->rightFill(),
        leftFill = view->viewData()->leftFill(),
        minRowFile = view->viewData()->minRowFile(),
        minColFile = view->viewData()->minColFile(),
        maxRowFile = view->viewData()->maxRowFile(),
        maxColFile = view->viewData()->maxColFile(),
        rowAlreadyFilled = view->viewData()->topFill(),
        colAlreadyFilled = view->viewData()->leftFill(),
        heightToCopy = 0,
        widthToCopy = 0,

    // Used by algo
        size = 0,
        alreadyFilled = 0,
        deltaIndex = 0,
        ulTileRowGlobal = 0,
        ulTileColGlobal = 0,
        rowFrom = 0,
        colFrom = 0,
        indexOffset = 0,
        realTileSize = 0;

    std::vector<typename AbstractBorderCreator<ViewType>::CopyPosition>
        positionsLeft,
        positionsTop,
        positionsRight,
        positionsBottom;

    alreadyFilled = 0;
    deltaIndex = 0;
    //Right indexes
    while (alreadyFilled < rightFill) {
      realTileSize = std::min(tileWidth, fullWidth - deltaIndex * tileWidth);
      size = std::min(rightFill - alreadyFilled, realTileSize);
      positionsRight.push_back({deltaIndex, 0, viewWidth - rightFill + alreadyFilled, size});
      alreadyFilled += size;
      ++deltaIndex;
      deltaIndex = deltaIndex % (numberTilesCol);
    }

    alreadyFilled = 0;
    deltaIndex = 0;
    //Left indexes
    while (alreadyFilled < leftFill) {
      indexOffset = numberTilesCol - 1 - deltaIndex;
      realTileSize = std::min(tileWidth, fullWidth - indexOffset * tileWidth);
      size = std::min(leftFill - alreadyFilled, realTileSize);
      positionsLeft.push_back({indexOffset, realTileSize - size, leftFill - alreadyFilled - size, size});
      alreadyFilled += size;
      ++deltaIndex;
      deltaIndex = deltaIndex % (numberTilesCol);
    }

    alreadyFilled = 0;
    deltaIndex = 0;
    //Bottom indexes
    while (alreadyFilled < bottomFill) {
      realTileSize = std::min(tileHeight, fullHeight - deltaIndex * tileHeight);
      size = std::min(bottomFill - alreadyFilled, realTileSize);
      positionsBottom.push_back({deltaIndex, 0, viewHeight - bottomFill + alreadyFilled, size});
      alreadyFilled += size;
      ++deltaIndex;
      deltaIndex = deltaIndex % (numberTilesRow);
    }

    alreadyFilled = 0;
    deltaIndex = 0;
    //Top indexes
    while (alreadyFilled < topFill) {
      indexOffset = numberTilesRow - 1 - deltaIndex;
      realTileSize = std::min(tileHeight, fullHeight - indexOffset * tileHeight);
      size = std::min(topFill - alreadyFilled, realTileSize);
      positionsTop.push_back({indexOffset, realTileSize - size, topFill - alreadyFilled - size, size});
      alreadyFilled += size;
      ++deltaIndex;
      deltaIndex = deltaIndex % (numberTilesRow);
    }

    for (size_t r = view->viewData()->indexRowMinTile(); r < view->viewData()->indexRowMaxTile(); ++r) {
      ulTileRowGlobal = r * tileHeight;
      rowFrom = ulTileRowGlobal <= minRowFile ? (minRowFile - ulTileRowGlobal) : 0;
      heightToCopy = std::min(maxRowFile, ulTileRowGlobal + tileHeight) - rowFrom - ulTileRowGlobal;
      // Left TileRequests
      for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionLeft : positionsLeft) {
        auto tileRequest = std::make_shared<TileRequest < ViewType>>
        (r, positionLeft.indexTile, view);
        tileRequest->addCopy(
            CopyRectangle(
                {rowFrom, positionLeft.posBeginTile, heightToCopy, positionLeft.size},
                {rowAlreadyFilled, positionLeft.posBeginView, heightToCopy, positionLeft.size}
            )
        );
        tileRequests.push_back(tileRequest);
      }
      // Right TileRequests
      for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionRight : positionsRight) {
        auto tileRequest = std::make_shared<TileRequest < ViewType>>
        (r, positionRight.indexTile, view);
        tileRequest->addCopy(
            CopyRectangle(
                {rowFrom, positionRight.posBeginTile, heightToCopy, positionRight.size},
                {rowAlreadyFilled, positionRight.posBeginView, heightToCopy, positionRight.size}
            )
        );
        tileRequests.push_back(tileRequest);
      }
      rowAlreadyFilled += heightToCopy;
    }

    for (size_t c = view->viewData()->indexColMinTile(); c < view->viewData()->indexColMaxTile(); ++c) {
      ulTileColGlobal = c * tileWidth;
      colFrom = ulTileColGlobal <= minColFile ? (minColFile - ulTileColGlobal) : 0;
      widthToCopy = std::min(maxColFile, ulTileColGlobal + tileWidth) - colFrom - ulTileColGlobal;
      // Top TileRequests
      for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionTop : positionsTop) {
        auto tileRequest = std::make_shared<TileRequest < ViewType>>
        (positionTop.indexTile, c, view);
        tileRequest->addCopy(
            CopyRectangle(
                {positionTop.posBeginTile, colFrom, positionTop.size, widthToCopy},
                {positionTop.posBeginView, colAlreadyFilled, positionTop.size, widthToCopy}
            )
        );
        tileRequests.push_back(tileRequest);
      }
      // Bottom TileRequests
      for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionBottom : positionsBottom) {
        auto tileRequest = std::make_shared<TileRequest < ViewType>>
        (positionBottom.indexTile, c, view);
        tileRequest->addCopy(
            CopyRectangle(
                {positionBottom.posBeginTile, colFrom, positionBottom.size, widthToCopy},
                {positionBottom.posBeginView, colAlreadyFilled, positionBottom.size, widthToCopy}
            )
        );
        tileRequests.push_back(tileRequest);
      }
      colAlreadyFilled += widthToCopy;
    }

    for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionTop : positionsTop) {
      // Top Left TileRequests
      for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionLeft : positionsLeft) {
        auto tileRequest =
            std::make_shared<TileRequest < ViewType>>
        (positionTop.indexTile, positionLeft.indexTile, view);
        tileRequest->addCopy(
            CopyRectangle(
                {positionTop.posBeginTile, positionLeft.posBeginTile, positionTop.size, positionLeft.size},
                {positionTop.posBeginView, positionLeft.posBeginView, positionTop.size, positionLeft.size}
            )
        );
        tileRequests.push_back(tileRequest);
      }
      // Top Right TileRequests
      for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionRight : positionsRight) {
        auto
            tileRequest =
            std::make_shared<TileRequest < ViewType>>
        (positionTop.indexTile, positionRight.indexTile, view);
        tileRequest->addCopy(
            CopyRectangle(
                {positionTop.posBeginTile, positionRight.posBeginTile, positionTop.size, positionRight.size},
                {positionTop.posBeginView, positionRight.posBeginView, positionTop.size, positionRight.size}
            )
        );
        tileRequests.push_back(tileRequest);
      }
    }

    for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionBottom : positionsBottom) {
      // Bottom Left TileRequests
      for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionLeft : positionsLeft) {
        auto tileRequest =
            std::make_shared<TileRequest < ViewType>>
        (positionBottom.indexTile, positionLeft.indexTile, view);
        tileRequest->addCopy(
            CopyRectangle(
                {positionBottom.posBeginTile, positionLeft.posBeginTile, positionBottom.size, positionLeft.size},
                {positionBottom.posBeginView, positionLeft.posBeginView, positionBottom.size, positionLeft.size}
            )
        );
        tileRequests.push_back(tileRequest);
      }
      // Bottom Right TileRequests
      for (typename AbstractBorderCreator<ViewType>::CopyPosition const &positionRight : positionsRight) {
        auto tileRequest =
            std::make_shared<TileRequest < ViewType>>
        (positionBottom.indexTile, positionRight.indexTile, view);
        tileRequest->addCopy(
            CopyRectangle(
                {positionBottom.posBeginTile, positionRight.posBeginTile, positionBottom.size, positionRight.size},
                {positionBottom.posBeginView, positionRight.posBeginView, positionBottom.size, positionRight.size}
            )
        );
        tileRequests.push_back(tileRequest);
      }
    }
    return tileRequests;
  }

  /// @brief Copy existing values, from the view into the view, in this case do nothing
  void fillBorderWithExistingValues([[maybe_unused]]std::shared_ptr<ViewType> const &) override {}
};
}
}
#endif //FASTLOADER_WRAP_BORDER_CREATOR_H
