/**
 * libdatachannel streamer example
 * Copyright (c) 2020 Filip Klembara (in2core)
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef h264fileparser_hpp
#define h264fileparser_hpp

#include "fileparser.hpp"
#include "rtc/common.hpp"

class H264FileParser: public FileParser {
    rtc::optional<std::vector<rtc::byte>> previousUnitType5 = rtc::nullopt;
    rtc::optional<std::vector<rtc::byte>> previousUnitType7 = rtc::nullopt;
    rtc::optional<std::vector<rtc::byte>> previousUnitType8 = rtc::nullopt;

public:
    H264FileParser(std::string directory, uint32_t fps, bool loop);
    void loadNextSample() override;
    std::vector<rtc::byte> initialNALUS();
};

#endif /* h264fileparser_hpp */
