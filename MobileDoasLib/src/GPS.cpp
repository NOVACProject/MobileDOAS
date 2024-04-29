#include <MobileDoasLib/GPS.h>
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

namespace mobiledoas
{

    CGPS::CGPS()
        : m_gpsInfo(), fRun(false)
    {
    }

    CGPS::CGPS(const char* pCOMPort, long pBaudrate, std::string& outputDirectory)
        : m_gpsInfo(), fRun(false)
    {
        m_logFile = outputDirectory + "/gps.log";

        serial.SetBaudrate(pBaudrate);
        serial.SetPort(pCOMPort);

        if (!Connect())
        {
            // MessageBox(nullptr, "Could not communicate with GPS. No GPS-data can be retrieved!", "Error", MB_OK | MB_SYSTEMMODAL);
        }
    }

    CGPS::CGPS(CSerialConnection&& serial)
        : m_gpsInfo(), fRun(false)
    {
        // Take ownership of the serial connection.
        this->serial = std::move(serial);
    }

    CGPS::CGPS(CGPS&& other)
    {
        this->serial = std::move(other.serial);
        this->fRun = other.fRun;
        this->m_gotContact = other.m_gotContact;
        this->m_gpsInfo = other.m_gpsInfo;
        this->m_logFile = other.m_logFile;
    }

    CGPS& CGPS::operator=(CGPS&& other)
    {
        this->serial = std::move(other.serial);
        this->fRun = other.fRun;
        this->m_gotContact = other.m_gotContact;
        this->m_gpsInfo = other.m_gpsInfo;
        this->m_logFile = other.m_logFile;
        return *this;
    }

    CGPS::~CGPS()
    {
        serial.Close();
    }

    bool CGPS::Connect()
    {
        if (!serial.Init())
        {
            return false;
        }
        return true;
    }

    void CGPS::Get(mobiledoas::GpsData& dst)
    {
        std::lock_guard<std::mutex> guard(this->m_gpsInfoMutex); // lock the access to the 'm_gpsInfo' while we're copying out the data
        dst = this->m_gpsInfo;
    }


    bool CGPS::ReadGPS()
    {
        const long bytesToRead = 512;
        char gpstxt[512];

        gpstxt[0] = 0;

        // copy the old data into the temp structure (not all sentences provide all data...)
        mobiledoas::GpsData localGpsInfo = this->m_gpsInfo;

        do {
            long cnt = 0;
            // serial.FlushSerialPort(10);
            if (serial.Check(2000))
            {
                while (serial.Check(100) && cnt < bytesToRead)
                {
                    // Read GPRMC and GPGGA
                    serial.Read(gpstxt + cnt, 1);
                    if (gpstxt[cnt] == '\n')
                    {
                        break; // each sentence ends with newline
                    }
                    cnt++;

                    if (!this->fRun) {
                        return true;
                    }
                }
                m_gotContact = true;
            }
            else
            {
                std::cerr << "timeout in getting gps." << std::endl;
                serial.FlushSerialPort(1);
                m_gotContact = false;
                this->m_gpsInfo.status = "NA";
                return false;
            }
        } while (!Parse(gpstxt, localGpsInfo));

        // Copy the parsed data to our member structure in a controlled fashion
        {
            std::lock_guard<std::mutex> guard(this->m_gpsInfoMutex); // lock the access to the 'm_gpsInfo' while we're copying out the data
            this->m_gpsInfo = localGpsInfo;
        }

#ifdef _DEBUG
        if (m_logFile.size() > 0) {
            FILE* f = fopen(m_logFile.c_str(), "a+");
            if (f != nullptr)
            {
                fprintf(f, "%1d\t%ld\t", localGpsInfo.date, localGpsInfo.time);
                fprintf(f, "%lf\t%lf\t%lf\t", localGpsInfo.latitude, localGpsInfo.longitude, localGpsInfo.altitude);
                fprintf(f, "%ld\t", localGpsInfo.nSatellitesTracked);
                fprintf(f, "%ld\n", localGpsInfo.nSatellitesSeen);
                fclose(f);
            }
        }
#endif

        return true;
    }

    void CGPS::CloseSerial()
    {
        this->serial.Close();
    }

    /** The async GPS data collection thread. This will take ownership of the CGPS which is passed in as reference
        and delete it when we're done. */
    void CollectGPSData(CGPS* gps)
    {

        gps->fRun = true;

        while (gps->fRun)
        {
            if (gps->ReadGPS())
            {
                /* make a small pause */
                std::this_thread::sleep_for(10ms);
            }
            else
            {
                // Error reading GPS.  Sleep longer and try again.
                gps->CloseSerial();
                std::this_thread::sleep_for(1000ms);
                gps->Connect();
                std::this_thread::sleep_for(1000ms);
            }
        }

        gps->fRun = false;

        delete gps;

        return;
    }

    GpsAsyncReader::GpsAsyncReader(const char* pCOMPort, long baudrate, std::string& outputDirectory)
    {
        m_gps = new CGPS(pCOMPort, baudrate, outputDirectory);

        m_gpsThread = std::thread(&CollectGPSData, m_gps); // AfxBeginThread(CollectGPSData, (LPVOID)m_gps, THREAD_PRIORITY_NORMAL, 0, 0, nullptr);
    }

    GpsAsyncReader::GpsAsyncReader(CGPS&& gps)
    {
        m_gps = new CGPS(std::move(gps));

        m_gpsThread = std::thread(&CollectGPSData, m_gps); // AfxBeginThread(CollectGPSData, (LPVOID)m_gps, THREAD_PRIORITY_NORMAL, 0, 0, nullptr);
    }


    GpsAsyncReader::~GpsAsyncReader()
    {
        if (m_gpsThread.joinable()) {
            m_gpsThread.join();
        }

        m_gps = nullptr; // we don't get to delete the m_gps, its up to the background thread to do so.
    }

    void GpsAsyncReader::Stop()
    {
        if (nullptr != m_gps && m_gps->fRun == true)
        {
            m_gps->fRun = false;
        }
    }

    void GpsAsyncReader::Get(mobiledoas::GpsData& data)
    {
        m_gps->Get(data);
    }

    bool GpsAsyncReader::GotContact() const
    {
        return m_gps->m_gotContact;
    }

}
