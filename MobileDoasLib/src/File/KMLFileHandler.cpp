#include <MobileDoasLib/File/KMLFileHandler.h>
#include <vector>
#include <MobileDoasLib/Flux/Traverse.h>
#include <SpectralEvaluation/VectorUtils.h>

namespace mobiledoas {


    CKMLFileHandler::CKMLFileHandler(void)
    {
    }

    CKMLFileHandler::~CKMLFileHandler(void)
    {
    }

    int CKMLFileHandler::StoreTraverseAsKML(mobiledoas::CTraverse& traverse, const std::string& fileName, int scalingHeight) {
        int k;

        // 0. Check the input
        if (traverse.m_recordNum == 0)
            return 0;

        // 1. Open the file
        FILE* f = fopen(fileName.c_str(), "w");
        if (f == nullptr)
            return 1;

        // 2. We need to scale the columns to make sure that they are visible on the map
        std::vector<double> scaledColumns(traverse.m_recordNum);
        std::vector<int> levels(traverse.m_recordNum);

        // get the minimum and maximum column...
        double minColumn = Min(begin(traverse.columnArray), begin(traverse.columnArray) + traverse.m_recordNum);
        double scaleFactor = scalingHeight / Max(begin(traverse.columnArray), begin(traverse.columnArray) + traverse.m_recordNum);
        long nLevels = 25;

        for (k = 0; k < traverse.m_recordNum; ++k) {
            scaledColumns[k] = (traverse.columnArray[k] - minColumn) * scaleFactor;
            levels[k] = (int)(std::min(nLevels - 1.0, floor(scaledColumns[k] / (scalingHeight / double(nLevels)))));
        }

        // Write the header
        fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(f, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
        fprintf(f, "<Document>\n");

        fprintf(f, "\t<name>%s</name>\n", traverse.m_fileName.c_str());

        // write the list of available styles
        WriteStyles(nLevels, f);

        int start = 0;
        int end = 1;
        while (1) {
            if (levels[start] != levels[end]) {
                WritePlaceMark(traverse.longitude + start, traverse.latitude + start, scaledColumns.data() + start, end - start + 1, levels[start], f);
                start = end;
            }
            end = end + 1;
            if (end >= traverse.m_recordNum) {
                break;
            }
        }

        // Finish the document
        fprintf(f, "</Document>\n");
        fprintf(f, "</kml>\n");

        // Always remember to close the file
        fclose(f);

        return 0;
    }

    void CKMLFileHandler::WritePlaceMark(double* longitude, double* latitude, double* height, long length, int style, FILE* f) {
        long k;

        fprintf(f, "	<Placemark>\n");
        fprintf(f, "		<styleUrl>#style%d</styleUrl>\n", style);
        fprintf(f, "		<LineString>\n");
        fprintf(f, "			<extrude>1</extrude>\n");
        fprintf(f, "			<tessellate>1</tessellate>\n");
        fprintf(f, "			<altitudeMode>relativeToGround</altitudeMode>\n");

        fprintf(f, "			<coordinates>\n");

        // Write each of the data points 
        for (k = 0; k < length; ++k) {
            fprintf(f, "\t\t\t\t%.6lf,%.6lf,%.1lf\n", longitude[k], latitude[k], height[k]);
        }
        fprintf(f, "			</coordinates>\n");

        fprintf(f, "		</LineString>\n");
        fprintf(f, "	</Placemark>\n");

    }

    void CKMLFileHandler::WriteStyles(int numberOfStyles, FILE* f) {

        double halfC = numberOfStyles / 2.0;
        double halfC_inv = 1.0 / halfC;

        for (int level = 0; level < numberOfStyles; ++level) {

            // Get the color we should use for this style
            char colorStr[64];
            if (level < halfC) {
                double a = std::max(0, level) * halfC_inv;
                sprintf_s(colorStr, "%02x%02x00", (unsigned char)(255 * (1 - a)), (unsigned char)(255 * a));
            }
            else {
                double a = std::max(0.0, level - halfC) * halfC_inv;
                sprintf_s(colorStr, "00%02x%02x", (unsigned char)(255 * (1 - a)), (unsigned char)(255 * a));
            }

            // Write the style to file
            fprintf(f, "	<Style id=\"style%d\">\n", level);
            fprintf(f, "		<LineStyle>\n");
            fprintf(f, "			<color>7f%s</color>\n", colorStr);
            fprintf(f, "			<width>4</width>\n");
            fprintf(f, "		</LineStyle>\n");
            fprintf(f, "		<PolyStyle>\n");
            fprintf(f, "			<color>7f%s</color>\n", colorStr);
            fprintf(f, "		</PolyStyle>\n");
            fprintf(f, "	</Style>\n");
        }
    }

}
