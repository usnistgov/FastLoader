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

#ifndef FAST_LOADER_TRAITS_H
#define FAST_LOADER_TRAITS_H

#include <type_traits>

#include "data/view/abstract_view.h"
#include "../api/default_view.h"

/// @brief FastLoader namespace
namespace fl {
/// @brief FastLoader internal namespace
namespace internal {

/// @brief FastLoader traits namespace
namespace traits {
/// @brief FastLoader traits, to test if a data as the right accessor, false case
/// @tparam UserViewTest Type to test
template<typename UserViewTest, typename = void>
struct HasDataType : std::false_type {};

/// @brief FastLoader traits, to test if a data as the right accessor, true case
/// @details Test is a data_t accessor is available in  UserViewTest
/// @tparam UserViewTest Type to test
template<typename UserViewTest>
struct HasDataType<UserViewTest, std::void_t<typename UserViewTest::data_t>> : std::true_type {};

/// @brief Trait to test if a type is usable in FastLoader
/// @tparam ViewType Type to test
template<class ViewType>
struct IsView {
  constexpr static bool const value =
      (
          std::is_base_of_v<AbstractView < typename ViewType::data_t>, ViewType > &&
#ifdef HH_USE_CUDA
              std::disjunction_v<
              std::is_base_of<DefaultView < typename ViewType::data_t>, ViewType >,
              std::is_base_of<UnifiedView < typename ViewType::data_t>, ViewType >> &&
#else //HH_USE_CUDA
              std::is_base_of_v<DefaultView < typename ViewType::data_t>, ViewType > &&
#endif
              std::is_arithmetic_v<typename ViewType::data_t>); ///< Test's value
};

/// @brief Helper to access value of the IsView trait
/// @tparam ViewType Type to test
template<class ViewType>
inline constexpr bool is_view_v = IsView<ViewType>::value;
}
}
}
#endif //FAST_LOADER_TRAITS_H
