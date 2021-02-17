#include <gtest/gtest.h>
#include "../app/input_reader.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(Stop, Test) {
    using namespace std;
    std::string parsable_string = "Stop Tolstopaltsevo: 55.611087, 37.208290"s;

    Request *a = ParseRequestString(parsable_string);
    ASSERT_EQ(a->getRequestType(), RequestType::IsStop);
    ASSERT_EQ(a->getName(), "Tolstopaltsevo"s);

    ASSERT_DOUBLE_EQ(static_cast<Stop*>(a)->coordinates.lat, 55.611087);
    ASSERT_DOUBLE_EQ(static_cast<Stop*>(a)->coordinates.lng, 37.208290);
    delete a;
}

TEST(Bus, Test) {
    using namespace std;
    std::string parsable_string = "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s;

    Request *a = ParseRequestString(parsable_string);
    ASSERT_EQ(a->getRequestType(), RequestType::IsStop);
    ASSERT_EQ(a->getName(), "Tolstopaltsevo"s);

    ASSERT_DOUBLE_EQ(static_cast<Stop*>(a)->coordinates.lat, 55.611087);
    ASSERT_DOUBLE_EQ(static_cast<Stop*>(a)->coordinates.lng, 37.208290);
    delete a;
}