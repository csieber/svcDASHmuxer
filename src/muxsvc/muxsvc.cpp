#include "muxsvc.h"
#include <vector>
#include <cstring>
#include <boost/assert.hpp>

namespace muxsvc {

DataLenPair muxsvc(DataLenPair lower_layer, DataLenPair higher_layer, int32_t d_id_higher)
{
    BOOST_ASSERT(d_id_higher >= 1);

    // Use a vector to store the muxed data
    std::vector<char> muxed_data;

    // Reserve the size for the muxed data
    muxed_data.reserve(lower_layer.get<1>() + higher_layer.get<1>());

    const char* data_lower = lower_layer.get<0>().get();
    const char* data_higher = higher_layer.get<0>().get();

    // Position in the lower & higher layer data
    int32_t pos_lower  = 0;
    int32_t pos_higher = 0;

    StartSizePair next_higher_nal_pos(-1, -1);

    bool WORKAROUND_insert_double = false;

    // Loop through the lower layer's NAL unit
    while (true) {

        // Position of the next NAL unit in the stream
        StartSizePair next_nal_pos = next_nal(lower_layer, pos_lower);

        // We reached end of stream
        if (next_nal_pos.first == -1) {

            // We copy the rest of the higher data ..
            muxed_data.insert(muxed_data.end(),
                              &data_higher[pos_higher],
                              &data_higher[higher_layer.get<1>()]);

            // .. and exit
            break;
        }

        BOOST_ASSERT(next_nal_pos.second >= 5);

        // Find out what kind of NAL unit it is.
        nal_hdr hdr = parse_nal(lower_layer, next_nal_pos.first).first;

//        std::cout << "Type: " << (uint32_t)hdr.nal_unit_type << std::endl;

        bool insert_higher_nal = false;

        /*
         * If we are dealing with the dependency layer 1 we look for additional
         * NAL types.
         */
        if (d_id_higher == 1) {

            // After NAL types 1 and 5 we insert the higher layer NAL unit
            if ((uint32_t)hdr.nal_unit_type == NAL_T_CODED_SLICE_OF_AN_IDR_PICTURE ||
                (uint32_t)hdr.nal_unit_type == NAL_T_CODED_SLICE_OF_A_NON_IDR_PICTURE) {

                insert_higher_nal = true;

                // Check if we already got the next higher NAL unit
                if (next_higher_nal_pos.first == -1) {

                    next_higher_nal_pos = next_nal(higher_layer, pos_higher);
                    BOOST_ASSERT(next_higher_nal_pos.second != -1);

                    pos_higher += next_higher_nal_pos.second;
                }
            }
        } else {

            // After NAL type 20 we have to check if the NAL unit of type 20
            // belongs to the correct dependency layer.
            if ((uint32_t)hdr.nal_unit_type == NAL_T_SLICE_LAYER_EXTENSION_RBSP) {

                BOOST_ASSERT(next_nal_pos.second >= 9);

                // that means we have to parse the extension header to.
                nal_hdr_svc hdr_svc = parse_svc_nal(lower_layer, next_nal_pos.first).second;

//                std::cout << "SVC NAL Unit Lower: Dependency_id: " << (uint32_t)hdr_svc.dependency_id << std::endl;

                // Now we have to check if the temporal_id of the higher layer matches with the
                // lower one and we have to check of the dependency ids are in order of we
                // insert the higher one here.

                // Check if we already got the next higher NAL unit
                if (next_higher_nal_pos.first == -1) {

                    next_higher_nal_pos = next_nal(higher_layer, pos_higher);
                    BOOST_ASSERT(next_higher_nal_pos.second != -1);

                    pos_higher += next_higher_nal_pos.second;
                }

                // NAL Units of the higher layer have to consist of only NAL units
                // with the extension header.
                BOOST_ASSERT(next_higher_nal_pos.second >= 9);

                nal_hdr_svc hdr_svc_higher
                                = parse_svc_nal(higher_layer,
                                                next_higher_nal_pos.first
                                                    ).second;

//                std::cout << "SVC NAL Unit higher: Dependency_id: " << (uint32_t)hdr_svc_higher.dependency_id << std::endl;

                // We only insert the higher layer here if it matches
//TODO find out if this always works with the quality_ids that way
                if (((uint32_t)hdr_svc.dependency_id + 1) == (uint32_t)hdr_svc_higher.dependency_id &&
                    (uint32_t)hdr_svc.temporal_id == (uint32_t)hdr_svc_higher.temporal_id) {

                    insert_higher_nal = true;

                    WORKAROUND_insert_double = equals_next_nal_unit(higher_layer, next_higher_nal_pos.first + next_higher_nal_pos.second, hdr_svc_higher);
                }
            }
        }

        // First we insert the NAL from the lower layer
        muxed_data.insert(muxed_data.end(),
                          &data_lower[next_nal_pos.first],
                          &data_lower[next_nal_pos.first + next_nal_pos.second]);

        pos_lower += next_nal_pos.second;

        /*
         * We insert a NAL unit from the higher layer here.
         */
        if (insert_higher_nal) {

//            std::cout << "INSERTING HIGHER LAYER NAL, starting from " << pos_higher << " of " << higher_layer.second << std::endl;

            // This must not happen
            BOOST_ASSERT(next_higher_nal_pos.second != -1);

            // Insert the data
            muxed_data.insert(muxed_data.end(),
                            &data_higher[next_higher_nal_pos.first],
                            &data_higher[next_higher_nal_pos.first + next_higher_nal_pos.second]);

            next_higher_nal_pos = StartSizePair(-1, -1);

            //TODO WORKAROUND
            // Check if it was a splitted SVC NAL unit
            // i.e. if the next NAL unit is the same, we insert it to
            if (WORKAROUND_insert_double) {

                next_higher_nal_pos = next_nal(higher_layer, pos_higher);
                BOOST_ASSERT(next_higher_nal_pos.second != -1);

                pos_higher += next_higher_nal_pos.second;

                // Insert the data
                muxed_data.insert(muxed_data.end(),
                                &data_higher[next_higher_nal_pos.first],
                                &data_higher[next_higher_nal_pos.first + next_higher_nal_pos.second]);

                next_higher_nal_pos = StartSizePair(-1, -1);
            }
        }
    }

    // The size has to be correct
    BOOST_ASSERT(muxed_data.size() == (lower_layer.get<1>() + higher_layer.get<1>()));

    // Copy the data out of the vector and return it
    char* tmp = new char[muxed_data.size()];

    memcpy(tmp, &muxed_data[0], muxed_data.size());

    return DataLenPair(boost::shared_array<const char>(tmp), muxed_data.size());
}

DataLenPair muxsvc_vec(std::vector<DataLenPair> svc_vec)
{
    if (svc_vec.empty())
        return DataLenPair();

    if (svc_vec.size() == 1)
        return svc_vec[0];

    DataLenPair muxed_nal
        = muxsvc(svc_vec[0], svc_vec[1], 1);

    for (unsigned int i = 2; i < svc_vec.size(); i++) {

        muxed_nal = muxsvc(muxed_nal, svc_vec[i], i);
    }

    return muxed_nal;
}

NalHeaders parse_nal(DataLenPair search_data, int32_t start)
{
    nal_hdr hdr;
    memcpy(&hdr, &search_data.get<0>().get()[start + sizeof(nal_sep)], sizeof(nal_hdr));

    return NalHeaders(hdr, nal_hdr_svc());
}

NalHeaders parse_svc_nal(DataLenPair search_data, int32_t start)
{
    NalHeaders hdrs = parse_nal(search_data, start);

    BOOST_ASSERT((uint32_t)hdrs.first.nal_unit_type == NAL_T_SLICE_LAYER_EXTENSION_RBSP);

    const int32_t base = start + sizeof(nal_sep);

    char tmp[3];
    tmp[0] = search_data.get<0>().get()[base + 3];
    tmp[1] = search_data.get<0>().get()[base + 2];
    tmp[2] = search_data.get<0>().get()[base + 1];
    tmp[3] = '\0';

    nal_hdr_svc hdr_svc;

    memcpy(&hdr_svc, tmp, 4);

    hdrs.second = hdr_svc;

    return hdrs;
}

bool equals_next_nal_unit(DataLenPair search_data, int32_t start, nal_hdr_svc to_compare)
{
    StartSizePair next_nal_pair = next_nal(search_data, start);

    if (next_nal_pair.second == -1)
        return false;

    nal_hdr_svc hdr_svc = parse_svc_nal(search_data,start).second;

    if ((uint32_t)hdr_svc.dependency_id == (uint32_t)to_compare.dependency_id &&
        (uint32_t)hdr_svc.temporal_id == (uint32_t)to_compare.temporal_id &&
        (uint32_t)hdr_svc.quality_id == (uint32_t)to_compare.quality_id)
        return true;
    else
        return false;
}

StartSizePair next_nal(DataLenPair search_data, int32_t curr_pos)
{
    const char* data         = search_data.get<0>().get();
    const int32_t& data_len = search_data.get<1>();

    int32_t beginning = -1;

    while (curr_pos < data_len) {

        // Are we at the beginning of a NAL unit?
        if (memcmp(&data[curr_pos], &nal_sep[0], sizeof(nal_sep)) == 0) {

            // Are we looking for the beginning?
            if (beginning == -1) {

                beginning = curr_pos;
                curr_pos++;

            } else {

                return StartSizePair(beginning, curr_pos - beginning);
            }

        // If not forward the stream
        } else {

            curr_pos++;
        }
    }

    // That means there are no NAL units left in this stream
    if (beginning == -1) {

        return StartSizePair(-1, -1);

    // This means we reached the end of the stream
    } else {

        return StartSizePair(beginning, data_len - beginning);
    }
}

} // namespace muxsvc
