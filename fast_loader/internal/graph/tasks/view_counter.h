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
// Created by anb22 on 11/8/19.
//

#ifndef FASTLOADER_VIEW_COUNTER_H
#define FASTLOADER_VIEW_COUNTER_H

#include <hedgehog/hedgehog.h>
#include <list>
#include <ostream>

#include "../../data/view/abstract_view.h"
#include "../../../api/abstract_border_creator.h"
#include "../../data_type.h"
#include "../../data/tile_request.h"
#include "../../../api/index_request.h"
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief Task finalizing and providing the output view.
/// @details Receive the TileRequest<ViewType> from the AbstractTileLoader, mergeCount until the number of
/// TileRequest<ViewType> for a view is reached, fill the duplicated ghost values, and send the view. In case of
/// ordering, a succession list is used to know which view to send next.
/// @tparam ViewType
template<class ViewType>
class ViewCounter : public hh::AbstractTask<ViewType, TileRequest < ViewType>>
{
std::shared_ptr<AbstractBorderCreator < ViewType>>
borderCreator_ {
}; ///< Border creator to fill ghost pixels

std::shared_ptr<std::unordered_map<std::shared_ptr<ViewType>, size_t>>
    countMap_{};  ///< Map between the view, and the number of tiles loaded

std::shared_ptr<std::list<std::shared_ptr<ViewType>>>
    waitingList_{}; ///< Views stored because not in the right order

std::shared_ptr<std::queue<std::shared_ptr<IndexRequest>>>
    indexRequests_{}; ///< Current traversal

bool
    ordered_ = false; ///< Order preserved

std::mutex
    mutex_; ///< Mutex to protect the view ordering

public:
/// @brief ViewCounter constructor
/// @param borderCreator Border Creator used to fill the view with ghost value created from duplication
/// @param ordered Flag to determine if the ordering is requested
explicit ViewCounter(std::shared_ptr<AbstractBorderCreator < ViewType>>
borderCreator,
bool ordered
)
: hh::AbstractTask<ViewType, TileRequest < ViewType>>("AbstractView Counter"),

borderCreator_ (borderCreator), ordered_(ordered) {
  countMap_ = std::make_shared<std::unordered_map<std::shared_ptr<ViewType>, size_t>>();
  waitingList_ = std::make_shared<std::list<std::shared_ptr<ViewType>>>();
  indexRequests_ = std::make_shared<std::queue<std::shared_ptr<IndexRequest>>>();
}

/// @brief ViewCounter destructor
~

ViewCounter() = default;

/// @brief Add next view's index request to be send in case of ordering
/// @param indexRequest Next view's index request to be send
void addIndexRequest(std::shared_ptr<IndexRequest> const &indexRequest) {
  std::lock_guard<std::mutex> lk(mutex_);
  indexRequests_->push(indexRequest);
}

/// @brief Manage a TileRequest
/// @details Receive the TileRequest<ViewType> from the AbstractTileLoader, mergeCount until the number of
/// TileRequest<ViewType> for a view is reached, fill the duplicated ghost values, and send the view. In case of
/// ordering, a succession list is used to know which view to send next.
/// @param tileRequest TileRequest to manage
void execute(std::shared_ptr<TileRequest < ViewType>> tileRequest) override {
  // Tile directly ready
  if (tileRequest->view()->viewData()->numberTilesToLoad()== 1) {
    borderCreator_->fillBorderWithExistingValues(tileRequest->view());
    dataReady(tileRequest->view()
    );
  } else {
    // Tile not directly ready
    auto itPos = countMap_->find(tileRequest->view());
    if (itPos != countMap_->end()) {
      // Found pointer, so increment
      (*itPos).second++;
      // If all the tiles have been collected for the view, then the view is complete
      if ((*itPos).second == tileRequest->view()->viewData()->numberTilesToLoad()) {
        countMap_->erase(tileRequest->view());
        borderCreator_->fillBorderWithExistingValues(tileRequest->view());
        dataReady(tileRequest->view());
      }
    } else {
      // Pointer was not found, so add it in with initial mergeCount of 1
      countMap_->insert({tileRequest->view(), 1});
    }
  }
}

/// @brief ViewCounter output stream operator
/// @param os Output stream to print ViewCounter into
/// @param vc ViewCounter to print
/// @return Output stream for chaining
friend std::ostream &operator<<(std::ostream &os, ViewCounter &vc) {
  os << "BorderCreator: " << vc.borderCreator_ << std::endl;
  os << "Ordered: " << std::boolalpha << vc.ordered_ << std::endl;
  os << "CountMap: " << std::endl;
  for (std::pair<std::shared_ptr<ViewType>, size_t> const &count : *(vc.countMap_)) {
    os << "\t" << count.first << ": " << count.second << std::endl;
  }
  os << "Waiting List: ";
  std::copy(
      vc.waitingList_->begin(), vc.waitingList_->end(),
      std::ostream_iterator<std::shared_ptr<ViewType>>(os, ", "));
  os << std::endl;
  os << "Current traversal:" << std::endl;
  vc.printQueue(os, *(vc.currentTraversal_.get()));
  os << "Queue traversal:" << std::endl;
  vc.printQueue(os, *(vc.queueTraversals_.get()));
  return os;
}

private:

/// @brief Test if the view is the next one to be send in case of ordering
/// @param view AbstractView to test
/// @return True if view is the next one, else false
bool viewIsNext(std::shared_ptr<ViewType> &view) {
  return view->tileRowIndex() == indexRequests_->front()->indexRow_
      && view->tileColIndex() == indexRequests_->front()->indexCol_
      && view->level() == indexRequests_->front()->level_;
}

/// @brief Managed stored view on waiting list in case tthey are one to be send next in case of ordering
void handleStoredViews() {
  bool elementFound = true;
  while (elementFound) {
    elementFound = false;
    for (auto view = waitingList_->begin(); view != waitingList_->end(); ++view) {
      if (viewIsNext(*view)) {
        this->addResult(*view);
        waitingList_->erase(view);
        indexRequests_->pop();
        elementFound = true;
        break;
      }
    }
  }
}

/// @brief Store in waiting list or send the ready view
/// @param view AbstractView to manage
void dataReady(std::shared_ptr<ViewType> view) {
  if (!ordered_) { this->addResult(view); }
  else {
    std::lock_guard<std::mutex> lk(mutex_);
    if (viewIsNext(view)) {
      this->addResult(view);
      indexRequests_->pop();
      handleStoredViews();
    } else {
      waitingList_->push_back(view);
    }
  }
}

};
}
}
#endif //FASTLOADER_VIEW_COUNTER_H
