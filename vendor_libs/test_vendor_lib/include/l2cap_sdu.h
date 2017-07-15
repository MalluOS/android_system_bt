/******************************************************************************
 *
 *  Copyright (C) 2017 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#pragma once
#include <cstdint>
#include <iterator>
#include <vector>

namespace test_vendor_lib {

// Abstract representation of an SDU packet that contains an L2CAP
// payload. This class is meant to be used in collaboration with
// the L2cap class defined in l2cap.h. For example, an SDU packet
// may look as follows:
//
// vector<uint8_t> sdu = {0x04, 0x00, 0x48, 0x00, 0x04, 0x00, 0xab,
//                       0xcd, 0x78, 0x56}
//
// The first two bytes (in little endian) should be read as 0x0004
// and is the length of the payload of the SDU packet.
//
// The next two bytes (also in little endian) are the channel ID
// and should be read as 0x0048. These should remain the same for
// any number of SDUs that are a part of the same packet stream.
//
// Following the CID bytes, are the total length bytes. Since this
// SDU only requires a single packet, the length here is the same
// as the length in the first two bytes of the packet. Again stored
// in little endian.
//
// Next comes the two control bytes. These begin the L2CAP payload
// of the SDU packet; however, they will not be added to the L2CAP
// packet that is being constructed in the assemble function that
// will be creating an L2CAP packet from a stream of L2capSdu
// objects.
//
// The final two bytes are the frame check sequence that should be
// calculated from the start of the vector to the end of the
// payload.
//
// Thus, calling assemble on this example would create a
// zero-length L2CAP packet because the information payload of the
// L2CAP packet will not include either of the control or FCS
// bytes.
//
class L2capSdu {
 public:
  // Returns a completed L2capSdu object.
  L2capSdu(std::vector<uint8_t> create_from);

  // Adds an FCS to create_from and returns an L2capSdu object
  static L2capSdu L2capSduBuilder(std::vector<uint8_t> create_from);

  // Get a vector iterator that points to the first byte of the
  // L2CAP payload within an SDU. The offset parameter will be the
  // number of bytes that are in the SDU header. This should always
  // be 6 bytes with the exception being the first SDU of a stream
  // of SDU packets where the first SDU packet will have an extra
  // two bytes and the offset should be 8 bytes.
  auto get_payload_begin(const unsigned int offset) const {
    return std::next(sdu_data_.begin(), offset);
  }

  // Get a vector iterator that points to the last bytes of the
  // L2CAP payload within an SDU packet. There is no offset
  // parameter for this function because there will always be two
  // FCS bytes and nothing else at the end of each SDU.
  auto get_payload_end() const { return std::prev(sdu_data_.end(), 2); }

  // Get the FCS bytes from the end of the L2CAP payload of an SDU
  // packet.
  uint16_t get_fcs() const;

  uint16_t get_payload_length() const;

  uint16_t calculate_fcs() const;

  // Get the two control bytes that begin the L2CAP payload. These
  // bytes will contain information such as the Segmentation and
  // Reassembly bits, and the TxSeq/ReqSeq numbers.
  uint16_t get_controls() const;

  uint16_t get_total_l2cap_length() const;

  size_t get_vector_size() const;

  uint16_t get_channel_id() const;

 private:
  // This is the SDU packet in bytes.
  std::vector<uint8_t> sdu_data_;

  // Table for precalculated lfsr values.
  static const uint16_t lfsr_table_[256];

  uint16_t convert_from_little_endian(const unsigned int starting_index) const;

};  // L2capSdu

}  // namespace test_vendor_lib