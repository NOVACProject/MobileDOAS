#include "catch.hpp"
#include <MobileDoasLib/Measurement/MeasuredSpectrum.h>
#include <numeric>

using namespace mobiledoas;


TEST_CASE("MeasuredSpectrum - Default constructor sets maximum size", "[MeasuredSpectrum]")
{
    // Act
    MeasuredSpectrum sut;

    // Assert
    REQUIRE(sut.NumberOfChannels() == 2);
    REQUIRE(sut.SpectrumLength() == 3648);
}

TEST_CASE("MeasuredSpectrum - Size constructor sets expected size", "[MeasuredSpectrum]")
{
    // Act
    MeasuredSpectrum sut(3, 1468);

    // Assert
    REQUIRE(sut.NumberOfChannels() == 3);
    REQUIRE(sut.SpectrumLength() == 1468);
}

TEST_CASE("MeasuredSpectrum - Resize changes both channel size and spectrum length", "[MeasuredSpectrum]")
{
    // Arrange
    MeasuredSpectrum sut(3, 1468);

    // Act
    sut.Resize(7, 1796);

    // Assert
    REQUIRE(7 == sut.data.size());
    REQUIRE(1796 == sut.data[0].size());
    REQUIRE(1796 == sut.data[1].size());
    REQUIRE(1796 == sut.data[2].size());
    REQUIRE(1796 == sut.data[6].size());
    REQUIRE(sut.NumberOfChannels() == 7);
    REQUIRE(sut.SpectrumLength() == 1796);
}

TEST_CASE("MeasuredSpectrum SetToZero resets all values to zero", "[MeasuredSpectrum]")
{
    // Arrange, fill up the sut with some values
    MeasuredSpectrum sut(3, 1468);
    std::iota(begin(sut[0]), end(sut[0]), 2);
    std::iota(begin(sut[1]), end(sut[1]), 5);
    std::iota(begin(sut[2]), end(sut[2]), 9);
    // Check some assumptions here
    REQUIRE(Approx(2) == sut[0][0]);
    REQUIRE(Approx(2 + 1467) == sut[0][1467]);
    REQUIRE(Approx(5) == sut[1][0]);
    REQUIRE(Approx(5 + 1467) == sut[1][1467]);
    REQUIRE(Approx(9) == sut[2][0]);
    REQUIRE(Approx(9 + 1467) == sut[2][1467]);

    // Act
    sut.SetToZero();

    // Assert, the size didn't change but the values are all zero
    REQUIRE(sut.NumberOfChannels() == 3);
    REQUIRE(sut.SpectrumLength() == 1468);

    REQUIRE(Approx(0) == sut[0][0]);
    REQUIRE(Approx(0) == sut[0][1467]);
    REQUIRE(Approx(0) == sut[1][0]);
    REQUIRE(Approx(0) == sut[1][1467]);
    REQUIRE(Approx(0) == sut[2][0]);
    REQUIRE(Approx(0) == sut[2][1467]);
}

TEST_CASE("MeasuredSpectrum - CopyTo resizes destination to this and copies data", "[MeasuredSpectrum]")
{
    // Arrange
    MeasuredSpectrum source(2, 67);
    std::iota(begin(source[0]), end(source[0]), 2);
    std::iota(begin(source[1]), end(source[1]), 5);
    MeasuredSpectrum destination;

    // Act
    source.CopyTo(destination);

    // Assert
    REQUIRE(2 == destination.data.size());
    REQUIRE(67 == destination.data[0].size());
    REQUIRE(67 == destination.data[1].size());

    REQUIRE(Approx(2) == destination[0][0]);
    REQUIRE(Approx(2 + 66) == destination[0][66]);
    REQUIRE(Approx(5) == destination[1][0]);
    REQUIRE(Approx(5 + 66) == destination[1][66]);
}

TEST_CASE("MeasuredSpectrum - CopyFrom copies all data from source", "[MeasuredSpectrum]")
{
    // Arrange
    double sourceData[] { 11, 22, 33, 44, 55, 66, 77, 88, 99, 101 };

    SECTION("desination is too short when copying to channel zero")
    {
        MeasuredSpectrum destination(2, 3);

        // Act
        destination.CopyFrom(0, sourceData, 10);

        // Assert, the destination was resized and all values copied over
        REQUIRE(2 == destination.data.size());
        REQUIRE(10 == destination.data[0].size());
        REQUIRE(10 == destination.data[1].size());

        REQUIRE(Approx(11) == destination.data[0][0]);
        REQUIRE(Approx(101) == destination.data[0][9]);
    }

    SECTION("destination longer than source")
    {
        MeasuredSpectrum destination(2, 19);

        // Act
        destination.CopyFrom(0, sourceData, 10);

        // Assert, the destination was resized and all values copied over
        REQUIRE(2 == destination.data.size());
        REQUIRE(10 == destination.data[0].size());
        REQUIRE(10 == destination.data[1].size());

        REQUIRE(Approx(11) == destination.data[0][0]);
        REQUIRE(Approx(101) == destination.data[0][9]);
    }
}