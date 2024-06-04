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

#ifndef FAST_LOADER_VIRTUAL_FILE_TILE_LOADER_H
#define FAST_LOADER_VIRTUAL_FILE_TILE_LOADER_H

#include <utility>

#include "../../fast_loader/fast_loader.h"

class VirtualFileTileLoader : public fl::AbstractTileLoader<fl::DefaultView<int>> {
  std::vector<size_t> const fullDimension_, tileDimension_;
  std::vector<size_t> stridePerDimension_;
  std::shared_ptr<std::vector<int>> file_{};
  std::vector<std::string> names_{};

 public:
  VirtualFileTileLoader(size_t const numberThreads, std::vector<size_t> fullDimension, std::vector<size_t> tileDimension)
      : AbstractTileLoader("VirtualFileTileLoader", "filePath", numberThreads),
        fullDimension_(std::move(fullDimension)), tileDimension_(std::move(tileDimension)) {
    file_ = std::make_shared<std::vector<int>>(std::accumulate(fullDimension_.cbegin(),
                                                               fullDimension_.cend(),
                                                               1,
                                                               std::multiplies<>()));
    for (size_t dimension = 0; dimension < tileDimension_.size(); ++dimension) {
      stridePerDimension_.push_back(
          std::accumulate(tileDimension_.cbegin() + (long) (dimension + 1),
                          tileDimension_.cend(),
                          (size_t) 1,
                          std::multiplies<>())
      );
    }
    fillFile();
    names_ = std::vector<std::string>(fullDimension_.size(), "");
  }

  ~VirtualFileTileLoader() override = default;

  void loadTileFromFile(
      std::shared_ptr<std::vector<int>> tile,
      std::vector<size_t> const &index, [[maybe_unused]]size_t level) override {
    std::fill_n(tile->begin(), tile->size(), 69);
    std::vector<size_t> nbCopiesPerDimension(index.size(), 0);
    fillBuffer(tile, index, nbCopiesPerDimension);
  }

  [[nodiscard]] size_t nbDims() const override { return fullDimension_.size(); }
  [[nodiscard]] size_t nbPyramidLevels() const override { return 1; }
  [[nodiscard]] std::vector<size_t> const & fullDims([[maybe_unused]] size_t const level) const override { return fullDimension_; }
  [[nodiscard]] std::vector<size_t> const & tileDims([[maybe_unused]] size_t const level) const override { return tileDimension_; }
  [[nodiscard]] std::vector<std::string> const & dimNames() const override { return names_; }

  std::shared_ptr<AbstractTileLoader> copyTileLoader() override {
    return std::make_shared<VirtualFileTileLoader>(this->numberThreads(), fullDimension_, tileDimension_);
  }

 private:
  inline void fillFile(size_t const &dimension = 0) {
    size_t
    const nbFills
        = std::accumulate(
            fullDimension_.cbegin(), fullDimension_.cbegin() + (long) dimension + 1, (size_t) 1, std::multiplies<>()),
        stride
        =
        std::accumulate(fullDimension_.cbegin() + (long) dimension + 1, fullDimension_.cend(), (size_t) 1, std::multiplies<>());

    for (size_t fillId = 0; fillId < nbFills; ++fillId) {
      std::for_each(
          file_->begin() + (long) (fillId * stride), file_->begin() + (long) ((fillId * stride) + stride),
          [&fillId, &dimension, this](auto &value) {
            value +=
                (int) ((fillId % fullDimension_.at(dimension)) * (size_t) std::pow(10, fullDimension_.size() - dimension - 1));
          }
      );
    }
    if (dimension != fullDimension_.size() - 1) { fillFile(dimension + 1); }
  }

  inline void fillBuffer(std::shared_ptr<std::vector<int>> const &tile,
                         std::vector<size_t> const &index,
                         std::vector<size_t> nbCopiesPerDimension,
                         size_t const &posStartCpySrc = 0,
                         size_t const &dimension = 0) {
    auto maxPos = std::min(fullDimension_.at(dimension), (index.at(dimension) + 1) * tileDimension_.at(dimension));
    if (dimension != fullDimension_.size() - 1) {
      for (size_t posSrc = index.at(dimension) * tileDimension_.at(dimension); posSrc < maxPos; ++posSrc) {
        fillBuffer(
            tile, index, nbCopiesPerDimension,
            posStartCpySrc
                + (posSrc * std::accumulate(
                    fullDimension_.cbegin() + (long) dimension + 1, fullDimension_.cend(), (size_t) 1, std::multiplies<>())),
            dimension + 1);
        nbCopiesPerDimension.at(dimension) += 1;
        std::fill(nbCopiesPerDimension.begin() + (long) (dimension + 1), nbCopiesPerDimension.end(), (size_t) 0);
      }
    } else {
      std::copy_n(
          file_->cbegin() + (long) (posStartCpySrc + index.at(dimension) * tileDimension_.at(dimension)),
          maxPos - (index.at(dimension) * tileDimension_.at(dimension)),
          tile->begin() + (long) std::inner_product(
              nbCopiesPerDimension.cbegin(), nbCopiesPerDimension.cend(), stridePerDimension_.cbegin(), (size_t) 0)
      );
      nbCopiesPerDimension.at(dimension) += 1;

    }
  }

};

#endif //FAST_LOADER_VIRTUAL_FILE_TILE_LOADER_H
