#pragma once

#include "../Flux/Traverse.h"

namespace FileHandler{
	class CKMLFileHandler
	{
	public:
		CKMLFileHandler(void);
		~CKMLFileHandler(void);
		
		
		/** Writes the information of one traverse to a KML file 
			@param traverse - the traverse to store to file
			@praram fileName - the name (including full path) of the file to write
			@return 0 on success.
		*/
		static int StoreTraverseAsKML(Flux::CTraverse &traverse, const CString &fileName, int scalingHeight);
		
	private:
		static void WritePlaceMark(double *longitude, double *latitude, double *height, long length, int style, FILE *f);

		static void WriteStyles(int numberOfStyles, FILE *f);
	
	};
}