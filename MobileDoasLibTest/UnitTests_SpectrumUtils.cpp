#include "catch.hpp"
#include <MobileDoasLib/Measurement/SpectrumUtils.h>

using namespace mobiledoas;

#pragma region AdjustIntegrationTimeToLastIntensity

TEST_CASE("AdjustIntegrationTimeToLastIntensity - Current intensity within tolerable rate returns current integration time", "[AdjustIntegrationTimeToLastIntensity]")
{
    double minTolerableRatio = 0.60;
    double maxTolerableRatio = 0.70;
    long minAllowedExposureTime = 10;
    long maxAllowedExposureTime = 1000;

    SECTION("Current ratio on lower edge")
    {
        // Arrange
        short currentIntegrationTime = 156;

        // Act
        short result = AdjustIntegrationTimeToLastIntensity(
            SpectrumIntensityMeasurement(minTolerableRatio, currentIntegrationTime),
            minTolerableRatio,
            maxTolerableRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(currentIntegrationTime == result);
    }

    SECTION("Current ratio on upper edge")
    {
        // Arrange
        short currentIntegrationTime = 156;

        // Act
        short result = AdjustIntegrationTimeToLastIntensity(
            SpectrumIntensityMeasurement(maxTolerableRatio, currentIntegrationTime),
            minTolerableRatio,
            maxTolerableRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(currentIntegrationTime == result);
    }
}

TEST_CASE("AdjustIntegrationTimeToLastIntensity - Current intensity too low, increases integration time", "[AdjustIntegrationTimeToLastIntensity]")
{
    double minTolerableRatio = 0.60;
    double maxTolerableRatio = 0.70;
    long minAllowedExposureTime = 10;
    long maxAllowedExposureTime = 1000;

    SECTION("Current ratio lower than desired")
    {
        // Arrange
        double currentRatio = 0.30;
        short currentIntegrationTime = 156;

        // Act
        short result = AdjustIntegrationTimeToLastIntensity(
            SpectrumIntensityMeasurement(currentRatio, currentIntegrationTime),
            minTolerableRatio,
            maxTolerableRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(result > currentIntegrationTime);
    }

    SECTION("Current ratio very low")
    {
        // Arrange
        double currentRatio = 0.01;
        short currentIntegrationTime = 156;

        // Act
        short result = AdjustIntegrationTimeToLastIntensity(
            SpectrumIntensityMeasurement(currentRatio, currentIntegrationTime),
            minTolerableRatio,
            maxTolerableRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(result > currentIntegrationTime);
    }

    SECTION("Current lower than desired but integration time equals maximum allowed - returns maximum allowed")
    {
        // Arrange
        double currentRatio = 0.30;
        short currentIntegrationTime = maxAllowedExposureTime;

        // Act
        short result = AdjustIntegrationTimeToLastIntensity(
            SpectrumIntensityMeasurement(currentRatio, currentIntegrationTime),
            minTolerableRatio,
            maxTolerableRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(result == maxAllowedExposureTime);
    }
}

TEST_CASE("AdjustIntegrationTimeToLastIntensity - Current intensity too high, lowers integration time", "[AdjustIntegrationTimeToLastIntensity]")
{
    double minTolerableRatio = 0.60;
    double maxTolerableRatio = 0.70;
    long minAllowedExposureTime = 10;
    long maxAllowedExposureTime = 1000;

    SECTION("Current ratio higher than desired")
    {
        // Arrange
        double currentRatio = 0.80;
        short currentIntegrationTime = 156;

        // Act
        short result = AdjustIntegrationTimeToLastIntensity(
            SpectrumIntensityMeasurement(currentRatio, currentIntegrationTime),
            minTolerableRatio,
            maxTolerableRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(result < currentIntegrationTime);
    }

    SECTION("Current spectrum saturated")
    {
        // Arrange
        double currentRatio = 1.0;
        short currentIntegrationTime = 156;

        // Act
        short result = AdjustIntegrationTimeToLastIntensity(
            SpectrumIntensityMeasurement(currentRatio, currentIntegrationTime),
            minTolerableRatio,
            maxTolerableRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(result < currentIntegrationTime);
    }

    SECTION("Current higher than desired but integration time equals minimum allowed - returns minimum allowed")
    {
        // Arrange
        double currentRatio = 0.90;
        short currentIntegrationTime = minAllowedExposureTime;

        // Act
        short result = AdjustIntegrationTimeToLastIntensity(
            SpectrumIntensityMeasurement(currentRatio, currentIntegrationTime),
            minTolerableRatio,
            maxTolerableRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(result == minAllowedExposureTime);
    }
}

#pragma endregion AdjustIntegrationTimeToLastIntensity

#pragma region EstimateNewIntegrationTime

TEST_CASE("EstimateNewIntegrationTime - Measurement already has desired saturation ratio.", "[EstimateNewIntegrationTime]")
{
    const short shortIntegrationTime = 10;
    const short longIntegrationTime = 50;
    double desiredSaturationRatio = 0.6;
    long minAllowedExposureTime = 10;
    long maxAllowedExposureTime = 1000;

    SECTION("Longer integration time measurement already has desired saturation ratio")
    {
        // Arrange
        double shortSaturationRatio = desiredSaturationRatio - 0.4;
        double longSaturationRatio = desiredSaturationRatio;

        // Act
        short result = EstimateNewIntegrationTime(
            SpectrumIntensityMeasurement(shortSaturationRatio, shortIntegrationTime),
            SpectrumIntensityMeasurement(longSaturationRatio, longIntegrationTime),
            desiredSaturationRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(longIntegrationTime == result);
    }

    SECTION("Shorter integration time measurement already has desired saturation ratio")
    {
        // Arrange
        double shortSaturationRatio = desiredSaturationRatio;
        double longSaturationRatio = desiredSaturationRatio + 0.3;

        // Act
        short result = EstimateNewIntegrationTime(
            SpectrumIntensityMeasurement(shortSaturationRatio, shortIntegrationTime),
            SpectrumIntensityMeasurement(longSaturationRatio, longIntegrationTime),
            desiredSaturationRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(shortIntegrationTime == result);
    }
}

TEST_CASE("EstimateNewIntegrationTime - Measurement gives higher saturation ratio than desired.", "[EstimateNewIntegrationTime]")
{
    const short shortIntegrationTime = 10;
    const short longIntegrationTime = 50;
    double desiredSaturationRatio = 0.6;
    long minAllowedExposureTime = 3;
    long maxAllowedExposureTime = 1000;

    // Arrange
    double shortSaturationRatio = 0.7;
    double longSaturationRatio = 0.9;

    // Act
    short result = EstimateNewIntegrationTime(
        SpectrumIntensityMeasurement(shortSaturationRatio, shortIntegrationTime),
        SpectrumIntensityMeasurement(longSaturationRatio, longIntegrationTime),
        desiredSaturationRatio,
        minAllowedExposureTime,
        maxAllowedExposureTime);

    // Assert
    REQUIRE(result < shortIntegrationTime);
}

TEST_CASE("EstimateNewIntegrationTime - Measurement gives lower saturation ratio than desired.", "[EstimateNewIntegrationTime]")
{
    const short shortIntegrationTime = 10;
    const short longIntegrationTime = 50;
    double desiredSaturationRatio = 0.6;
    long minAllowedExposureTime = 3;
    long maxAllowedExposureTime = 1000;

    SECTION("Intensity very much lower")
    {
        // Arrange
        double shortSaturationRatio = 0.03;
        double longSaturationRatio = 0.09;

        // Act
        short result = EstimateNewIntegrationTime(
            SpectrumIntensityMeasurement(shortSaturationRatio, shortIntegrationTime),
            SpectrumIntensityMeasurement(longSaturationRatio, longIntegrationTime),
            desiredSaturationRatio,
            minAllowedExposureTime,
            maxAllowedExposureTime);

        // Assert
        REQUIRE(result > longIntegrationTime);
    }
}

#pragma endregion EstimateNewIntegrationTime
