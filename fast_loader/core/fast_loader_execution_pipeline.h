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

#ifndef FAST_LOADER_FAST_LOADER_EXECUTION_PIPELINE_H
#define FAST_LOADER_FAST_LOADER_EXECUTION_PIPELINE_H

#include <hedgehog/hedgehog.h>
#include "../api/data/index_request.h"
#include "data/tile_request.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief Execution Pipeline used to generate a FastLoader for all levels
/// @details Send a IndexRequest to a graph depending on the level
/// @tparam ViewType Type of the view
template<class ViewType>
class FastLoaderExecutionPipeline : public hh::AbstractExecutionPipeline<1, IndexRequest, TileRequest<ViewType>> {
 public:
/// @brief FastLoaderExecutionPipeline constructor
/// @param graph FastLoader inside graph to duplicate for all levels
/// @param nbGraphDuplication Number of graphs into the execution pipeline
  FastLoaderExecutionPipeline(
      std::shared_ptr<hh::Graph<1, IndexRequest, TileRequest<ViewType>>> const &graph,
      size_t const &nbGraphDuplication)
      : hh::AbstractExecutionPipeline<1, IndexRequest, TileRequest<ViewType>>(
      graph, std::vector<int>(nbGraphDuplication, 0), "Fast Loader Levels") {}

  /// @brief Default destructor
  ~FastLoaderExecutionPipeline() = default;

/// @brief Rule to send an IndexRequest to a specific inside graph
/// @param data IndexRequest to send
/// @param graphId GraphId to test
/// @return True is the level == GraphId, else False
  bool sendToGraph(std::shared_ptr<IndexRequest> &data, size_t const &graphId) override {
    return data->level_ == graphId;
  }
};

} // fl
} // internal

#endif //FAST_LOADER_FAST_LOADER_EXECUTION_PIPELINE_H
