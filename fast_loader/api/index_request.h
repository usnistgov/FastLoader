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
// Created by anb22 on 11/18/19.
//

#ifndef FASTLOADER_INDEX_REQUEST_H
#define FASTLOADER_INDEX_REQUEST_H

#include <ostream>
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
/// @brief Data structure to represent a requested index, used for ordering
struct IndexRequest {
  uint32_t
      indexRow_, ///< Row index
      indexCol_, ///< Column index
      indexLayer_, ///< Layer index
      level_; ///< Level index

  /// @brief IndexRequest constructor
  /// @param indexRow Index request's row
  /// @param indexCol Index request's column
  /// @param indexLayer Index request's layer
  /// @param level Index request's level
  IndexRequest(uint32_t indexRow, uint32_t indexCol, uint32_t indexLayer, uint32_t level)
      : indexRow_(indexRow), indexCol_(indexCol), indexLayer_(indexLayer), level_(level) {}

  /// @brief Index request output stream operator
  /// @param os Output stream to print into
  /// @param request Request to print
  /// @return Output stream
  friend std::ostream &operator<<(std::ostream &os, IndexRequest const &request) {
    os << "(" << request.indexRow_ << ", " << request.indexCol_ << ", " << request.indexLayer_ << ", " << request.level_
    << ")";
    return os;
  }
};
}
#endif //FASTLOADER_INDEX_REQUEST_H
