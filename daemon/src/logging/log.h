namespace simplefs
{
	namespace log
	{
		//By default all logging is on
		void allowLoggingInfo(bool);
		void allowLoggingWarnings(bool);
		void allowLoggingErrors(bool);

		/*
		 * For below methods messages are printed like: 
		 *	YYYY.MM.dd HH:mm:ss: [TYPE][MODULE] <message formatted like printf>
		 *
		 * max lengtht of formatted message is 255 characters
		 */
		void logInfo(const char* module, const char* message, ...);
		void logWarning(const char* module, const char* message, ...);
		void logError(const char* module, const char* message, ...);
	}
}
