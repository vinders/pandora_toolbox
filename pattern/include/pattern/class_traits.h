/*******************************************************************************
MIT License
Copyright (c) 2021 Romain Vinders

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------------
Custom type traits for classes
*******************************************************************************/
#pragma once

#include <cstddef>
#include <type_traits>
#include <iterator>

namespace pandora {
  namespace pattern {
    // -- traits - comparisons --

    /// @brief Verify if a class contains the equality operator (==)
    template <typename _Type, typename _RhsType>
    struct has_operator_equals {
      template <typename _Left = _Type, typename _Right = _RhsType> static auto test(_Left&& lhs, _Right&& rhs) -> decltype(lhs == rhs, void(), std::true_type{});
      static auto test(...) -> std::false_type;
      using type = decltype(test(std::declval<_Type>(), std::declval<_RhsType>()));
    };
    /// @brief Verify if a class contains the 'less' operator (<)
    template <typename _Type, typename _RhsType>
    struct has_operator_less {
      template <typename _Left = _Type, typename _Right = _RhsType> static auto test(_Left&& lhs, _Right&& rhs) -> decltype(lhs < rhs, void(), std::true_type{});
      static auto test(...)->std::false_type;
      using type = decltype(test(std::declval<_Type>(), std::declval<_RhsType>()));
    };
    /// @brief Verify if a class contains the 'less or equal' operator (<=)
    template <typename _Type, typename _RhsType>
    struct has_operator_less_eq {
      template <typename _Left = _Type, typename _Right = _RhsType> static auto test(_Left&& lhs, _Right&& rhs) -> decltype(lhs <= rhs, void(), std::true_type{});
      static auto test(...)->std::false_type;
      using type = decltype(test(std::declval<_Type>(), std::declval<_RhsType>()));
    };
    /// @brief Verify if a class contains the 'greater' operator (>)
    template <typename _Type, typename _RhsType>
    struct has_operator_greater {
      template <typename _Left = _Type, typename _Right = _RhsType> static auto test(_Left&& lhs, _Right&& rhs) -> decltype(lhs > rhs, void(), std::true_type{});
      static auto test(...)->std::false_type;
      using type = decltype(test(std::declval<_Type>(), std::declval<_RhsType>()));
    };
    /// @brief Verify if a class contains the 'greater or equal' operator (>=)
    template <typename _Type, typename _RhsType>
    struct has_operator_greater_eq {
      template <typename _Left = _Type, typename _Right = _RhsType> static auto test(_Left&& lhs, _Right&& rhs) -> decltype(lhs >= rhs, void(), std::true_type{});
      static auto test(...)->std::false_type;
      using type = decltype(test(std::declval<_Type>(), std::declval<_RhsType>()));
    };


    // -- traits - iterators --

    /// @brief Verify if a class is an input iterator
    template <typename _Iterator>
    using is_input_iterator = typename std::is_convertible<typename std::iterator_traits<_Iterator>::iterator_category, std::input_iterator_tag>;
    /// @brief Verify if a class is an output iterator
    template <typename _Iterator>
    using is_output_iterator = typename std::is_convertible<typename std::iterator_traits<_Iterator>::iterator_category, std::output_iterator_tag>;
    /// @brief Verify if a class is a forward iterator
    template <typename _Iterator>
    using is_forward_iterator = typename std::is_convertible<typename std::iterator_traits<_Iterator>::iterator_category, std::forward_iterator_tag>;
    /// @brief Verify if a class is a bidirectional iterator
    template <typename _Iterator>
    using is_bidirectional_iterator = typename std::is_convertible<typename std::iterator_traits<_Iterator>::iterator_category, std::bidirectional_iterator_tag>;
    /// @brief Verify if a class is a random access iterator
    template <typename _Iterator>
    using is_random_access_iterator = typename std::is_convertible<typename std::iterator_traits<_Iterator>::iterator_category, std::random_access_iterator_tag>;


    // -- conditional operators - copy / move --

    /// @brief Enable if a type is copy constructible
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_copy_constructible = typename std::enable_if<!std::is_class<_DataType>::value || std::is_copy_constructible<_DataType>::value, _EnabledType>::type;
    /// @brief Enable if a type is copy assignable
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_copy_assignable = typename std::enable_if<!std::is_class<_DataType>::value || std::is_copy_assignable<_DataType>::value, _EnabledType>::type;

