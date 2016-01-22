#ifndef libdasvch_xTYPES_H_
#define libdasvch_xTYPES_H_

#include <map>
#include <vector>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_array.hpp>

typedef int32_t SegmentID;
typedef int32_t RepresentationID;

typedef boost::tuple<SegmentID, RepresentationID> SegRepPair;

typedef boost::tuple<
            boost::shared_array<const char>, int32_t> DataLenPair;

#define DATA(X) (X.get<0>().get())
#define LEN(X) (X.get<1>())

typedef std::map<SegmentID,
                 std::vector<RepresentationID> > SimpleBufStatus;

typedef std::map<RepresentationID,
                       DataLenPair> RepDataMap;

typedef std::map<SegmentID,
                 RepDataMap > BufferMap;

#endif /* libdasvch_xTYPES_H_ */
