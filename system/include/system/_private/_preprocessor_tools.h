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
*******************************************************************************/
#pragma once

#define __P_EXPAND(x) x
#define __P_CONCAT(a,b) a##b
#define __P_CONCAT3(a,b,c) a##b##c

// -- variadic arguments --

#ifdef _MSC_VER
# define _P_GET_ARG_COUNT(...)  __P_EXPAND_NTH_ARG(__VA_ARGS__)
# define __P_EXPAND_NTH_ARG(...) __P_EXPAND(__P_GET_NTH_ARG(0, __VA_ARGS__, 100, \
        99,98,97,96,95,94,93,92,91,90, \
        89,88,87,86,85,84,83,82,81,80, \
        79,78,77,76,75,74,73,72,71,70, \
        69,68,67,66,65,64,63,62,61,60, \
        59,58,57,56,55,54,53,52,51,50, \
        49,48,47,46,45,44,43,42,41,40, \
        39,38,37,36,35,34,33,32,31,30, \
        29,28,27,26,25,24,23,22,21,20, \
        19,18,17,16,15,14,13,12,11,10, \
        9,8,7,6,5,4,3,2,1,0))
# define __P_GET_NTH_ARG(\
        _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,  \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, \
        _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
        _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, \
        _91,_92,_93,_94,_95,_96,_97,_98,_99,_100, \
        _101, count, ...) count

#else
# define _P_GET_ARG_COUNT(...) __P_GET_NTH_ARG(0, ## __VA_ARGS__, 100, \
        99,98,97,96,95,94,93,92,91,90, \
        89,88,87,86,85,84,83,82,81,80, \
        79,78,77,76,75,74,73,72,71,70, \
        69,68,67,66,65,64,63,62,61,60, \
        59,58,57,56,55,54,53,52,51,50, \
        49,48,47,46,45,44,43,42,41,40, \
        39,38,37,36,35,34,33,32,31,30, \
        29,28,27,26,25,24,23,22,21,20, \
        19,18,17,16,15,14,13,12,11,10, \
        9,8,7,6,5,4,3,2,1,0)
# define __P_GET_NTH_ARG(_0, \
        _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,  \
        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
        _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, \
        _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
        _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, \
        _91,_92,_93,_94,_95,_96,_97,_98,_99,_100, \
        count, ...) count
#endif

#define __P_GET_FIRST_ARG(x,...)   x
#define __P_DROP_FIRST_ARG(x,...)  __VA_ARGS__

// -- ranged loop for variadic arguments --

#define __P_FOREACH_RANGE(n, macro, ...)  __P_FOREACH__RANGE_(__P_CONCAT(__P_FOREACH_,n), macro, __VA_ARGS__)
#define __P_FOREACH__RANGE_(N_MACRO, macro, ...)  N_MACRO(macro, __VA_ARGS__)
#define __P_PARAM_FOREACH_RANGE(n, macro, param, ...)  __P_PARAM_FOREACH__RANGE_(__P_CONCAT(__P_PARAM_FOREACH_,n), macro, param, __VA_ARGS__)
#define __P_PARAM_FOREACH__RANGE_(N_MACRO, macro, param, ...)  N_MACRO(macro, param, __VA_ARGS__)

#define __P_FOREACH_RANGE_COMMA(n, macro, ...)  __P_FOREACH__RANGE_(__P_CONCAT(__P_FOREACH_COMMA_,n), macro, __VA_ARGS__)
#define __P_FOREACH_RANGE_SEMICOLON(n, macro, ...)  __P_FOREACH__RANGE_(__P_CONCAT(__P_FOREACH_SEMICOLON_,n), macro, __VA_ARGS__)

