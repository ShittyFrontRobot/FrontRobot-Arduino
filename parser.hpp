//
// Created by berberman on 2019/12/31.
//

#ifndef FRONTROBOT_PARSER_HPP
#define FRONTROBOT_PARSER_HPP

#include <ArduinoSTL.h>
#include <func_exception>
#include <deque>
#include <numeric>
#include "consts.hpp"

template<class parser_t>
struct parse_engine_t {
    using word_t     = typename parser_t::word_t;
    using result_t   = typename parser_t::result_t;

    template<class iterator_t>
    void operator()(iterator_t begin,
                    iterator_t end,
                    void (*callback)(result_t &)) {
        // 连接到解析缓冲区
        buffer.insert(buffer.end(), begin, end);
        // 初始化迭代器
        decltype(buffer.begin())
                parse_begin = buffer.begin(),
                parse_end;
        // 解析到全部已检查
        do {
            parse_end = buffer.end();
            auto result = parser(parse_begin, parse_end);
            callback(result);
        } while (parse_end < buffer.end());
        // 清除已解析部分
        buffer.erase(buffer.begin(), parse_begin);
    }

private:
    std::deque<word_t> buffer{};
    parser_t parser;
};

enum class result_type_t : uint8_t {
    success, failed, nothing
};
template<size_t size>
struct packet_t {
    uint8_t data[size];
};

template<class iterator_t>
bool xor_check(iterator_t begin, iterator_t end) {
    auto check = std::accumulate(begin, end - 1, static_cast<uint8_t>(0),
                                 [](uint8_t acc, uint8_t it) { return acc ^ it; });
    return check == *(end - 1);
}

namespace nano {
    using motor_speed_packet_t    = packet_t<motor_speed_packet_info.size>;
    using motor_state_packet_t    = packet_t<motor_state_packet_info.size>;
    using encoder_data_packet_t   = packet_t<encoder_data_packet_info.size>;
    using encoder_reset_packet_t  = packet_t<encoder_reset_packet_info.size>;

    struct parser_t {

        using word_t = uint8_t;

        struct result_t {

            result_type_t type;

            union {
                word_t bytes[sizeof(motor_speed_packet_t) - 3];
                motor_speed_packet_t speeds;
                motor_state_packet_t states;
                encoder_reset_packet_t reset;
            };

            packet_info_t const *info{};

        };

        /**
             * 执行解析
             *
             * @tparam iterator_t 输入迭代器类型
             * @param begin 缓存起点迭代器 -> 下一帧起点迭代器
             * @param end   缓存终点迭代器 -> 本次解析终点迭代器
             * @return 结果类型
             */
        template<class iterator_t>
        result_t operator()(iterator_t &begin, iterator_t &end) const {
            while (*begin++ != packet_head);
            result_t result{result_type_t::nothing, {packet_head, *begin--}};

            packet_info_t const *info;
            switch (*begin++) {
                case motor_speed_packet_info.type:
                    info = &motor_speed_packet_info;
                    break;
                case motor_state_packet_info.type:
                    info = &motor_state_packet_info;
                    break;
                case encoder_reset_packet_info.type:
                    info = &encoder_reset_packet_info;
                    break;
                default:
                    return result;
            }
            auto size = info->size;
            if (end - begin < size) return result;
            auto frame_end = begin + size;
            if (xor_check(begin + 1, frame_end)) {
                std::copy(begin + 2, frame_end - 1, result.bytes + 2);
                result.type = result_type_t::success;
                result.info = info;
                begin = frame_end;
            } else
                result.type = result_type_t::failed;
            while (begin < end && *begin != packet_head) ++begin;
            end = begin;
            return result;
        }
    };

}

/*
namespace stm {
    using encoder_data_packet_t = packet_t<encoder_data_packet_info.size>;
    using encoder_reset_packet_t = packet_t<encoder_reset_packet_info.size>;

    struct parser_t {

        struct result_t {

            result_type_t type;

            packet_info_t const *info{};

            union {
                uint8_t bytes[sizeof(encoder_data_packet_info) - 3];
                encoder_data_packet_t data;
                encoder_reset_packet_t reset;
            };
        };

        template<class iterator_t>
        result_t operator()(iterator_t &begin, iterator_t &end) const {
            while (*begin++ != packet_head);
            result_t result{result_type_t::nothing, {packet_head, *begin--}};

            packet_info_t const *info;
            switch (*begin++) {
                case encoder_data_packet_info.type:
                    info = &encoder_data_packet_info;
                    break;
                case encoder_reset_packet_info.type:
                    info = &encoder_reset_packet_info;
                    break;
                default:
                    return result;
            }
            auto size = info->size;
            if (end - begin < size) return result;
            auto frame_end = begin + size;
            if (xor_check(begin + 1, frame_end)) {
                std::copy(begin + 2, frame_end - 1, result.bytes + 2);
                result.type = result_type_t::success;
                result.info = info;
                begin = frame_end;
            } else
                result.type = result_type_t::failed;
            while (begin < end && *begin != packet_head) ++begin;
            end = begin;
            return result;
        }
    };


}
*/


#endif //FRONTROBOT_PARSER_HPP