    /// @brief Enable if a type is move constructible
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_move_constructible = typename std::enable_if<!std::is_class<_DataType>::value || std::is_move_constructible<_DataType>::value, _EnabledType>::type;
    /// @brief Enable if a type is move assignable
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_move_assignable = typename std::enable_if<!std::is_class<_DataType>::value || std::is_move_assignable<_DataType>::value, _EnabledType>::type;

    /// @brief Enable if a type is copy constructible and not move constructible
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_copy_or_move_constructible = typename std::enable_if<!std::is_class<_DataType>::value || std::is_copy_constructible<_DataType>::value || std::is_move_constructible<_DataType>::value, _EnabledType>::type;
    /// @brief Enable if a type is copy assignable and not move assignable
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_copy_or_move_assignable = typename std::enable_if<!std::is_class<_DataType>::value || std::is_copy_assignable<_DataType>::value || std::is_move_assignable<_DataType>::value, _EnabledType>::type;

    /// @brief Enable if a type is copy constructible and not move constructible
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_copy_not_move_constructible = typename std::enable_if<!std::is_class<_DataType>::value || (std::is_copy_constructible<_DataType>::value && !std::is_move_constructible<_DataType>::value), _EnabledType>::type;
    /// @brief Enable if a type is copy assignable and not move assignable
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_copy_not_move_assignable = typename std::enable_if<!std::is_class<_DataType>::value || (std::is_copy_assignable<_DataType>::value && !std::is_move_assignable<_DataType>::value), _EnabledType>::type;


    // -- conditional operators - comparisons --

    /// @brief Enable if a type has the equality operator (==)
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_operator_equals = typename std::enable_if<!std::is_class<_DataType>::value || has_operator_equals<_DataType,_DataType>::type::value, _EnabledType>::type;
    /// @brief Enable if a type doesn't have the equality operator (==)
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_not_operator_equals = typename std::enable_if<!std::is_class<_DataType>::value && !has_operator_equals<_DataType, _DataType>::type::value, _EnabledType>::type;

    /// @brief Enable if a type has the 'less' operator (<)
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_operator_less = typename std::enable_if<!std::is_class<_DataType>::value || has_operator_less<_DataType, _DataType>::type::value, _EnabledType>::type;
    /// @brief Enable if a type has the 'less or equal' operator (<=)
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_operator_less_eq = typename std::enable_if<!std::is_class<_DataType>::value || has_operator_less_eq<_DataType, _DataType>::type::value, _EnabledType>::type;
    /// @brief Enable if a type has the 'greater' operator (>)
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_operator_greater = typename std::enable_if<!std::is_class<_DataType>::value || has_operator_greater<_DataType, _DataType>::type::value, _EnabledType>::type;
    /// @brief Enable if a type has the greater or equal' operator (>=)
    template <typename _DataType, typename _EnabledType = void>
    using enable_if_operator_greater_eq = typename std::enable_if<!std::is_class<_DataType>::value || has_operator_greater_eq<_DataType, _DataType>::type::value, _EnabledType>::type;


    // -- conditional operators - iterators --

    /// @brief Enable if a type is a valid iterator
    template <typename _Iterator, typename _EnabledType = void>
    using enable_if_iterator = typename std::enable_if<is_input_iterator<_Iterator>::value
                                                    || is_output_iterator<_Iterator>::value
                                                    || is_forward_iterator<_Iterator>::value
                                                    || is_bidirectional_iterator<_Iterator>::value
                                                    || is_random_access_iterator<_Iterator>::value, _EnabledType>::type;
    /// @brief Enable if a type is a valid iterator that can be used bi-directionally
    template <typename _Iterator, typename _EnabledType = void>
    using enable_if_iterator_reversible = typename std::enable_if<std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<_Iterator>::iterator_category>::value
                                                               || is_bidirectional_iterator<_Iterator>::value
                                                               || is_random_access_iterator<_Iterator>::value, _EnabledType>::type;
    /// @brief Enable if a type is a valid iterator that can NOT be used bi-directionally
    template <typename _Iterator, typename _EnabledType = void>
    using enable_if_iterator_not_reversible = typename std::enable_if<!std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<_Iterator>::iterator_category>::value
                                                                   && !is_bidirectional_iterator<_Iterator>::value
                                                                   && !is_random_access_iterator<_Iterator>::value, _EnabledType>::type;

