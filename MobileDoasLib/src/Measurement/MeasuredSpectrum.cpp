#include <MobileDoasLib/Measurement/MeasuredSpectrum.h>

namespace mobiledoas
{

void MeasuredSpectrum::SetToZero()
{
    for (size_t chn = 0; chn < data.size(); ++chn)
    {
        memset(data[chn].data(), 0, sizeof(double) * data[chn].size());
    }
}

void MeasuredSpectrum::Resize(int numberOfChannels, int spectrumLength)
{
    data.resize(numberOfChannels);
    for (int i = 0; i < numberOfChannels; ++i)
    {
        data[i].resize(spectrumLength);
    }
}

void MeasuredSpectrum::CopyTo(MeasuredSpectrum& destination) const
{
    const int channels = this->NumberOfChannels();
    const int length = this->SpectrumLength();
    destination.Resize(channels, length);
    for (int chn = 0; chn < channels; ++chn)
    {
        memcpy(destination.data[chn].data(), this->data[chn].data(), sizeof(double) * length);
    }
}

void MeasuredSpectrum::CopyFrom(int channelNumber, double source[], int numberOfElements)
{
    _ASSERT(channelNumber >= 0 && channelNumber < this->data.size());

    this->Resize(this->NumberOfChannels(), numberOfElements);

    memcpy(
    (void*)this->data[channelNumber].data(),
    (void*)source, 
    sizeof(double) * numberOfElements);
}

}
