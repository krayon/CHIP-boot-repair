#include <stdio.h>
#ifdef WINDOWS
#include <io.h>
#define DUP2 _dup2
#define DUP _dup
#define FILENO _fileno
#define CLOSE _close
#else
#include <unistd.h>
#define DUP2 dup2
#define DUP dup
#define FILENO fileno
#define CLOSE close
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sstream>

extern "C" {
#include "libsunxi.h"

/* Notes on translation from c to cpp
 *
 * In felw.c:
 * 		replaced "exit(" with "throw_exit("
 * 		replaced "assert(" with "throw_assert("
 * 		renamed main() to fel_main via ifdef
 * In this file:
 * 		created fel_main() which is the entry point and calls fel_main_body
 * 		fel_main() redirects stdout to fel.out, and puts it back to console when done
 */






void throw_exit(int val) {
	throw (val);
}
void throw_assert(int val) {
	if (!val)
		throw ((bool)val);
}

//http://stackoverflow.com/questions/18816126/c-read-the-whole-file-in-buffer
/** Read file into string. */
std::string slurp (const std::string& path)
{
  std::ostringstream buf;
  std::ifstream input (path.c_str());
  buf << input.rdbuf();
  return buf.str();
}

// Copy a file's contents into a c buffer
char * slurpFileIntoBuffer(const std::string& path)
{
	std::string data = slurp(path);
	char * result = (char*) malloc(data.size()+1);
	strcpy(result,data.c_str());
	return result;
}
//
//int call_main(char ** returnBuffer, ...) {
//	va_list ap;
//	va_start(ap,returnBuffer);
//
//}


// caller needs to free the returned returnBuffer!
int call_main(int argc, char **argv, MAIN_FUNC main_func, char ** returnBuffer)
{
	std::string tempFileName = std::tmpnam(nullptr); // get a temp file name
	std::string tempFileNameStdErr = std::tmpnam(nullptr); // get a temp file name

	/* See http://stackoverflow.com/questions/7664788/freopen-stdout-and-console */
	/* redirecting stdout to a file */
    int stdout_dupfd;
    FILE *temp_out;

    FILE * stdErrFile = freopen(tempFileNameStdErr.c_str(),"w",stderr);

    /* duplicate stdout */
    stdout_dupfd = DUP(1);

    temp_out = fopen(tempFileName.c_str(), "w");

    /* replace stdout with our output fd */
    DUP2(FILENO(temp_out), 1);

    int result = 0;
	try
	{
		result = (*main_func)(argc,argv);
	} catch (bool assertValue) {
		result = -999;
	} catch (int exitValue) {
		result = exitValue;
	} catch (...) {
		fprintf(stderr,"some other exception");

	}


    /* flush output so it goes to our file */
    fflush(stdErrFile);
    fclose(stdErrFile);


	fflush(stdout);
    fclose(temp_out);

    /* Now restore stdout */
    DUP2(stdout_dupfd, 1);
    CLOSE(stdout_dupfd);

	if (result == 0) {
		*returnBuffer = slurpFileIntoBuffer(tempFileName); // remember to free this later
	} else {
		*returnBuffer = slurpFileIntoBuffer(tempFileNameStdErr); // remember to free this later
	}

	remove(tempFileName.c_str()); // get rid of temp files
	remove(tempFileNameStdErr.c_str()); // get rid of temp files
	return result;
}


int fel(int argc, char **argv, char ** returnBuffer)
{
	int result = call_main(argc, argv, fel_main, returnBuffer);
	if (result != 0) {
		if (strstr(*returnBuffer, "permission") != NULL)
			result = FEL_NO_PERMISSION;
		else if (strstr(*returnBuffer, "not found") != NULL)
			result = FEL_NOT_FOUND;
		else if (strstr(*returnBuffer, "lsusb_claim_interface") != NULL)
			result = FEL_CANNOT_CLAIM_INTERFACE;
		else if (strstr(*returnBuffer, "No USB FEL device") != NULL)
			result = FEL_NOT_FOUND;
	}
	return result;
}

}
