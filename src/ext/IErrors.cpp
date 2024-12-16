#include "ext/IErrors.h"
//#include "common/IDebugLog.h"
#include <cstdlib>

__declspec(noreturn) static void IErrors_Halt(void)
{
	// crash
	*((int *)0) = 0xDEADBEEF;
}

/**
 *	Report a failed assertion and exit the program
 *	
 *	@param file the file where the error occured
 *	@param line the line number where the error occured
 *	@param desc an error message
 */
void _AssertionFailed(const char * file, unsigned long line, const char * desc)
{
	logger::critical("Assertion failed in {} ({}): {}", file, line, desc);

	IErrors_Halt();
}

/**
 *	Report a failed assertion and exit the program
 *	
 *	@param file the file where the error occured
 *	@param line the line number where the error occured
 *	@param desc an error message
 *	@param code the error code
 */
void _AssertionFailed_ErrCode(const char * file, unsigned long line, const char * desc, unsigned long long code)
{
	if(code & 0xFFFFFFFF00000000)
		logger::critical("Assertion failed in {} ({}): {} (code = {} ({}))", file, line, desc, code, code);
	else
	{
		UInt32	code32 = code;
		logger::critical("Assertion failed in {} ({}): {} (code = {} ({}))", file, line, desc, code32, code32);
	}
	
	IErrors_Halt();
}

/**
 *	Report a failed assertion and exit the program
 *	
 *	@param file the file where the error occured
 *	@param line the line number where the error occured
 *	@param desc an error message
 *	@param code the error code
 */
void _AssertionFailed_ErrCode(const char * file, unsigned long line, const char * desc, const char * code)
{
	logger::critical("Assertion failed in {} ({}): {} (code = {})", file, line, desc, code);

	IErrors_Halt();
}
