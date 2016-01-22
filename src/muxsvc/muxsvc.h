#ifndef MUXSVC_H_
#define MUXSVC_H_

#include <inttypes.h>
#include <vector>
#include <boost/shared_array.hpp>
#include "types.h"
#include "../types.h"

namespace muxsvc {

DataLenPair muxsvc(DataLenPair lower_layer, DataLenPair higher_layer, int32_t d_id_higher);

DataLenPair muxsvc_vec(std::vector<DataLenPair> svc_vec);


typedef std::pair<int32_t, int32_t> StartSizePair;

StartSizePair next_nal(DataLenPair search_data, int32_t curr_pos);

typedef std::pair<nal_hdr, nal_hdr_svc> NalHeaders;

NalHeaders parse_nal(DataLenPair search_data, int32_t start);
NalHeaders parse_svc_nal(DataLenPair search_data, int32_t start);

bool equals_next_nal_unit(DataLenPair search_data, int32_t start, nal_hdr_svc to_compare);

} // namespace muxsvc

#endif /* MUXSVC_H_ */