    // -- constructors / assignment enablers --

    template <bool _Copyable, bool _Movable> 
    struct ConstructorEnabler {};
    template <> struct ConstructorEnabler<false, false> {
      ConstructorEnabler() = default;
      ConstructorEnabler(const ConstructorEnabler& rhs) = delete;
      ConstructorEnabler(ConstructorEnabler&& rhs) = delete;
      ConstructorEnabler& operator=(const ConstructorEnabler& rhs) = default;
      ConstructorEnabler& operator=(ConstructorEnabler&& rhs) noexcept = default;
    };
    template <> struct ConstructorEnabler<true, false> { 
      ConstructorEnabler() = default;
      ConstructorEnabler(const ConstructorEnabler& rhs) = default;
      ConstructorEnabler(ConstructorEnabler&& rhs) = delete;
      ConstructorEnabler& operator=(const ConstructorEnabler& rhs) = default;
      ConstructorEnabler& operator=(ConstructorEnabler&& rhs) noexcept = default;
    };
    template <> struct ConstructorEnabler<false, true> {
      ConstructorEnabler() = default;
      ConstructorEnabler(const ConstructorEnabler& rhs) = delete;
      ConstructorEnabler(ConstructorEnabler&& rhs) noexcept = default;
      ConstructorEnabler& operator=(const ConstructorEnabler& rhs) = default;
      ConstructorEnabler& operator=(ConstructorEnabler&& rhs) noexcept = default;
    };
    template <> struct ConstructorEnabler<true, true> {
      ConstructorEnabler() = default;
      ConstructorEnabler(const ConstructorEnabler& rhs) = default;
      ConstructorEnabler(ConstructorEnabler&& rhs) noexcept = default;
      ConstructorEnabler& operator=(const ConstructorEnabler& rhs) = default;
      ConstructorEnabler& operator=(ConstructorEnabler&& rhs) noexcept = default;
    };

    template <typename T>
    using ConstructorSelector = ConstructorEnabler<!std::is_class<T>::value || std::is_copy_constructible<T>::value, !std::is_class<T>::value || std::is_move_constructible<T>::value>;

    template <bool _Copyable, bool _Movable>
    struct AssignmentEnabler {};
    template <> struct AssignmentEnabler<false, false> {
      AssignmentEnabler() = default;
      AssignmentEnabler(const AssignmentEnabler& rhs) = default;
      AssignmentEnabler(AssignmentEnabler&& rhs) noexcept = default;
      AssignmentEnabler& operator=(const AssignmentEnabler& rhs) = delete;
      AssignmentEnabler& operator=(AssignmentEnabler&& rhs) = delete;
    };
    template <> struct AssignmentEnabler<true, false> {
      AssignmentEnabler() = default;
      AssignmentEnabler(const AssignmentEnabler& rhs) = default;
      AssignmentEnabler(AssignmentEnabler&& rhs) noexcept = default;
      AssignmentEnabler& operator=(const AssignmentEnabler& rhs) = default;
      AssignmentEnabler& operator=(AssignmentEnabler&& rhs) = delete;
    };
    template <> struct AssignmentEnabler<false, true> {
      AssignmentEnabler() = default;
      AssignmentEnabler(const AssignmentEnabler& rhs) = default;
      AssignmentEnabler(AssignmentEnabler&& rhs) noexcept = default;
      AssignmentEnabler& operator=(const AssignmentEnabler& rhs) = delete;
      AssignmentEnabler& operator=(AssignmentEnabler&& rhs) noexcept = default;
    };
    template <> struct AssignmentEnabler<true, true> {
      AssignmentEnabler() = default;
      AssignmentEnabler(const AssignmentEnabler& rhs) = default;
      AssignmentEnabler(AssignmentEnabler&& rhs) noexcept = default;
      AssignmentEnabler& operator=(const AssignmentEnabler& rhs) = default;
      AssignmentEnabler& operator=(AssignmentEnabler&& rhs) noexcept = default;
    };

    template <typename T>
    using AssignmentSelector = AssignmentEnabler<!std::is_class<T>::value || std::is_copy_assignable<T>::value, !std::is_class<T>::value || std::is_move_assignable<T>::value>;
  }
}