#define __P_FOREACH_1(macro, item)  __P_EXPAND(macro(item))
#define __P_FOREACH_2(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_1(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_3(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_2(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_4(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_3(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_5(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_4(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_6(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_5(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_7(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_6(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_8(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_7(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_9(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_8(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_10(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_9(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_11(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_10(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_12(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_11(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_13(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_12(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_14(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_13(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_15(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_14(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_16(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_15(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_17(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_16(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_18(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_17(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_19(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_18(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_20(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_19(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_21(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_20(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_22(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_21(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_23(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_22(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_24(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_23(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_25(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_24(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_26(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_25(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_27(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_26(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_28(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_27(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_29(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_28(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_30(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_29(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_31(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_30(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_32(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_31(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_33(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_32(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_34(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_33(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_35(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_34(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_36(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_35(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_37(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_36(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_38(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_37(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_39(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_38(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_40(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_39(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_41(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_40(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_42(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_41(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_43(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_42(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_44(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_43(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_45(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_44(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_46(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_45(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_47(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_46(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_48(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_47(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_49(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_48(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_50(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_49(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_51(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_50(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_52(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_51(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_53(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_52(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_54(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_53(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_55(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_54(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_56(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_55(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_57(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_56(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_58(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_57(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_59(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_58(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_60(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_59(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_61(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_60(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_62(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_61(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_63(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_62(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_64(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_63(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_65(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_64(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_66(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_65(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_67(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_66(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_68(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_67(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_69(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_68(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_70(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_69(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_71(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_70(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_72(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_71(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_73(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_72(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_74(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_73(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_75(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_74(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_76(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_75(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_77(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_76(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_78(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_77(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_79(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_78(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_80(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_79(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_81(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_80(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_82(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_81(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_83(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_82(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_84(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_83(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_85(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_84(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_86(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_85(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_87(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_86(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_88(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_87(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_89(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_88(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_90(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_89(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_91(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_90(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_92(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_91(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_93(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_92(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_94(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_93(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_95(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_94(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_96(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_95(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_97(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_96(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_98(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_97(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_99(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_98(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_100(macro, ...) __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))) __P_FOREACH_99(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_1(macro, param, item)   __P_EXPAND(macro(param,item))
#define __P_PARAM_FOREACH_2(macro, param, ...)   __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_1(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_3(macro, param, ...)   __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_2(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_4(macro, param, ...)   __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_3(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_5(macro, param, ...)   __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_4(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_6(macro, param, ...)   __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_5(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_7(macro, param, ...)   __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_6(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_8(macro, param, ...)   __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_7(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_9(macro, param, ...)   __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_8(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_10(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_9(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_11(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_10(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_12(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_11(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_13(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_12(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_14(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_13(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_15(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_14(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_16(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_15(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_17(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_16(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_18(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_17(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_19(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_18(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_20(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_19(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_21(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_20(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_22(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_21(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_23(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_22(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_24(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_23(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_25(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_24(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_26(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_25(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_27(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_26(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_28(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_27(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_29(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_28(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_30(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_29(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_31(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_30(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_32(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_31(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_33(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_32(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_34(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_33(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_35(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_34(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_36(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_35(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_37(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_36(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_38(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_37(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_39(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_38(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_40(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_39(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_41(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_40(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_42(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_41(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_43(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_42(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_44(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_43(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_45(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_44(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_46(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_45(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_47(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_46(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_48(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_47(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_49(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_48(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_50(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_49(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_51(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_50(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_52(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_51(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_53(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_52(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_54(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_53(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_55(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_54(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_56(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_55(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_57(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_56(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_58(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_57(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_59(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_58(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_60(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_59(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_61(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_60(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_62(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_61(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_63(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_62(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_64(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_63(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_65(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_64(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_66(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_65(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_67(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_66(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_68(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_67(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_69(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_68(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_70(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_69(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_71(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_70(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_72(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_71(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_73(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_72(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_74(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_73(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_75(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_74(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_76(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_75(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_77(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_76(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_78(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_77(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_79(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_78(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_80(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_79(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_81(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_80(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_82(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_81(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_83(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_82(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_84(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_83(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_85(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_84(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_86(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_85(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_87(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_86(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_88(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_87(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_89(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_88(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_90(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_89(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_PARAM_FOREACH_91(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_90(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_92(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_91(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_93(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_92(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_94(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_93(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_95(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_94(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_96(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_95(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_97(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_96(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_98(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_97(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_99(macro, param, ...)  __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_98(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_PARAM_FOREACH_100(macro, param, ...) __P_EXPAND(macro(param,__P_GET_FIRST_ARG(__VA_ARGS__))) __P_PARAM_FOREACH_99(macro, param, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_1(macro, item)  __P_EXPAND(macro(item))
#define __P_FOREACH_COMMA_2(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_1(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_3(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_2(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_4(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_3(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_5(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_4(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_6(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_5(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_7(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_6(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_8(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_7(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_9(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_8(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_10(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_9(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_11(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_10(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_12(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_11(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_13(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_12(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_14(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_13(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_15(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_14(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_16(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_15(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_17(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_16(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_18(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_17(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_19(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_18(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_20(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_19(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_21(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_20(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_22(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_21(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_23(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_22(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_24(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_23(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_25(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_24(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_26(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_25(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_27(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_26(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_28(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_27(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_29(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_28(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_30(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_29(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_31(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_30(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_32(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_31(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_33(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_32(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_34(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_33(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_35(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_34(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_36(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_35(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_37(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_36(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_38(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_37(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_39(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_38(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_40(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_39(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_41(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_40(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_42(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_41(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_43(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_42(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_44(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_43(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_45(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_44(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_46(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_45(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_47(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_46(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_48(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_47(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_49(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_48(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_50(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_49(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_51(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_50(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_52(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_51(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_53(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_52(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_54(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_53(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_55(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_54(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_56(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_55(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_57(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_56(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_58(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_57(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_59(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_58(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_60(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_59(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_61(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_60(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_62(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_61(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_63(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_62(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_64(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_63(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_65(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_64(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_66(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_65(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_67(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_66(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_68(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_67(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_69(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_68(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_70(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_69(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_71(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_70(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_72(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_71(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_73(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_72(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_74(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_73(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_75(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_74(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_76(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_75(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_77(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_76(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_78(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_77(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_79(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_78(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_80(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_79(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_81(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_80(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_82(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_81(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_83(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_82(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_84(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_83(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_85(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_84(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_86(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_85(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_87(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_86(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_88(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_87(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_89(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_88(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_90(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_89(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_COMMA_91(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_90(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_92(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_91(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_93(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_92(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_94(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_93(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_95(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_94(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_96(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_95(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_97(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_96(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_98(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_97(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_99(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_98(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_COMMA_100(macro, ...) __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_COMMA_99(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_1(macro, item)  __P_EXPAND(macro(item))
#define __P_FOREACH_SEMICOLON_2(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_1(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_3(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_2(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_4(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_3(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_5(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_4(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_6(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_5(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_7(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_6(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_8(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_7(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_9(macro, ...)   __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_8(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_10(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_9(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_11(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_10(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_12(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_11(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_13(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_12(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_14(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_13(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_15(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_14(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_16(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_15(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_17(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_16(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_18(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_17(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_19(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_18(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_20(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_19(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_21(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_20(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_22(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_21(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_23(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_22(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_24(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_23(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_25(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_24(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_26(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_25(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_27(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_26(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_28(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_27(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_29(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_28(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_30(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_29(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_31(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_30(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_32(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_31(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_33(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_32(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_34(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_33(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_35(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_34(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_36(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_35(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_37(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_36(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_38(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_37(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_39(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_38(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_40(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_39(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_41(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_40(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_42(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_41(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_43(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_42(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_44(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_43(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_45(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_44(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_46(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_45(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_47(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_46(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_48(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_47(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_49(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_48(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_50(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_49(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_51(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_50(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_52(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_51(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_53(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_52(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_54(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_53(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_55(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_54(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_56(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_55(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_57(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_56(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_58(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_57(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_59(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_58(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_60(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_59(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_61(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_60(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_62(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_61(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_63(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_62(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_64(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_63(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_65(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_64(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_66(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_65(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_67(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_66(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_68(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_67(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_69(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_68(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_70(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_69(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_71(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_70(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_72(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_71(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_73(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_72(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_74(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_73(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_75(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_74(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_76(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_75(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_77(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_76(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_78(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_77(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_79(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_78(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_80(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_79(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_81(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_80(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_82(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_81(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_83(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_82(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_84(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_83(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_85(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_84(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_86(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_85(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_87(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_86(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_88(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_87(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_89(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_88(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_90(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_89(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))

#define __P_FOREACH_SEMICOLON_91(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_90(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_92(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_91(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_93(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_92(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_94(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_93(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_95(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_94(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_96(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_95(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_97(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_96(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_98(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_97(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_99(macro, ...)  __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_98(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
#define __P_FOREACH_SEMICOLON_100(macro, ...) __P_EXPAND(macro(__P_GET_FIRST_ARG(__VA_ARGS__))), __P_FOREACH_SEMICOLON_99(macro, __P_DROP_FIRST_ARG(__VA_ARGS__))
