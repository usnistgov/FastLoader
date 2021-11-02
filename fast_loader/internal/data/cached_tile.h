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

#ifndef FASTLOADER_CACHED_TILE_H
#define FASTLOADER_CACHED_TILE_H

#include <cstdint>
#include <iostream>
#include <vector>
/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {
/// @brief Tile Cached from the file.
/// @details The cached tile is used to prevent excess IO (i.e. from disk). Once a tile has been loaded from the file it
/// is saved to the cache.
/// @tparam DataType Type of data
template<class DataType>
class CachedTile {
 protected:
  std::shared_ptr<std::vector<DataType>>
      data_{};         ///< Tile data.

  uint32_t
      indexRow_{},      ///< Row index tile asked in global coordinate
      indexCol_{},      ///< Column index tile asked in global coordinate
      indexLayer_{};    ///< Layer index tile asked in global coordinate

  bool
      newTile_{};       ///< True if the tile is a new tile, else not

  std::mutex
      accessMutex_{};   ///< Access mutex

  uint32_t
      tileWidth_{},     ///< Tile width
      tileHeight_{},    ///< Tile height
      tileDepth_{},     ///< Tile depth
      numberChannels_{}; ///< Number of channels

 public:
  /// \brief CachedTile Constructor, allocate the data buffer
  /// \param tileWidth Tile width
  /// \param tileHeight Tile height
  /// \param tileDepth Tile depth
  /// \param numberChannel Number of pixel channels
  explicit CachedTile(size_t tileWidth, size_t tileHeight, size_t tileDepth, size_t numberChannel) :
      indexRow_(0), indexCol_(0), indexLayer_(0),
      newTile_(true),
      tileWidth_(tileWidth), tileHeight_(tileHeight), tileDepth_(tileDepth), numberChannels_(numberChannel) {
        try{
          data_ = std::make_shared<std::vector<DataType>>(tileWidth * tileHeight * tileDepth * numberChannel);
        }catch (std::bad_alloc const & except){
          std::ostringstream oss;
          oss << "Problem while allocating a cacge tile with the dimension (" << ": " << tileWidth << "x" << tileHeight << "x" << tileDepth << "x" << numberChannel << ") :";
          oss << except.what();
          throw std::runtime_error(oss.str());
        }
      }

  /// \brief CachedTile Destructor
  virtual ~CachedTile() = default;

  /// @brief Data accessor
  /// @return Inside data as vector
  [[nodiscard]] std::shared_ptr<std::vector<DataType>> &data() { return data_; }
  /// @brief Tile row index accessor
  /// @return Tile row index
  [[nodiscard]] uint32_t indexRow() const { return indexRow_; }
  /// @brief Tile column index accessor
  /// @return Tile column index
  [[nodiscard]] uint32_t indexCol() const { return indexCol_; }
  /// @brief Tile layer index accessor
  /// @return Tile layer index
  [[nodiscard]] uint32_t indexLayer() const { return indexLayer_; }

  /// @brief New tile flag accessor
  /// @return New tile flag
  [[nodiscard]] bool isNewTile() const { return newTile_; }
  /// @brief Tile width accessor
  /// @return Tile width
  [[nodiscard]] uint32_t tileWidth() const { return tileWidth_; }
  /// @brief Tile height accessor
  /// @return Tile height
  [[nodiscard]] uint32_t tileHeight() const { return tileHeight_; }
  /// @brief Tile depth accessor
  /// @return Tile depth
  [[nodiscard]] uint32_t tileDepth() const { return tileDepth_; }

  /// \brief Getter to the number of channels
  /// \return Number of pixel's channels
  [[nodiscard]] virtual uint32_t numberChannels() const { return numberChannels_; }

  /// @brief Mutex accessor
  /// @return mutex
  [[nodiscard]] std::mutex &accessMutex() { return accessMutex_; }

  /// @brief Tile row index setter
  /// @param indexRow Tile row to set
  void indexRow(uint32_t indexRow) { indexRow_ = indexRow; }
  /// @brief Tile column index setter
  /// @param indexCol Tile column to set
  void indexCol(uint32_t indexCol) { indexCol_ = indexCol; }
  /// @brief Tile depth index setter
  /// @param indexLayer Tile depth to set
  void indexLayer(uint32_t indexLayer) { indexLayer_ = indexLayer; }
  /// @brief New tile flag setter
  /// @param newTile New tile flag
  void newTile(bool newTile) { newTile_ = newTile; }

  /// @brief Lock inner mutex
  void lock() { this->accessMutex().lock(); }
  /// @brief Unlock inner mutex
  void unlock() { this->accessMutex().unlock(); }

  /// \brief Output stream to print a cached tile
  /// \param os Stream to put the tile information
  /// \param tile The CachedTile to print
  /// \return the output stream with the information
  friend std::ostream &operator<<(std::ostream &os, const CachedTile &tile) {
    os
        << "CachedTile indexRow: " << tile.indexRow() << " indexCol: " << tile.indexCol() << " indexLayer: " << tile.indexLayer()
        << " tileWidth: " << tile.tileWidth() << " tileHeight: " << tile.tileHeight()
        << " tileDepth: " << tile.tileDepth() << " numberChannels: " << tile.numberChannels()
        << " newTile: " << std::boolalpha << tile.isNewTile() << "\n";

    for(uint32_t layer = 0; layer < tile.tileDepth_; ++layer) {
      os << "Layer: " << layer << "\n";
      for (uint32_t row = 0; row < tile.tileHeight_; ++row) {
        os << "\t";
        for (uint32_t column = 0; column < tile.tileWidth_; ++column) {
          for (uint32_t channel = 0; channel < tile.numberChannels(); ++channel) {
            os << tile.data_->at(
                (layer * (tile.tileWidth_ * tile.tileHeight_) + row * tile.tileWidth_ + column) * tile.numberChannels_
                + channel) << "/";
          }
          os << " ";
        }
        os << std::endl;
      }
      os << std::endl;
    }
    return os;
  }
};
}
}
#endif //FASTLOADER_CACHED_TILE_H
