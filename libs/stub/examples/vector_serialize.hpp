#ifndef _VECTOR_SERIALIZE_HPP_
#define _VECTOR_SERIALIZE_HPP_
#include <boost/serialization/serialization.hpp>
#include <boost/fusion/include/for_each.hpp>

namespace mace { namespace stub {
template<typename Archive>
struct item_serializer {
    item_serializer(Archive& ar):ar(ar) {}

    template<typename T>
    void operator()(const T& o) const {
        ar << o;
    }
    Archive& ar;
};

template<typename Archive, typename V>
Archive& serialize_fusion_vector(Archive& ar, const V& v) {
    boost::fusion::for_each(v, item_serializer<Archive>(ar));
    return ar;
}

template<typename Archive>
struct item_deserializer {
    item_deserializer(Archive& ar):ar(ar) {}

    template<typename T>
    void operator()(T& o) const {
        ar >> o;
    }
    Archive& ar;
};

template<typename Archive, typename V>
Archive& deserialize_fusion_vector(Archive& ar, V& v) {
    boost::fusion::for_each(v, item_deserializer<Archive>(ar));
    return ar;
}

} }

#endif // _VECTOR_SERIALIZE_HPP_
