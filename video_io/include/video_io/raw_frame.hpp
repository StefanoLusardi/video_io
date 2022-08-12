#pragma once

#include <vector>
#include <cstdint>

namespace vc
{
struct raw_frame
{
    explicit raw_frame() = default;
    ~raw_frame() = default;
    
    std::vector<uint8_t> data = std::vector<uint8_t>();
	double pts = 0.0;
};

}